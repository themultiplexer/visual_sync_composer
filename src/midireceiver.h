#if 0
#ifndef MIDIRECEIVER_H
#define MIDIRECEIVER_H

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

    void send(int col, int row, bool release) const;
    void send(int note, bool release) const;

signals:
    void onButtonPressed(int button);
    void onButtonReleased(int button);
    void onSliderChanged(int slider, int value);
    void onKnobChanged(int slider, int value);

private:
    RtMidiIn *midiin;
    RtMidiOut *midiout;
    void mycallback(double deltatime, std::vector<unsigned char> *message, void *userData);
};

#endif // MIDIRECEIVER_H
#endif
