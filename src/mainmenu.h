// The title menu ("bgm/th07_01.mid"). Only the fields known so far.

#pragma once

#include <windows.h>

#include "common.h"
#include "sprite.h"

class MainMenu
{
public:
    // The update element's callback: runs the state's function, then
    // steps the sprites.
    static int OnUpdate(MainMenu *menu);

    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters. Not in th06 as a
    // function.
    void SetState(int state);
    // The state functions OnUpdate runs. Not in th06.
    int Func4555dd();
    int Func45624d();
    int Func456f6b();
    int Func45798b();
    int Func457fe5();
    int Func459518();
    int Func45a1dd();
    int Func45a924();

    unsigned char unk_0[0x8];
    int unk_8;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char unk_10[0x54];
    // The state before the current one.
    int prevState;
    unsigned char unk_68[0xb058];
    int unk_b0c0;
    // The menu's sprites, spriteCount of them.
    Sprite *sprites;
    Sprite *unk_b0c8;
    unsigned char unk_b0cc[0x2028];
    int spriteCount;
    // The menu's state.
    int state;
    int unk_d0fc;
    unsigned char unk_d100[0x58];
};
ASSERT_SIZE(MainMenu, 0xd158);
