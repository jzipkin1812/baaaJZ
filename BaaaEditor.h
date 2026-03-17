#pragma once

#include "BaaaProcessor.h"

static void outlinedText(juce::Graphics& g, juce::GlyphArrangement glyphs, juce::Colour outline, juce::Colour inner, juce::PathStrokeType stroke) {
    juce::Path textPath;
    glyphs.createPath(textPath);

    // Draw outline
    g.setColour(outline);
    g.strokePath(textPath, stroke);

    // Fill inner
    g.setColour(inner);
    g.fillPath(textPath);
}

// The following class was written primarily using Chat GPT 5.

// Prompt for outlined text:
// - Can we make all the text in the editor high-contrast by giving it two different colors - an inner color (dark) and a border color (light)? Then it can be easily seen always.
// - For the incrementer buttons, can I have them be smaller, and can I also leave room to replace the buttons with my own custom images?

// Prompt for sheep code:
// Assume you have access to a juce::Image called sheepImage. Adjust the rotary slider so that, in the direct center of the slider component, we draw the sheep image, but rotate it according to the rotary slider's angle, so the sheep rotates as the user shifts the parameter. Make sure to scale the image appropriately based on the size of the component, which changes as the window is resized. The image itself is square.

// Prompt for special slider overlay class:
// How can we update this to draw the text of the value of the button parameter centered (will show up between the two + and - buttons)? Or should it be a different function
class BaaaLookAndFeel : public juce::LookAndFeel_V4
{
    public:
    juce::Colour outlineColour = juce::Colours::lightcyan;
    juce::Colour innerColour = juce::Colours::darkgreen;
    juce::PathStrokeType stroke = juce::PathStrokeType(3.0f);
    juce::Image sheepImage = juce::ImageCache::getFromMemory(BinaryData::sheep_png, BinaryData::sheep_pngSize);

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        auto text = label.getText();
        auto bounds = label.getLocalBounds().toFloat();

        juce::Font font = label.getFont();

        juce::GlyphArrangement glyphs;
        glyphs.addFittedText(font, text,
                             bounds.getX(), bounds.getY(),
                             bounds.getWidth(), bounds.getHeight(),
                             label.getJustificationType(),
                             1);

        outlinedText(g, glyphs, outlineColour, innerColour, stroke);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced(10.0f).withTrimmedLeft(40.0f).withTrimmedRight(40.0f);

        float cornerSize = 4.0f;

        juce::Colour base = juce::Colours::lemonchiffon;

        if (isButtonDown)
            base = base.darker(0.3f);
        else if (isMouseOverButton)
            base = base.brighter(0.2f);

        g.setColour(base);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(juce::Colours::saddlebrown);
        g.drawRoundedRectangle(bounds, cornerSize, 2.0f);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(25.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // ==========================================
        // Sheep Image (rotates with knob)
        // ==========================================

        if (!sheepImage.isNull())
        {
            // Scale image relative to knob size
            // Adjust multiplier to taste (0.6–0.8 works well)
            float imageSize = radius * 1.4f;   // diameter-ish scale

            float imgW = imageSize;
            float imgH = imageSize;

            // Create transform:
            juce::AffineTransform transform;

            transform = transform
                .translated(-sheepImage.getWidth() * 0.5f,
                            -sheepImage.getHeight() * 0.5f) // move image center to origin
                .scaled(imgW / sheepImage.getWidth(),
                        imgH / sheepImage.getHeight())     // scale to component size
                .rotated(angle)                            // rotate around origin
                .translated(centre.x, centre.y);           // move to knob center

            g.drawImageTransformed(sheepImage, transform);
        }

        // Indicator Arc
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y,
                            radius, radius,
                            0.0f,
                            rotaryStartAngle,
                            rotaryEndAngle,
                            true);

        g.setColour(juce::Colour::fromRGB(200, 255, 100));
        g.strokePath(valueArc, juce::PathStrokeType(5.0));

        // Indicator Dot
        auto dotRadius = 8.0f;
        auto dotPoint = centre.getPointOnCircumference(radius, angle);

        g.setColour(juce::Colour::fromRGB(255, 255, 255)); // dot color
        g.fillEllipse(dotPoint.x - dotRadius,
                    dotPoint.y - dotRadius,
                    dotRadius * 2.0f,
                    dotRadius * 2.0f);

        // 4. Value Text
        juce::String valueText = slider.getTextFromValue(slider.getValue());
        juce::Font font(juce::FontOptions("Comic Sans MS", 30.0f, juce::Font::plain));

        // Create glyph layout
        juce::GlyphArrangement glyphs;
        glyphs.addFittedText(font,
                            valueText,
                            bounds.getX(),
                            bounds.getY() - 20,
                            bounds.getWidth(),
                            bounds.getHeight(),
                            juce::Justification::centredTop,
                            1);

        outlinedText(g, glyphs, outlineColour, innerColour, stroke);
    }

    void drawButtonText (juce::Graphics& g,
                    juce::TextButton& button,
                    bool /*isMouseOverButton*/,
                    bool /*isButtonDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        juce::String text = button.getButtonText(); // "+" or "-"

        juce::Font font(juce::FontOptions("Comic Sans MS", 20.0f, juce::Font::plain));

        juce::GlyphArrangement glyphs;
        glyphs.addFittedText(font,
                            text,
                            bounds.getX(),
                            bounds.getY(),
                            bounds.getWidth(),
                            bounds.getHeight(),
                            juce::Justification::centred,
                            1);

        outlinedText(g, glyphs, juce::Colours::saddlebrown, juce::Colours::lemonchiffon, stroke);
    }
};


class SliderValueOverlay : public juce::Component
{
public:
    SliderValueOverlay(juce::Slider& s) : slider(s)
    {
        slider.onValueChange = [this] { repaint(); };
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::String text =
            slider.getTextFromValue(slider.getValue());

        juce::Font font(juce::FontOptions("Comic Sans MS", 40.0f, juce::Font::plain));

        juce::GlyphArrangement glyphs;
        glyphs.addFittedText(font,
                             text,
                             bounds.getX(),
                             bounds.getY(),
                             bounds.getWidth(),
                             bounds.getHeight(),
                             juce::Justification::centred,
                             1);
        outlinedText(g, glyphs, juce::Colours::saddlebrown, juce::Colours::lemonchiffon, juce::PathStrokeType(3.0f));
    }

private:
    juce::Slider& slider;
};

//==============================================================================
class BaaaPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BaaaPluginAudioProcessorEditor (BaaaPluginAudioProcessor&);
    ~BaaaPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Slider shiftSlider;
    juce::Slider gainSlider;
    juce::Slider upDupeSlider;
    juce::Slider downDupeSlider;
    juce::Slider centerSlider;
    juce::Slider falloffSlider;


    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shiftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upDupeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> downDupeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> centerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> falloffAttachment;

    juce::Label shiftLabel;
    juce::Label gainLabel;
    juce::Label upDupeLabel;
    juce::Label downDupeLabel;
    juce::Label centerLabel;
    juce::Label falloffLabel;

    juce::Image backgroundImage;
    juce::Image sheepImage;
    BaaaLookAndFeel customLook;

    std::unique_ptr<SliderValueOverlay> upOverlay;
    std::unique_ptr<SliderValueOverlay> downOverlay;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BaaaPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaaaPluginAudioProcessorEditor)

    void stylizeSlider(juce::Label &l, juce::Slider &s, juce::String str);
    void stylizeStepper(juce::Label &l, juce::Slider &s, juce::String str);


    const juce::Colour outlineColour  = juce::Colours::darkgreen;
    const juce::Colour innerColour = juce::Colours::lightcyan;
    const float textSizeLarge = 30.0f;
    const float textSizeSmall = 20.0f;
    
    const juce::Font fontLarge = juce::Font(juce::FontOptions("Comic Sans MS", 35.0f, juce::Font::plain));
    const juce::Font fontSmall = juce::Font(juce::FontOptions("Comic Sans MS", 25.0f, juce::Font::plain));

};

