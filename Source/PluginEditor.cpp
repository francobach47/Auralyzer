#include "PluginProcessor.h"
#include "PluginEditor.h"

static float maxDB = 24.0f;

//==============================================================================
OscilloscopeAudioProcessorEditor::OscilloscopeAudioProcessorEditor(OscilloscopeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), timeVisualizer(p), frequencyVisualizer(p)
{
    tooltipWindow->setMillisecondsBeforeTipAppears(1000);
    
    // TODO: CHANGE AND FIX THIS
    // INSTANCIATE TIME WHEN YOU RUN AND START THE PLUGIN.
    // IF YOU CLICK THE BUTTON WITHOUT SOUND IT CRASHES
    timeFreqButton.setButtonText("Time");
    timeFreqButton.setClickingTogglesState(true);
    timeFreqButton.setBounds(0, 0, 100, 30);
    timeFreqButton.setLookAndFeel(ButtonLookAndFeel::get());
    addAndMakeVisible(timeFreqButton);
    timeFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, timeFreqParamID.getParamID(), timeFreqButton
    );
    audioProcessor.apvts.addParameterListener(timeFreqParamID.getParamID(), this);

    plotGroup.addAndMakeVisible(timeVisualizer);
    plotGroup.addChildComponent(frequencyVisualizer);
    addAndMakeVisible(plotGroup);

    timeVisualizer.setVisible(true);
    frequencyVisualizer.setVisible(false);

    optionsGroup.addAndMakeVisible(rangeKnob);
    optionsGroup.addAndMakeVisible(modeKnob);
    addAndMakeVisible(optionsGroup);

    horizontalGroup.setText("Horizontal");
    horizontalGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    horizontalGroup.addAndMakeVisible(horizontalPositionKnob);
    horizontalGroup.addAndMakeVisible(horizontalScaleKnob);
    addAndMakeVisible(horizontalGroup);

    verticalGroup.setText("Vertical");
    verticalGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    verticalGroup.addAndMakeVisible(verticalPositionKnob);
    verticalGroup.addAndMakeVisible(verticalScaleKnob);
    addAndMakeVisible(verticalGroup);

    setLookAndFeel(&mainLF);

    setSize(1200, 490);

#ifdef JUCE_OPENGL
        openGLContext.attachTo(*getTopLevelComponent());
#endif     
}

OscilloscopeAudioProcessorEditor::~OscilloscopeAudioProcessorEditor()
{
    setLookAndFeel(nullptr);

#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void OscilloscopeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colors::background);
}

void OscilloscopeAudioProcessorEditor::resized()
{
    int y = 20;
    int height = 100;
    int plotSectionWidth = 900;
    int plotSectionHeight = 420;
    int verhorSectionHeight = 260;
    int verhorSectionWidth = 110;
    int optionsHeight = 150;
    int space = 10;
    int plotBoxesSpace = 50;

    // Position the groups
    plotGroup.setBounds(25, y, plotSectionWidth, plotSectionHeight);
    optionsGroup.setBounds(plotSectionWidth + plotBoxesSpace, y, (verhorSectionWidth * 2) + space, optionsHeight);
    horizontalGroup.setBounds(plotSectionWidth + plotBoxesSpace, y + optionsHeight + space, verhorSectionWidth, verhorSectionHeight);
    verticalGroup.setBounds(horizontalGroup.getRight() + space, y + optionsHeight + space, verhorSectionWidth, verhorSectionHeight);

    // Position the knobs inside the groups
    rangeKnob.setTopLeftPosition(20, y);
    modeKnob.setTopLeftPosition(rangeKnob.getRight() + 50, y);
    horizontalPositionKnob.setTopLeftPosition(20, 20);
    horizontalScaleKnob.setTopLeftPosition(horizontalPositionKnob.getX(), horizontalPositionKnob.getBottom() + 10);
    verticalPositionKnob.setTopLeftPosition(20, 20);
    verticalScaleKnob.setTopLeftPosition(verticalPositionKnob.getX(), verticalPositionKnob.getBottom() + 10);
    
    // Position the button
    timeFreqButton.setTopLeftPosition(25, 450);

    // Position the time visualizer
    frequencyVisualizer.setBounds(plotGroup.getLocalBounds());
    timeVisualizer.setBounds(plotGroup.getLocalBounds());
}

//void OscilloscopeAudioProcessorEditor::buttonClicked(juce::Button* button)
//{
//    if (button == &timeFreqButton)
//    {
//        bool showFrequency = timeFreqButton.getToggleState();
//        timeFreqButton.setButtonText(showFrequency ? "Time" : "Frequency");
//        
//        timeVisualizer.setVisible(!showFrequency);
//        frequencyVisualizer.setVisible(showFrequency);
//    }
//}

void OscilloscopeAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == timeFreqParamID.getParamID())
    {
        bool showFrequency = newValue > 0.5f;
        timeVisualizer.setVisible(!showFrequency);
        frequencyVisualizer.setVisible(showFrequency);
        timeFreqButton.setButtonText(showFrequency ? "Time" : "Frequency");
    }
}