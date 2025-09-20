#ifndef RUNNING_MEAN_H
#define RUNNING_MEAN_H

#include <array>
#include <cstddef>

template <std::size_t WindowSize, typename T = double>
class RunningMean {
public:
    RunningMean() : sum(0), index(0), count(0) {
        buffer.fill(0);
    }

    // Add a sample and return the current mean
    T addSample(T value) {
        if (count < WindowSize) {
            buffer[count++] = value;
            sum += value;
        } else {
            sum -= buffer[index];
            buffer[index] = value;
            sum += value;
            index = (index + 1) % WindowSize;
        }
        return sum / static_cast<T>(count);
    }

private:
    std::array<T, WindowSize> buffer;
    T sum;
    std::size_t index;
    std::size_t count;
};

#endif // RUNNING_MEAN_H
