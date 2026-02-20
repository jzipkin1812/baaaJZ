#pragma once

#include "BaaaProcessor.h"

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

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BaaaPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaaaPluginAudioProcessorEditor)
};
