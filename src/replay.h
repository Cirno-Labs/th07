// Replays ("./replay/th7_%.2d.rpy"). Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// A stage of a replay. Only the fields known so far.
struct ReplayStage
{
    // The score the stage started with, divided by 10.
    int score;
};

// One frame of a replay.
struct ReplayFrame
{
    // The buttons held.
    WORD input;
    // ReplayManager::unk_d6 for the frame.
    WORD events;
};

// The start of a replay file. Only the fields known so far.
struct ReplayHeader
{
    // "T7RP".
    char magic[4];
    // 0x100 in its low 12 bits.
    WORD version;
    BYTE unk_06[0x16];
    // Indexed by stage. In the file, the offsets of the stages' data.
    ReplayStage *stages[7];
    BYTE unk_38[0x1e];
    // character * 2 + shot type.
    BYTE shotType;
    BYTE difficulty;
    // The date it was saved, "%m/%d".
    char date[6];
    // The player's name.
    char name[0xe];
    DWORD score;
    BYTE unk_70[0x5c];
    // How far the game ran below full speed, in percent.
    float slowdown;
    BYTE unk_d0[0x18];
};
ASSERT_SIZE(ReplayHeader, 0xe8);

// Checks and decodes the replay file `data` of `size` bytes. Returns a
// new copy, or NULL if it is not a valid replay.
ReplayHeader *ValidateReplay(void *data, int size);
// Saves the replay being recorded to `path`, under `name`.
void SaveReplay(char *path, char *name);

// Records and plays back replays, behind the pointer at 0x4b9e48. Only
// the fields known so far.
class ReplayManager
{
public:
    // The second update callback while recording, run before the game's
    // own: clears unk_d6 (marking 0x100 when Unknown626270::unk_93d0 is
    // set, which it clears), keeps the random seed in unk_d4 and clears
    // the count of random numbers. Returns 1. Not in th06.
    static int RecordOnUpdate2(ReplayManager *replay);
    // The update callback while recording: while bit 2 of
    // Unknown626270::unk_93d8 is set, plays by g_Buttons and, unless the
    // config's unk_25 or bit 3 of App::unk_18c is set, records them with
    // unk_d6, and every 30 frames the frame rate (0x80 set while
    // App::unk_2a4 is), then counts the frame. Returns 1.
    static int RecordOnUpdate(ReplayManager *replay);
    // The second update callback while playing: while bit 2 of
    // Unknown626270::unk_93d8 is set, returns 6 on two frames of every
    // three while g_Unknown49fbf0's Func42ad66 and Func427be2 hold, and
    // on four of every five while unk_93df is 2 and
    // g_Unknown9a9b00.Func4232a0() does not; otherwise 1.
    static int PlayOnUpdate2(ReplayManager *replay);
    // The update callback while playing: while bit 2 of
    // Unknown626270::unk_93d8 is set and the config's unk_25 is not,
    // plays the recorded buttons for the frame, with key repeat as in
    // App::OnUpdate, and every 30 frames puts the recorded frame rate
    // back into App::unk_188 and unk_2a8, then counts the frame.
    // Returns 1.
    static int PlayOnUpdate(ReplayManager *replay);

    // The current frame.
    int frame;
    unsigned char unk_4[0x80];
    // The last frame recorded or played; [stage - 1] is where that
    // stage's frames end (stages past 7 share the last).
    ReplayFrame *cursor;
    ReplayFrame *stageEnds[7];
    // The same for the frame rates kept every 30 frames.
    BYTE *extraCursor;
    BYTE *extraEnds[7];
    unsigned char unk_c4[0x10];
    // The random seed at the start of the frame.
    WORD unk_d4;
    // Bits set during a frame: 2 when the player is hit.
    WORD unk_d6;
};
extern ReplayManager *g_ReplayManager;
