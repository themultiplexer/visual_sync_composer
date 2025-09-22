#ifndef MIDIRECEIVER_H
#define MIDIRECEIVER_H

#include "RtMidi.h"
#include <qobject.h>
#include <qwindowdefs.h>


class MidiReceiver : public QObject
{
Q_OBJECT

public:
    MidiReceiver();
    ~MidiReceiver();
    void start();

    bool getDone() const;
    void setDone(bool newDone);
    void mycallback(double deltatime, std::vector<unsigned char> *message);

signals:
    void onButtonPressed(int button);
    void onSliderChanged(int slider, int value);
    void onKnobChanged(int slider, int value);

private:
    RtMidiIn *midiin;
    void mycallback(double deltatime, std::vector<unsigned char> *message, void *userData);
};

#endif // MIDIRECEIVER_H
