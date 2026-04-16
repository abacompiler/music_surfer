#include "ui/MainWindow.hpp"

namespace music_surfer::ui
{
namespace
{
TrackListViewModel makeTrackListViewModel(const std::vector<core::Track>& tracks,
                                         const std::optional<core::TrackId>& selectedTrackId)
{
    TrackListViewModel viewModel;
    viewModel.selectedTrackId = selectedTrackId;
    viewModel.tracks.reserve(tracks.size());
    for (const auto& track : tracks)
    {
        viewModel.tracks.push_back(TrackListItemViewModel{track.id(), track.title()});
    }
    return viewModel;
}

PlaylistPanelViewModel makePlaylistPanelViewModel(const std::vector<core::Playlist>& playlists)
{
    PlaylistPanelViewModel viewModel;
    viewModel.playlists.reserve(playlists.size());
    for (const auto& playlist : playlists)
    {
        viewModel.playlists.push_back(PlaylistItemViewModel{playlist.id(), playlist.name(), playlist.trackIds()});
    }
    return viewModel;
}
} // namespace

TrackListPresenter::TrackListPresenter(services::LibraryManager& libraryManager,
                                       services::AudioPlayer& audioPlayer,
                                       ITrackListView& view,
                                       IPlaybackControlsView& playbackView)
    : libraryManager_(libraryManager), audioPlayer_(audioPlayer), view_(view), playbackView_(playbackView)
{
}

void TrackListPresenter::refresh()
{
    view_.renderTrackList(makeTrackListViewModel(libraryManager_.listTracks(), selectedTrackId_));
}

void TrackListPresenter::selectTrack(const core::TrackId& trackId)
{
    selectedTrackId_ = trackId;
    refresh();
}

void TrackListPresenter::loadSelectedTrack()
{
    if (!selectedTrackId_.has_value())
    {
        return;
    }

    const auto track = libraryManager_.findTrack(*selectedTrackId_);
    if (!track.has_value())
    {
        return;
    }

    audioPlayer_.load(*track);
    playbackView_.renderPlaybackControls(PlaybackControlsViewModel{audioPlayer_.state(), track->id()});
}

PlaybackControlsPresenter::PlaybackControlsPresenter(services::AudioPlayer& audioPlayer, IPlaybackControlsView& view)
    : audioPlayer_(audioPlayer), view_(view)
{
}

void PlaybackControlsPresenter::play()
{
    audioPlayer_.play();
    refresh();
}

void PlaybackControlsPresenter::pause()
{
    audioPlayer_.pause();
    refresh();
}

void PlaybackControlsPresenter::stop()
{
    audioPlayer_.stop();
    refresh();
}

void PlaybackControlsPresenter::refresh()
{
    std::optional<core::TrackId> loadedTrackId;
    const auto currentTrack = audioPlayer_.currentTrack();
    if (currentTrack.has_value())
    {
        loadedTrackId = currentTrack->id();
    }

    view_.renderPlaybackControls(PlaybackControlsViewModel{audioPlayer_.state(), loadedTrackId});
}

PlaylistPanelPresenter::PlaylistPanelPresenter(services::PlaylistManager& playlistManager, IPlaylistPanelView& view)
    : playlistManager_(playlistManager), view_(view)
{
}

void PlaylistPanelPresenter::refresh()
{
    view_.renderPlaylists(makePlaylistPanelViewModel(playlistManager_.listPlaylists()));
}

void PlaylistPanelPresenter::createPlaylist(const std::string& name)
{
    if (!name.empty())
    {
        (void)playlistManager_.createPlaylist(name);
    }
    refresh();
}

void PlaylistPanelPresenter::deletePlaylist(const core::PlaylistId& playlistId)
{
    playlistManager_.deletePlaylist(playlistId);
    refresh();
}

void PlaylistPanelPresenter::addTrackToPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    playlistManager_.addTrack(playlistId, trackId);
    refresh();
}

void PlaylistPanelPresenter::removeTrackFromPlaylist(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    playlistManager_.removeTrack(playlistId, trackId);
    refresh();
}

void TrackListComponent::bindPresenter(TrackListPresenter& presenter)
{
    presenter_ = &presenter;
    presenter_->refresh();
}

void TrackListComponent::onTrackSelected(const core::TrackId& trackId)
{
    if (presenter_ != nullptr)
    {
        presenter_->selectTrack(trackId);
    }
}

void TrackListComponent::onLoadTrackRequested()
{
    if (presenter_ != nullptr)
    {
        presenter_->loadSelectedTrack();
    }
}

const TrackListViewModel& TrackListComponent::viewModel() const noexcept { return viewModel_; }

void TrackListComponent::renderTrackList(const TrackListViewModel& viewModel) { viewModel_ = viewModel; }

void PlaybackControlsComponent::bindPresenter(PlaybackControlsPresenter& presenter)
{
    presenter_ = &presenter;
    presenter_->refresh();
}

void PlaybackControlsComponent::onPlayPressed()
{
    if (presenter_ != nullptr)
    {
        presenter_->play();
    }
}

void PlaybackControlsComponent::onPausePressed()
{
    if (presenter_ != nullptr)
    {
        presenter_->pause();
    }
}

void PlaybackControlsComponent::onStopPressed()
{
    if (presenter_ != nullptr)
    {
        presenter_->stop();
    }
}

const PlaybackControlsViewModel& PlaybackControlsComponent::viewModel() const noexcept { return viewModel_; }

void PlaybackControlsComponent::renderPlaybackControls(const PlaybackControlsViewModel& viewModel) { viewModel_ = viewModel; }

void PlaylistPanelComponent::bindPresenter(PlaylistPanelPresenter& presenter)
{
    presenter_ = &presenter;
    presenter_->refresh();
}

void PlaylistPanelComponent::onCreatePlaylistRequested(const std::string& name)
{
    if (presenter_ != nullptr)
    {
        presenter_->createPlaylist(name);
    }
}

void PlaylistPanelComponent::onDeletePlaylistRequested(const core::PlaylistId& playlistId)
{
    if (presenter_ != nullptr)
    {
        presenter_->deletePlaylist(playlistId);
    }
}

void PlaylistPanelComponent::onAddTrackRequested(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    if (presenter_ != nullptr)
    {
        presenter_->addTrackToPlaylist(playlistId, trackId);
    }
}

void PlaylistPanelComponent::onRemoveTrackRequested(const core::PlaylistId& playlistId, const core::TrackId& trackId)
{
    if (presenter_ != nullptr)
    {
        presenter_->removeTrackFromPlaylist(playlistId, trackId);
    }
}

const PlaylistPanelViewModel& PlaylistPanelComponent::viewModel() const noexcept { return viewModel_; }

void PlaylistPanelComponent::renderPlaylists(const PlaylistPanelViewModel& viewModel) { viewModel_ = viewModel; }

MainWindow::MainWindow(services::LibraryManager& libraryManager,
                       services::AudioPlayer& audioPlayer,
                       services::PlaylistManager& playlistManager)
    : trackListPresenter_(libraryManager, audioPlayer, trackList_, playbackControls_),
      playbackControlsPresenter_(audioPlayer, playbackControls_),
      playlistPanelPresenter_(playlistManager, playlistPanel_)
{
    trackList_.bindPresenter(trackListPresenter_);
    playbackControls_.bindPresenter(playbackControlsPresenter_);
    playlistPanel_.bindPresenter(playlistPanelPresenter_);
}

TrackListComponent& MainWindow::trackListComponent() noexcept { return trackList_; }

PlaybackControlsComponent& MainWindow::playbackControlsComponent() noexcept { return playbackControls_; }

PlaylistPanelComponent& MainWindow::playlistPanelComponent() noexcept { return playlistPanel_; }
} // namespace music_surfer::ui
