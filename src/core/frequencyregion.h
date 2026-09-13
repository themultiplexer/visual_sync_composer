#ifndef FREQUENCYREGION_H
#define FREQUENCYREGION_H

#include "core/helper.h"
#include "qobject.h"
#include "qobjectdefs.h"

class FrequencyRegion : public QObject {
Q_OBJECT

public:
    FrequencyRegion(int index, float min, float max, int step, std::string name = "unnamed");

    void setThresh(float newThresh);
    float getThresh() const;

    float getStart() const;
    void setStart(float newStart);

    float getEnd() const;
    void setEnd(float newEnd);

    int getScaledMin();
    int getScaledMax();

    float getColor() const;
    int getStep() const;
    bool processData(std::array<float, 1024> &data);

    float getLevel() const;
    float getPeak() const;

    int getBeatMillis() const;

    int getIndex() const;
    void setIndex(int newIndex);

    std::string getName() const;

signals:
    void valueChanged();

private:
    float start, end, thresh;
    int min, max, step;
    std::string name;
    int beatMillis;
    int index;

    std::chrono::time_point<std::chrono::steady_clock> lastBeat;
    float peak, level;
    float dx, prestart, preend, color;
};

#endif // FREQUENCYREGION_H
