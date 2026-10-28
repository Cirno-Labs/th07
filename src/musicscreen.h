// The music screen: lists the game's tunes with their comments and plays
// the one picked.

#pragma once

#include <string.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"

// A tune in the music screen's list.
struct MusicTune
{
    MusicTune()
    {
        memset(this, 0, sizeof(MusicTune));
    }

    // The MIDI file; App::PlayMusic finds the wave files from it.
    char path[0x40];
    char title[0x42];
    // The comment shown for the tune, 8 lines.
    char comment[8][0x42];
};
ASSERT_SIZE(MusicTune, 0x292);

// The music screen, the static at 0x135e2a8. Runs on two chain
// elements: one updates it each frame, one draws it.
class MusicScreen
{
public:
    // The update chain element's added callback: loads the screen's
    // image, data/music00.anm and data/musiccmt.txt, the list of tunes,
    // and sets up the title and comment sprites. Returns 0, or -1 on
    // failure.
    static int AddedCallback(MusicScreen *screen);
    // The update chain element's callback: runs the current state's
    // handler and counts its frames, then steps the sprites. Returns 0
    // while a handler asks to stop, otherwise 1.
    static int OnUpdate(MusicScreen *screen);
    // The draw chain element's callback: draws the list, with an arrow
    // at the tune under the cursor, and the comment. Returns 1.
    static int OnDraw(MusicScreen *screen);
    // State 0: on its first frame, sends interrupt 1 to the title under
    // the cursor and the comment lines and 2 to the other titles; after
    // 8 frames moves to state 1. Returns 0.
    int UpdateIntro();
    // State 1: the list of tunes. Up and down move the cursor, sending
    // interrupt 1 to the title under it and 2 to the others; shot or
    // enter plays the tune and shows its comment. Returns nonzero to
    // stop.
    int UpdateList();

    ChainElem *calcChain;
    ChainElem *drawChain;
    // Frames spent in `state`.
    int stateFrames;
    int state;
    // The tune under the cursor.
    int cursor;
    // The tune playing.
    int playing;
    // The first tune the list shows; it shows 10.
    int listTop;
    int tuneCount;
    MusicTune *tunes;
    // An array of one: it is constructed in a loop.
    Sprite unk_0024[1];
    // One per tune in the list.
    Sprite titles[0x1f];
    // One per line of the selected tune's comment.
    Sprite comment[8];
};
ASSERT_SIZE(MusicScreen, 0x5c04);
