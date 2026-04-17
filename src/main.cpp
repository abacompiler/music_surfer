#include "core/Track.hpp"
#include "database/Database.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "services/PlaylistManagerImpl.hpp"
#include "ui/MainWindow.hpp"
#if MUSIC_SURFER_HAS_JUCE
#include "ui/JuceMainWindow.hpp"
#include <juce_gui_extra/juce_gui_extra.h>
#endif
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
#include <vector>

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

std::optional<std::filesystem::path> parsePathOption(const std::vector<std::string>& args, std::string_view optionName)
{
    for (std::size_t index = 0; index < args.size(); ++index)
    {
        if (args[index] == optionName)
        {
            if (index + 1 >= args.size())
            {
                throw std::invalid_argument("Missing value for option: " + std::string(optionName));
            }
            return std::filesystem::path(args[index + 1]);
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

std::filesystem::path resolveDatabasePath(const std::vector<std::string>& args)
{
    if (auto cliDbPath = parsePathOption(args, "--db-path"))
    {
        return *cliDbPath;
    }

    if (auto envDbPath = nonEmptyPathFromEnv(kDbPathEnvVar))
    {
        return *envDbPath;
    }

    if (auto cliConfigDir = parsePathOption(args, "--config-dir"))
    {
        return *cliConfigDir / std::string(kDatabaseFileName);
    }

    if (auto envConfigDir = nonEmptyPathFromEnv(kConfigDirEnvVar))
    {
        return *envConfigDir / std::string(kDatabaseFileName);
    }

    return defaultUserLocalDataDir() / "MusicSurfer" / std::string(kDatabaseFileName);
}

void seedLibraryIfEmpty(music_surfer::services::LibraryManager& libraryManager)
{
    if (!libraryManager.listTracks().empty())
    {
        return;
    }

    libraryManager.upsertTrack(music_surfer::core::Track{"track-001", "Midnight Drift", "artist-demo", "album-demo"});
    libraryManager.upsertTrack(music_surfer::core::Track{"track-002", "Oceanic Pulse", "artist-demo", "album-demo"});
    libraryManager.upsertTrack(music_surfer::core::Track{"track-003", "Neon Skyline", "artist-demo", "album-demo"});
}

void seedPlaylistsIfEmpty(music_surfer::services::PlaylistManager& playlistManager)
{
    if (!playlistManager.listPlaylists().empty())
    {
        return;
    }

    (void)playlistManager.createPlaylist("Favorites");
    (void)playlistManager.createPlaylist("Roadtrip");
}

std::vector<std::string> argsFromArgv(int argc, char** argv)
{
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i)
    {
        args.emplace_back(argv[i]);
    }
    return args;
}

#if MUSIC_SURFER_HAS_JUCE
std::vector<std::string> argsFromCommandLine(const juce::String& commandLine)
{
    auto tokens = juce::StringArray::fromTokens(commandLine, true);
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(tokens.size()));
    for (const auto& token : tokens)
    {
        args.emplace_back(token.toStdString());
    }
    return args;
}

class MusicSurferApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "MusicSurfer"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        try
        {
            const auto arguments = argsFromCommandLine(commandLine);
            const auto databasePath = resolveDatabasePath(arguments);
            std::filesystem::create_directories(databasePath.parent_path());

            repository_ = std::make_shared<music_surfer::database::Database>(databasePath);

            libraryManager_ = std::make_unique<music_surfer::services::LibraryManager>();
            libraryManager_->setRepository(repository_);
            libraryManager_->setFileScanner(std::make_shared<music_surfer::utils::FileSystemScanner>());
            libraryManager_->setMetadataReader(std::make_shared<music_surfer::utils::MetadataParser>());
            seedLibraryIfEmpty(*libraryManager_);

            playlistManager_ = std::make_unique<music_surfer::services::PlaylistManagerImpl>(repository_);
            seedPlaylistsIfEmpty(*playlistManager_);

            audioPlayer_ = std::make_unique<music_surfer::services::AudioPlayer>();

            logicalMainWindow_ = std::make_unique<music_surfer::ui::MainWindow>(
                *libraryManager_, *audioPlayer_, *playlistManager_);

            mainWindow_ = std::make_unique<music_surfer::ui::JuceMainWindow>(getApplicationName(), *logicalMainWindow_);

            std::cout << "MusicSurfer GUI started. DB: " << databasePath << std::endl;
        }
        catch (const std::exception& exception)
        {
            std::cerr << "Failed to start MusicSurfer: " << exception.what() << std::endl;
            quit();
        }
    }

    void shutdown() override
    {
        mainWindow_.reset();
        logicalMainWindow_.reset();
        audioPlayer_.reset();
        playlistManager_.reset();
        libraryManager_.reset();
        repository_.reset();
    }

    void systemRequestedQuit() override { quit(); }

private:
    std::shared_ptr<music_surfer::database::Database> repository_;
    std::unique_ptr<music_surfer::services::LibraryManager> libraryManager_;
    std::unique_ptr<music_surfer::services::PlaylistManagerImpl> playlistManager_;
    std::unique_ptr<music_surfer::services::AudioPlayer> audioPlayer_;
    std::unique_ptr<music_surfer::ui::MainWindow> logicalMainWindow_;
    std::unique_ptr<music_surfer::ui::JuceMainWindow> mainWindow_;
};

START_JUCE_APPLICATION(MusicSurferApplication)

#else

int main(int argc, char** argv)
{
    try
    {
        const auto arguments = argsFromArgv(argc, argv);
        const auto databasePath = resolveDatabasePath(arguments);
        std::filesystem::create_directories(databasePath.parent_path());

        auto repository = std::make_shared<music_surfer::database::Database>(databasePath);

        music_surfer::services::LibraryManager libraryManager;
        libraryManager.setRepository(repository);
        libraryManager.setFileScanner(std::make_shared<music_surfer::utils::FileSystemScanner>());
        libraryManager.setMetadataReader(std::make_shared<music_surfer::utils::MetadataParser>());

        auto playlistManager = std::make_shared<music_surfer::services::PlaylistManagerImpl>(repository);

        seedLibraryIfEmpty(libraryManager);
        seedPlaylistsIfEmpty(*playlistManager);

        music_surfer::services::AudioPlayer audioPlayer;
        music_surfer::ui::MainWindow mainWindow(libraryManager, audioPlayer, *playlistManager);
        (void)mainWindow;

        std::cout << "MusicSurfer starter app (logical UI only). DB: " << databasePath << std::endl;
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Failed to start MusicSurfer: " << exception.what() << std::endl;
        return 1;
    }
}

#endif
