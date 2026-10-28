// The music screen: lists the game's tunes with their comments and plays
// the one picked.

#pragma once

#include "chain.h"
#include "common.h"
#include "sprite.h"

// A tune in the music screen's list.
struct MusicTune
{
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
    // State 0: on its first frame, sends interrupt 1 to the title under
    // the cursor and the comment lines and 2 to the other titles; after
    // 8 frames moves to state 1. Returns 0.
    int UpdateIntro();

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
