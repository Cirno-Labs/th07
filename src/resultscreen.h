// The result screen: the score file ("score.dat") and its high score
// lists.

#pragma once

#include <string.h>
#include <windows.h>

#include "common.h"
#include "sprite.h"

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
    BYTE unk_0a[0x2];
    DWORD score;
    // How much the game slowed down, in percent.
    float slowdown;
    // The shot and difficulty of its list.
    BYTE shot;
    BYTE difficulty;
    // The stage reached, 99 for a clear.
    BYTE stage;
    // Eight characters and a NUL.
    char name[9];
    // "%m/%d" and a NUL.
    char date[6];
    BYTE unk_26;
    BYTE unk_27;
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
    BYTE unk_2a;
    char name[0x3d];
    // The result screen shows "？？？？？" for the name while it is 0.
    WORD unk_68;
    BYTE unk_6a[0xe];
};
ASSERT_SIZE(Catk, 0x78);

// A chunk of the score file ("CLRD") with what a shot type cleared.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    // By difficulty: the number of stages cleared, 99 ('c') for the
    // whole game, without continues and with.
    BYTE stagesClearedNoContinue[6];
    BYTE stagesCleared[6];
    BYTE unk_18[0x4];
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
// Links `entry` in after `head`, before the first entry whose score is no
// higher. Returns its rank, from 0.
int InsertScore(ListNode *head, Hscr *entry);

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
// The same with left and right.
int MoveCursorHorizontal(ResultScreen *screen, int count);

// The shot types' names ("博麗 霊夢 (霊)", ...), by character * 2 +
// shotType, then "全主人公合計".
extern char *g_ShotNames[7];
// The characters a name can be made of, 16 to a row; the last two
// stand for a space and the end.
extern char *g_NameAlphabet;

// Writes today's date as "%m/%d" to `date`. Not in th06.
void FormatTodayDate(char *date);

// The result screen's scene object (0xce6c bytes). Only the fields
// known so far.
class ResultScreen
{
public:
    // Inserts `entry` into the high score list for `difficulty` and
    // `shot`.
    int AddScore(Hscr *entry, int difficulty, int shot);
    // The menu (states 0 and 1), the score lists (3 to 8, by
    // difficulty), the spell cards (9), after a game the name entry
    // (10), replay saving (11 to 15) and statistics (16, 17), and states
    // 0x14 to 0x16 (Func448098). Steps the sprites. Returns 1, or 0 once
    // it sets the scene back to the title.
    static int OnUpdate(ResultScreen *screen);

    // Lets the player enter a name for the score just made. Returns 0.
    int HandleNameEntry();
    // Offers to save the replay and lists the slots. Returns 0.
    int HandleReplaySaving();
    // After a name is entered, waits for the shot button. Returns 0.
    int WaitAfterNameEntry();
    // Runs states 0x14 to 0x16, entered from menu item 7. Returns
    // nonzero to go back to the menu.
    int Func448098();
    // Frees the entries of the high score list for `difficulty` and
    // `shot`.
    void ReleaseScores(int difficulty, int shot);
    // The score file, as loaded.
    ScoreDat *scoreDat;
    // Counts the frames in the current state.
    int frame;
    int state;
    // The list state (3 to 9, 0x14) last entered from the menu.
    int listState;
    // The menu item selected.
    int cursor;
    // The cursors of the score lists and of the spell card pages, kept
    // while back in the menu.
    int savedCursor;
    int savedSpellPage;
    BYTE unk_1c[0x4];
    // The character of g_NameAlphabet selected.
    int charIndex;
    // A second cursor, moved by LoadScreenPlstChunk: the spell cards' shot type.
    int unk_24;
    // The unk_24 that textSprites show; with spellPage.
    int unk_28;
    int unk_2c;
    // The shot type whose name textSprites show; -1 to redraw.
    int character;
    // The spell card page (ten cards each) textSprites show; -1 to
    // redraw.
    int spellPage;
    // The menu item whose list is shown.
    int difficulty;
    // How far the button sequence that marks every stage cleared has
    // got: three of 0x10, two of 0x2000, two of 0x20, three of 0x200
    // while 4 or 0x100 is held.
    int cheatProgress;
    // When set, the name entry starts on the end mark.
    int unk_40;
    // The name entered last.
    char name[0xc];
    // By shot type (and all of them): the spell cards captured.
    int unk_50[7];
    BYTE unk_6c[0x4];
    Sprite sprites[0x29];
    // Text drawn with RenderText; the first holds the shot's name.
    Sprite textSprites[0x11];
    // The high score lists, by difficulty and shot.
    ListNode scores[6][6];
    BYTE unk_8758[0x3840];
    // The score being entered.
    Hscr entry;
    // The "TH7K" chunk SaveScoreDat writes.
    Th7k th7k;
    Lsnm lsnm;
    BYTE unk_bfe4[0xe88];
};
ASSERT_SIZE(ResultScreen, 0xce6c);
