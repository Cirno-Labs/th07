// The title menu ("bgm/th07_01.mid"). Only the fields known so far.

#pragma once

#include <windows.h>

#include "common.h"
#include "sprite.h"
#include "unsorted.h"

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
    int UpdateDifficultyMenu();
    int Func457fe5();
    int Func459518();
    int Func45a1dd();
    int Func45a924();
    // Gives every button of controllerMapping assigned `button` the
    // value `replacement`. Unlike th06's, `unk` is unused.
    void ReassignButton(short button, short replacement, int unk);
    // Shows `value` as two digits on `vm` and the sprite after it, or
    // hides both when it is negative. Returns 0. Not in th06.
    int ShowButtonDigits(Sprite *vm, short value);
    // Moves the cursor up or down among `count` entries, wrapping, when
    // up or down is pressed or repeats. Returns -1 for up, 1 for down,
    // else 0.
    int MoveMenuCursor(int count);

    // The selected entry.
    int cursor;
    unsigned char unk_4[0x4];
    int unk_8;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char unk_10[0x54];
    // The state before the current one.
    int prevState;
    int unk_68;
    unsigned char unk_6c[0xb054];
    int unk_b0c0;
    // The menu's sprites, spriteCount of them.
    Sprite *sprites;
    Sprite *unk_b0c8;
    unsigned char unk_b0cc[0x2028];
    int spriteCount;
    // The menu's state.
    int state;
    int unk_d0fc;
    unsigned char unk_d100[0xc];
    // The button mapping the options screen edits.
    ControllerMapping controllerMapping;
    unsigned char unk_d11e[0x3a];
};
ASSERT_SIZE(MainMenu, 0xd158);
