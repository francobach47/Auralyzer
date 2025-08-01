#pragma once

#include <JuceHeader.h>

namespace Colors
{
	const juce::Colour background{ 245, 240, 235 };
	const juce::Colour header{ 40, 40, 40 };

	namespace Knob
	{
		const juce::Colour trackBackground{ 205, 200, 195 };
		const juce::Colour trackActive{ 73, 125, 0 };
		const juce::Colour outline{ 255, 250, 245 };
		const juce::Colour gradientTop{ 250, 245, 240 };
		const juce::Colour gradientBottom{ 240, 235, 230 };
		const juce::Colour dial{ 100, 100, 100 };
		const juce::Colour dropShadow{ 195, 190, 185 };
		const juce::Colour label{ 80, 80, 80 };
		const juce::Colour textBoxBackground{ 80, 80, 80 };
		const juce::Colour value{ 240, 240, 240 };
		const juce::Colour caret{ 255, 255, 255 };
	}

	namespace Group
	{
		const juce::Colour label{ 160, 155, 150 };
		const juce::Colour outline{ 220, 216, 211 };
	}

	namespace Button
	{
		const juce::Colour textToggled{ 40,40,40 };
		const juce::Colour backgroundToggled{ 255, 250, 245 };
		const juce::Colour outline{ 220, 216, 211 };
	}

	namespace PlotSection
	{
		const juce::Colour frequencyResponse { 124, 207, 0 };
		const juce::Colour timeResponse{ 124, 207, 0 };
		const juce::Colour background{ 66, 66, 66 };
		const juce::Colour outline{ 220, 216, 211 };
		const juce::Colour triggerMarker{ 255, 255, 255 }; 
		const juce::Colour dcResponse{ 124, 207, 0 };
	}
}

class Fonts
{
public:
	Fonts() = delete;

	static juce::Font getFont(float height = 16.0f);

private:
	static const juce::Typeface::Ptr typeface;
};

class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
	RotaryKnobLookAndFeel();

	static RotaryKnobLookAndFeel* get()
	{
		static RotaryKnobLookAndFeel instance;
		return &instance;
	}

	void drawRotarySlider(
		juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float rotaryStartAngle,
		float rotaryEndAngle, juce::Slider& slider) override;

	juce::Font getLabelFont(juce::Label&) override;

	juce::Label* createSliderTextBox(juce::Slider&) override;

	void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override { }
	void fillTextEditorBackground(juce::Graphics&, int width, int height, juce::TextEditor&) override;

private:
	juce::DropShadow dropShadow{ Colors::Knob::dropShadow, 6, { 0, 3} };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobLookAndFeel)
};

class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
	MainLookAndFeel();

	juce::Font getLabelFont(juce::Label&) override;

	void drawComboBox(juce::Graphics& g, int width, int height,
		bool isButtonDown, int buttonX, int buttonY,
		int buttonW, int buttonH, juce::ComboBox& box) override;

	void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
		bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
		const juce::String& text, const juce::String& shortcutKeyText,
		const juce::Drawable* icon, const juce::Colour* textColour) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};

class ButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ButtonLookAndFeel();

	static ButtonLookAndFeel* get()
	{
		static ButtonLookAndFeel instance;
		return &instance;
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button,
		const juce::Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override;

	void drawButtonText(juce::Graphics& g, juce::TextButton& button,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};