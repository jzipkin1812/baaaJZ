
#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <iostream>
using namespace std;

void PhaseVocoderPitchShifter::prepare (double sr)
{
    sampleRate = sr;

    const unsigned int N = stft.numBins();

    binData.clear();
    binData.resize(N, gam::Complex<float>(0, 0));

    prevPhase.assign(N, 0.0f);
    sumPhase.assign(N, 0.0f);

    expectedPhaseAdvance = 2.0f * float(M_PI) * hopSize / fftSize;

    prepared = true;
}

float PhaseVocoderPitchShifter::processSample (float input)
{
    if (!prepared || pitchRatio <= 0.0f)
        return 0.0f;

    if (stft(input))
    {
        const unsigned int N = stft.numBins();

        // Copy bins and clear output bins
        for (unsigned int k = 0; k < N; ++k)
        {
            binData[k] = stft.bin(k);
            stft.bin(k) = gam::Complex<float>(0, 0);
        }

        for (unsigned int k = 1; k < N - 1; ++k)
        {
            float src = k / pitchRatio;

            if (src < 0.0f || src >= N - 1)
                continue;

            gam::Complex<float> c = binData.lookup(src);

            float mag = c.mag();
            float phase = c.arg();

            // Phase difference
            float delta = phase - prevPhase[k];
            prevPhase[k] = phase;

            // Remove expected advance
            delta -= k * expectedPhaseAdvance;

            // Wrap to -pi..pi
            delta = std::fmod(delta + float(M_PI), 2.0f * float(M_PI)) - float(M_PI);

            // Scale phase increment
            float trueFreq = k * expectedPhaseAdvance + delta;
            sumPhase[k] += trueFreq * pitchRatio;

            // Reconstruct bin
            stft.bin(k) = gam::Polar<float>(mag, sumPhase[k]);
        }

        // Zero DC and Nyquist (stability)
        stft.bin(0) = gam::Complex<float>(0, 0);
        if (N > 1)
            stft.bin(N - 1) = gam::Complex<float>(0, 0);
    }

    float output = stft();

    // NaN safety
    if (!std::isfinite(output))
        output = 0.0f;

    return output;
}

void PhaseVocoderPitchShifter::setPitchRatio (float newRatio)
{
    pitchRatio = juce::jlimit (0.25f, 32.0f, newRatio);
}
