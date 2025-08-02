# 🔥💀 SSHDOOM 💀🔥

```
        .d8888. .d8888. db   db d8888b.  .d88b.   .d88b.  .88b  d88. 
        88'  YP 88'  YP 88   88 88  `8D .8P  Y8. .8P  Y8. 88'YbdP`88 
        `8bo.   `8bo.   88ooo88 88   88 88    88 88    88 88  88  88 
          `Y8b.   `Y8b. 88~~~88 88   88 88    88 88    88 88  88  88 
        db   8D db   8D 88   88 88  .8D `8b  d8' `8b  d8' 88  88  88 
        `8888Y' `8888Y' YP   YP Y8888D'  `Y88P'   `Y88P'  YP  YP  YP
```

https://github.com/user-attachments/assets/4edfef4f-57a9-4748-9fe1-837745a018ff

### 🎮 What is it?

SSHDOOM is DOOM but rendered with [**SIXELS**](https://en.wikipedia.org/wiki/Sixel) in your terminal! If you'd play DOOM on a pregnancy tester, you'd play DOOM in terminal 💥

Supported on *iTerm2*, *xterm* and others. For a list of supported terminal emulators, see https://www.arewesixelyet.com/ .

### 💥 Play it right now

You can start playing DOOM right now by opening your terminal and typing:
```bash
ssh -C 666@sshdoom.megidish.net
```

If you want to run this locally (my home server is in Frankfurt, might be too slow for you), then you can just run:
```bash
docker run -i -t --platform linux/amd64 gmegidish/sshdoom:latest
```

### 😱 Future of this

I intended this to be a fun hack, and nothing serious. You may want to take this implementation forward:

- Arrow keys auto release when a new key is pressed: this sucks, it's a bug
- Mouse movement support: vt supports this!
- Multiplayer on the same host: because why not!
- Optimize sixels to use 256 colors: right now we convert to rgb888 and it's just wasteful on cpu

### 🛠️ Building this blasphemy

Clone this repo and its submodules using:
```bash
git clone --recursive https://github.com/gmegidish/sshdoom.git 
cd sshdoom
```

Install libsixel using:
```bash
# mac
brew install libsixel

# linux
apt-get install libsixel-dev libsixel-bin
```

And then kick off the build using:

```bash
make

# Embrace the chaos
./doomsixel
```

Alternatively, you can just build using the Dockerfile

### 🎮 Controls 

- Use the arrow keys to move forward, backward and turn.
- Use `wasd` keys to move and strafe
- `Space` to open door and push buttons like a gentleman
- `Z` to pew pew
- `ESC` to pause or open menu

### Credits

This port could not have been made without the works of:
- [ozkl's doomgeneric](https://github.com/ozkl/doomgeneric) which is built upon
- [maximevince's fbDOOM](https://github.com/maximevince/fbDOOM) which is built upon
- [insane-adding-machines's DOOM](https://github.com/insane-adding-machines/DOOM) which is built upon
- [id-software's DOOM](https://github.com/id-Software/DOOM), the gods themselves

And with the use of the mighty [libsixel](https://github.com/saitoha/libsixel) by [Hayaki Saito](https://github.com/saitoha).

Code was 100% vibe coded with [Claude Code](https://www.anthropic.com/claude-code), ugly, but ships!

### 📜 License

Released under the **GNU General Public License (GPL)**

