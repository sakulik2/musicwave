cmake_minimum_required(VERSION 3.15)
project(MusicPlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
    libswresample
)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

find_package(Threads REQUIRED)

add_executable(music_player
    main.cpp
    MusicPlayer.cpp
)

target_link_libraries(music_player PRIVATE
    PkgConfig::FFMPEG
    PkgConfig::SDL2
    Threads::Threads
)

target_compile_options(music_player PRIVATE ${FFMPEG_CFLAGS_OTHER})