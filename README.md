# musicwave

A simple C++ music player built with FFmpeg and SDL2 that supports various audio formats including MP3, FLAC, WAV, OGG, and more.
Created with Claude Sonnet 4.5 by Anthropic.

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

## Building 

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
