#ifndef HELPER_H
#define HELPER_H

#include <array>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <math.h>
#include <sstream>

static int base = 40;

static float f(float x) {
    return (pow(base, x) - 1.0) / (base - 1.0);
}

static float g(float x) {
    return log(x * (base - 1.0) + 1.0) / log(base);
}

template <typename T, int MaxLen, typename Container = std::deque<T>>
class FixedQueue : public Container {
public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
            this->pop_front(); // Corrected: Directly call pop_front
        }
        this->push_back(value); // Corrected: Use push_back instead of push
    }
};

static uint8_t bitsToBytes(uint8_t bits[8])
{
    unsigned int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum<<=1;
        sum += bits[i];
    }
    return sum;
}


template<typename T, std::size_t N>
std::string arrayToString(const std::array<T, N>& arr) {
    std::ostringstream oss;
    oss << "[ ";
    for (std::size_t i = 0; i < N; ++i) {
        oss << +arr[i]; // Unary + to promote uint8_t to int
        if (i != N - 1) {
            oss << ", ";
        }
    }
    oss << " ]";
    return oss.str();
}

static std::string arrayToHexString(const std::array<uint8_t, 6>& arr) {
    std::stringstream ss;
    for (auto byte : arr) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
    }
    return ss.str();
}

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = 0.0;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
        if( in.g >= max )
            out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
        else
            out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

#endif // HELPER_H
