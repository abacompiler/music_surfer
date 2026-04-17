set(MUSIC_SURFER_CORE_SOURCES
    core/Track.cpp
    services/LibraryManager.cpp
    services/AudioPlayer.cpp
    services/PlaylistManagerImpl.cpp
    audio/AudioEngine.cpp
    database/Database.cpp
    ui/MainWindow.cpp
    utils/FileSystemScanner.cpp
    utils/MetadataParser.cpp
    utils/Logger.cpp
)

add_library(music_surfer_core STATIC ${MUSIC_SURFER_CORE_SOURCES})

target_compile_features(music_surfer_core PUBLIC cxx_std_17)

target_include_directories(music_surfer_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

music_surfer_set_project_warnings(music_surfer_core)
music_surfer_enable_sanitizers(music_surfer_core)

# =========================
# TAGLIB
# =========================
if(MUSIC_SURFER_ENABLE_TAGLIB)
    find_package(TagLib CONFIG REQUIRED)
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_TAGLIB=1)
    target_link_libraries(music_surfer_core PUBLIC TagLib::tag)
else()
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_TAGLIB=0)
endif()

# =========================
# SQLITE (FIXED for vcpkg)
# =========================
if(MUSIC_SURFER_ENABLE_SQLITE)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_SQLITE=1)
    target_link_libraries(music_surfer_core PUBLIC unofficial::sqlite3::sqlite3)
else()
    target_compile_definitions(music_surfer_core PUBLIC MUSIC_SURFER_HAS_SQLITE=0)
endif()

# =========================
# APP
# =========================
add_executable(music_surfer_app main.cpp)

target_link_libraries(music_surfer_app PRIVATE music_surfer_core)

target_include_directories(music_surfer_app PRIVATE ${PROJECT_SOURCE_DIR}/src)

music_surfer_set_project_warnings(music_surfer_app)
music_surfer_enable_sanitizers(music_surfer_app)
