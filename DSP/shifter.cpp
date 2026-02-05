// This code is vibecoded by ChatGPT. All of it is about to change.

#include "shifter.h"
#include "math.h"
void PhaseVocoderPitchShifter::prepare (double sr, int size, int hop)
{
    sampleRate = sr;
    fftSize = size;
    hopSize = hop;

    fft = juce::dsp::FFT (log2(fftSize));
    window.resize (fftSize);
    for (int i = 0; i < fftSize; ++i)
    {
        window[i] = 0.5f * (1.0f
            - std::cos (2.0f * juce::MathConstants<float>::pi * i / fftSize));
    }


    inputBuffer.assign (fftSize, 0.0f);
    outputBuffer.assign (fftSize, 0.0f);
    fftData.assign (2 * fftSize, 0.0f);

    prevPhase.assign (fftSize / 2 + 1, 0.0f);
    phaseAccumulator.assign (fftSize / 2 + 1, 0.0f);
}

float PhaseVocoderPitchShifter::processSample (float input)
{
    inputBuffer[inputWritePos++] = input;

    float out = outputBuffer[outputReadPos++];
    outputBuffer[outputReadPos - 1] = 0.0f;

    if (inputWritePos >= fftSize)
    {
        processFrame();
        inputWritePos = 0;
        outputReadPos = 0;
    }

    return out;
}

void PhaseVocoderPitchShifter::processFrame()
{
    // Window input
    for (int i = 0; i < fftSize; ++i)
        fftData[2 * i] = inputBuffer[i] * window[i];

    std::fill (fftData.begin() + 1, fftData.end(), 0.0f);

    fft.performRealOnlyForwardTransform (fftData.data());

    const int numBins = fftSize / 2;

    for (int k = 0; k <= numBins; ++k)
    {
        int idx = 2 * k;
        float real = fftData[idx];
        float imag = fftData[idx + 1];

        float mag = std::sqrt (real * real + imag * imag);
        float phase = std::atan2 (imag, real);

        float deltaPhase = phase - prevPhase[k];
        prevPhase[k] = phase;

        float expected = 2.0f * juce::MathConstants<float>::pi * k * hopSize / fftSize;
        float phaseError = deltaPhase - expected;

        phaseError -= juce::MathConstants<float>::twoPi
            * std::round (phaseError / juce::MathConstants<float>::twoPi);

        float trueFreq = expected + phaseError;
        phaseAccumulator[k] += trueFreq * pitchRatio;

        int targetBin = int (k * pitchRatio);
        if (targetBin <= numBins)
        {
            fftData[2 * targetBin]     += mag * std::cos (phaseAccumulator[k]);
            fftData[2 * targetBin + 1] += mag * std::sin (phaseAccumulator[k]);
        }
    }

    fft.performRealOnlyInverseTransform (fftData.data());

    // Overlap-add
    for (int i = 0; i < fftSize; ++i)
        outputBuffer[i] += fftData[2 * i] * window[i] / fftSize;
}

void PhaseVocoderPitchShifter::setPitchRatio (float newRatio)
{
    pitchRatio = juce::jlimit (0.5f, 2.0f, newRatio);
}
