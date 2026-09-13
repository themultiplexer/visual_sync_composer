#ifndef GLFREQUENCYREGION_H
#define GLFREQUENCYREGION_H

#include "core/frequencyregion.h"
#include "qobject.h"
#include "qobjectdefs.h"

class GLFrequencyRegion : public QObject {
Q_OBJECT

public:
    GLFrequencyRegion(FrequencyRegion* region, int step);

    void mouseEvent(float x, float y);
    void mouseClick(float x, float y);
    void mouseReleased(float x, float y);

    float getColor() const;
    float getLevel() const;
    float getPeak() const;

    int getBeatMillis() const;
    bool getDragging() const;

    bool getHovering() const;
    bool getNewInside() const;
    bool getNewOnLine() const;
    bool getNewOnStart() const;
    bool getNewOnEnd() const;

    FrequencyRegion *region;

signals:
    void valueChanged();


private:
    bool mouseDown;
    int beatMillis, step;

    bool hovering, dragging, draggingStart, draggingEnd, onLine, inside, newInside, newOnLine, newOnStart, newOnEnd;
    float peak, level;
    float dx, prestart, preend, color;
};

#endif // GLFREQUENCYREGION_H
