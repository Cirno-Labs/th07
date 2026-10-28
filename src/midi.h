// MIDI playback through WinMM.

#pragma once

#include <windows.h>
#include <mmsystem.h>

#include "common.h"

// Wraps a MIDI output device (midiOutOpen and friends).
class MidiDevice
{
public:
    MidiDevice();
    ~MidiDevice();

    // Returns -1 if no device is open, 0 once it is closed.
    int Close();

    HMIDIOUT handle;
    UINT deviceId;
};
