#ifndef FREQUENCYREGION_H
#define FREQUENCYREGION_H

#include "helper.h"
#include "qobject.h"
#include "qobjectdefs.h"

class FrequencyRegion : public QObject {
Q_OBJECT

public:
    FrequencyRegion(int index, int min, int max, int step, std::string name = "unnamed");

    void setThresh(float newThresh);
    float getThresh() const;

    float getStart() const;
    void setStart(float newStart);

    float getEnd() const;
    void setEnd(float newEnd);

    int getScaledMin();
    int getScaledMax();

    void mouseEvent(float x, float y);
    void mouseClick(float x, float y);
    void mouseReleased(float x, float y);

    float getColor() const;
    bool processData(std::vector<float> &data);
    float getLevel() const;
    float getPeak() const;

    std::chrono::time_point<std::chrono::steady_clock> getLastBeat() const;

    int getBeatMillis() const;
    float getSmoothLevel() const;
    bool getDragging() const;

    bool getHovering() const;
    bool getNewInside() const;
    bool getNewOnLine() const;
    bool getNewOnStart() const;

    bool getNewOnEnd() const;
    int getIndex() const;
    void setIndex(int newIndex);

signals:
    void valueChanged();


private:
    float start, end, thresh;
    int min, max, step;
    std::string name;
    bool mouseDown;
    int beatMillis;
    int index;

    std::chrono::time_point<std::chrono::steady_clock> lastBeat;
    bool hovering, dragging, draggingStart, draggingEnd, onLine, inside, newInside, newOnLine, newOnStart, newOnEnd;
    float peak, level, smoothLevel;
    float dx, prestart, preend, color;
};

#endif // FREQUENCYREGION_H
