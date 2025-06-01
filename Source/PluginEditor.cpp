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
    audioProcessor.apvts.addParameterListener(modeParamID.getParamID(), this);

    audioProcessor.apvts.addParameterListener(verticalScaleParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(verticalPositionParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(horizontalScaleParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(horizontalPositionParamID.getParamID(), this);
    audioProcessor.apvts.addParameterListener(triggerLevelParamID.getParamID(), this);

    isFrequencyMode = audioProcessor.apvts.getRawParameterValue(plotModeParamID.getParamID())->load() > 0.5f;
    plotModeButton.setButtonText(isFrequencyMode ? "Frequency" : "Time");

    plotGroup.addChildComponent(timeVisualizer);
    plotGroup.addChildComponent(frequencyVisualizer);
    timeVisualizer.setVisible(!isFrequencyMode);
    frequencyVisualizer.setVisible(isFrequencyMode);
    addAndMakeVisible(plotGroup);

    // Inicializar modo DC si corresponde
    bool initialDC = audioProcessor.params.modeValue == 1;
    timeVisualizer.setModeDC(initialDC);

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
    calibrationButton.setClickingTogglesState(true);
    calibrationButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::transparentBlack);
    calibrationButton.setColour(juce::ToggleButton::ColourIds::textColourId, juce::Colours::white);
    calibrationButton.setLookAndFeel(ButtonLookAndFeel::get());
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

    triggerGroup.setText("Trigger");
    triggerGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    triggerGroup.addAndMakeVisible(triggerLevelKnob);
    movingAverageButton.setButtonText("Moving Average");
    movingAverageButton.changeWidthToFitText();
    movingAverageButton.setClickingTogglesState(true);
    movingAverageButton.setBounds(0, 0, 70, 50);
    movingAverageButton.setLookAndFeel(ButtonLookAndFeel::get());
    triggerGroup.addAndMakeVisible(movingAverageButton);
    addAndMakeVisible(triggerGroup);

    // Forzar valores iniciales para visualizacion
    float hScale = std::pow(2.0f, *audioProcessor.apvts.getRawParameterValue(horizontalScaleParamID.getParamID()));
    float hOffset = *audioProcessor.apvts.getRawParameterValue(horizontalPositionParamID.getParamID());
    float vScale = std::pow(2.0f, *audioProcessor.apvts.getRawParameterValue(verticalScaleParamID.getParamID()));
    float vOffset = *audioProcessor.apvts.getRawParameterValue(verticalPositionParamID.getParamID());

    timeVisualizer.setHorizontalScale(hScale);
    timeVisualizer.setHorizontalOffset(hOffset);
    timeVisualizer.setVerticalGain(vScale);
    timeVisualizer.setVerticalOffset(vOffset);

    setLookAndFeel(&mainLF);

    setSize(1325, 500);

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
    audioProcessor.apvts.removeParameterListener(triggerLevelParamID.getParamID(), this);

    setLookAndFeel(nullptr);
    audioProcessor.apvts.removeParameterListener(rangeParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(modeParamID.getParamID(), this);
    audioProcessor.apvts.removeParameterListener(plotModeParamID.getParamID(), this);

#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void OscilloscopeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(Colors::background);

    auto image_logo = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    int destX = 1165;
    int destY = 445;
    int destWidth = image_logo.getWidth();
    int destHeight = image_logo.getHeight();

    g.drawImage(image_logo,
        destX, destY, destWidth, destHeight,
        0, 0, image_logo.getWidth(), image_logo.getHeight());
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
    
    triggerLevelKnob.setTopLeftPosition(20, 20);
    movingAverageButton.setTopLeftPosition(20, 140);

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

void OscilloscopeAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == plotModeParamID.getParamID())
    {
        bool isFrequencyMode = newValue > 0.5f; // 0=Time, 1=Frequency
        timeVisualizer.setVisible(!isFrequencyMode);
        frequencyVisualizer.setVisible(isFrequencyMode);
        plotModeButton.setButtonText(isFrequencyMode ? "Frequency" : "Time");
    }

    // Esto debe ejecutarse SIEMPRE
    if (parameterID == modeParamID.getParamID())
    {
        bool isDC = std::round(newValue) == 1; // 0=AC, 1=DC
        DBG("→ modo cambiado: esModoDC = " << (isDC ? "true" : "false"));
        timeVisualizer.setModeDC(isDC);
    }

    if (parameterID == verticalScaleParamID.getParamID())
    {
        float verticalGain = std::pow(2.0f, newValue);
        timeVisualizer.setVerticalGain(verticalGain);
    }

    if (parameterID == verticalPositionParamID.getParamID())
    {
        float verticalOffset = newValue * (timeVisualizer.getHeight() / 2);
        timeVisualizer.setVerticalOffset(verticalOffset);
    }

    if (parameterID == horizontalScaleParamID.getParamID())
    {
        float horizontalGain = std::pow(2.0f, newValue);
        timeVisualizer.setHorizontalScale(horizontalGain);
    }

    if (parameterID == horizontalPositionParamID.getParamID())
    {
        float horizontalOffset = newValue * (timeVisualizer.getHeight() / 2);
        timeVisualizer.setHorizontalOffset(newValue);
    }

    DBG("parameterChanged: " << parameterID << " = " << newValue);

    //SOLO ESTO depende del control por el plugin
    if (!pluginIsInControl)
        return;

    juce::MessageManager::callAsync([this, parameterID, newValue]
        {
            if (parameterID == modeParamID.getParamID())
            {
                audioProcessor.getSerialDevice().setMode(newValue < 0.5f ? 0 : 1);
            }
            else if (parameterID == rangeParamID.getParamID())
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
    pluginIsInControl = false;
    bloquearControles(false); // deja los knobs bloqueados visualmente

    // MODO
    if (auto* modeParam = audioProcessor.apvts.getParameter(modeParamID.getParamID()))
    {
        modeParam->beginChangeGesture();
        modeParam->setValueNotifyingHost(modeParam->convertTo0to1((float)modo));
        modeParam->endChangeGesture();
    }

    // RANGO
    if (auto* rangeParam = audioProcessor.apvts.getParameter(rangeParamID.getParamID()))
    {
        rangeParam->beginChangeGesture();
        rangeParam->setValueNotifyingHost(rangeParam->convertTo0to1((float)rango));
        rangeParam->endChangeGesture();
    }
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