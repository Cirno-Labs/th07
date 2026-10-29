// The title menu ("bgm/th07_01.mid"). Only the fields known so far.

#pragma once

#include <windows.h>

#include "chain.h"
#include "common.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

// The 0x38 bytes of App's config, from controllerMapping, as one block
// to copy.
struct ConfigCopy
{
    unsigned char unk_0[0x1e];
    BYTE colorMode16Bits;
    unsigned char unk_1f[0x3];
    BYTE windowed;
    unsigned char unk_23[0x15];
};
ASSERT_SIZE(ConfigCopy, 0x38);

class MainMenu
{
public:
    // Constructs the sprites, then zeroes the menu. Inline, but too large
    // to expand: SetUpTitleScene calls the copy at 0x45547d.
    MainMenu()
    {
        memset(this, 0, sizeof(MainMenu));
    }

    // The update element's callback: runs the state's function, then
    // steps the sprites.
    static int OnUpdate(MainMenu *menu);
    // The draw element's callback: draws the background, the replay
    // menu's or the practice scores' text, then the sprites. Returns 1.
    static int OnDraw(MainMenu *menu);
    // The update element's added callback: runs InitTitleScene.
    static int AddedCallback(MainMenu *menu);
    // The update element's deleted callback: releases the title's ANM
    // slots (0x20 to 0x29) and surface 0, removes the draw element,
    // frees the menu's buffers and the menu. Returns 0.
    static int DeletedCallback(MainMenu *menu);
    // The body of the added callback: resets the game's numbers, reloads the score file, shows the
    // Phantasm splash once it opens, puts the cursor on the entry the
    // previous scene came from, loads "data/title01.anm" and the title
    // music. Returns 0, or -1 if the ANM fails to load.
    int InitTitleScene();

    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters. Not in th06 as a
    // function. Inline (defined in mainmenu.cpp): StartTitle expands it,
    // the larger state functions have spent their inline budget and call
    // the copy at 0x455435.
    void SetState(int state);
    // The state functions OnUpdate runs. Not in th06.
    int StartTitle();
    int UpdateOptions();
    int UpdateKeyConfig();
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
    // Frees loadedReplay and the sprites. Returns 0. Not in th06.
    int ReleaseTitleScene();
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
    // The cursor the title menu last highlighted.
    int prevCursor;
    int subState;
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
    Sprite *helpSprite;
    Sprite helpSprites[0xe];
    int spriteCount;
    // The menu's state.
    int state;
    int inputLockout;
    // The frames the title menu has waited without input; the demo
    // starts after 900.
    int idleFrames;
    // The update element (priority 3, with the added and deleted
    // callbacks) and the draw element (priority 0), created by
    // SetUpTitleScene.
    ChainElem *calcChain;
    ChainElem *drawChain;
    // The button mapping the options screen edits.
    ControllerMapping controllerMapping;
    unsigned char unk_d11e[0x2];
    // A copy of g_App's config, taken as StartTitle enters state 2.
    ConfigCopy savedConfig;
};
ASSERT_SIZE(MainMenu, 0xd158);

// Creates the title menu and adds its chain elements. Returns 0, or -1
// if the update element cannot be added. `unk` is unused.
int SetUpTitleScene(int unk);

// The help line the title menu shows for each entry.
extern char *g_TitleHelpTexts[8];
// The help line the options screen shows for each entry.
extern char *g_OptionsHelpTexts[9];
// The help line the key config shows for each entry.
extern char *g_KeyConfigHelpTexts[12];
// The button the key config saw pressed last frame, so that a held
// button is assigned once.
extern short g_LastPressedKey;
// The button mapping the key config's reset entry restores.
extern ControllerMapping g_DefaultControllerMapping;
// The demo replays the title menu plays in turn.
extern char *g_DemoReplayPaths[3];
// The names the replay menu shows for stages (then Extra and Phantasm),
// difficulties and shot types.
extern char *g_MenuStageNames[8];
extern char *g_MenuDifficultyNames[6];
extern char *g_MenuShotTypeNames[6];
// The stage names the practice scores show.
extern char *g_PracticeStageNames[6];
