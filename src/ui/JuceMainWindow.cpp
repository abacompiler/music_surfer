#include "ui/JuceMainWindow.hpp"

#if MUSIC_SURFER_HAS_JUCE

namespace music_surfer::ui
{
JuceMainWindow::JuceMainWindow(juce::String name, MainWindow& logicalWindow)
    : juce::DocumentWindow(name,
                           juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                               juce::ResizableWindow::backgroundColourId),
                           juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);

    mainComponent_ = std::make_unique<JuceMainComponent>(logicalWindow);
    setContentOwned(mainComponent_.release(), true);

    centreWithSize(1024, 640);
    setVisible(true);
}

void JuceMainWindow::closeButtonPressed() { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
} // namespace music_surfer::ui

#endif
