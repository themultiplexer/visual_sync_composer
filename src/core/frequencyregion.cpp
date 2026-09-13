#include "core/frequencyregion.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <cmath>

FrequencyRegion::FrequencyRegion(int index, float min, float max, int step, std::string name):
    index(index), step(step), thresh(0.7), peak(0.0), name(name), start(min), end(max) {

}

float FrequencyRegion::getEnd() const
{
    return end > start ? end : start;
}

void FrequencyRegion::setEnd(float newEnd)
{
    end = std::fmax(std::fmin(newEnd, 1.0), -1.0);
}

float FrequencyRegion::getStart() const
{
    return start < end ? start : end;
}

void FrequencyRegion::setStart(float newStart)
{
    start = std::fmax(std::fmin(newStart, 1.0), -1.0);
}

float FrequencyRegion::getThresh() const
{
    return thresh;
}

int FrequencyRegion::getIndex() const
{
    return index;
}

void FrequencyRegion::setThresh(float newThresh)
{
    thresh = newThresh;
}

bool FrequencyRegion::processData(std::array<float, 1024> &data)
{
    auto now = std::chrono::steady_clock::now();
    level = 0.0;
    for (int i = f(start) * step; i < f(end) * step; ++i) {
        //level += data[i];
        level = data[i] > level ? data[i] : level;
    }
    float beta = 0.002;
    bool lowpeak = (level > getThresh());
    beatMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBeat).count();
    bool debounce = (beatMillis > 100);
    bool peaked = lowpeak && debounce;

    if (peaked) {
        peak = 1.0;
        thresh = std::max(level - 0.2f, thresh);
    } else {
        thresh = std::max((beta * (level + 0.2)) + (1.0 - beta) * thresh, 0.25);
        if (peak > 0) {
            peak -= 0.05;
        }
    }

    if (peaked) {
        lastBeat = now;
        return true;
    }
    return false;
}

float FrequencyRegion::getPeak() const
{
    return peak;
}

float FrequencyRegion::getLevel() const
{
    return level;
}

float FrequencyRegion::getColor() const
{
    return index;
}

std::string FrequencyRegion::getName() const
{
    return name;
}