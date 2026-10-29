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

    unsigned char unk_0[0xd4];
    // The random seed at the start of the frame.
    WORD unk_d4;
    // Bits set during a frame: 2 when the player is hit.
    WORD unk_d6;
};
extern ReplayManager *g_ReplayManager;
