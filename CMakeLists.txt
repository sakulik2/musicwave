cmake_minimum_required(VERSION 3.10)
project(MusicPlayer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find FFmpeg libraries
pkg_check_modules(FFMPEG REQUIRED
    libavformat
    libavcodec
    libavutil
    libswresample
)

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

# Add executable
add_executable(music_player
    main.cpp
    MusicPlayer.cpp
)

# Link libraries
target_link_libraries(music_player
    ${FFMPEG_LIBRARIES}
    ${SDL2_LIBRARIES}
    pthread
)

# Compiler flags
target_compile_options(music_player PRIVATE ${FFMPEG_CFLAGS_OTHER})

# Add custom target for installation of dependencies (Arch Linux)
add_custom_target(install-deps-arch
    COMMAND sudo pacman -Syu
    COMMAND sudo pacman -S --needed 
        ffmpeg 
        sdl2 
        pkg-config 
        base-devel 
        cmake
    COMMENT "Installing dependencies on Arch Linux"
)

# Add custom target for installation of dependencies (Ubuntu/Debian)
add_custom_target(install-deps
    COMMAND sudo apt-get update
    COMMAND sudo apt-get install -y 
        libavformat-dev 
        libavcodec-dev 
        libavutil-dev 
        libswresample-dev 
        libsdl2-dev 
        pkg-config 
        cmake 
        build-essential
    COMMENT "Installing dependencies on Ubuntu/Debian"
)

# Print helpful information
message(STATUS "FFmpeg libraries: ${FFMPEG_LIBRARIES}")
message(STATUS "FFmpeg includes: ${FFMPEG_INCLUDE_DIRS}")
message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
message(STATUS "SDL2 includes: ${SDL2_INCLUDE_DIRS}")

# Installation
install(TARGETS music_player DESTINATION bin)

# CPack configuration for creating packages
set(CPACK_PACKAGE_NAME "MusicPlayer")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FFmpeg-based Music Player")
set(CPACK_PACKAGE_VENDOR "Music Player Project")
set(CPACK_GENERATOR "DEB;TGZ")

include(CPack)