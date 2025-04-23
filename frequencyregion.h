#ifndef FREQUENCYREGION_H
#define FREQUENCYREGION_H

#include "helper.h"
#include "qobject.h"
#include "qobjectdefs.h"

class FrequencyRegion : public QObject {
Q_OBJECT

public:
FrequencyRegion(int min, int max, int step, std::string name = "unnamed");

    int getMin() const;
    int getMax() const;

    void setThresh(float newThresh);

    float getThresh() const;

    float getStart() const;
    void setStart(float newStart);

    float getEnd() const;
    void setEnd(float newEnd);

    void setMin(int newMin);
    void setMax(int newMax);

    void mouseEvent(float x, float y);
    void mouseClick(float x, float y);
    void mouseReleased(float x, float y);


    FixedQueue<double, 5> samples;
    float peak, level, smoothLevel;
    FixedQueue<float, 1> levels;

    std::chrono::time_point<std::chrono::steady_clock> lastBeat;
    bool hovering, dragging, onLine, inside, newInside, newOnLine;

    float dx;
    float prestart, preend, color;

    float getColor() const;

    bool processData(std::vector<float> &data);
    float getLevel() const;

    float getPeak() const;

    std::chrono::time_point<std::chrono::steady_clock> getLastBeat() const;

    int getBeatMillis() const;

    float getSmoothLevel() const;

    bool getDragging() const;

signals:
    void valueChanged();

private:
    float start, end, thresh;
    int min, max, step;
    std::string name;
    bool mouseDown;
    int beatMillis;
};

#endif // FREQUENCYREGION_H
