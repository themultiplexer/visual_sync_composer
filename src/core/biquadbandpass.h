#ifndef BIQUADBANDPASS_H
#define BIQUADBANDPASS_H

// Biquad bandpass filter
class BiquadBandpass {
public:
    BiquadBandpass(double lowCut, double highCut, double sampleRate);
    void designBandpass(double low, double high);
    double process(double input);

private:
    double b0, b1, b2, a1, a2;
    double z1 = 0, z2 = 0;
    double out1 = 0, out2 = 0;
};

#endif // BIQUADBANDPASS_H
