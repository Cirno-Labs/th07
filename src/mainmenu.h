// The title menu ("bgm/th07_01.mid"). Only the fields known so far.

#pragma once

#include <windows.h>

#include "common.h"

class MainMenu
{
public:
    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters. Not in th06 as a
    // function.
    void SetState(int state);

    unsigned char unk_0[0x8];
    int unk_8;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char unk_10[0x54];
    // The state before the current one.
    int prevState;
    unsigned char unk_68[0xb058];
    int unk_b0c0;
    unsigned char unk_b0c4[0x2034];
    // The menu's state.
    int state;
    int unk_d0fc;
    unsigned char unk_d100[0x58];
};
ASSERT_SIZE(MainMenu, 0xd158);
