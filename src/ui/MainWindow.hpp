#pragma once

#include "core/Playlist.hpp"
#include "core/Track.hpp"
#include "services/AudioPlayer.hpp"
#include "services/LibraryManager.hpp"
#include "services/PlaylistManager.hpp"

#include <optional>
#include <string>
#include <vector>

#if MUSIC_SURFER_HAS_JUCE
#include <juce_gui_basics/juce_gui_basics.h>
#endif

namespace music_surfer::ui
{
#if MUSIC_SURFER_HAS_JUCE
using ComponentBase = juce::Component;
#else
class ComponentBase
{
public:
    virtual ~ComponentBase() = default;
};
#endif

struct TrackListItemViewModel
{
    core::TrackId id;
    std::string title;
};

struct TrackListViewModel
{
    std::vector<TrackListItemViewModel> tracks;
    std::optional<core::TrackId> selectedTrackId;
};

struct PlaybackControlsViewModel
{
    services::AudioPlayer::State state{services::AudioPlayer::State::Stopped};
    std::optional<core::TrackId> loadedTrackId;
};

struct PlaylistItemViewModel
{
    core::PlaylistId id;
    std::string name;
    std::vector<core::TrackId> trackIds;
};

struct PlaylistPanelViewModel
{
    std::vector<PlaylistItemViewModel> playlists;
};

class ITrackListView
{
public:
    virtual ~ITrackListView() = default;
    virtual void renderTrackList(const TrackListViewModel& viewModel) = 0;
};

class IPlaybackControlsView
{
public:
    virtual ~IPlaybackControlsView() = default;
    virtual void renderPlaybackControls(const PlaybackControlsViewModel& viewModel) = 0;
};

class IPlaylistPanelView
{
public:
    virtual ~IPlaylistPanelView() = default;
    virtual void renderPlaylists(const PlaylistPanelViewModel& viewModel) = 0;
};

class TrackListPresenter
{
public:
    TrackListPresenter(services::LibraryManager& libraryManager,
                       services::AudioPlayer& audioPlayer,
                       ITrackListView& view,
                       IPlaybackControlsView& playbackView);

    void refresh();
    void selectTrack(const core::TrackId& trackId);
    void loadSelectedTrack();

private:
    services::LibraryManager& libraryManager_;
    services::AudioPlayer& audioPlayer_;
    ITrackListView& view_;
    IPlaybackControlsView& playbackView_;
    std::optional<core::TrackId> selectedTrackId_;
};

class PlaybackControlsPresenter
{
public:
    PlaybackControlsPresenter(services::AudioPlayer& audioPlayer, IPlaybackControlsView& view);

    void play();
    void pause();
    void stop();
    void refresh();

private:
    services::AudioPlayer& audioPlayer_;
    IPlaybackControlsView& view_;
};

class PlaylistPanelPresenter
{
public:
    PlaylistPanelPresenter(services::PlaylistManager& playlistManager, IPlaylistPanelView& view);

    void refresh();
    void createPlaylist(const std::string& name);
    void deletePlaylist(const core::PlaylistId& playlistId);
    void addTrackToPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId);
    void removeTrackFromPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId);

private:
    services::PlaylistManager& playlistManager_;
    IPlaylistPanelView& view_;
};

class TrackListComponent final : public ComponentBase, public ITrackListView
{
public:
    void bindPresenter(TrackListPresenter& presenter);

    void onTrackSelected(const core::TrackId& trackId);
    void onLoadTrackRequested();

    const TrackListViewModel& viewModel() const noexcept;
    void renderTrackList(const TrackListViewModel& viewModel) override;

private:
    TrackListPresenter* presenter_{nullptr};
    TrackListViewModel viewModel_;
};

class PlaybackControlsComponent final : public ComponentBase, public IPlaybackControlsView
{
public:
    void bindPresenter(PlaybackControlsPresenter& presenter);

    void onPlayPressed();
    void onPausePressed();
    void onStopPressed();

    const PlaybackControlsViewModel& viewModel() const noexcept;
    void renderPlaybackControls(const PlaybackControlsViewModel& viewModel) override;

private:
    PlaybackControlsPresenter* presenter_{nullptr};
    PlaybackControlsViewModel viewModel_;
};

class PlaylistPanelComponent final : public ComponentBase, public IPlaylistPanelView
{
public:
    void bindPresenter(PlaylistPanelPresenter& presenter);

    void onCreatePlaylistRequested(const std::string& name);
    void onDeletePlaylistRequested(const core::PlaylistId& playlistId);
    void onAddTrackRequested(const core::PlaylistId& playlistId, const core::TrackId& trackId);
    void onRemoveTrackRequested(const core::PlaylistId& playlistId, const core::TrackId& trackId);

    const PlaylistPanelViewModel& viewModel() const noexcept;
    void renderPlaylists(const PlaylistPanelViewModel& viewModel) override;

private:
    PlaylistPanelPresenter* presenter_{nullptr};
    PlaylistPanelViewModel viewModel_;
};

class MainWindow
{
public:
    MainWindow(services::LibraryManager& libraryManager,
               services::AudioPlayer& audioPlayer,
               services::PlaylistManager& playlistManager);

    TrackListComponent& trackListComponent() noexcept;
    PlaybackControlsComponent& playbackControlsComponent() noexcept;
    PlaylistPanelComponent& playlistPanelComponent() noexcept;

private:
    TrackListComponent trackList_;
    PlaybackControlsComponent playbackControls_;
    PlaylistPanelComponent playlistPanel_;

    TrackListPresenter trackListPresenter_;
    PlaybackControlsPresenter playbackControlsPresenter_;
    PlaylistPanelPresenter playlistPanelPresenter_;
};
} // namespace music_surfer::ui
