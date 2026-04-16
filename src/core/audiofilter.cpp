#include "audiofilter.h"

#include <cmath>

const unsigned int SAMPLE_RATE = 48000;

// RtAudio callback function
int audiofilter::sample(void* inputBuffer, void* outputBuffer,int nBufferFrames)
{
    float* in = static_cast<float*>(inputBuffer);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned int i = 0; i < nBufferFrames; i++)
    {
        double filteredSample = filter.process(in[i]);
        out[i] = static_cast<float>(filteredSample);
    }

    return 0;
}

int audiofilter::getLower() const
{
    return lower;
}

void audiofilter::setLower(int newLower)
{
    lower = newLower;
    filter = BiquadBandpass(lower, upper, SAMPLE_RATE);
}

int audiofilter::getUpper() const
{
    return upper;
}

void audiofilter::setUpper(int newUpper)
{
    upper = newUpper;
    filter = BiquadBandpass(lower, upper, SAMPLE_RATE);
}

audiofilter::audiofilter(): lower(10), upper(200), filter(lower, upper, SAMPLE_RATE)
{

}
