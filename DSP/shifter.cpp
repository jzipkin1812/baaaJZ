#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <iostream>

using namespace std;

void PhaseVocoderPitchShifter::prepare (float sr)
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
        doShift();
    }

    float output = stft();

    if (!std::isfinite(output))
        output = 0.0f;

    return output;
}

// This code was started with the following chat GPT prompt and then heavily revised:
// This simple C++ pitch shifter is functional but has a low-quality output sound. Improve its design with minimal changes. 
// Do not change the public API but you may want to add private variables. Make sure: 
// - The output sound's gain/volume is not generally adjusted very much 
// - Sanity checks are added to the code to make sure nothing goes horribly wrong 
// - Still use Gamma's STFT; don't insert other random dependencies

void PhaseVocoderPitchShifter::doShift()
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

        float mag = c.mag() *
            (pitchRatio > 1 ? std::sqrt(pitchRatio)
                            : 1.0f / std::sqrt(pitchRatio));

        float phase = c.arg();

        float delta = phase - prevPhase.lookup(src);
        prevPhase[src] = phase;

        float expectedSrc = src * expectedPhaseAdvance;
        delta -= expectedSrc;

        while (delta >  M_PI) delta -= 2.0f * M_PI;
        while (delta < -M_PI) delta += 2.0f * M_PI;

        float trueFreq = expectedSrc + delta;

        float expectedDest = k * expectedPhaseAdvance;
        float scaledFreq = trueFreq * pitchRatio;

        float deltaDest = scaledFreq - expectedDest;

        sumPhase[k] += expectedDest + deltaDest;

        // Bandpass filter
        if (makeItShepard && falloff < 2.99f)
        {
            float nyquist = sampleRate * 0.5f;

            float binFreq = (float(k) / float(N)) * nyquist;

            if (binFreq > 1.0f && centerFrequency > 1.0f)
            {
                float logDistance = std::log2(binFreq / centerFrequency);

                float weight = std::exp(
                    -(logDistance * logDistance) /
                    (2.0f * falloff * falloff)
                );

                mag *= weight;
            }
        }

        stft.bin(k) = gam::Polar<float>(mag, sumPhase[k]);
    }


    // Zero DC and Nyquist (stability)
    stft.bin(0) = gam::Complex<float>(0, 0);
    if (N > 1)
        stft.bin(N - 1) = gam::Complex<float>(0, 0);
}