
#pragma once
#include <vector>
#include <JuceHeader.h>
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include "utility.h"

class PhaseVocoderPitchShifter
{
public:
    void prepare (float sampleRate);
    void setPitchRatio (float newRatio) {pitchRatio = juce::jlimit (0.25f, 32.0f, newRatio);}
    void setCenterFrequency (float newFrequency) {centerFrequency = newFrequency;}
    void setFalloff (float newFalloff) {falloff = newFalloff;}
    float processSample (float input);
    PhaseVocoderPitchShifter(float pRatio,
                              float sRate,
                              unsigned int fSize,
                              unsigned int hSize)
            : sampleRate(sRate),
            fftSize(fSize),
            hopSize(hSize),
            pitchRatio(pRatio),
            makeItShepard(false),
            centerFrequency(0.0),
            falloff(0.0),
            stft(fSize, hSize, 0, gam::HANN, gam::COMPLEX)
    {
    }

    PhaseVocoderPitchShifter(float pRatio,
                              float sRate,
                              unsigned int fSize,
                              unsigned int hSize,
                              float center,
                              float falloffAmount)
            : sampleRate(sRate),
            fftSize(fSize),
            hopSize(hSize),
            pitchRatio(pRatio),
            makeItShepard(true),
            centerFrequency(center),
            falloff(falloffAmount),
            stft(fSize, hSize, 0, gam::HANN, gam::COMPLEX)
    {
    }

private:
    float sampleRate;
    unsigned int fftSize;
    unsigned int hopSize;
    float pitchRatio;

    // This shifter is used in conjunction with the Shepard class.
    // It can be a standalone shifter OR
    // can also accomplish the job of modifying volume
    // based on a center frequency.
    bool makeItShepard;
    float centerFrequency;
    float falloff;

    gam::STFT stft;
    // Supplemental state info for STFT
    LerpArray<gam::STFT::bin_type> binData;
    std::vector<float> prevPhase;
    std::vector<float> sumPhase;
    float expectedPhaseAdvance = 0.0f;
    bool prepared = false;



    PhaseVocoderPitchShifter(const PhaseVocoderPitchShifter&) = delete;
    PhaseVocoderPitchShifter& operator=(const PhaseVocoderPitchShifter&) = delete;
    PhaseVocoderPitchShifter(PhaseVocoderPitchShifter&&) = delete;
    PhaseVocoderPitchShifter& operator=(PhaseVocoderPitchShifter&&) = delete;


};
