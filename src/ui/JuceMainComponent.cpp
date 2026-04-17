#include "ui/JuceMainComponent.hpp"

#if MUSIC_SURFER_HAS_JUCE

#include <iostream>

namespace music_surfer::ui
{
JuceMainComponent::JuceMainComponent(MainWindow& logicalWindow)
    : logicalWindow_(logicalWindow), playlistModel_(*this), trackModel_(*this)
{
    playlistTitle_.setText("Playlists", juce::dontSendNotification);
    trackTitle_.setText("Tracks", juce::dontSendNotification);
    playbackStateLabel_.setText("Ready", juce::dontSendNotification);

    addAndMakeVisible(playlistTitle_);
    addAndMakeVisible(trackTitle_);
    addAndMakeVisible(playbackStateLabel_);

    playlistList_.setModel(&playlistModel_);
    trackList_.setModel(&trackModel_);
    addAndMakeVisible(playlistList_);
    addAndMakeVisible(trackList_);

    addAndMakeVisible(loadTrackButton_);
    addAndMakeVisible(playButton_);
    addAndMakeVisible(pauseButton_);
    addAndMakeVisible(stopButton_);

    loadTrackButton_.addListener(this);
    playButton_.addListener(this);
    pauseButton_.addListener(this);
    stopButton_.addListener(this);

    syncViewModels();
}

JuceMainComponent::~JuceMainComponent()
{
    loadTrackButton_.removeListener(this);
    playButton_.removeListener(this);
    pauseButton_.removeListener(this);
    stopButton_.removeListener(this);
}

void JuceMainComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour::fromRGB(24, 28, 34));
}

void JuceMainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(12);

    auto header = bounds.removeFromTop(28);
    playlistTitle_.setBounds(header.removeFromLeft(220));
    trackTitle_.setBounds(header.removeFromLeft(300));
    playbackStateLabel_.setBounds(header);

    auto controls = bounds.removeFromBottom(56);
    auto content = bounds;

    playlistList_.setBounds(content.removeFromLeft(220).reduced(0, 4));
    trackList_.setBounds(content.reduced(8, 4));

    loadTrackButton_.setBounds(controls.removeFromLeft(100).reduced(4));
    playButton_.setBounds(controls.removeFromLeft(100).reduced(4));
    pauseButton_.setBounds(controls.removeFromLeft(100).reduced(4));
    stopButton_.setBounds(controls.removeFromLeft(100).reduced(4));
}

void JuceMainComponent::buttonClicked(juce::Button* button)
{
    if (button == &loadTrackButton_)
    {
        logicalWindow_.trackListComponent().onLoadTrackRequested();
        std::cout << "Load requested" << std::endl;
    }
    else if (button == &playButton_)
    {
        logicalWindow_.playbackControlsComponent().onPlayPressed();
        std::cout << "Play pressed" << std::endl;
    }
    else if (button == &pauseButton_)
    {
        logicalWindow_.playbackControlsComponent().onPausePressed();
        std::cout << "Pause pressed" << std::endl;
    }
    else if (button == &stopButton_)
    {
        logicalWindow_.playbackControlsComponent().onStopPressed();
        std::cout << "Stop pressed" << std::endl;
    }

    syncViewModels();
}

void JuceMainComponent::syncViewModels()
{
    refreshPlaylistList();
    refreshTrackList();

    const auto& playbackModel = logicalWindow_.playbackControlsComponent().viewModel();
    juce::String stateText = "Stopped";
    if (playbackModel.state == services::AudioPlayer::State::Playing)
    {
        stateText = "Playing";
    }
    else if (playbackModel.state == services::AudioPlayer::State::Paused)
    {
        stateText = "Paused";
    }

    juce::String loadedTrackText = "None";
    if (playbackModel.loadedTrackId.has_value())
    {
        loadedTrackText = playbackModel.loadedTrackId->c_str();
    }

    playbackStateLabel_.setText("State: " + stateText + " | Loaded: " + loadedTrackText, juce::dontSendNotification);
}

void JuceMainComponent::refreshTrackList()
{
    trackRows_.clearQuick();
    const auto& trackModel = logicalWindow_.trackListComponent().viewModel();
    for (const auto& track : trackModel.tracks)
    {
        trackRows_.add(juce::String(track.title) + " [" + track.id + "]");
    }

    if (trackRows_.isEmpty())
    {
        trackRows_.add("No tracks available");
    }

    trackList_.updateContent();

    if (trackModel.selectedTrackId.has_value())
    {
        for (std::size_t index = 0; index < trackModel.tracks.size(); ++index)
        {
            if (trackModel.tracks[index].id == *trackModel.selectedTrackId)
            {
                trackList_.selectRow(static_cast<int>(index), false, false);
                break;
            }
        }
    }
}

void JuceMainComponent::refreshPlaylistList()
{
    playlistRows_.clearQuick();
    const auto& playlistModel = logicalWindow_.playlistPanelComponent().viewModel();
    for (const auto& playlist : playlistModel.playlists)
    {
        playlistRows_.add(juce::String(playlist.name) + " (" + juce::String(static_cast<int>(playlist.trackIds.size())) + " tracks)");
    }

    if (playlistRows_.isEmpty())
    {
        playlistRows_.add("No playlists available");
    }

    playlistList_.updateContent();
}

void JuceMainComponent::paintRow(const juce::StringArray& rows,
                                 int rowNumber,
                                 juce::Graphics& graphics,
                                 int width,
                                 int height,
                                 bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= rows.size())
    {
        return;
    }

    auto background = juce::Colour::fromRGB(36, 40, 47);
    if (rowIsSelected)
    {
        background = juce::Colour::fromRGB(64, 97, 136);
    }

    graphics.setColour(background);
    graphics.fillRect(0, 0, width, height);

    graphics.setColour(juce::Colours::white);
    graphics.drawText(rows[rowNumber], 8, 0, width - 16, height, juce::Justification::centredLeft);
}

int JuceMainComponent::PlaylistListModel::getNumRows() { return owner_.playlistRows_.size(); }

void JuceMainComponent::PlaylistListModel::paintListBoxItem(int rowNumber,
                                                            juce::Graphics& graphics,
                                                            int width,
                                                            int height,
                                                            bool rowIsSelected)
{
    JuceMainComponent::paintRow(owner_.playlistRows_, rowNumber, graphics, width, height, rowIsSelected);
}

int JuceMainComponent::TrackListModel::getNumRows() { return owner_.trackRows_.size(); }

void JuceMainComponent::TrackListModel::paintListBoxItem(int rowNumber,
                                                         juce::Graphics& graphics,
                                                         int width,
                                                         int height,
                                                         bool rowIsSelected)
{
    JuceMainComponent::paintRow(owner_.trackRows_, rowNumber, graphics, width, height, rowIsSelected);
}

void JuceMainComponent::TrackListModel::selectedRowsChanged(int lastRowSelected)
{
    const auto& trackViewModel = owner_.logicalWindow_.trackListComponent().viewModel();
    if (lastRowSelected >= 0 && static_cast<std::size_t>(lastRowSelected) < trackViewModel.tracks.size())
    {
        owner_.logicalWindow_.trackListComponent().onTrackSelected(
            trackViewModel.tracks[static_cast<std::size_t>(lastRowSelected)].id);
        owner_.syncViewModels();
    }
}
} // namespace music_surfer::ui

#endif
