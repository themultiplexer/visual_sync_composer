#ifndef MIDIRECEIVER_H
#define MIDIRECEIVER_H

class MidiReceiver
{
public:
    MidiReceiver();
    void start();

    bool getDone() const;
    void setDone(bool newDone);

private:
    bool done;
};

#endif // MIDIRECEIVER_H
