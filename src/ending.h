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
    // The update chain element's callback: runs the script and steps
    // the sprites, up to 5 times a frame while skipping. Returns 0 at the
    // script's end, otherwise 1.
    static int OnUpdate(Ending *ending);
    // The draw chain element's callback: draws the background, the
    // sprites and the fade. Returns 1.
    static int OnDraw(Ending *ending);
    // Reads a number at the cursor and moves the cursor past it and the
    // NULs after it.
    int ReadInt();
    // Steps the fade and draws it over the screen.
    void DrawFade();
    // Runs the script until it waits: shows text lines, runs the '@'
    // commands. Returns 0, or -1 at its end ("@z") or on failure.
    int RunScript();
    // Loads the script at `path` and starts it. Returns 0, or -1 after
    // logging an error.
    int Load(char *path);
    // The update chain element's added callback: marks the game cleared,
    // loads data/staff01.anm, records the clear and loads the shot
    // type's ending (a bad ending after a continue). Returns 0, or -1 on
    // failure.
    static int AddedCallback(Ending *ending);
    // The update chain element's deleted callback: frees what the ending
    // loaded, removes the draw chain element and frees the ending.
    static int DeletedCallback(Ending *ending);

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

// The endings' scripts, by shot type (character * 2 + shot), and the
// bad endings after a continue, by character.
extern char *g_EndingFiles[];
extern char *g_BadEndingFiles[];
