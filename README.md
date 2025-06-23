# musicwave

A C++ music player built with FFmpeg and SDL2 that supports various audio formats including MP3, FLAC, WAV, OGG, and more.
Created with Anthropic Claude Sonnet 4.

## Features

- **Multi-format support**: Play MP3, FLAC, WAV, OGG, M4A, and other formats supported by FFmpeg
- **Playback controls**: Play, pause, stop, seek, volume control
- **Metadata display**: Show track information (title, artist, album, genre)
- **Real-time status**: Current time, duration, playback state
- **Command-line interface**: Easy-to-use interactive commands
- **Multi-threaded**: Separate decoding and audio playback threads for smooth performance

## Dependencies

### Arch Linux
```bash
# Install dependencies
sudo pacman -S ffmpeg sdl2 cmake pkg-config base-devel

# Optional: Install from AUR if you prefer AUR packages
yay -S ffmpeg-git sdl2-git  # if using yay AUR helper
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswresample-dev \
    libsdl2-dev \
    pkg-config \
    cmake \
    build-essential
```

### macOS (with Homebrew)
```bash
brew install ffmpeg sdl2 cmake pkg-config
```

### Windows
You'll need to install:
- FFmpeg development libraries
- SDL2 development libraries
- CMake
- Visual Studio or MinGW

## Complete Installation Guide for Arch Linux

### Step-by-Step Installation

1. **Install dependencies:**
   ```bash
   sudo pacman -S ffmpeg sdl2 cmake pkg-config base-devel
   ```

2. **Download the source code:**
   ```bash
   # Create a directory for the project
   mkdir ~/music-player
   cd ~/music-player
   
   # Copy all the source files here:
   # - MusicPlayer.h
   # - MusicPlayer.cpp  
   # - main.cpp
   # - CMakeLists.txt
   # - Makefile
   ```

3. **Build with CMake (recommended):**
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j$(nproc)
   ```

4. **Or build with Make:**
   ```bash
   make
   ```

5. **Run the player:**
   ```bash
   ./music_player /path/to/your/music.mp3
   ```

### Alternative Installation Methods

**Using Makefile helper:**
```bash
make install-deps-arch  # Install dependencies
make                     # Build the project
make install            # Install to /usr/local/bin (optional)
```

**For development:**
```bash
make debug              # Build with debug symbols
make run               # Build and run immediately
```

### Troubleshooting on Arch

If you encounter issues:

1. **Missing FFmpeg development headers:**
   ```bash
   # Arch includes headers in the main package
   sudo pacman -S ffmpeg
   ```

2. **SDL2 not found:**
   ```bash
   sudo pacman -S sdl2
   ```

3. **pkg-config errors:**
   ```bash
   sudo pacman -S pkg-config
   ```

4. **Compiler not found:**
   ```bash
   sudo pacman -S base-devel
   ```

### Audio System Issues

**"Audio subsystem is not initialized" or "Failed to open audio device":**

1. **Check if audio is working:**
   ```bash
   # Test system audio
   speaker-test -c2 -t wav
   
   # Or play a test sound
   paplay /usr/share/sounds/alsa/Front_Left.wav
   ```

2. **PulseAudio issues:**
   ```bash
   # Check if PulseAudio is running
   pulseaudio --check -v
   
   # Start PulseAudio if not running
   pulseaudio --start
   
   # Restart PulseAudio if needed
   pulseaudio -k && pulseaudio --start
   ```

3. **ALSA issues:**
   ```bash
   # List audio devices
   aplay -l
   
   # Test ALSA directly
   aplay /usr/share/sounds/alsa/Front_Left.wav
   ```

4. **User permissions:**
   ```bash
   # Check if user is in audio group
   groups $USER | grep audio
   
   # Add user to audio group if needed
   sudo usermod -a -G audio $USER
   # Then log out and back in
   ```

5. **Try different SDL audio drivers:**
   ```bash
   # Try ALSA driver
   SDL_AUDIODRIVER=alsa ./music_player
   
   # Try PulseAudio driver
   SDL_AUDIODRIVER=pulse ./music_player
   
   # List available drivers
   SDL_AUDIODRIVER=dummy ./music_player
   ```

6. **Install audio packages:**
   ```bash
   # Install common audio packages
   sudo pacman -S pulseaudio pulseaudio-alsa alsa-utils
   
   # For JACK users
   sudo pacman -S jack2 pulseaudio-jack
   ```

7. **Debug audio configuration:**
   ```bash
   # Check what's using audio
   lsof /dev/snd/*
   
   # Check audio processes
   ps aux | grep -E "(pulse|alsa|jack)"
   ```

## Building on Other Systems

### Quick Installation
```bash
# Install dependencies
sudo pacman -S ffmpeg sdl2 cmake pkg-config base-devel

# Clone or download the source files
mkdir build && cd build
cmake ..
make

# Alternative: Use Makefile
make install-deps-arch
make
```

### Using AUR (if preferred)
```bash
# If you prefer development versions from AUR
yay -S ffmpeg-git sdl2-git cmake pkg-config base-devel
# or with paru
paru -S ffmpeg-git sdl2-git cmake pkg-config base-devel
```

## Building on Other Systems

### Using CMake (Recommended)
```bash
# Clone or download the source files
mkdir build
cd build
cmake ..
make

# Optional: Install dependencies first
# Ubuntu/Debian: make install-deps
# Arch Linux: make install-deps-arch
```

### Manual Compilation (Linux/macOS)
```bash
g++ -std=c++17 -o music_player main.cpp MusicPlayer.cpp \
    `pkg-config --cflags --libs libavformat libavcodec libavutil libswresample` \
    `sdl2-config --cflags --libs` -lpthread
```

## Usage

### Basic Usage
```bash
# Run the player
./music_player

# Load and play a file directly
./music_player /path/to/your/music/file.mp3
```

### Interactive Commands

| Command | Description | Example |
|---------|-------------|---------|
| `load <file>` | Load an audio file | `load song.mp3` |
| `play` | Start playback | `play` |
| `pause` | Pause playback | `pause` |
| `stop` | Stop playback | `stop` |
| `seek <seconds>` | Seek to time | `seek 120` |
| `volume <0-100>` | Set volume | `volume 75` |
| `info` | Show track info | `info` |
| `status` | Show player status | `status` |
| `help` | Show help | `help` |
| `quit` | Exit player | `quit` |

### Example Session
```
FFmpeg Music Player v1.0
Type 'help' for commands

> load /home/user/Music/song.mp3
Loading: /home/user/Music/song.mp3
Successfully loaded: /home/user/Music/song.mp3

=== Track Information ===
File: /home/user/Music/song.mp3
Duration: 3:42
Title: Example Song
Artist: Example Artist
Album: Example Album
Genre: Rock
=========================

> play
Playing...

> volume 80
Volume set to 80%

> seek 60
Seeking to 1:00

> status

=== Player Status ===
State: PLAYING
Time: 1:05 / 3:42
Volume: 80%
=====================

> quit
Goodbye!
```

## Supported Formats

The player supports all audio formats that FFmpeg can decode, including:
- **MP3** (.mp3)
- **FLAC** (.flac)
- **WAV** (.wav)
- **OGG Vorbis** (.ogg)
- **AAC/M4A** (.m4a, .aac)
- **WMA** (.wma)
- **AIFF** (.aiff)
- **And many more...**

## Architecture

The music player consists of several key components:

### MusicPlayer Class
- **Audio decoding**: Uses FFmpeg to decode various audio formats
- **Format conversion**: Converts audio to SDL2-compatible format using libswresample
- **Threading**: Separate decoding thread for smooth playback
- **Buffer management**: Queue-based audio buffer system

### Key Files
- `MusicPlayer.h/cpp`: Core player implementation
- `main.cpp`: Command-line interface
- `CMakeLists.txt`: Build configuration

### Threading Model
1. **Main thread**: Handles user input and player control
2. **Decoding thread**: Reads and decodes audio frames
3. **Audio callback**: SDL2 audio callback for real-time playback

## Advanced Features

### Volume Control
- Real-time volume adjustment (0-100%)
- Applied during audio mixing for best quality

### Seeking
- Accurate seeking to any position in the track
- Automatic buffer clearing and decoder flushing

### Metadata Support
- Reads embedded metadata from audio files
- Displays title, artist, album, genre information

## Troubleshooting

### Common Issues

**"Failed to open input file"**
- Check if the file path is correct
- Ensure the file format is supported
- Verify file permissions

**"Failed to open audio device"**
- Check if audio system is working
- Try running with different audio drivers
- Ensure no other applications are using audio

**"Codec not found"**
- Install additional FFmpeg codecs
- Check FFmpeg installation

### Debug Mode
Add debug output by modifying the source:
```cpp
// In MusicPlayer.cpp, add debug prints
std::cout << "Debug: " << message << std::endl;
```

## Performance Notes

- **Buffer size**: Adjust `MAX_QUEUE_SIZE` for different memory/latency trade-offs
- **Audio latency**: Modify SDL audio buffer size in `setupAudioConversion()`
- **Threading**: The player uses lock-free atomics where possible for performance

## License

This project is provided as-is for educational and personal use. Please ensure you comply with FFmpeg and SDL2 licensing terms.

## Contributing

Feel free to submit issues and pull requests. Areas for improvement:
- GUI interface
- Playlist support
- Audio effects/equalizer
- Network streaming
- Additional format support

## Version History

- **v0.0.1**: Initial release with basic playback functionality
