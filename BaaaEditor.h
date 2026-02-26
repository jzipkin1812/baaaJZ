#pragma once

#include "BaaaProcessor.h"

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

        juce::Path textPath;
        glyphs.createPath(textPath);

        // Draw outline
        g.setColour(outlineColour);
        g.strokePath(textPath, stroke);

        // Fill inner
        g.setColour(innerColour);
        g.fillPath(textPath);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
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

            // Top-left so image is centered
            float imgX = centre.x - imgW * 0.5f;
            float imgY = centre.y - imgH * 0.5f;

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

        // Convert to path
        juce::Path textPath;
        glyphs.createPath(textPath);

        // Draw outline first
        g.setColour(outlineColour);
        g.strokePath(textPath, stroke);

        // Fill inner
        g.setColour(innerColour);
        g.fillPath(textPath);    
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

        juce::Path textPath;
        glyphs.createPath(textPath);

        // Outline
        g.setColour(juce::Colours::saddlebrown);
        g.strokePath(textPath, stroke);

        // Fill
        g.setColour(juce::Colours::lemonchiffon);
        g.fillPath(textPath);
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

        juce::Path path;
        glyphs.createPath(path);

        // Outline
        g.setColour(juce::Colours::saddlebrown);
        g.strokePath(path, juce::PathStrokeType(3.0f));

        // Fill
        g.setColour(juce::Colours::lemonchiffon);
        g.fillPath(path);
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

