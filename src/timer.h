// A frame counter that can run in fractions of a frame.

#pragma once

#include <windows.h>

struct Timer
{
    // Sets the count to `frames`, with no fraction and no previous count
    // (-999).
    void Set(int frames)
    {
        this->current = frames;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    // Advances by one frame through App::TickTimer. Inline; defined in
    // unsorted.h.
    void Tick();

    // The count with its fraction.
    float AsFloat()
    {
        return (float)this->current + this->subFrame;
    }
    BOOL operator<(int frames)
    {
        return this->current < frames;
    }
    BOOL operator>=(int frames)
    {
        return this->current >= frames;
    }

    // `current` before the last change.
    int previous;
    float subFrame;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
