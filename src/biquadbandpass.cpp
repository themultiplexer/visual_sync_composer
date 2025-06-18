#include "biquadbandpass.h"
#include <cmath>

BiquadBandpass::BiquadBandpass(double lowCut, double highCut, double sampleRate)
{
    double nyquist = 0.5 * sampleRate;
    double low = lowCut / nyquist;
    double high = highCut / nyquist;
    designBandpass(low, high);
}

void BiquadBandpass::designBandpass(double low, double high)
{
    // Simple butterworth bandpass using bilinear transform approximation.
    // This is just an example, you may want to use more precise filter design

    double center = (low + high) / 2.0;
    double Q = (center / (high - low));  // Q factor

    double omega = 2.0 * M_PI * center;
    double alpha = sin(omega) / (2.0 * Q);

    double cos_omega = cos(omega);
    double norm = 1.0 / (1.0 + alpha);

    b0 = alpha * norm;
    b1 = 0;
    b2 = -alpha * norm;
    a1 = -2.0 * cos_omega * norm;
    a2 = (1.0 - alpha) * norm;

    // reset state
    z1 = z2 = 0;
}

double BiquadBandpass::process(double input)
{
    double output = b0 * input + b1 * z1 + b2 * z2 - a1 * out1 - a2 * out2;
    z2 = z1;
    z1 = input;
    out2 = out1;
    out1 = output;
    return output;
}
