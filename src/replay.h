// Replays ("./replay/th7_%.2d.rpy"). Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// The start of a replay file. Only the fields known so far.
struct ReplayHeader
{
    // "T7RP".
    char magic[4];
    // 0x100 in its low 12 bits.
    WORD version;
    BYTE unk_06[0x52];
    // The date it was saved, "%m/%d".
    char date[6];
    BYTE unk_5e[0xe];
    DWORD score;
    BYTE unk_70[0x78];
};
ASSERT_SIZE(ReplayHeader, 0xe8);

// Checks and decodes the replay file `data` of `size` bytes. Returns a
// new copy, or NULL if it is not a valid replay.
ReplayHeader *ValidateReplay(void *data, int size);
// Saves the replay being recorded to `path`, under `name`.
void SaveReplay(char *path, char *name);
