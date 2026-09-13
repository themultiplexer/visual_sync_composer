#include "glfrequencyregion.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <cmath>

GLFrequencyRegion::GLFrequencyRegion(FrequencyRegion* region, int step):
    region(region), step(step), level(0.0), peak(0.0), mouseDown(false), dragging(false), hovering(false), inside(false), newInside(false), newOnLine(false) {

}

void GLFrequencyRegion::mouseEvent(float x, float y)
{
    int base = 20;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), 0.0);
    newInside = (vx > region->getStart() && vx < region->getEnd());

    newOnLine = newInside && ((1.0 - y) < region->getThresh() + 0.05 && (1.0 - y) > region->getThresh() - 0.05);
    newOnStart = (x < region->getStart() + 0.01 && x > region->getStart() - 0.01);
    newOnEnd = (x < region->getEnd() + 0.01 && x > region->getEnd() - 0.01);

    onLine = newOnLine;

    if (mouseDown) {
        if (dragging) {
            region->setThresh(1.0 - y);
            emit valueChanged();
        } else if (draggingStart) {
            float newx = prestart + (rx - dx);
            region->setStart(newx);
        } else if (draggingEnd) {
            float newy = preend + (rx - dx);
            region->setEnd(newy);
        } else if (hovering) {
            float newx = prestart + (rx - dx);
            float newy = preend + (rx - dx);
            region->setStart(newx);
            region->setEnd(newy);
        } else {
            region->setEnd(rx);
        }
    }
}

void GLFrequencyRegion::mouseClick(float x, float y) {
    int base = 20;
    mouseDown = true;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));

    hovering = newInside;
    draggingStart = newOnStart;
    draggingEnd = newOnEnd;
    dragging = onLine;
    dx = rx;
    prestart = region->getStart();
    preend = region->getEnd();
    if (!dragging && !hovering) {
        region->setStart(rx);
        region->setEnd(rx);
    }
}

void GLFrequencyRegion::mouseReleased(float x, float y)
{
    mouseDown = false;
    float rx = g(f(x) - fmod(f(x), 1.0/(float)step));
    float vx = std::fmax(std::fmin(rx, 1.0), -1.0);

    if (dragging) {
        region->setThresh(1.0 - y);
    } else if(!hovering) {
        region->setEnd(vx);
    }
    dragging = false;
    hovering = false;
}

bool GLFrequencyRegion::getDragging() const
{
    return dragging;
}

bool GLFrequencyRegion::getHovering() const
{
    return hovering;
}

bool GLFrequencyRegion::getNewInside() const
{
    return newInside;
}

bool GLFrequencyRegion::getNewOnLine() const
{
    return newOnLine;
}

bool GLFrequencyRegion::getNewOnStart() const
{
    return newOnStart;
}

bool GLFrequencyRegion::getNewOnEnd() const
{
    return newOnEnd;
}

int GLFrequencyRegion::getBeatMillis() const
{
    return beatMillis;
}

float GLFrequencyRegion::getPeak() const
{
    return peak;
}
