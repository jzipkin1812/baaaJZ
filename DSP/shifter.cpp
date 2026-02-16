
#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <iostream>
using namespace std;

void PhaseVocoderPitchShifter::prepare (double sr)
{
    binData.clear();
    binData.resize(stft.numBins(), gam::Complex<float>(0, 0));
}

float PhaseVocoderPitchShifter::processSample (float input)
{
    if (stft(input)) {
        // make a copy of the bins and zero the output bins
        for (unsigned int k = 0; k < stft.numBins(); ++k) {
            binData[k] = stft.bin(k);
            stft.bin(k) *= 0.0;
        }

        const unsigned int N = stft.numBins();

        for (unsigned int k = 0; k < N; ++k) {
            float src = k / pitchRatio;

            if (src >= 0.0f && src < N - 1) {
                stft.bin(k) = binData.lookup(src);
            } else {
                stft.bin(k) = gam::Complex<float>(0, 0);
            }
            stft.bin(k) *= pitchRatio;
        }
    }

    float output = stft();
    return(output);
}
void PhaseVocoderPitchShifter::processFrame()
{
}
void PhaseVocoderPitchShifter::setPitchRatio (float newRatio)
{
    pitchRatio = juce::jlimit (0.25f, 4.0f, newRatio);
}
