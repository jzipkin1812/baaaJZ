
#include "shifter.h"
#include "shepard.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <iostream>
using namespace std;

Shepard::Shepard(unsigned int channels, unsigned int shiftPerChannel, float sr) {
    sampleRate = sr;
    numberOfChannels = channels;
    shiftersPerChannel = shiftPerChannel;
    if(shiftersPerChannel % 2 == 1) shiftersPerChannel += 1;

    shifters.reserve(channels * (shiftPerChannel + 1));

    for (size_t i = 0; i < channels * (shiftPerChannel + 1); ++i)
    {
        auto shifter = std::make_unique<PhaseVocoderPitchShifter>(
            1.0f,
            sampleRate,
            2048,
            2048/4,
            1000.0f,
            100.0f
        );
        shifter->prepare(sampleRate);
        shifters.push_back(std::move(shifter));
    }

    superpositionsDown = 1;
    superpositionsUp = 1;
    shifters.back()->setPitchRatio(1.0f);
}

std::unique_ptr<PhaseVocoderPitchShifter>& Shepard::getShifter(unsigned int channel, int pos) {
    // If there are max s superpositions per channel (s will be even):
    // pos is in the range [-s/2, s/2] but cannot be 0
    unsigned int index = (channel * shiftersPerChannel) + (unsigned int)(pos + int(shiftersPerChannel / 2));
    if(pos > 0) index -= 1;
    return(shifters[index]);
}


float Shepard::processSample(float input, unsigned int channel)
{
    if (channel >= numberOfChannels)
        return 0.0f;

    if (shifters.empty())
        return input;

    float output = 0.0f;
    unsigned int activeVoices = 0;

    // ----- DOWN SHIFTS -----
    for (unsigned int i = 1; i <= superpositionsDown; ++i)
    {
        float ratio = 1.0f / std::pow(pitchRatio, (float)(i));
        // std::cout << "For i =" << i << ", the ratio is" << ratio << endl;

        auto& sh = getShifter(channel, -(int)i);
        sh->setPitchRatio(ratio);
        sh->setFalloff(falloff);
        sh->setCenterFrequency(centerFrequency);

        output += sh->processSample(input);
        ++activeVoices;
    }

    // ----- UP SHIFTS -----
    for (unsigned int i = 1; i <= superpositionsUp; ++i)
    {
        float ratio = std::pow(pitchRatio, (float)(i));

        auto& sh = getShifter(channel, (int)i);
        sh->setPitchRatio(ratio);
        sh->setFalloff(falloff);
        sh->setCenterFrequency(centerFrequency);

        output += sh->processSample(input);
        ++activeVoices;
    }

    // Center Voice
    {
        unsigned int index = channel * (shiftersPerChannel + 1) + shiftersPerChannel;
        auto& sh = shifters[index];
        
        sh->setCenterFrequency(centerFrequency);
        sh->setFalloff(falloff);
        output += sh->processSample(input) * 0.85;
        ++activeVoices;
    }

    // Normalize to prevent gain explosion
    if (activeVoices > 0)
        output /= (float)(std::sqrt(activeVoices));

    // Safety clamp
    if (!std::isfinite(output))
        output = 0.0f;

    return output;
}
