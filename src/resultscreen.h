// The result screen: the score file ("score.dat") and its high score
// lists.

#pragma once

#include <string.h>
#include <windows.h>

#include "common.h"

// A practice score chunk of the score file ("PSCR").
struct Pscr
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    DWORD score;
    BYTE unk_10[0x4];
    BYTE shot;
    BYTE difficulty;
    BYTE stage;
    BYTE unk_17;
};
ASSERT_SIZE(Pscr, 0x18);

// A chunk of the score file ("PLST"), new in th07.
struct Plst
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x157];
};
ASSERT_SIZE(Plst, 0x160);

// A node of the score file's lists. Each list starts with a head node
// whose `data` is NULL; the entries after it are sorted.
struct ListNode
{
    ListNode *prev;
    ListNode *next;
    struct Hscr *data;
};

// Frees every node after `head`.
void FreeListNodes(ListNode *head);

// "score.dat" as loaded: a 0x14-byte header, then chunks.
struct ScoreDat
{
    BYTE unk_00[0x8];
    // Where the chunks start.
    int dataOffset;
    // The list of the score entries read from the chunks.
    ListNode *scores;
    int fileSize;
};
ASSERT_SIZE(ScoreDat, 0x14);

// Fills `pscrs` (by shot, stage and difficulty, 6 by 6 by 4) with fresh
// chunks, then copies the score file's "PSCR" chunks over them, stopping
// at one out of range. Returns 0, or -1 if `pscrs` is NULL.
int LoadPracticeScores(void *scoreDat, Pscr *pscrs);
// Copies the score file's last version 1 "PLST" chunk to `plst`.
// Returns 0.
int LoadPlstChunk(void *scoreDat, Plst *plst);
// Frees `dat` and its list.
void ReleaseScoreDat(ScoreDat *dat);
