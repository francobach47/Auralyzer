#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../DSP/Trigger.h"
#include "LookAndFeel.h"

class TimeVisualizer : public juce::Component,
                       public juce::Timer
{
public:
    TimeVisualizer(OscilloscopeAudioProcessor& processor);
    ~TimeVisualizer() override;

    void paint(juce::Graphics&) override;
    void timerCallback() override;

    // Visualization parameters
    void setVerticalGain(float gain) { verticalGain = gain; }
    void setVerticalOffset(float offset) { verticalOffset = offset; }
    void setHorizontalScale(float scale) { horizontalScale = scale; }
    void setHorizontalOffset(float offset) { horizontalOffset = offset; }
    void setVerticalOffsetInDivisions(float divisions);  // nueva función

   void updateTriggerParameters(float level, float offset, bool filterEnabled);
   float currentTriggerLevel = 0.0f;   // -1 … +1

   //DC Mode
   void setModeDC(bool enabled);

   //gridPlot
   void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);
   
   //Para visualizar en tiempo real el valor de tension
   float lastVpp = -1.0f;
   int frameCounter = 0;

private:
    OscilloscopeAudioProcessor& processor;
    Trigger trigger;

    float verticalGain = 1.0f;
    float verticalOffset = 0.0f;

    float horizontalScale = 1.0f;
    float horizontalOffset = 0.0f;
    static constexpr int numVerticalDivisions = 8;

    bool modeDC = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeVisualizer)
};

