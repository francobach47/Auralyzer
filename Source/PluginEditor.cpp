#include "PluginProcessor.h"
#include "PluginEditor.h"

static float maxDB = 24.0f;

//==============================================================================
OscilloscopeAudioProcessorEditor::OscilloscopeAudioProcessorEditor(OscilloscopeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), timeVisualizer(p), frequencyVisualizer(p)
{
    tooltipWindow->setMillisecondsBeforeTipAppears(1000);
 
    audioProcessor.apvts.addParameterListener(rangeParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(modeParamID.getParamID(), this);
    
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

    serialPortLabel.attachToComponent(&serialPortSelector, true);
    addAndMakeVisible(serialPortSelector);

    serialPortSelector.onChange = [this]()
        {
            auto selectedText = serialPortSelector.getText();
            if (selectedText.contains("("))
                selectedText = selectedText.upToFirstOccurrenceOf("(", false, false).trim();  // obtener solo COMx

            if (!selectedText.isEmpty())
            {
                juce::Logger::writeToLog("[SerialCombo] Seleccionado: " + selectedText);

                auto& device = audioProcessor.getSerialDevice();
                startTimer(100);              // 100 ms para darle tiempo a mandar la data a la ESP

                device.close();
                device.init(selectedText);  //  esto asigna serialPortName dentro de SerialDevice
                
                // Callback de controlModeStatus (GIOP23)
                device.onControlStatusReceived = [this](bool pluginControls)
                    {
                        juce::MessageManager::callAsync([this, pluginControls]
                            {
                                bloquearControles(pluginControls);
                            });
                    };

                // Callback de syncKnobs
                device.onSyncKnobsReceived = [this](uint8_t modo, uint8_t rango)
                    {
                        if (!pluginIsInControl)
                        {
                            juce::MessageManager::callAsync([this, modo, rango]
                                {
                                    actualizarKnobsDesdeESP(modo, rango);
                                });
                        }
                    };

                // Abrimos el puerto              
                device.open();
            }
        };

    auto portList = SerialPort::getSerialPortPaths();
    for (int i = 0; i < portList.size(); ++i)
    {
        const auto path = portList.getAllKeys()[i];
        const auto desc = portList.getAllValues()[i];

        serialPortSelector.addItem(path + " (" + desc + ")", i + 1);
    }

    audioProcessor.getSerialDevice().serialPortListMonitor.onPortListChanged = [this](const juce::StringPairArray& portList)
        {
            juce::MessageManager::callAsync([this, portList]() {
                auto selectedText = serialPortSelector.getText();
                serialPortSelector.clear(true);  // limpiar

                for (int i = 0; i < portList.size(); ++i)
                {
                    auto path = portList.getAllKeys()[i];
                    auto desc = portList.getAllValues()[i];
                    serialPortSelector.addItem(path + " (" + desc + ")", i + 1);
                }

                // restaurar selección anterior si sigue existiendo
                for (int i = 0; i < serialPortSelector.getNumItems(); ++i)
                {
                    if (serialPortSelector.getItemText(i).startsWith(selectedText))
                    {
                        serialPortSelector.setSelectedItemIndex(i);
                        break;
                    }
                }
                });
        };

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

    calibrationButton.setButtonText("Calibration");
    calibrationButton.setClickingTogglesState(true); // importante para modo toggle
    calibrationButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::transparentBlack); // sin check
    calibrationButton.setColour(juce::ToggleButton::ColourIds::textColourId, juce::Colours::white);
    calibrationButton.setLookAndFeel(ButtonLookAndFeel::get()); // opcional: si tenés un estilo
    addAndMakeVisible(calibrationButton);
    calibrationButton.setClickingTogglesState(true);
    calibrationButton.onClick = [this]()
        {
            bool isOn = calibrationButton.getToggleState();
            audioProcessor.getSerialDevice().setCalibrationMode(isOn ? 1 : 0);
        };

    addAndMakeVisible(calibrationButton);

    setLookAndFeel(&mainLF);
    
    setSize(1200, 490);

#ifdef JUCE_OPENGL
        openGLContext.attachTo(*getTopLevelComponent());
#endif     
}

OscilloscopeAudioProcessorEditor::~OscilloscopeAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    audioProcessor.apvts.removeParameterListener(rangeParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(modeParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(plotModeParamID.getParamID(), this);

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

    //Position the COM Port List
    serialPortSelector.setBounds(150, 453, 200, 24);
    
    //Calibration
    calibrationButton.setBounds(400, 453, 100, 30);

}

void OscilloscopeAudioProcessorEditor::parameterChanged(const juce::String& paramID, float newValue)
{
    if (paramID == plotModeParamID.getParamID())
    {
        bool isFrequencyMode = newValue > 0.5f; // 0=Time, 1=Frequency
        timeVisualizer.setVisible(!isFrequencyMode);
        frequencyVisualizer.setVisible(isFrequencyMode);
        plotModeButton.setButtonText(isFrequencyMode ? "Frequency" : "Time");
    }
    
    if (!pluginIsInControl)
        return;

    juce::MessageManager::callAsync([this, paramID, newValue]
        {
            if (paramID == modeParamID.getParamID())
            {
                audioProcessor.getSerialDevice().setMode(newValue < 0.5f ? 0 : 1);
            }
            else if (paramID == rangeParamID.getParamID())
            {
                audioProcessor.getSerialDevice().setRange(static_cast<uint8_t>(juce::roundToInt(newValue)));
            }
        });
}

void OscilloscopeAudioProcessorEditor::timerCallback()
{
    stopTimer();

    auto* modeParam = audioProcessor.apvts.getRawParameterValue(modeParamID.getParamID());
    auto* rangeParam = audioProcessor.apvts.getRawParameterValue(rangeParamID.getParamID());

    if (modeParam != nullptr)
        parameterChanged(modeParamID.getParamID(), modeParam->load());

    if (rangeParam != nullptr)
        parameterChanged(rangeParamID.getParamID(), rangeParam->load());
}

void OscilloscopeAudioProcessorEditor::actualizarKnobsDesdeESP(uint8_t modo, uint8_t rango)
{
    auto* modoParam = audioProcessor.apvts.getParameter(modeParamID.getParamID());
    auto* rangoParam = audioProcessor.apvts.getParameter(rangeParamID.getParamID());

    if (modoParam != nullptr)
        modoParam->setValueNotifyingHost(static_cast<float>(modo));

    if (rangoParam != nullptr)
        rangoParam->setValueNotifyingHost(static_cast<float>(rango) / 3.0f);
}

void OscilloscopeAudioProcessorEditor::bloquearControles(bool pluginControls)
{
    pluginIsInControl = pluginControls;

    // Solo se habilitan si el plugin está en control
    modeKnob.setEnabled(pluginIsInControl);
    rangeKnob.setEnabled(pluginIsInControl);

    // LED testigo inverso
    audioProcessor.getSerialDevice().setLightColor(pluginIsInControl ? 0x0000 : 0xFFFF);
}
