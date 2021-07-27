#include "MainComponent.h"

MainComponent::MainComponent()
    : state (TransportState::Stopped)
{
    setSize (350, 100);
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open File...");
    openButton.onClick = [this] { openButtonClicked(); };
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkkhaki);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    stopButton.setEnabled(false);

    addAndMakeVisible(&loopingToggle);
    loopingToggle.setButtonText("Repeat");
    loopingToggle.onClick = [this] { loopButtonChanged(); };

    addAndMakeVisible(&currentPositionLabel);
    currentPositionLabel.setText(" ", juce::dontSendNotification);

    startTimer(20);

    // permissions to open input channels
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{

    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
    
}

void MainComponent::releaseResources()
{
    //transportSource.releaseResources();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void MainComponent::resized()
{
    openButton.setBounds(10, 10, (getWidth() / 3) - 20, 40);    
    playButton.setBounds((getWidth() / 3) + 10, 10, (getWidth() / 3) - 20, 40);
    stopButton.setBounds((getWidth() / 3) * 2 + 10, 10, (getWidth() / 3) - 20, 40);
    loopingToggle.setBounds(10, 60, getWidth() - 20, 20);
    currentPositionLabel.setBounds((getWidth() / 3) * 2 + 10, 60, getWidth() - 20, 20);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(TransportState::Playing);
        else if ((state == TransportState::Stopping) || (state == TransportState::Playing))
            changeState(TransportState::Stopped);
        else if (TransportState::Pausing == state)
            changeState(TransportState::Paused);
    }
}

void MainComponent::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case TransportState::Stopped:
            playButton.setButtonText("Play");
            stopButton.setButtonText("Stop");
            stopButton.setEnabled(false);
            transportSource.setPosition(0.0);
            break;

        case TransportState::Starting:
            transportSource.start();
            break;

        case TransportState::Playing:
            playButton.setButtonText("Pause");
            stopButton.setButtonText("Stop");
            stopButton.setEnabled(true);
            break;

        case TransportState::Pausing:
            transportSource.stop();
            break;

        case TransportState::Paused:
            playButton.setButtonText("Resume");
            stopButton.setButtonText("Return to Zero");
            break;

        case TransportState::Stopping:
            transportSource.stop();
            break;
        }
    }
}

void MainComponent::openButtonClicked()
{
    juce::FileChooser chooser("Please select the file you want to play...");

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        auto* reader = formatManager.createReaderFor(file);

        if (reader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            readerSource.reset(newSource.release());
        }
    }
}

void MainComponent::playButtonClicked()
{
    if ((state == TransportState::Stopped) || (state == TransportState::Paused))
        changeState(TransportState::Starting);
    else if (state == TransportState::Playing)
        changeState(TransportState::Pausing);
}

void MainComponent::stopButtonClicked()
{
    if (state == TransportState::Paused)
        changeState(TransportState::Stopped);
    else
        changeState(TransportState::Stopping);

    currentPositionLabel.setText("00:00:000", juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    if (transportSource.isPlaying())
    {
        juce::RelativeTime position(transportSource.getCurrentPosition());

        auto minutes = ((int)position.inMinutes()) % 60;
        auto seconds = ((int)position.inSeconds()) % 60;
        auto millis = ((int)position.inMilliseconds()) % 99;

        auto positionString = juce::String::formatted("%02d:%02d:%03d", minutes, seconds, millis);

        currentPositionLabel.setText(positionString, juce::dontSendNotification);
    }
    else
    {
        //currentPositionLabel.setText(" ", juce::dontSendNotification);
    }
}

void MainComponent::updateLoopState(bool shouldLoop)
{
    if (readerSource.get() != nullptr)
        readerSource->setLooping(shouldLoop);
}

void MainComponent::loopButtonChanged()
{
    updateLoopState(loopingToggle.getToggleState());
}
