#include "LookAndFeel.h"

const juce::Typeface::Ptr Fonts::typeface = juce::Typeface::createSystemTypefaceFor(
	BinaryData::LatoMedium_ttf, BinaryData::LatoMedium_ttfSize);

juce::Font Fonts::getFont(float height)
{
	return juce::FontOptions(typeface).withMetricsKind(juce::TypefaceMetricsKind::legacy).withHeight(height);
}

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel()
{
	setColour(juce::Label::textColourId, Colors::Knob::label);
	setColour(juce::Slider::textBoxTextColourId, Colors::Knob::label);
	setColour(juce::Slider::rotarySliderFillColourId, Colors::Knob::trackActive);
	setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	setColour(juce::CaretComponent::caretColourId, Colors::Knob::caret);
}

void RotaryKnobLookAndFeel::drawRotarySlider(
	juce::Graphics& g,
	int x, int y, int width, [[maybe_unused]] int height,
	float sliderPos,
	float rotaryStartAngle, float rotaryEndAngle,
	juce::Slider& slider)
{
	auto bounds = juce::Rectangle<int>(x, y, width, width).toFloat();
	auto knobRect = bounds.reduced(10.0f, 10.0f);

	auto path = juce::Path();
	path.addEllipse(knobRect);
	dropShadow.drawForPath(g, path);

	g.setColour(Colors::Knob::outline);
	g.fillEllipse(knobRect);

	auto innerRect = knobRect.reduced(2.0f, 2.0f);
	auto gradient = juce::ColourGradient(
		Colors::Knob::gradientTop, 0.0f, innerRect.getY(),
		Colors::Knob::gradientBottom, 0.0f, innerRect.getBottom(), false);
	g.setGradientFill(gradient);
	g.fillEllipse(innerRect);

	auto center = bounds.getCentre();
	auto radius = bounds.getWidth() / 2.0f;
	auto lineWidth = 3.0f;
	auto arcRadius = radius - lineWidth / 2.0f;

	// Track
	juce::Path backgroundArc;
	backgroundArc.addCentredArc(center.x,
		center.y,
		arcRadius,
		arcRadius,
		0.0f,
		rotaryStartAngle,
		rotaryEndAngle,
		true);

	auto strokeType = juce::PathStrokeType(
		lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
	g.setColour(Colors::Knob::trackBackground);
	g.strokePath(backgroundArc, strokeType);

	// Dial
	auto dialRadius = innerRect.getHeight() / 2.0f - lineWidth;
	auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

	juce::Point<float> dialStart(center.x + 10.0f * std::sin(toAngle),
		center.y - 10.0f * std::cos(toAngle));
	juce::Point<float> dialEnd(center.x + dialRadius * std::sin(toAngle),
		center.y - dialRadius * std::cos(toAngle));

	juce::Path dialPath;
	dialPath.startNewSubPath(dialStart);
	dialPath.lineTo(dialEnd);
	g.setColour(Colors::Knob::dial);
	g.strokePath(dialPath, strokeType);

	// Coloring the track
	if (slider.isEnabled()) {
		float fromAngle = rotaryStartAngle;
		if (slider.getProperties()["drawFromMiddle"]) {
			fromAngle += (rotaryEndAngle - rotaryStartAngle) / 2.0f;
		}

		juce::Path valueArc;
		valueArc.addCentredArc(center.x,
			center.y,
			arcRadius,
			arcRadius,
			0.0f,
			fromAngle,
			toAngle,
			true);

		g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
		g.strokePath(valueArc, strokeType);
	}
}

juce::Font RotaryKnobLookAndFeel::getLabelFont([[maybe_unused]] juce::Label& label)
{
	return Fonts::getFont();
}

MainLookAndFeel::MainLookAndFeel()
{
	setColour(juce::GroupComponent::textColourId, Colors::Group::label);
	setColour(juce::GroupComponent::outlineColourId, Colors::Group::outline);
	setColour(juce::ComboBox::textColourId, juce::Colours::black);

}

juce::Font MainLookAndFeel::getLabelFont([[maybe_unused]] juce::Label& label)
{
	return Fonts::getFont();
}

class RotaryKnobLabel : public juce::Label
{
public:
	RotaryKnobLabel() : juce::Label() {}

	void mouseWheelMove(const juce::MouseEvent&,
		const juce::MouseWheelDetails&) override {}

	std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
	{
		return createIgnoredAccessibilityHandler(*this);
	}

	juce::TextEditor* createEditorComponent() override
	{
		auto* ed = new juce::TextEditor(getName());
		ed->applyFontToAllText(getLookAndFeel().getLabelFont(*this));
		copyAllExplicitColoursTo(*ed);

		ed->setBorder(juce::BorderSize<int>());
		ed->setIndents(2, 1);
		ed->setJustification(juce::Justification::centredTop);
		ed->setPopupMenuEnabled(false);
		ed->setInputRestrictions(8);
		return ed;
	}
};

juce::Label* RotaryKnobLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
	auto l = new RotaryKnobLabel();
	l->setJustificationType(juce::Justification::centred);
	l->setKeyboardType(juce::TextInputTarget::decimalKeyboard);
	l->setColour(juce::Label::textColourId,
		slider.findColour(juce::Slider::textBoxTextColourId));
	l->setColour(juce::TextEditor::textColourId, Colors::Knob::value);
	l->setColour(juce::TextEditor::highlightedTextColourId, Colors::Knob::value);
	l->setColour(juce::TextEditor::highlightColourId,
		slider.findColour(juce::Slider::rotarySliderFillColourId));
	l->setColour(juce::TextEditor::backgroundColourId,
		Colors::Knob::textBoxBackground);
	return l;
}

void RotaryKnobLookAndFeel::fillTextEditorBackground(
	juce::Graphics& g,
	[[maybe_unused]] int width, [[maybe_unused]] int height,
	juce::TextEditor& textEditor)
{
	g.setColour(Colors::Knob::textBoxBackground);
	g.fillRoundedRectangle(textEditor.getLocalBounds().reduced(4, 0).toFloat(), 4.0f);
}

ButtonLookAndFeel::ButtonLookAndFeel()
{
	setColour(juce::TextButton::textColourOffId, Colors::Button::textToggled);
	setColour(juce::TextButton::textColourOnId, Colors::Button::textToggled);
	setColour(juce::TextButton::buttonColourId, Colors::Button::backgroundToggled);
	setColour(juce::TextButton::buttonOnColourId, Colors::Button::backgroundToggled);
}

void ButtonLookAndFeel::drawButtonBackground(
	juce::Graphics& g,
	juce::Button& button,
	const juce::Colour& backgroundColour,
	[[maybe_unused]] bool shouldDrawButtonAsHighlighted,
	[[maybe_unused]] bool shouldDrawButtonAsDown)
{
	auto bounds = button.getLocalBounds().toFloat();
	auto cornerSize = bounds.getHeight() * 0.25f;
	auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);

	if (button.isDown())
		buttonRect.translate(0.0f, 1.0f);

	// Color de fondo según estado
	juce::Colour fill = button.getToggleState()
		? juce::Colour::fromRGB(247, 254, 231)
		: backgroundColour;                   // Color original si no lo está

	g.setColour(fill);
	g.fillRoundedRectangle(buttonRect, cornerSize);

	// Contorno del botón
	g.setColour(Colors::Button::outline);
	g.drawRoundedRectangle(buttonRect, cornerSize, 2.0f);

	if (button.getName() == "Sine")
	{
		juce::Path sinePath;
		auto bounds = button.getLocalBounds().toFloat().reduced(6.0f);

		float periodWidth = bounds.getWidth() * 0.6f; // sólo el 60% del ancho
		float xStart = bounds.getCentreX() - periodWidth / 2.0f;
		float yMid = bounds.getCentreY();
		float amplitude = bounds.getHeight() * 0.4f;
		int steps = 50;

		for (int i = 0; i <= steps; ++i)
		{
			float x = xStart + (i / (float)steps) * periodWidth;
			float y = yMid - std::sin(juce::MathConstants<float>::twoPi * i / steps) * amplitude;

			if (i == 0)
				sinePath.startNewSubPath(x, y);
			else
				sinePath.lineTo(x, y);
		}

		g.setColour(Colors::Button::textToggled);
		g.strokePath(sinePath, juce::PathStrokeType(1.5f));
	}

	if (button.getName() == "Probes")
	{
		juce::Path squarePath;
		auto bounds = button.getLocalBounds().toFloat().reduced(6.0f);

		float periodWidth = bounds.getWidth() * 0.6f;
		float xStart = bounds.getCentreX() - periodWidth / 2.0f;
		float yMid = bounds.getCentreY();
		float amp = bounds.getHeight() * 0.4f;
		float topY = yMid - amp;
		float botY = yMid + amp;

		float step = periodWidth / 2.0f; 

		squarePath.startNewSubPath(xStart, topY);              
		squarePath.lineTo(xStart + step, topY);                      
		squarePath.lineTo(xStart + step, botY);                      
		squarePath.lineTo(xStart + 2 * step, botY);                 

		g.setColour(Colors::Button::textToggled);
		g.strokePath(squarePath, juce::PathStrokeType(1.5f));
	}
}

void ButtonLookAndFeel::drawButtonText(
	juce::Graphics& g,
	juce::TextButton& button,
	[[maybe_unused]] bool shouldDrawButtonAsHighlighted,
	bool shouldDrawButtonAsDown)
{
	auto bounds = button.getLocalBounds().toFloat();
	auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);

	if (shouldDrawButtonAsDown) {
		buttonRect.translate(0.0f, 1.0f);
	}

	if (button.getToggleState()) {
		g.setColour(button.findColour(juce::TextButton::textColourOnId));
	}
	else {
		g.setColour(button.findColour(juce::TextButton::textColourOffId));
	}

	g.setFont(Fonts::getFont());
	
	const int yIndent = button.proportionOfHeight(0.1f);
	const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

	const int leftIndent = cornerSize / (button.isConnectedOnLeft() ?
		yIndent * 2 : yIndent);
	const int rightIndent = cornerSize / (button.isConnectedOnRight() ?
		yIndent * 2 : yIndent);
	const int textWidth = button.getWidth() - leftIndent - rightIndent;

	if (textWidth > 0)
		g.drawFittedText(button.getButtonText(),
			leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
			juce::Justification::centred, 2);
}

void MainLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
	bool isButtonDown, int /*buttonX*/, int /*buttonY*/,
	int /*buttonW*/, int /*buttonH*/, juce::ComboBox& box)
{
	auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(1.0f);
	float cornerSize = bounds.getHeight() * 0.25f;

	// Fondo 
	g.setColour(juce::Colour::fromRGB(255, 250, 245)); 
	g.fillRoundedRectangle(bounds, cornerSize);

	// Contorno
	g.setColour(Colors::Button::outline);
	g.drawRoundedRectangle(bounds, cornerSize, 1.5f);

	// Flecha
	g.setColour(juce::Colours::black);
	juce::Path arrow;
	float arrowSize = 6.0f;
	float cx = width - 12.0f, cy = height / 2.0f;

	arrow.addTriangle(cx - arrowSize / 2, cy - arrowSize / 4,
		cx + arrowSize / 2, cy - arrowSize / 4,
		cx, cy + arrowSize / 3);

	g.fillPath(arrow);
}

void MainLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
	bool isSeparator, bool isActive, bool isHighlighted,
	bool isTicked, bool hasSubMenu, const juce::String& text,
	const juce::String& shortcutKeyText, const juce::Drawable* icon,
	const juce::Colour* textColour)
{
	if (isSeparator)
	{
		g.setColour(juce::Colours::grey);
		g.drawLine(area.getX() + 5.0f, area.getCentreY(), area.getRight() - 5.0f, area.getCentreY());
		return;
	}

	// Fondo del item
	if (isHighlighted)
		g.setColour(juce::Colour(247, 254, 231)); // fondo seleccionado
	else
		g.setColour(juce::Colour(25, 46, 3));    // fondo normal

	g.fillRect(area);

	// Color del texto
	juce::Colour textCol = textColour != nullptr ? *textColour
		: (isHighlighted ? juce::Colours::black : juce::Colours::lightgrey);

	g.setColour(textCol);
	g.setFont(Fonts::getFont(14.0f));
	g.drawText(text, area.reduced(8, 0), juce::Justification::centredLeft, true);
}
