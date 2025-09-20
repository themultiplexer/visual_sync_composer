#ifndef RUNNING_VARIANCE_H
#define RUNNING_VARIANCE_H

#include <array>
#include <cstddef>

template <std::size_t WindowSize, typename T = double>
class RunningVariance {
public:
    RunningVariance() : sum(0), sumSq(0), index(0), count(0) {
        buffer.fill(0);
    }

    // Add a sample and return the current variance (population variance)
    T addSample(T value) {
        if (count < WindowSize) {
            buffer[count++] = value;
            sum += value;
            sumSq += value * value;
        } else {
            T old = buffer[index];
            sum -= old;
            sumSq -= old * old;

            buffer[index] = value;
            sum += value;
            sumSq += value * value;

            index = (index + 1) % WindowSize;
        }

        T mean = sum / static_cast<T>(count);
        T variance = (sumSq / static_cast<T>(count)) - (mean * mean);
        return variance;
    }

private:
    std::array<T, WindowSize> buffer;
    T sum;
    T sumSq;
    std::size_t index;
    std::size_t count;
};

#endif // RUNNING_VARIANCE_H
