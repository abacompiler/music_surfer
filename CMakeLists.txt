cmake_minimum_required(VERSION 3.20)
project(MusicSurfer VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MUSIC_SURFER_ENABLE_JUCE "Enable JUCE integration" OFF)
option(MUSIC_SURFER_ENABLE_TAGLIB "Enable TagLib integration" OFF)
option(MUSIC_SURFER_ENABLE_SQLITE "Enable SQLite integration" ON)
option(MUSIC_SURFER_BUILD_TESTS "Build test targets" ON)

include(cmake/ProjectOptions.cmake)

add_subdirectory(src)

if(MUSIC_SURFER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
