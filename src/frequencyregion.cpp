#include "frequencyregion.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <cmath>

FrequencyRegion::FrequencyRegion(int min, int max, int step, std::string name):
    step(step), level(0.0), thresh(0.7), peak(0.0), mouseDown(false), dragging(false), hovering(false), inside(false), newInside(false), newOnLine(false),  smoothLevel(0.0), start(f((float)min/(float)step)), end(f((float)max/(float)step)), name(name) {

}

float FrequencyRegion::getEnd() const
{
    return end > start ? end : start;
}

void FrequencyRegion::setEnd(float newEnd)
{
    end = std::fmax(std::fmin(newEnd, 1.0), -1.0);
}

int FrequencyRegion::getScaledMin() {
    // Warning this is also used in the shader
    float x = start < end ? start : end;
    return f(x) * step;
}

int FrequencyRegion::getScaledMax() {
    // Warning this is also used in the shader
    float x = start < end ? end : start;
    return f(x) * step;
}

bool FrequencyRegion::processData(std::vector<float> &data)
{
    auto now = std::chrono::steady_clock::now();
    level = 0.0;
    for (int i = getScaledMin(); i < getScaledMax(); ++i) {
        //level += data[i];
        level = data[i] > level ? data[i] : level;
    }
    //level /= (getMax() - getMin());

    bool lowpeak = (level > getThresh());
    beatMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBeat).count();
    bool debounce = (beatMillis > 100);
    bool peaked = lowpeak && debounce;

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

    if (peaked) {
        lastBeat = now;
        return true;
    }
    return false;
}

void FrequencyRegion::mouseEvent(float x, float y)
{
    int base = 20;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), 0.0);
    newInside = (vx > getStart() && vx < getEnd());

    newOnLine = newInside && ((1.0 - y) < getThresh() + 0.05 && (1.0 - y) > getThresh() - 0.05);
    newOnStart = (x < getStart() + 0.01 && x > getStart() - 0.01);
    newOnEnd = (x < getEnd() + 0.01 && x > getEnd() - 0.01);

    onLine = newOnLine;

    if (mouseDown) {
        if (dragging) {
            setThresh(1.0 - y);
            emit valueChanged();
        } else if (draggingStart) {
            float newx = prestart + (rx - dx);
            setStart(newx);
        } else if (draggingEnd) {
            float newy = preend + (rx - dx);
            setEnd(newy);
        } else if (hovering) {
            float newx = prestart + (rx - dx);
            float newy = preend + (rx - dx);
            setStart(newx);
            setEnd(newy);
        } else {
            setEnd(rx);
        }
    }
}

void FrequencyRegion::mouseClick(float x, float y) {
    int base = 20;
    mouseDown = true;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));

    hovering = newInside;
    draggingStart = newOnStart;
    draggingEnd = newOnEnd;
    dragging = onLine;
    dx = rx;
    prestart = getStart();
    preend = getEnd();
    if (!dragging && !hovering) {
        setStart(rx);
        setEnd(rx);
    }
}

void FrequencyRegion::mouseReleased(float x, float y)
{
    mouseDown = false;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);

    if (dragging) {
        setThresh(1.0 - y);
    } else if(!hovering) {
        setEnd(vx);
    }
    dragging = false;
    hovering = false;
    qDebug() << name.c_str() << " " << getScaledMin() << " " << getScaledMax() << getScaledMax() - getScaledMin();
}

bool FrequencyRegion::getDragging() const
{
    return dragging;
}

bool FrequencyRegion::getHovering() const
{
    return hovering;
}

bool FrequencyRegion::getNewInside() const
{
    return newInside;
}

bool FrequencyRegion::getNewOnLine() const
{
    return newOnLine;
}

bool FrequencyRegion::getNewOnStart() const
{
    return newOnStart;
}

bool FrequencyRegion::getNewOnEnd() const
{
    return newOnEnd;
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
