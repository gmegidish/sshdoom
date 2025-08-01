FROM ubuntu:22.04

# Set environment variables to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install required packages
RUN \
    apt-get update && \
    apt-get install -y gcc make libsixel-dev libsixel-bin clang && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the entire doomgeneric directory
COPY . .

# Build the project using Makefile.sixels
RUN make -f Makefile

# Set the default command to run the game
CMD ["./doomsixels"]
