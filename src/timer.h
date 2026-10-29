// A frame counter that can run in fractions of a frame.

#pragma once

#include <windows.h>

struct Timer
{
    // Starts counting from 0.
    Timer()
    {
        this->current = 0;
        this->previous = -999;
        this->subFrame = 0.0f;
    }

    // Starts counting from 0 again, as the constructor.
    void Initialize()
    {
        this->current = 0;
        this->previous = -999;
        this->subFrame = 0.0f;
    }

    // Sets the count to `frames`, with no fraction and no previous count
    // (-999).
    void Set(int frames)
    {
        this->current = frames;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    // Advances by `frames`, in fractions of a frame while the game runs
    // slow; first by one more frame, with no fraction or previous count,
    // while bit 0x20 of g_App.unk_18c is set.
    void Add(int frames);
    // The same backwards.
    void Sub(int frames);
    // Advances by one frame through App::TickTimer. Inline; defined in
    // unsorted.h.
    void Tick();
    // Tick, returning the new count. Out of line in ascii.cpp, as th06's
    // in its Gui file.
    int NextTick();
    void operator++()
    {
        this->NextTick();
    }

    // The count with its fraction.
    float AsFloat()
    {
        return (float)this->current + this->subFrame;
    }
    // The count without its fraction.
    int AsFrames()
    {
        return this->current;
    }
    // Whether the count changed since the last Tick or Set.
    BOOL HasTicked()
    {
        return this->current != this->previous;
    }
    BOOL operator==(int frames)
    {
        return this->current == frames;
    }
    BOOL operator<(int frames)
    {
        return this->current < frames;
    }
    BOOL operator>(int frames)
    {
        return this->current > frames;
    }
    BOOL operator>=(int frames)
    {
        return this->current >= frames;
    }
    BOOL operator<=(int frames)
    {
        return this->current <= frames;
    }

    // `current` before the last change.
    int previous;
    float subFrame;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
