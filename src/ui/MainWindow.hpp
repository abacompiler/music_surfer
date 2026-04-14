#pragma once

#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"

namespace music_surfer::ui
{
struct TrackListPanel
{
    services::LibraryManager* libraryManager{nullptr};
};

struct TransportControlsPanel
{
    services::AudioPlayer* audioPlayer{nullptr};
};

struct PlaylistPanel
{
    services::LibraryManager* libraryManager{nullptr};
};

class MainWindow
{
public:
    void bindServices(services::LibraryManager* libraryManager, services::AudioPlayer* audioPlayer);

    const TrackListPanel& trackListPanel() const noexcept;
    const TransportControlsPanel& transportPanel() const noexcept;
    const PlaylistPanel& playlistPanel() const noexcept;

private:
    TrackListPanel trackList_;
    TransportControlsPanel transport_;
    PlaylistPanel playlists_;
};
} // namespace music_surfer::ui
