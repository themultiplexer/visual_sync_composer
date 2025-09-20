#include "midireceiver.h"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <thread>
#include <vector>
#include <RtMidi.h>

static bool done;

static void finish(int ignore){ done = true; }

MidiReceiver::MidiReceiver() : midiin(nullptr) {

}

static void static_callback(double deltatime, std::vector< unsigned char > *message, void *userData) {
    return static_cast<MidiReceiver*>(userData)->mycallback(deltatime, message);
}

void MidiReceiver::mycallback( double deltatime, std::vector< unsigned char > *message)
//void MidiReceiver::mycallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
    unsigned int nBytes = message->size();

    if (nBytes > 2) {
        if ((int)message->at(0) == 144) {
            emit onButtonPressed((int)message->at(1) - 36);
        }
        if ((int)message->at(0) == 176) {
            emit onSliderChanged((int)message->at(1) - 5,(int)message->at(2) * 2);
        }
    }

    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << deltatime << std::endl;

}

void MidiReceiver::start() {
    midiin = new RtMidiIn();

    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if ( nPorts == 0 ) {
        std::cout << "No ports available!\n";
        return;
    }

    midiin->openPort( 0 );

    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue.
    //midiin->setCallback( std::bind(&MidiReceiver::mycallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) );
    midiin->setCallback(static_callback, this);

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes( false, false, false );

    std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);
}

bool MidiReceiver::getDone() const
{
    return done;
}

void MidiReceiver::setDone(bool newDone)
{
    done = newDone;
}

MidiReceiver::~MidiReceiver()
{
    delete midiin;
}
