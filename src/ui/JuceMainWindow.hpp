#pragma once

#include "ui/JuceMainComponent.hpp"

#if MUSIC_SURFER_HAS_JUCE

#include <juce_gui_basics/juce_gui_basics.h>

namespace music_surfer::ui
{
class JuceMainWindow final : public juce::DocumentWindow
{
public:
    JuceMainWindow(juce::String name, MainWindow& logicalWindow);

    void closeButtonPressed() override;

private:
    std::unique_ptr<JuceMainComponent> mainComponent_;
};
} // namespace music_surfer::ui

#endif
