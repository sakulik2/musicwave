#include "MusicPlayer.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>

void printHelp() {
    std::cout << "\n=== Music Player Commands ===" << std::endl;
    std::cout << "load <filename>  - Load an audio file" << std::endl;
    std::cout << "play             - Start playback" << std::endl;
    std::cout << "pause            - Pause playback" << std::endl;
    std::cout << "stop             - Stop playback" << std::endl;
    std::cout << "seek <seconds>   - Seek to specific time" << std::endl;
    std::cout << "volume <0-100>   - Set volume (0-100)" << std::endl;
    std::cout << "info             - Show current track info" << std::endl;
    std::cout << "status           - Show playback status" << std::endl;
    std::cout << "debug            - Show debug information" << std::endl;
    std::cout << "help             - Show this help" << std::endl;
    std::cout << "quit             - Exit the player" << std::endl;
    std::cout << "=============================" << std::endl;
}

std::string formatTime(double seconds) {
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return std::to_string(minutes) + ":" + 
           (secs < 10 ? "0" : "") + std::to_string(secs);
}

std::string stateToString(MusicPlayer::State state) {
    switch (state) {
        case MusicPlayer::State::STOPPED: return "STOPPED";
        case MusicPlayer::State::PLAYING: return "PLAYING";
        case MusicPlayer::State::PAUSED: return "PAUSED";
        default: return "UNKNOWN";
    }
}

void printTrackInfo(const MusicPlayer& player) {
    std::cout << "\n=== Track Information ===" << std::endl;
    std::cout << "File: " << player.getCurrentFile() << std::endl;
    std::cout << "Duration: " << formatTime(player.getDuration()) << std::endl;
    std::cout << "Title: " << player.getMetadata("title") << std::endl;
    std::cout << "Artist: " << player.getMetadata("artist") << std::endl;
    std::cout << "Album: " << player.getMetadata("album") << std::endl;
    std::cout << "Genre: " << player.getMetadata("genre") << std::endl;
    std::cout << "=========================" << std::endl;
}

void printStatus(const MusicPlayer& player) {
    std::cout << "\n=== Player Status ===" << std::endl;
    std::cout << "State: " << stateToString(player.getState()) << std::endl;
    std::cout << "Time: " << formatTime(player.getCurrentTime()) 
              << " / " << formatTime(player.getDuration()) << std::endl;
    std::cout << "Volume: " << static_cast<int>(player.getVolume() * 100) << "%" << std::endl;
    std::cout << "=====================" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "FFmpeg Music Player v1.0" << std::endl;
    std::cout << "Type 'help' for commands" << std::endl;
    
    MusicPlayer player;
    std::string command;
    
    // Auto-load file if provided as argument
    if (argc > 1) {
        std::string filename = argv[1];
        std::cout << "Loading: " << filename << std::endl;
        
        if (player.loadFile(filename)) {
            std::cout << "Successfully loaded: " << filename << std::endl;
            printTrackInfo(player);
            
            // Auto-play the loaded file
            std::cout << "\nStarting playback..." << std::endl;
            if (player.play()) {
                std::cout << "Playing! (Type 'help' for controls)" << std::endl;
            } else {
                std::cout << "Failed to start playback. Try typing 'play'" << std::endl;
            }
        } else {
            std::cout << "Failed to load: " << filename << std::endl;
        }
    }
    
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);
        
        if (command.empty()) {
            continue;
        }
        
        // Parse command
        size_t spacePos = command.find(' ');
        std::string cmd = command.substr(0, spacePos);
        std::string arg = (spacePos != std::string::npos) ? 
                          command.substr(spacePos + 1) : "";
        
        if (cmd == "quit" || cmd == "exit" || cmd == "q") {
            break;
        }
        else if (cmd == "help" || cmd == "h") {
            printHelp();
        }
        else if (cmd == "load" || cmd == "l") {
            if (arg.empty()) {
                std::cout << "Usage: load <filename>" << std::endl;
                continue;
            }
            
            std::cout << "Loading: " << arg << std::endl;
            if (player.loadFile(arg)) {
                std::cout << "Successfully loaded: " << arg << std::endl;
                printTrackInfo(player);
            } else {
                std::cout << "Failed to load: " << arg << std::endl;
            }
        }
        else if (cmd == "play" || cmd == "p") {
            if (player.play()) {
                std::cout << "Playing..." << std::endl;
            } else {
                std::cout << "Cannot play. Load a file first." << std::endl;
            }
        }
        else if (cmd == "pause") {
            if (player.pause()) {
                std::cout << "Paused." << std::endl;
            } else {
                std::cout << "Cannot pause." << std::endl;
            }
        }
        else if (cmd == "stop" || cmd == "s") {
            if (player.stop()) {
                std::cout << "Stopped." << std::endl;
            }
        }
        else if (cmd == "seek") {
            if (arg.empty()) {
                std::cout << "Usage: seek <seconds>" << std::endl;
                continue;
            }
            
            try {
                double seconds = std::stod(arg);
                if (player.seek(seconds)) {
                    std::cout << "Seeking to " << formatTime(seconds) << std::endl;
                } else {
                    std::cout << "Cannot seek. Load a file first." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "Invalid time format." << std::endl;
            }
        }
        else if (cmd == "volume" || cmd == "vol" || cmd == "v") {
            if (arg.empty()) {
                std::cout << "Current volume: " << static_cast<int>(player.getVolume() * 100) << "%" << std::endl;
                continue;
            }
            
            try {
                int volume = std::stoi(arg);
                if (volume >= 0 && volume <= 100) {
                    player.setVolume(volume / 100.0f);
                    std::cout << "Volume set to " << volume << "%" << std::endl;
                } else {
                    std::cout << "Volume must be between 0 and 100." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "Invalid volume value." << std::endl;
            }
        }
        else if (cmd == "info" || cmd == "i") {
            if (player.getCurrentFile().empty()) {
                std::cout << "No file loaded." << std::endl;
            } else {
                printTrackInfo(player);
            }
        }
        else if (cmd == "status" || cmd == "st") {
            printStatus(player);
        }
        else if (cmd == "debug" || cmd == "d") {
            std::cout << "\n=== Debug Information ===" << std::endl;
            std::cout << "Player State: " << stateToString(player.getState()) << std::endl;
            std::cout << "Volume: " << static_cast<int>(player.getVolume() * 100) << "%" << std::endl;
            std::cout << "Current Time: " << formatTime(player.getCurrentTime()) << std::endl;
            std::cout << "Duration: " << formatTime(player.getDuration()) << std::endl;
            std::cout << "File: " << player.getCurrentFile() << std::endl;
            std::cout << "\nSystem Audio Check:" << std::endl;
            std::cout << "Try: 'aplay /usr/share/sounds/alsa/Front_Left.wav'" << std::endl;
            std::cout << "Or: 'speaker-test -c2 -t wav -l1'" << std::endl;
            std::cout << "=========================" << std::endl;
        }
        else {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Type 'help' for available commands." << std::endl;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}