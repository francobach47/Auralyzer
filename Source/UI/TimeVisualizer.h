#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class TimeVisualizer : public juce::Component,
    public juce::Timer
{
public:
    TimeVisualizer(OscilloscopeAudioProcessor& processor);
    ~TimeVisualizer() override;

    void paint(juce::Graphics&) override;

    void timerCallback() override;

    void setVerticalGain(float gain) { verticalGain = gain; }
    void setVerticalOffset(float offset) { verticalOffset = offset; }
    void setHorizontalScale(float scale) { horizontalScale = scale; }
    void setHorizontalOffset(float offset) { horizontalOffset = offset; }

    void setTriggerLevel(float level) { triggerLevel = level; }
    void setTriggerOffset(float offset) { triggerOffset = offset; }


private:
    OscilloscopeAudioProcessor& processor;

    float verticalGain = 1.0f;
    float verticalOffset = 0.0f;
    float horizontalScale = 1.0f;
    float horizontalOffset = 0.0f;

    float triggerLevel = 0.0f;     // Nivel para detectar flanco ascendente
    float triggerOffset = 0.0f;    // Desplazamiento horizontal en tiempo (0..1 relativo al buffer)


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeVisualizer)
};

