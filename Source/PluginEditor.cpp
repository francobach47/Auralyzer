#include "PluginProcessor.h"
#include "PluginEditor.h"

static float maxDB = 24.0f;

//==============================================================================
OscilloscopeAudioProcessorEditor::OscilloscopeAudioProcessorEditor(OscilloscopeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), timeVisualizer(p), frequencyVisualizer(p)
{
    tooltipWindow->setMillisecondsBeforeTipAppears(1000);
    
    plotModeButton.setButtonText("Time");
    plotModeButton.setClickingTogglesState(true);
    plotModeButton.setBounds(0, 0, 100, 30);
    plotModeButton.setLookAndFeel(ButtonLookAndFeel::get());
    addAndMakeVisible(plotModeButton);

    plotModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, plotModeParamID.getParamID(), plotModeButton
    );
    audioProcessor.apvts.addParameterListener(plotModeParamID.getParamID(), this);

    isFrequencyMode = audioProcessor.apvts.getRawParameterValue(plotModeParamID.getParamID())->load() > 0.5f;
    plotModeButton.setButtonText(isFrequencyMode ? "Frequency" : "Time");

    plotGroup.addChildComponent(timeVisualizer);
    plotGroup.addChildComponent(frequencyVisualizer);
    timeVisualizer.setVisible(!isFrequencyMode);
    frequencyVisualizer.setVisible(isFrequencyMode);
    addAndMakeVisible(plotGroup);

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
    plotModeButton.setTopLeftPosition(25, 450);

    // Position the time visualizer
    frequencyVisualizer.setBounds(plotGroup.getLocalBounds());
    timeVisualizer.setBounds(plotGroup.getLocalBounds());
}

void OscilloscopeAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == plotModeParamID.getParamID())
    {
        bool isFrequencyMode = newValue > 0.5f; // 0=Time, 1=Frequency
        timeVisualizer.setVisible(!isFrequencyMode);
        frequencyVisualizer.setVisible(isFrequencyMode);
        plotModeButton.setButtonText(isFrequencyMode ? "Frequency" : "Time");
    }
}