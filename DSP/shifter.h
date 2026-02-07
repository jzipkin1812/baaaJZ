// This code is vibecoded by ChatGPT. All of it is about to change.

#pragma once
#include <vector>
#include <JuceHeader.h>

class PhaseVocoderPitchShifter
{
public:
    void prepare (double sampleRate);
    void setPitchRatio (float newRatio);
    float processSample (float input);
    PhaseVocoderPitchShifter(float pitchRatio = 2.0f, float sampleRate = 44100.0f, int fftSize = 1024, int hopSize = 256);

private:
    void processFrame();

    double sampleRate;
    int fftSize;
    int hopSize;

    juce::dsp::FFT fft { 10 }; // 2^10 = 1024

    int hopCounter = 0;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;

    std::vector<float> window;

    std::vector<float> prevPhase;
    std::vector<float> phaseAcc;

    std::vector<float> fftIn;
    std::vector<float> fftOut;

    int inputWritePos;
    int outputReadPos;

    float pitchRatio;
    int outputWritePos = 0;

};
