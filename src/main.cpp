#include "database/Database.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "services/PlaylistManagerImpl.hpp"
#include "ui/MainWindow.hpp"
#include "utils/FileSystemScanner.hpp"
#include "utils/MetadataParser.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
constexpr std::string_view kDatabaseFileName = "music_surfer.db";
constexpr std::string_view kDbPathEnvVar = "MUSIC_SURFER_DB_PATH";
constexpr std::string_view kConfigDirEnvVar = "MUSIC_SURFER_CONFIG_DIR";

std::optional<std::filesystem::path> nonEmptyPathFromEnv(std::string_view envVar)
{
#if defined(_WIN32)
    char* value = nullptr;
    std::size_t valueLength = 0;
    if (_dupenv_s(&value, &valueLength, envVar.data()) == 0 && value != nullptr)
    {
        std::optional<std::filesystem::path> envPath;
        if (*value != '\0')
        {
            envPath = std::filesystem::path(value);
        }
        std::free(value);
        return envPath;
    }
    return std::nullopt;
#else
    if (const char* value = std::getenv(envVar.data()); value != nullptr && *value != '\0')
    {
        return std::filesystem::path(value);
    }
    return std::nullopt;
#endif
}

std::optional<std::filesystem::path> parseCliPathOption(int argc, char** argv, std::string_view optionName)
{
    for (int index = 1; index < argc; ++index)
    {
        if (std::string_view(argv[index]) == optionName)
        {
            if (index + 1 >= argc)
            {
                throw std::invalid_argument("Missing value for option: " + std::string(optionName));
            }
            return std::filesystem::path(argv[index + 1]);
        }
    }
    return std::nullopt;
}

std::filesystem::path defaultUserLocalDataDir()
{
#if defined(_WIN32)
    if (auto localAppData = nonEmptyPathFromEnv("LOCALAPPDATA"))
    {
        return *localAppData;
    }
    if (auto appData = nonEmptyPathFromEnv("APPDATA"))
    {
        return *appData;
    }
#elif defined(__APPLE__)
    if (auto home = nonEmptyPathFromEnv("HOME"))
    {
        return *home / "Library" / "Application Support";
    }
#else
    if (auto xdgDataHome = nonEmptyPathFromEnv("XDG_DATA_HOME"))
    {
        return *xdgDataHome;
    }
    if (auto home = nonEmptyPathFromEnv("HOME"))
    {
        return *home / ".local" / "share";
    }
#endif

    return std::filesystem::temp_directory_path();
}

std::filesystem::path resolveDatabasePath(int argc, char** argv)
{
    if (auto cliDbPath = parseCliPathOption(argc, argv, "--db-path"))
    {
        return *cliDbPath;
    }

    if (auto envDbPath = nonEmptyPathFromEnv(kDbPathEnvVar))
    {
        return *envDbPath;
    }

    if (auto cliConfigDir = parseCliPathOption(argc, argv, "--config-dir"))
    {
        return *cliConfigDir / std::string(kDatabaseFileName);
    }

    if (auto envConfigDir = nonEmptyPathFromEnv(kConfigDirEnvVar))
    {
        return *envConfigDir / std::string(kDatabaseFileName);
    }

    return defaultUserLocalDataDir() / "MusicSurfer" / std::string(kDatabaseFileName);
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        const auto databasePath = resolveDatabasePath(argc, argv);
        std::filesystem::create_directories(databasePath.parent_path());

        auto repository = std::make_shared<music_surfer::database::Database>(databasePath);

        music_surfer::services::LibraryManager libraryManager;
        libraryManager.setRepository(repository);
        libraryManager.setFileScanner(std::make_shared<music_surfer::utils::FileSystemScanner>());
        libraryManager.setMetadataReader(std::make_shared<music_surfer::utils::MetadataParser>());

        auto playlistManager = std::make_shared<music_surfer::services::PlaylistManagerImpl>(repository);

        music_surfer::services::AudioPlayer audioPlayer;
        music_surfer::ui::MainWindow mainWindow(libraryManager, audioPlayer, *playlistManager);
        (void)mainWindow;

        std::cout << "MusicSurfer starter app (window + services wired). DB: " << databasePath << std::endl;
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Failed to start MusicSurfer: " << exception.what() << std::endl;
        return 1;
    }
}
