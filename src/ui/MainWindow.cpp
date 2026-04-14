#include "ui/MainWindow.hpp"

namespace music_surfer::ui
{
void MainWindow::bindServices(services::LibraryManager* libraryManager, services::AudioPlayer* audioPlayer)
{
    trackList_.libraryManager = libraryManager;
    playlists_.libraryManager = libraryManager;
    transport_.audioPlayer = audioPlayer;
}

const TrackListPanel& MainWindow::trackListPanel() const noexcept { return trackList_; }

const TransportControlsPanel& MainWindow::transportPanel() const noexcept { return transport_; }

const PlaylistPanel& MainWindow::playlistPanel() const noexcept { return playlists_; }
} // namespace music_surfer::ui
