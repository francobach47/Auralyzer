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

    audioProcessor.apvts.addParameterListener(verticalScaleParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(verticalPositionParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(horizontalScaleParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(horizontalPositionParamID.getParamID(), this);
    //audioProcessor.apvts.addParameterListener(triggerLevelParamID.getParamID(), this);

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

    triggerGroup.setText("Trigger");
    triggerGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    //triggerGroup.addAndMakeVisible(triggerLevelKnob);
    addAndMakeVisible(triggerGroup);

    // Forzar valores iniciales para visualización
    //float hScale = std::pow(2.0f, *audioProcessor.apvts.getRawParameterValue(horizontalScaleParamID.getParamID()));
    //float hOffset = *audioProcessor.apvts.getRawParameterValue(horizontalPositionParamID.getParamID());
    //float vScale = std::pow(2.0f, *audioProcessor.apvts.getRawParameterValue(verticalScaleParamID.getParamID()));
    //float vOffset = *audioProcessor.apvts.getRawParameterValue(verticalPositionParamID.getParamID());
    //float trigLevel = *audioProcessor.apvts.getRawParameterValue(triggerLevelParamID.getParamID());

    //timeVisualizer.setHorizontalScale(hScale);
    //timeVisualizer.setHorizontalOffset(hOffset);
    //timeVisualizer.setVerticalGain(vScale);
    //timeVisualizer.setVerticalOffset(vOffset);
   // timeVisualizer.setTriggerLevel(trigLevel);

    setLookAndFeel(&mainLF);

    setSize(1350, 490);

#ifdef JUCE_OPENGL
    openGLContext.attachTo(*getTopLevelComponent());
#endif     
}

OscilloscopeAudioProcessorEditor::~OscilloscopeAudioProcessorEditor()
{
    audioProcessor.apvts.removeParameterListener(plotModeParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(verticalScaleParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(verticalPositionParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(horizontalScaleParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(horizontalPositionParamID.getParamID(), this);
    //audioProcessor.apvts.removeParameterListener(triggerLevelParamID.getParamID(), this);

    setLookAndFeel(nullptr);

#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void OscilloscopeAudioProcessorEditor::paint(juce::Graphics& g)
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
    optionsGroup.setBounds(plotSectionWidth + plotBoxesSpace, y, (verhorSectionWidth * 3) + space * 2, optionsHeight);
    horizontalGroup.setBounds(plotSectionWidth + plotBoxesSpace, y + optionsHeight + space, verhorSectionWidth, verhorSectionHeight);
    verticalGroup.setBounds(horizontalGroup.getRight() + space, y + optionsHeight + space, verhorSectionWidth, verhorSectionHeight);
    triggerGroup.setBounds(verticalGroup.getRight() + space, y + optionsHeight + space, verhorSectionWidth, verhorSectionHeight);

    // Position the knobs inside the groups
    // TO DO: Terminar de acomodar las perillas de rango y modo
    rangeKnob.setTopLeftPosition(height - 20, y);
    modeKnob.setTopLeftPosition((height * 2 + space) - 20, y);

    horizontalPositionKnob.setTopLeftPosition(20, 20);
    horizontalScaleKnob.setTopLeftPosition(horizontalPositionKnob.getX(), horizontalPositionKnob.getBottom() + 10);
    verticalPositionKnob.setTopLeftPosition(20, 20);
    verticalScaleKnob.setTopLeftPosition(verticalPositionKnob.getX(), verticalPositionKnob.getBottom() + 10);
    //triggerLevelKnob.setTopLeftPosition(horizontalScaleKnob.getX(), horizontalScaleKnob.getBottom() + 10);

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

    if (parameterID == verticalScaleParamID.getParamID())
    {
        float verticalGain = std::pow(2.0f, newValue); // escalar exponencial
        timeVisualizer.setVerticalGain(verticalGain);
    }

    if (parameterID == verticalPositionParamID.getParamID())
    {
        float verticalOffset = newValue * (timeVisualizer.getHeight() / 2); // ajustar como prefieras
        timeVisualizer.setVerticalOffset(verticalOffset);
    }


    if (parameterID == horizontalScaleParamID.getParamID())
    {
        float horizontalGain = std::pow(2.0f, newValue); // escalar exponencial
        timeVisualizer.setHorizontalScale(horizontalGain);
    }

    if (parameterID == horizontalPositionParamID.getParamID())
    {
        float horizontalOffset = newValue * (timeVisualizer.getHeight() / 2); // ajustar como prefieras
        timeVisualizer.setHorizontalOffset(newValue); // pasa directamente el valor del pote
    }

    /*if (parameterID == triggerLevelParamID.getParamID())
    {
        timeVisualizer.setTriggerLevel(newValue);
    }*/
}