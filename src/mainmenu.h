// The title menu ("bgm/th07_01.mid"). Only the fields known so far.

#pragma once

#include <windows.h>

#include "common.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

class MainMenu
{
public:
    // The update element's callback: runs the state's function, then
    // steps the sprites.
    static int OnUpdate(MainMenu *menu);
    // The draw element's callback: draws the background, the replay
    // menu's or the practice scores' text, then the sprites. Returns 1.
    static int OnDraw(MainMenu *menu);

    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters. Not in th06 as a
    // function.
    void SetState(int state);
    // The state functions OnUpdate runs. Not in th06.
    int Func4555dd();
    int Func45624d();
    int Func456f6b();
    int UpdateDifficultyMenu();
    int UpdateCharacterMenu();
    int UpdateShotTypeMenu();
    int UpdateGameStartMenu();
    // The replay menu (state 7): lists the replays in "./replay" (the
    // numbered slots, then up to 45 "th7_ud????.rpy" files), then lets
    // the player pick one, one of its stages and an option, and starts
    // it (scene 2, returns 0). Returns 1 otherwise.
    int UpdateReplayMenu();
    // In the replay menu, lists the page of 15 replays around
    // replayCursor, the one at the cursor highlighted, and in sub-states
    // 2 and 3 the picked replay's slowdown and the score each stage
    // started with. Returns 1.
    int DrawReplayMenu();
    // Lists the practice high scores (and play counts) of the six
    // stages for the shot type and difficulty, the one at the cursor
    // highlighted and those not reached yet dark. Returns 1.
    int DrawPracticeScores();
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
    // MoveMenuCursor with left and right (the sound after the wrap).
    // Not in th06.
    int MoveMenuCursorLR(int count);

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
    // The replay menu's entries: their files' paths, the numbers shown
    // for them, and the replays' headers.
    char replayPaths[60][0x200];
    char replayLabels[60][8];
    ReplayHeader replays[60];
    // The replay picked in the replay menu.
    ReplayHeader *loadedReplay;
    int unk_b0b0;
    // The number of replays listed, the one at the cursor and the
    // stage picked from it.
    int replayCount;
    int replayCursor;
    int replayStage;
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

// The names the replay menu shows for stages (then Extra and Phantasm),
// difficulties and shot types.
extern char *g_MenuStageNames[8];
extern char *g_MenuDifficultyNames[6];
extern char *g_MenuShotTypeNames[6];
// The stage names the practice scores show.
extern char *g_PracticeStageNames[6];
