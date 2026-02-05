// This code is vibecoded by ChatGPT. All of it is about to change.

#pragma once
#include <vector>
#include <JuceHeader.h>

class PhaseVocoderPitchShifter
{
public:
    void prepare (double sampleRate, int fftSize = 1024, int hopSize = 256);
    void setPitchRatio (float newRatio);
    float processSample (float input);

private:
    void processFrame();

    double sampleRate = 44100.0;
    int fftSize = 1024;
    int hopSize = 256;

    juce::dsp::FFT fft { 10 }; // 2^10 = 1024
    std::vector<float> window;


    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<float> fftData;

    std::vector<float> prevPhase;
    std::vector<float> phaseAccumulator;

    int inputWritePos = 0;
    int outputReadPos = 0;

    float pitchRatio = 1.0f;
};
