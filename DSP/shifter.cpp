
#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"

PhaseVocoderPitchShifter::PhaseVocoderPitchShifter(float pRatio, float sRate, int fSize, int hSize) 
{
    gam::STFT stft{
      2048,      // Window size
      2048 / 4,  // Hop size; number of samples between transforms
      0,         // Pad size; number of zero-valued samples appended to window
      gam::HANN,      // Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
                 //		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
      gam::COMPLEX    // Format of frequency samples:
                 //		COMPLEX, MAG_PHASE, or MAG_FREQ
    };

    this->pitchRatio = pRatio;
    this->sampleRate = sRate;
    this->fftSize = fSize;
    this->hopSize = hSize;

    this->inputWritePos = 0;
    this->outputReadPos = 0;

}

void PhaseVocoderPitchShifter::prepare (double sr)
{
    sampleRate = sr;
    fftSize = 1024;
    hopSize = 256;

    fft = juce::dsp::FFT (10);

    inputBuffer.assign (fftSize, 0.0f);
    outputBuffer.assign (fftSize, 0.0f);

    fftIn.assign (2 * fftSize, 0.0f);
    fftOut.assign (2 * fftSize, 0.0f);

    prevPhase.assign (fftSize / 2 + 1, 0.0f);
    phaseAcc.assign (fftSize / 2 + 1, 0.0f);

    window.resize (fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5f * (1.0f -
            std::cos (2.0f * juce::MathConstants<float>::pi * i / fftSize));
}

float PhaseVocoderPitchShifter::processSample (float input)
{
    // write input into circular buffer
    inputBuffer[inputWritePos] = input;

    // read output from circular buffer
    float out = outputBuffer[outputReadPos];
    outputBuffer[outputReadPos] = 0.0f;

    // advance pointers
    inputWritePos  = (inputWritePos + 1) % fftSize;
    outputReadPos  = (outputReadPos + 1) % fftSize;

    // hop logic
    if (++hopCounter >= hopSize)
    {
        hopCounter = 0;
        processFrame();
        outputWritePos = (outputWritePos + hopSize) % fftSize;
    }

    return out;
}
void PhaseVocoderPitchShifter::processFrame()
{
    std::fill (fftIn.begin(), fftIn.end(), 0.0f);
    std::fill (fftOut.begin(), fftOut.end(), 0.0f);

    // --- Analysis ---
    for (int i = 0; i < fftSize; ++i)
    {
        int idx = (inputWritePos + i) % fftSize;
        fftIn[2 * i] = inputBuffer[idx] * window[i];
    }

    fft.performRealOnlyForwardTransform (fftIn.data());

    const int numBins = fftSize / 2;
    const float expectedPhaseAdvance =
        juce::MathConstants<float>::twoPi * hopSize / fftSize;

    for (int k = 1; k <= numBins; ++k)
    {
        const int idx = 2 * k;

        float real = fftIn[idx];
        float imag = fftIn[idx + 1];

        float mag   = std::hypot (real, imag);
        float phase = std::atan2 (imag, real);

        float delta = phase - prevPhase[k];
        prevPhase[k] = phase;

        float expected = expectedPhaseAdvance * k;
        delta -= expected;

        delta -= juce::MathConstants<float>::twoPi *
                 std::round (delta / juce::MathConstants<float>::twoPi);

        float truePhaseAdvance = expected + delta;
        phaseAcc[k] += truePhaseAdvance * pitchRatio;

        int targetBin = int (k * pitchRatio);
        if (targetBin > 0 && targetBin <= numBins)
        {
            fftOut[2 * targetBin]     += mag * std::cos (phaseAcc[k]);
            fftOut[2 * targetBin + 1] += mag * std::sin (phaseAcc[k]);
        }
    }

    fft.performRealOnlyInverseTransform (fftOut.data());

    // --- Overlap-add ---
    float norm = 1.0f / (fftSize * 0.5f); // Hann correction

    for (int i = 0; i < fftSize; ++i)
    {
        int idx = (outputWritePos + i) % fftSize;
        outputBuffer[idx] += fftOut[2 * i] * window[i] * norm;
    }
}
void PhaseVocoderPitchShifter::setPitchRatio (float newRatio)
{
    pitchRatio = juce::jlimit (-4.0f, 4.0f, newRatio);
}
