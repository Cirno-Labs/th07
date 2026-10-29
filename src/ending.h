// The ending and staff roll ("data/end00.end"): a text script of lines
// shown one at a time, with commands after '@', as th06's.

#pragma once

#include <windows.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

class Ending
{
public:
    // Reads a number at the cursor and moves the cursor past it and the
    // NULs after it.
    int ReadInt();

    ChainElem *calcChain;
    ChainElem *drawChain;
    // Where the background image (surface 0) is copied from: "@v" sets
    // the y, which "@V" scrolls back to 0 by its speed per frame.
    float scrollX;
    float scrollY;
    float scrollSpeed;
    // The text lines and the "@a" pictures.
    Sprite sprites[0x10];
    // The script file.
    char *file;
    // Set once the ending has been seen: the skip button (0x100) then
    // runs it up to 5 times as fast.
    int canSkip;
    Timer timer;
    // The wait after a line ("@w" or the line delay), and "@r"'s wait,
    // which clears the text when it ends. A press of the shot button ends
    // them, once their minimum frames have passed.
    Timer lineTimer;
    Timer waitTimer;
    int waitMinFrames;
    int lineMinFrames;
    // The frames to wait after a line ("@s"), and while the shot button is
    // held.
    int lineDelay;
    int lineDelayFast;
    unsigned char unk_2510[0x4];
    // The line the text goes to.
    int line;
    // "@c": the text color.
    D3DCOLOR textColor;
    // "@0" to "@3": fades in and out of black and white.
    D3DCOLOR fadeColor;
    int fadeTimer;
    int fadeDuration;
    int fadeType;
    // The script position.
    char *cursor;
};
ASSERT_SIZE(Ending, 0x2530);
