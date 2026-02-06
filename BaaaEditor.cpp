#include "BaaaProcessor.h"
#include "BaaaEditor.h"
#include <iostream>
#include <vector>


static void stylizeSlider(juce::Label &l, juce::Slider &s, juce::String str) {
    // Stylize the slider
    s.setSliderStyle(juce::Slider::LinearVertical);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::darkslategrey);

    // Stylize the label
    juce::Component *c = &s;
    l.setText(str, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.attachToComponent(c, false);
    l.setColour(juce::Label::textColourId, juce::Colours::darkslategrey);
}

//==============================================================================
BaaaPluginAudioProcessorEditor::BaaaPluginAudioProcessorEditor (BaaaPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Slider labels
    stylizeSlider(freqLabel, freqSlider, "Frequency (Hz)");
    stylizeSlider(gainLabel, gainSlider, "Output (dB)");
    stylizeSlider(upDupeLabel, upDupeSlider, "Superpositions Up");
    stylizeSlider(downDupeLabel, downDupeSlider, "Superpositions Down");

    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(upDupeSlider);
    addAndMakeVisible(downDupeSlider);

    // Attachments for sliders
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "frequency", freqSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "outputGain", gainSlider);

    upDupeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "upCount", upDupeSlider);
    
    downDupeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "downCount", downDupeSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, false);
    setResizeLimits(500, 200, 5000, 2000);
    setSize (800, 300);
}

BaaaPluginAudioProcessorEditor::~BaaaPluginAudioProcessorEditor()
{
}

//==============================================================================
void BaaaPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto gradient = juce::ColourGradient(
        juce::Colour::fromRGB(127, 255, 0),
        juce::Point<float>(0.0, 0.0),
        juce::Colour::fromRGB(176, 224, 230),
        juce::Point<float>((float)getWidth(), (float)getHeight()),
        false
    );

    juce::Image gradientImage(juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics tg(gradientImage);
    tg.setGradientFill(gradient);
    tg.fillAll();
    g.drawImage(gradientImage, getLocalBounds().toFloat());

    g.setColour(juce::Colours::darkslategrey);
    g.setFont(20.0f);
    g.drawFittedText("Welcome to Baaaaaaaa", getLocalBounds(), juce::Justification::centredBottom, 1);
}

void BaaaPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(30);
    auto top = area.removeFromTop(int(area.getHeight() / 1.25));

    // Sliders
    int sliderWidth = top.getWidth() / 4;
    freqSlider.setBounds(top.removeFromLeft(sliderWidth));
    gainSlider.setBounds(top.removeFromLeft(sliderWidth));
    upDupeSlider.setBounds(top.removeFromLeft(sliderWidth));
    downDupeSlider.setBounds(top.removeFromLeft(sliderWidth));

}
