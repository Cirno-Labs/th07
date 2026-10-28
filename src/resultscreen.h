// The result screen: the score file ("score.dat") and its high score
// lists.

#pragma once

#include <string.h>
#include <windows.h>

#include "common.h"

// A high score chunk of the score file ("HSCR"). Only the fields known
// so far.
struct Hscr
{
    DWORD magic;
    WORD size;
    // The size again.
    WORD size2;
    BYTE version;
    // Set on the entry just added, which is the one being named.
    BYTE isNew;
    BYTE unk_0a[0xa];
    // The shot and difficulty of its list.
    BYTE shot;
    BYTE difficulty;
    BYTE unk_16[0x12];
};
ASSERT_SIZE(Hscr, 0x28);

// The score file's first chunk ("TH7K").
struct Th7k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
};
ASSERT_SIZE(Th7k, 0xc);

// A spell card chunk of the score file ("CATK"), one per card.
struct Catk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x1f];
    // The card's number.
    WORD index;
    BYTE unk_2a[0x4e];
};
ASSERT_SIZE(Catk, 0x78);

// A chunk of the score file ("CLRD") with what a shot type cleared.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x13];
};
ASSERT_SIZE(Clrd, 0x1c);

// The chunk with the last name entered ("LSNM"), new in th07.
struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    char name[0xc];
};
ASSERT_SIZE(Lsnm, 0x18);

// The chunk with the game's version ("VRSM"), new in th07: written
// last, from the executable's size and checksum.
struct Vrsm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09;
    BYTE unk_0a[0x2];
    // "0100b".
    char gameVersion[0x8];
    DWORD exeSize;
    DWORD exeChecksum;
};
ASSERT_SIZE(Vrsm, 0x1c);

// A practice score chunk of the score file ("PSCR").
struct Pscr
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    DWORD unk_0c;
    DWORD score;
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

// "score.dat" as loaded: a 0x1c-byte header, then chunks. On disk the
// chunks are compressed, and every byte from the third on is xored with
// a key that runs over the bytes before it.
struct ScoreDat
{
    BYTE unk_00;
    // Where the key starts; random.
    BYTE key;
    // The sum of the bytes from the fifth on.
    WORD checksum;
    // 0xb.
    WORD version;
    BYTE unk_06;
    BYTE unk_07;
    // Where the chunks start.
    int dataOffset;
    // The list of the score entries read from the chunks.
    ListNode *scores;
    // The size with the chunks uncompressed, header included.
    int fileSize;
    // The size of the chunks, uncompressed and compressed.
    int dataSize;
    int packedSize;
};
ASSERT_SIZE(ScoreDat, 0x1c);

// Fills `pscrs` (by shot, stage and difficulty, 6 by 6 by 4) with fresh
// chunks, then copies the score file's "PSCR" chunks over them, stopping
// at one out of range. Returns 0, or -1 if `pscrs` is NULL.
int LoadPracticeScores(void *scoreDat, Pscr *pscrs);
// Copies the score file's last version 1 "PLST" chunk to `plst`.
// Returns 0.
int LoadPlstChunk(void *scoreDat, Plst *plst);
// Frees `dat` and its list.
void ReleaseScoreDat(ScoreDat *dat);
// Writes "score.dat" from `screen`'s file header, a "TH7K" chunk, its
// high score lists (ten entries each at most), g_Unknown626270's clear,
// spell card and practice chunks, the "LSNM", "PLST" and "VRSM" chunks,
// compressed and encrypted.
void SaveScoreDat(class ResultScreen *screen);


// Moves `screen`'s cursor up or down through `count` items, wrapping.
// Returns -1 for up, 1 for down, 0 if it did not move.
int MoveCursorVertical(ResultScreen *screen, int count);
// The same for `screen`'s unk_24.
int LoadScreenPlstChunk(ResultScreen *screen, int count);

// The result screen's scene object (0xce6c bytes). Only the fields
// known so far.
class ResultScreen
{
public:
    // The score file, as loaded.
    ScoreDat *scoreDat;
    BYTE unk_4[0xc];
    // The menu item selected.
    int cursor;
    BYTE unk_14[0x10];
    // A second cursor, moved by LoadScreenPlstChunk.
    int unk_24;
    BYTE unk_28[0x8580];
    // The high score lists, by difficulty and shot.
    ListNode scores[6][6];
    BYTE unk_8758[0x3868];
    // The "TH7K" chunk SaveScoreDat writes.
    Th7k th7k;
    Lsnm lsnm;
    BYTE unk_bfe4[0xe88];
};
ASSERT_SIZE(ResultScreen, 0xce6c);
