#include "PluginProcessor.h"
#include "PluginEditor.h"

static float maxDB = 24.0f;

//==============================================================================
OscilloscopeAudioProcessorEditor::OscilloscopeAudioProcessorEditor (OscilloscopeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    tooltipWindow->setMillisecondsBeforeTipAppears(1000);

    horizontalGroup.setText("Horizontal");
    horizontalGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    horizontalGroup.addAndMakeVisible(horizontalScaleKnob);
    horizontalGroup.addAndMakeVisible(horizontalPositionKnob);
    addAndMakeVisible(horizontalGroup);

    verticalGroup.setText("Vertical");
    verticalGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    addAndMakeVisible(verticalGroup);

    auto size = audioProcessor.getSavedSize();
    setResizable(false, false);
    setSize(size.x, size.y);

#ifdef JUCE_OPENGL
        openGLContext.attachTo(*getTopLevelComponent());
#endif

    startTimerHz(30);
}

OscilloscopeAudioProcessorEditor::~OscilloscopeAudioProcessorEditor()
{
#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void OscilloscopeAudioProcessorEditor::paint (juce::Graphics& g)
{
    //const auto frequencyResponseColor = juce::Colours::greenyellow;
    //
    //juce::Graphics::ScopedSaveState state(g);

    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setFont(12.0f);
    //g.setColour(juce::Colours::silver);
    //g.drawRoundedRectangle(plotFrame.toFloat(), 5, 2);

    //for (int i = 0; i < 10; ++i)
    //{
    //    g.setColour(juce::Colours::silver.withAlpha(0.3f));
    //    auto x = plotFrame.getX() + plotFrame.getWidth() * i * 0.1f;
    //    if (i > 0)
    //        g.drawVerticalLine(juce::roundToInt(x), float(plotFrame.getY()), float(plotFrame.getBottom()));
    //        
    //    g.setColour(juce::Colours::silver);
    //    auto freq = getFrequencyForPosition(i * 0.1f);
    //    g.drawFittedText((freq < 1000) ? juce::String(freq) + " Hz"
    //                                   : juce::String(freq / 1000, 1) + " kHz",
    //                                   juce::roundToInt(x + 3), plotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    //}

    //g.setColour(juce::Colours::silver.withAlpha(0.3f));
    //g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.25 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));
    //g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.75 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));

    //g.setColour(juce::Colours::silver);
    //g.drawFittedText(juce::String(maxDB) + " dB", plotFrame.getX() + 3, plotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
    //g.drawFittedText(juce::String(maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.25 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    //g.drawFittedText(" 0 dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.5 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    //g.drawFittedText(juce::String(-maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.75 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);

    //g.reduceClipRegion(plotFrame);

    //g.setFont(16.0f);
    //audioProcessor.createAnalyserPlot(analyzerPath, plotFrame, 20.0f);
    //g.setColour(frequencyResponseColor);
    //g.strokePath(analyzerPath, juce::PathStrokeType(1.0f));

    ////g.setColour(juce::Colours::silver);
    ////g.strokePath(frequencyResponse, juce::PathStrokeType(1.0f));
}

void OscilloscopeAudioProcessorEditor::resized()
{
    //audioProcessor.setSavedSize({ getWidth(), getHeight() });
    //plotFrame = getLocalBounds().reduced(3, 3);
    
    auto bounds = getLocalBounds();

    int y = 10;
    int height = bounds.getHeight() - 20;

    // Position the groups
    horizontalGroup.setBounds(10, y, 110, height);
    verticalGroup.setBounds(bounds.getWidth() - 160, y, 150, height);

    // Position the knobs inside the groups
    horizontalScaleKnob.setTopLeftPosition(20, 20);
    horizontalPositionKnob.setTopLeftPosition(horizontalScaleKnob.getX(), horizontalScaleKnob.getBottom() + 10);
}

void OscilloscopeAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.checkForNewAnalyserData())
    {
        repaint(plotFrame);
    }
}

float OscilloscopeAudioProcessorEditor::getFrequencyForPosition(float pos)
{
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}