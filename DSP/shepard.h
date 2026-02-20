#pragma once
#include "shifter.h"
#include "math.h"
#include "Gamma/Analysis.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
#include <vector>
#include <iostream>
using namespace std;

class Shepard 
{
public:
    Shepard(unsigned int channels, unsigned int shiftPerChannel, float sr);
    void prepare();
    std::unique_ptr<PhaseVocoderPitchShifter>& getShifter(unsigned int channel, int pos);
    float processSample(float input, unsigned int channel);

    void setPitchRatio(float newRatio) {pitchRatio = newRatio;}
    void setFalloff(float newFalloff) {falloff = newFalloff;}
    void setCenter(float newCenter) {centerFrequency = newCenter;}

    void setSuperpositionsUp(unsigned int sup) {superpositionsUp = sup;}
    void setSuperpositionsDown(unsigned int sup) {superpositionsDown = sup;}

private:
    std::vector<std::unique_ptr<PhaseVocoderPitchShifter>> shifters;
    unsigned int numberOfChannels;
    unsigned int shiftersPerChannel;
    float sampleRate;
    float pitchRatio;
    float falloff;
    float centerFrequency;
    unsigned int superpositionsUp;
    unsigned int superpositionsDown;

};