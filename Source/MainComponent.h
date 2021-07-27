#pragma once
#include <JuceHeader.h>

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    
    enum class TransportState
    {
        Stopping,
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused
    };

    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void resized() override;

    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

private:

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    juce::ToggleButton loopingToggle;
    juce::Label currentPositionLabel;
    
    TransportState state;

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    void timerCallback();
    void updateLoopState(bool shouldLoop);
    void loopButtonChanged();

    void changeListenerCallback(juce::ChangeBroadcaster* source);
    void changeState(TransportState newState);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
