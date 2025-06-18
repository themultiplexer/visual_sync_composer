#ifndef AUDIOFILTER_H
#define AUDIOFILTER_H
#include "biquadbandpass.h"

class audiofilter
{
public:
    audiofilter();
    int sample(void *inputBuffer, void *outputBuffer, int nBufferFrames);

    int getLower() const;
    void setLower(int newLower);

    int getUpper() const;
    void setUpper(int newUpper);

private:
    int lower, upper;
    BiquadBandpass filter;
};

#endif // AUDIOFILTER_H
