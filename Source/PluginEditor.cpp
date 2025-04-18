#include "PluginProcessor.h"
#include "PluginEditor.h"

static float maxDB = 24.0f;

//==============================================================================
OscilloscopeAudioProcessorEditor::OscilloscopeAudioProcessorEditor (OscilloscopeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    tooltipWindow->setMillisecondsBeforeTipAppears(1000);

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

    setSize(1200, 450);

#ifdef JUCE_OPENGL
        openGLContext.attachTo(*getTopLevelComponent());
#endif

    startTimerHz(30);
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
    g.fillAll (Colors::background);
}

void OscilloscopeAudioProcessorEditor::resized()
{
    //audioProcessor.setSavedSize({ getWidth(), getHeight() });
    //plotFrame = getLocalBounds().reduced(3, 3);
    
    auto bounds = getLocalBounds();

    int y = 20;
    int height = 100;

    int plotSectionWidth = 900;

    // TODO: Change values into variables and applied them

    // Position the groups
    plotGroup.setBounds(25, y, plotSectionWidth, 410);
    optionsGroup.setBounds(plotSectionWidth + 50, y, (110 * 2) + 10, 150);
    horizontalGroup.setBounds(plotSectionWidth + 50, y + 150, 110, 260);
    verticalGroup.setBounds(horizontalGroup.getRight() + 10, y + 150, 110, 260);

    // Position the knobs inside the groups
    rangeKnob.setTopLeftPosition(20, y);
    modeKnob.setTopLeftPosition(rangeKnob.getRight() + 50, y);
    horizontalPositionKnob.setTopLeftPosition(20, 20);
    horizontalScaleKnob.setTopLeftPosition(horizontalPositionKnob.getX(), horizontalPositionKnob.getBottom() + 10);
    verticalPositionKnob.setTopLeftPosition(20, 20);
    verticalScaleKnob.setTopLeftPosition(verticalPositionKnob.getX(), verticalPositionKnob.getBottom() + 10);
}

void OscilloscopeAudioProcessorEditor::timerCallback()
{
    //if (audioProcessor.checkForNewAnalyserData())
    //{
    //    repaint(plotFrame);
    //}
}

//float OscilloscopeAudioProcessorEditor::getFrequencyForPosition(float pos)
//{
//    return 20.0f * std::pow(2.0f, pos * 10.0f);
//}