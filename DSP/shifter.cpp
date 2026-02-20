
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

            // Magnitude = intensity, complex portion = phase
            float mag = c.mag() * (pitchRatio > 1 ? std::sqrt(pitchRatio) : 1.0f / std::sqrt(pitchRatio));

            float phase = c.arg();
            size_t srcIndex = (size_t)src;

            float delta = phase - prevPhase[srcIndex];
            prevPhase[srcIndex] = phase;

            // Expected phase advance based on SOURCE bin
            float expectedSrc = srcIndex * expectedPhaseAdvance;
            delta -= expectedSrc;

            while (delta >  M_PI) delta -= 2.0f * M_PI;
            while (delta < -M_PI) delta += 2.0f * M_PI;

            // Instantaneous frequency at SOURCE
            float trueFreq = expectedSrc + delta;

            // Now scale frequency to DESTINATION bin
            float scaledFreq = trueFreq * pitchRatio;

            // Accumulate at destination bin
            sumPhase[k] += scaledFreq;

            // For shepard tones: Scale frequency.
            if(makeItShepard) {
                float nyquist = sampleRate / 2.0f;
                float hertz = float(k) / stft.numBins() * nyquist;
            }

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