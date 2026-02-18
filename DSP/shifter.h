
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
    void prepare (double sampleRate);
    void setPitchRatio (float newRatio);
    float processSample (float input);
    PhaseVocoderPitchShifter(float pRatio,
                              float sRate,
                              unsigned int fSize,
                              unsigned int hSize)
            : sampleRate(sRate),
            fftSize(fSize),
            hopSize(hSize),
            pitchRatio(pRatio),
            stft(fSize, hSize, 0, gam::HANN, gam::COMPLEX)
    {
    }

private:
    double sampleRate;
    unsigned int fftSize;
    unsigned int hopSize;
    float pitchRatio;
    gam::STFT stft;
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
