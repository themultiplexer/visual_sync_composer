#include "frequencyregion.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <cmath>

FrequencyRegion::FrequencyRegion(int min, int max, int step, std::string name):
    step(step), levels(), level(0.0), smoothLevel(0.0), min(min), max(max), start(-1.0 + 2 *((float)min/(float)step)), end(-1.0 + 2 *((float)max/(float)step)), name(name) {

}

float FrequencyRegion::getEnd() const
{
    return end > start ? end : start;
}

void FrequencyRegion::setEnd(float newEnd)
{
    end = std::fmax(std::fmin(newEnd, 1.0), -1.0);
}

void FrequencyRegion::setMin(int newMin)
{
    min = newMin;
}

void FrequencyRegion::setMax(int newMax)
{
    max = newMax;
}

bool FrequencyRegion::processData(std::vector<float> &data)
{
    auto now = std::chrono::steady_clock::now();
    level = 0.0;
    for (int i = getMin(); i < getMax(); ++i) {
        level += data[i];
    }
    level /= (getMax() - getMin());

    samples.push(level);
    double deriv = 0.0;
    if (samples.size() == 5) {
        deriv = ((-samples[4] + 8 * samples[3] - 8 * samples[1] + samples[0]) / 12.0) * 2.0;
    }

    bool lowpeak = (level > getThresh());
    beatMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBeat).count();
    bool debounce = (beatMillis > 100);
    bool peaked = lowpeak && debounce;

    /* Do time calculation stuff */

    if (peaked) {
        peak = 1.0;
    } else {
        if (peak > 0) {
            peak -= 0.05;
        }
    }

    if (level > smoothLevel) {
        smoothLevel = level;
    } else {
        if (smoothLevel > 0) {
            smoothLevel -= 0.05;
        }
    }

    levels.push(level);

    float sum = std::accumulate(levels.begin(), levels.end(), 0.0);
    float mean = sum / levels.size();

    if (peaked) {
        lastBeat = now;
        return true;
    }
    return false;
}

void FrequencyRegion::mouseEvent(float x, float y)
{
    float rx = (x - fmod(x, 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);
    float px = -1.0 + 2 * vx;
    newInside = (px > getStart() && px < getEnd());

    newOnLine = newInside && ((1.0 - y) < getThresh() + 0.05 && (1.0 - y) > getThresh() - 0.05);
    onLine = newOnLine;

    if (mouseDown) {
        if (dragging) {
            setThresh(1.0 - y);
            emit valueChanged();
        } else if (hovering) {
            setStart(prestart + (px - dx));
            setEnd(preend + (px - dx));
        } else {
            setEnd(px);
        }
    }
}

void FrequencyRegion::mouseClick(float x, float y)
{
    mouseDown = true;
    float rx = (x - fmod(x, 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);
    float px = -1.0 + 2 * vx;
    hovering = newInside;
    dragging = onLine;
    dx = px;
    prestart = getStart();
    preend = getEnd();
    if (!dragging && !hovering) {
        setStart(px);
        setEnd(px);
        setMin(vx * step);
        setMax(vx * step);
    }
}

void FrequencyRegion::mouseReleased(float x, float y)
{
    mouseDown = false;
    float rx = (x - fmod(x, 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);

    if (dragging) {
        setThresh(1.0 - y);
    } else if(!hovering) {
        setEnd(-1.0 + 2 * vx);
        setMax(vx * step);
    }
    dragging = false;
    hovering = false;
    qDebug() << name.c_str() << " " << getMin() << " " << getMax();
}

bool FrequencyRegion::getDragging() const
{
    return dragging;
}

float FrequencyRegion::getSmoothLevel() const
{
    return smoothLevel;
}

std::chrono::time_point<std::chrono::steady_clock> FrequencyRegion::getLastBeat() const
{
    return lastBeat;
}

int FrequencyRegion::getBeatMillis() const
{
    return beatMillis;
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
    return name == "low";
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

void FrequencyRegion::setThresh(float newThresh)
{
    thresh = newThresh;
}

int FrequencyRegion::getMax() const
{
    return max > min ? max : min;
}

int FrequencyRegion::getMin() const
{
    return min < max ? min : max;
}
