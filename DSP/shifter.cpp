
#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <iostream>
using namespace std;

PhaseVocoderPitchShifter::PhaseVocoderPitchShifter(float pRatio, float sRate, unsigned int fSize, unsigned int hSize) 
{
    pitchRatio = pRatio;
    sampleRate = sRate;
    fftSize = fSize;
    hopSize = hSize;
}

void PhaseVocoderPitchShifter::prepare (double sr)
{
    sampleRate = sr;

    gam::sampleRate(sr);

    stft = gam::STFT{
        fftSize,
        hopSize,
        0,
        gam::HANN,
        gam::COMPLEX
    };

    binData.clear();
    binData.resize(stft.numBins(), gam::Complex<float>(0, 0));
}

float PhaseVocoderPitchShifter::processSample (float input)
{
    if (stft(input)) {
        cout << "Making copies of bins for sample " << input << endl;
        // make a copy of the bins and zero the output bins
        for (int k = 0; k < stft.numBins(); ++k) {
            binData[k] = stft.bin(k);
            stft.bin(k) *= 0.0;
        }

        const int N = stft.numBins();

        for (int k = 0; k < N; ++k) {
            float src = k / pitchRatio;

            if (src >= 0.0f && src < N - 1) {
                cout << "Looking up at src " << src << " for input " << input << endl;
                stft.bin(k) = binData.lookup(src);
            } else {
                stft.bin(k) = gam::Complex<float>(0, 0);
            }
            stft.bin(k) *= pitchRatio;
        }
    }

    float output = stft();
    cout << "done with output" << output << endl;
    return(output);
}
void PhaseVocoderPitchShifter::processFrame()
{
}
void PhaseVocoderPitchShifter::setPitchRatio (float newRatio)
{
    pitchRatio = juce::jlimit (-4.0f, 4.0f, newRatio);
}
