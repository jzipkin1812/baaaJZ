#include "BaaaProcessor.h"
#include "BaaaEditor.h"
#include <iostream>
#include <vector>

static void stylizeSlider(juce::Slider& s) {
    s.setSliderStyle(juce::Slider::LinearVertical);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::darkslategrey);
    return;
}

static void stylizeLabel(juce::Label &l, juce::Component* c, juce::String str) {
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
    stylizeSlider(freqSlider);
    stylizeSlider(gainSlider);
    stylizeSlider(filterSlider);

    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(filterSlider);

    // Slider labels
    stylizeLabel(freqLabel, &freqSlider, "Frequency (Hz)");
    stylizeLabel(gainLabel, &gainSlider, "Output (dB)");
    stylizeLabel(filterLabel, &filterSlider, "Filter");


    // Attachments for sliders
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "frequency", freqSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "outputGain", gainSlider);

    filterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "filter", filterSlider);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, false);
    setResizeLimits(500, 200, 5000, 2000);
    setSize (600, 300);
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
    auto top = area.removeFromTop(int(area.getHeight() / 1.4));

    // Sliders
    int sliderWidth = top.getWidth() / 3;
    freqSlider.setBounds(top.removeFromLeft(sliderWidth));
    gainSlider.setBounds(top.removeFromLeft(sliderWidth));
    filterSlider.setBounds(top.removeFromLeft(sliderWidth));

    // Waveform selector box
    auto bottom = getLocalBounds().reduced(20);
    bottom.removeFromTop((int)((float)(getHeight()) / 1.6f));
}
