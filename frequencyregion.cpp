#include "frequencyregion.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <cmath>

FrequencyRegion::FrequencyRegion(int min, int max, int step, std::string name):
    step(step), min(min), max(max), start(-1.0 + 2 *((float)min/(float)step)), end(-1.0 + 2 *((float)max/(float)step)), name(name) {

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

void FrequencyRegion::mouseEvent(float x, float y)
{
    float rx = (x - fmod(x, 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);
    float px = -1.0 + 2 * vx;
    float py = -1.0 + 2 * y;
    newInside = (px > getStart() && px < getEnd());

    newOnLine = newInside && (py < getThresh() + 0.05 && py > getThresh() - 0.05);
    onLine = newOnLine;

    if (mouseDown) {
        if (dragging) {
            setThresh(y);
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
    float px = -1.0 + 2 * vx;
    if (dragging) {
        setThresh(y);
    } else if(!hovering) {
        setEnd(-1.0 + 2 * vx);
        setMax(vx * step);
    }
    dragging = false;
    hovering = false;
    qDebug() << name.c_str() << " " << getMin() << " " << getMax();
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
