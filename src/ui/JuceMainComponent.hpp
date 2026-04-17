#pragma once

#include "ui/MainWindow.hpp"

#if MUSIC_SURFER_HAS_JUCE

#include <juce_gui_basics/juce_gui_basics.h>

namespace music_surfer::ui
{
class JuceMainComponent final : public juce::Component, private juce::Button::Listener
{
public:
    explicit JuceMainComponent(MainWindow& logicalWindow);
    ~JuceMainComponent() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    class PlaylistListModel final : public juce::ListBoxModel
    {
    public:
        explicit PlaylistListModel(JuceMainComponent& owner) : owner_(owner) {}

        int getNumRows() override;
        void paintListBoxItem(int rowNumber,
                              juce::Graphics& graphics,
                              int width,
                              int height,
                              bool rowIsSelected) override;

    private:
        JuceMainComponent& owner_;
    };

    class TrackListModel final : public juce::ListBoxModel
    {
    public:
        explicit TrackListModel(JuceMainComponent& owner) : owner_(owner) {}

        int getNumRows() override;
        void paintListBoxItem(int rowNumber,
                              juce::Graphics& graphics,
                              int width,
                              int height,
                              bool rowIsSelected) override;
        void selectedRowsChanged(int lastRowSelected) override;

    private:
        JuceMainComponent& owner_;
    };

    void buttonClicked(juce::Button* button) override;

    void syncViewModels();
    void refreshTrackList();
    void refreshPlaylistList();

    static void paintRow(const juce::StringArray& rows,
                         int rowNumber,
                         juce::Graphics& graphics,
                         int width,
                         int height,
                         bool rowIsSelected);

    MainWindow& logicalWindow_;

    PlaylistListModel playlistModel_;
    TrackListModel trackModel_;

    juce::Label playlistTitle_;
    juce::Label trackTitle_;
    juce::Label playbackStateLabel_;

    juce::ListBox playlistList_;
    juce::ListBox trackList_;

    juce::TextButton loadTrackButton_{"Load"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton pauseButton_{"Pause"};
    juce::TextButton stopButton_{"Stop"};

    juce::StringArray playlistRows_;
    juce::StringArray trackRows_;
};
} // namespace music_surfer::ui

#endif
