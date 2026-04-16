#if 0

#include "midireceiver.h"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <thread>
#include <vector>
#include <RtMidi.h>

// Matrix button MIDI note mapping (4x4 grid → Notes 36-51)
const int MIDI_NOTE_MATRIX_BASE = 36;  // Starting note for matrix (1,1)

// MIDI channel (0-based, so channel 1 = 0)
const int MIDI_CHANNEL = 0;

// MIDI message types
const unsigned char MIDI_NOTE_ON = 0x90;   // Note On message
const unsigned char MIDI_NOTE_OFF = 0x80;  // Note Off message
const unsigned char MIDI_CC = 0xB0;        // Control Change message
const unsigned char MIDI_VELOCITY_ON = 127; // Full velocity for button press
const unsigned char MIDI_VELOCITY_OFF = 0;  // Zero velocity for button release

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

    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << deltatime << std::endl;


    if (nBytes > 2) {
        if ((int)message->at(0) == 144) {
            emit onButtonPressed((int)message->at(1) - 36);
        }
        if ((int)message->at(0) == 128) {
            emit onButtonReleased((int)message->at(1) - 36);
        }
        if ((int)message->at(1) >= 5 && (int)message->at(1) <= 8) {
            emit onSliderChanged((int)message->at(1) - 5,(int)message->at(2) * 2);
        }
        if ((int)message->at(1) >= 1 && (int)message->at(1) <= 4) {
            emit onKnobChanged((int)message->at(1) - 1,(int)message->at(2) * 2);
        }
    }
}

void MidiReceiver::start() {
    midiin = new RtMidiIn();
    midiout = new RtMidiOut();

    // Find ports by name
    for (unsigned int i = 0; i < midiin->getPortCount(); ++i) {
        if (midiin->getPortName(i).find("F1_Controller_Out") != std::string::npos) {
            midiin->openPort(i);
        }
    }

    for (unsigned int i = 0; i < midiout->getPortCount(); ++i) {
        if (midiout->getPortName(i).find("F1_Controller_In") != std::string::npos) {
            midiout->openPort(i);
        }
    }

    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if ( nPorts == 0 ) {
        std::cout << "No ports available!\n";
        return;
    }


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

void MidiReceiver::send(int col, int row, bool release) const
{
    send(MIDI_NOTE_MATRIX_BASE + (row * 4) + col, release);
}

void MidiReceiver::send(int note, bool release) const
{
    std::vector<unsigned char> message(3);

    message[0] = (release ? MIDI_NOTE_OFF : MIDI_NOTE_ON) + MIDI_CHANNEL;           // Note On + channel
    message[1] = note;                                                              // Note number
    message[2] = MIDI_VELOCITY_ON;                                                  // Velocity (127)

    midiout->sendMessage(&message);
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

#endif
