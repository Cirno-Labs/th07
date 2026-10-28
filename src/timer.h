// A frame counter that can run in fractions of a frame.

#pragma once

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

    // `current` before the last change.
    int previous;
    float subFrame;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
