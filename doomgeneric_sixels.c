//doomgeneric for terminal sixels rendering

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <sixel.h>

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static struct termios original_termios;
static bool terminal_setup = false;
static sixel_output_t *sixel_output = NULL;

static int write_sixel_data(char *data, int size, void *priv)
{
    return fwrite(data, 1, size, stdout);
}

static void cleanup_terminal(void)
{
    if (terminal_setup) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        terminal_setup = false;
    }
    if (sixel_output) {
        sixel_output_unref(sixel_output);
        sixel_output = NULL;
    }
    printf("\033[?25h"); // Show cursor
    fflush(stdout);
}

static void signal_handler(int sig)
{
    cleanup_terminal();
    exit(0);
}

#define ARROW_UP    1
#define ARROW_DOWN  2  
#define ARROW_RIGHT 3
#define ARROW_LEFT  4

static unsigned char convertToDoomKey(unsigned char key)
{
    switch (key) {
    case '\r':
    case '\n':
        return KEY_ENTER;
    case 27: // ESC
        return KEY_ESCAPE;
    case ARROW_UP:
        return KEY_UPARROW;
    case ARROW_DOWN:
        return KEY_DOWNARROW;
    case ARROW_LEFT:
        return KEY_LEFTARROW;
    case ARROW_RIGHT:
        return KEY_RIGHTARROW;
    case 'w':
    case 'W':
        return KEY_UPARROW;
    case 's':
    case 'S':
        return KEY_DOWNARROW;
    case 'a':
    case 'A':
        return KEY_LEFTARROW;
    case 'd':
    case 'D':
        return KEY_RIGHTARROW;
    case ' ':
        return KEY_USE;
    case '\t':
        return KEY_TAB;
    case '=':
    case '+':
        return KEY_EQUALS;
    case '-':
        return KEY_MINUS;
    default:
        if (key >= 'A' && key <= 'Z') {
            return key - 'A' + 'a';
        }
        return key;
    }
}

static void addKeyToQueue(int pressed, unsigned char keyCode)
{
    unsigned char key = convertToDoomKey(keyCode);
    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void handleKeyInput(void)
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    unsigned char ch;
    while (read(STDIN_FILENO, &ch, 1) == 1) {
        // Handle escape sequences
        if (ch == 27) {
            unsigned char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 1) {
                if (seq[0] == '[') {
                    if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                        unsigned char arrow_key = 0;
                        switch (seq[1]) {
                        case 'A': // Up arrow
                            arrow_key = ARROW_UP;
                            break;
                        case 'B': // Down arrow
                            arrow_key = ARROW_DOWN;
                            break;
                        case 'C': // Right arrow
                            arrow_key = ARROW_RIGHT;
                            break;
                        case 'D': // Left arrow
                            arrow_key = ARROW_LEFT;
                            break;
                        }
                        if (arrow_key != 0) {
                            addKeyToQueue(1, arrow_key); // Just key press, let game handle timing
                        }
                    }
                } else {
                    addKeyToQueue(1, 27); // ESC key press only
                }
            } else {
                addKeyToQueue(1, 27); // ESC key press only
            }
        } else {
            addKeyToQueue(1, ch); // Just key press, let game handle timing
        }
    }
    
    fcntl(STDIN_FILENO, F_SETFL, flags);
}

void DG_Init(void)
{
    // Setup terminal for raw input
    if (tcgetattr(STDIN_FILENO, &original_termios) == 0) {
        struct termios raw = original_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        terminal_setup = true;
    }

    // Setup signal handlers for cleanup
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    atexit(cleanup_terminal);

    // Initialize sixel output
    SIXELSTATUS status = sixel_output_new(&sixel_output, write_sixel_data, NULL, NULL);
    if (SIXEL_FAILED(status)) {
        fprintf(stderr, "Failed to create sixel output\n");
        exit(1);
    }

    // We'll create dither objects per frame for perfect color accuracy

    // Hide cursor and clear screen
    printf("\033[?25l"); // Hide cursor
    printf("\033[2J");   // Clear screen
    printf("\033[H");    // Move cursor to home
    fflush(stdout);
}

void DG_DrawFrame(void)
{
    // Convert screen buffer to RGB format for sixel
    unsigned char *rgb_buffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 3);
    if (!rgb_buffer) {
        return;
    }

    // Convert from RGBA to RGB
    uint32_t *screen = (uint32_t*)DG_ScreenBuffer;
    for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++) {
        uint32_t pixel = screen[i];
        rgb_buffer[i * 3 + 0] = (pixel >> 16) & 0xFF; // R
        rgb_buffer[i * 3 + 1] = (pixel >> 8) & 0xFF;  // G
        rgb_buffer[i * 3 + 2] = pixel & 0xFF;         // B
    }

    // Create a fresh dither object for each frame - this gives perfect colors
    sixel_dither_t *frame_dither = NULL;
    SIXELSTATUS status = sixel_dither_new(&frame_dither, 256, NULL);
    if (SIXEL_SUCCEEDED(status)) {
        // Initialize with current frame data for optimal palette
        status = sixel_dither_initialize(
            frame_dither,
            rgb_buffer,
            DOOMGENERIC_RESX,
            DOOMGENERIC_RESY,
            SIXEL_PIXELFORMAT_RGB888,
            SIXEL_LARGE_AUTO,
            SIXEL_REP_AUTO,
            SIXEL_QUALITY_HIGH
        );
        
        if (SIXEL_SUCCEEDED(status)) {
            // Disable dithering for clean output
            sixel_dither_set_diffusion_type(frame_dither, SIXEL_DIFFUSE_NONE);
            
            // Encode sixel
            printf("\033[H");
            fflush(stdout);
            
            sixel_encode(rgb_buffer, DOOMGENERIC_RESX, DOOMGENERIC_RESY, 3, frame_dither, sixel_output);
            fflush(stdout);
        }
        
        // Clean up the frame dither
        sixel_dither_unref(frame_dither);
    }

    free(rgb_buffer);
    handleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
    usleep(ms * 1000);
}

uint32_t DG_GetTicksMs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
        // Key queue is empty
        return 0;
    } else {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;

        return 1;
    }
}

void DG_SetWindowTitle(const char * title)
{
    // Set terminal title using escape sequence
    printf("\033]0;%s\007", title);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++) {
        doomgeneric_Tick();
    }

    return 0;
}
