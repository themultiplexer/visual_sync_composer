#ifndef HELPER_H
#define HELPER_H

#include <cstdint>
#include <deque>

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


#endif // HELPER_H
