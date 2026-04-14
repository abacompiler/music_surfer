cmake_minimum_required(VERSION 3.20)
project(MusicSurfer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MUSIC_SURFER_ENABLE_JUCE "Enable JUCE integration" OFF)
option(MUSIC_SURFER_ENABLE_TAGLIB "Enable TagLib integration" OFF)
option(MUSIC_SURFER_ENABLE_SQLITE "Enable SQLite integration" OFF)
option(MUSIC_SURFER_BUILD_TESTS "Build test targets" ON)

set(MUSIC_SURFER_CORE_SOURCES
    src/core/Track.cpp
    src/services/LibraryManager.cpp
    src/audio/AudioEngine.cpp
    src/ui/MainWindow.cpp
    src/database/Database.cpp
    src/utils/Logger.cpp
)

add_library(music_surfer_core STATIC ${MUSIC_SURFER_CORE_SOURCES})

target_compile_features(music_surfer_core PUBLIC cxx_std_17)

target_include_directories(music_surfer_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

if(MUSIC_SURFER_ENABLE_JUCE)
    find_package(JUCE CONFIG REQUIRED)
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_JUCE=1)
    # target_link_libraries(music_surfer_core PUBLIC juce::juce_core)
endif()

if(MUSIC_SURFER_ENABLE_TAGLIB)
    find_package(TagLib REQUIRED)
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_TAGLIB=1)
    target_link_libraries(music_surfer_core PUBLIC TagLib::TagLib)
endif()

if(MUSIC_SURFER_ENABLE_SQLITE)
    find_package(SQLite3 REQUIRED)
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_SQLITE=1)
    target_link_libraries(music_surfer_core PUBLIC SQLite::SQLite3)
endif()

add_executable(music_surfer_app src/main.cpp)
target_link_libraries(music_surfer_app PRIVATE music_surfer_core)

target_include_directories(music_surfer_app
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

if(MUSIC_SURFER_BUILD_TESTS)
    enable_testing()

    add_executable(music_surfer_tests tests/test_smoke.cpp)
    target_link_libraries(music_surfer_tests PRIVATE music_surfer_core)

    target_include_directories(music_surfer_tests
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/tests
    )

    add_test(NAME music_surfer_smoke_test COMMAND music_surfer_tests)
endif()
