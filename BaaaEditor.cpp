#include "BaaaProcessor.h"
#include "BaaaEditor.h"
#include <iostream>
#include <vector>


void BaaaPluginAudioProcessorEditor::stylizeSlider(juce::Label &l, juce::Slider &s, juce::String str) {
    // Stylize the slider
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::darkslategrey);

    // Stylize the label
    juce::Component *c = &s;
    l.setText(str, juce::dontSendNotification);
    l.setFont(fontSmall);
    l.setJustificationType(juce::Justification::centred);
    l.attachToComponent(c, false);
    l.setColour(juce::Label::textColourId, juce::Colours::darkslategrey);
}
// Same as above but for small integer steps
void BaaaPluginAudioProcessorEditor::stylizeStepper(juce::Label &l, juce::Slider &s, juce::String str) 
{
    s.setSliderStyle(juce::Slider::IncDecButtons);
    s.setRange(0, 5, 1);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    l.setText(str, juce::dontSendNotification);
    l.setFont(fontSmall);
    l.setJustificationType(juce::Justification::centred);
    l.attachToComponent(&s, false);
}
//==============================================================================
BaaaPluginAudioProcessorEditor::BaaaPluginAudioProcessorEditor (BaaaPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setLookAndFeel(&customLook);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png,
                                                  BinaryData::background_pngSize);
    // Slider labels
    stylizeSlider(shiftLabel, shiftSlider, "Shift Amount (ct)");
    stylizeSlider(gainLabel, gainSlider, "Output (dB)");
    stylizeStepper(upDupeLabel, upDupeSlider, "Superpositions Up");
    stylizeStepper(downDupeLabel, downDupeSlider, "Superpositions Down");    
    stylizeSlider(centerLabel, centerSlider, "Center Frequency (Hz)");
    stylizeSlider(falloffLabel, falloffSlider, "Bandpass Filter Width");

    addAndMakeVisible(shiftSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(upDupeSlider);
    addAndMakeVisible(downDupeSlider);
    addAndMakeVisible(centerSlider);
    addAndMakeVisible(falloffSlider);

    // Attachments for sliders
    shiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "shiftAmt", shiftSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "outputGain", gainSlider);

    upDupeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "upCount", upDupeSlider);
    
    downDupeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "downCount", downDupeSlider);

    centerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "centerFrequency", centerSlider);

    falloffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "falloff", falloffSlider);

    upOverlay = std::make_unique<SliderValueOverlay>(upDupeSlider);
    addAndMakeVisible(upOverlay.get());
    downOverlay = std::make_unique<SliderValueOverlay>(downDupeSlider);
    addAndMakeVisible(downOverlay.get());

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, false);
    setResizeLimits(700, 400, 2000, 1000);
    setSize (1000, 500);
}

BaaaPluginAudioProcessorEditor::~BaaaPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BaaaPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    int imgW = backgroundImage.getWidth();
    int imgH = backgroundImage.getHeight();

    int x = getWidth()  - imgW;
    int y = getHeight() - imgH;

    g.drawImageAt(backgroundImage, x, y);
    g.setFont(textSizeLarge);

    juce::GlyphArrangement glyphs;
    glyphs.addFittedText(fontLarge, "Welcome to Baaaaaaaa",
                        0, (float)(getHeight()-40),
                        float(getWidth()), 40.0f,
                        juce::Justification::centred,
                        1);

    juce::Path textPath;
    glyphs.createPath(textPath);

    g.setColour(outlineColour);
    g.strokePath(textPath, juce::PathStrokeType(3.0f));

    g.setColour(innerColour);
    g.fillPath(textPath);
}

void BaaaPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(40);

    int half = area.getHeight() / 2;

    auto knobRow = area.removeFromTop(half + 50);
    auto bottomRow = area.removeFromTop(100);

    int knobWidth = knobRow.getWidth() / 4;

    shiftSlider.setBounds(knobRow.removeFromLeft(knobWidth));
    gainSlider.setBounds(knobRow.removeFromLeft(knobWidth));
    centerSlider.setBounds(knobRow.removeFromLeft(knobWidth));
    falloffSlider.setBounds(knobRow.removeFromLeft(knobWidth));

    int stepWidth = bottomRow.getWidth() / 2;
    upDupeSlider.setBounds(bottomRow.removeFromLeft(stepWidth));
    downDupeSlider.setBounds(bottomRow);

    upOverlay->setBounds(upDupeSlider.getBounds());
    downOverlay->setBounds(downDupeSlider.getBounds());
}
