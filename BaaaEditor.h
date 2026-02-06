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

    juce::Slider freqSlider;
    juce::Slider gainSlider;
    juce::Slider upDupeSlider;
    juce::Slider downDupeSlider;


    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> upDupeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> downDupeAttachment;

    juce::Label freqLabel;
    juce::Label gainLabel;
    juce::Label upDupeLabel;
    juce::Label downDupeLabel;



private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BaaaPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaaaPluginAudioProcessorEditor)
};
