#include "database/Database.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "services/PlaylistManagerImpl.hpp"
#include "ui/MainWindow.hpp"
#include "utils/FileSystemScanner.hpp"
#include "utils/MetadataParser.hpp"

#include <iostream>
#include <memory>

int main()
{
    auto repository = std::make_shared<music_surfer::database::Database>("music_surfer.db");

    music_surfer::services::LibraryManager libraryManager;
    libraryManager.setRepository(repository);
    libraryManager.setFileScanner(std::make_shared<music_surfer::utils::FileSystemScanner>());
    libraryManager.setMetadataReader(std::make_shared<music_surfer::utils::MetadataParser>());

    auto playlistManager = std::make_shared<music_surfer::services::PlaylistManagerImpl>(repository);

    music_surfer::services::AudioPlayer audioPlayer;
    music_surfer::ui::MainWindow mainWindow;
    mainWindow.bindServices(&libraryManager, &audioPlayer);
    (void)playlistManager;

    std::cout << "MusicSurfer starter app (window + services wired)" << std::endl;
    return 0;
}
