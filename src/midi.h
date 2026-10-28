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

    // Opens the given device, closing another open one first. Returns
    // nonzero on failure, 0 on success or if the device is already open.
    int Open(UINT deviceId);

    // Prepares and sends a system-exclusive message. Returns 0 if no
    // device is open, nonzero if either step fails.
    int SendLongMsg(LPMIDIHDR header);

    HMIDIOUT handle;
    UINT deviceId;
};
