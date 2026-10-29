// The result screen: the score file ("score.dat") and its high score
// lists.

#pragma once

#include <string.h>
#include <windows.h>

#include "chain.h"
#include "common.h"
#include "replay.h"
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
    BYTE pad_0a[0x2];
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
    BYTE continuesUsed;
    BYTE unusedByte27;
};
ASSERT_SIZE(Hscr, 0x28);

// The score file's first chunk ("TH7K").
struct Th7k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
};
ASSERT_SIZE(Th7k, 0xc);

// A spell card chunk of the score file ("CATK"), one per card.
struct Catk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
    // By shot type, then with every shot type: the best bonus got.
    DWORD maxBonus[7];
    // The card's number.
    WORD index;
    BYTE checksum;
    char name[0x31];
    // By shot type, then with every shot type: the times tried and
    // captured. The result screen shows "？？？？？" for the name while
    // no shot type has tried it.
    WORD attempts[7];
    WORD captures[7];
};
ASSERT_SIZE(Catk, 0x78);

// A chunk of the score file ("CLRD") with what a shot type cleared.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
    // By difficulty: the number of stages cleared, 99 ('c') for the
    // whole game, without continues and with.
    BYTE stagesClearedNoContinue[6];
    BYTE stagesCleared[6];
    // The shot type.
    BYTE shot;
    BYTE pad_19[0x3];
};
ASSERT_SIZE(Clrd, 0x1c);

// The chunk with the last name entered ("LSNM"), new in th07.
struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
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
    BYTE pad_0a[0x2];
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
    BYTE pad_09[0x3];
    DWORD playCount;
    DWORD score;
    BYTE shot;
    BYTE difficulty;
    BYTE stage;
    BYTE unusedByte17;
};
ASSERT_SIZE(Pscr, 0x18);

// One difficulty's line of the play statistics.
struct PlstCounts
{
    // The games played: [0] with every shot type, then by shot type.
    int plays[7];
    int retries;
    int clears;
    int continues;
    int practices;
};
ASSERT_SIZE(PlstCounts, 0x2c);

// A chunk of the score file ("PLST") with the play statistics, new in
// th07.
struct Plst
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
    // How long the game has run, App::AddPlayTime keeps it up to date.
    DWORD runHours;
    DWORD runMinutes;
    DWORD runSeconds;
    DWORD runMilliseconds;
    // How long it has been played, App::AddTotalPlayTime keeps it up to date.
    DWORD playHours;
    DWORD playMinutes;
    DWORD playSeconds;
    DWORD playMilliseconds;
    // By difficulty, then the totals.
    PlstCounts counts[7];
};
ASSERT_SIZE(Plst, 0x160);

// A node of the score file's lists. Each list starts with a head node
// whose `data` is NULL; the entries after it are sorted.
struct ListNode
{
    ListNode()
    {
        this->prev = NULL;
        this->next = NULL;
        this->data = NULL;
    }

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
// Inserts the score file's high score chunks for `shot` and
// `difficulty` into `list`, or into the file's own list if `list` is
// NULL. Stores the best entry's unk_26 in `unk` if it is not NULL.
// Returns the best score, at least 100000.
DWORD LoadHighScores(void *scoreDat, ListNode *list, int shot, int difficulty, BYTE *unk);
// Copies the score file's version 1 "LSNM" chunk to `lsnm`. Returns 1,
// or 0 if there is none. Not in th06.
int LoadLsnmChunk(void *scoreDat, Lsnm *lsnm);
// Reads "score.dat" at `path`: decodes it, checks the checksum, the
// header size and version 0xb, unpacks the chunks into a new block with
// room for 0xa0000 bytes, then checks every "VRSM" chunk against the
// executable, that no chunk has size 0 and that a version 1 "TH7K"
// chunk is there. On any failure it makes a new, empty one. Either way
// it gives it an empty list.
ScoreDat *LoadScoreDat(char *path);
// Fills `clrds` (by shot type, 6) with fresh chunks, then copies the
// score file's version 1 "CLRD" chunks over them, stopping at one out of
// range. Returns 0, or -1 if `clrds` is NULL.
int LoadClears(void *scoreDat, Clrd *clrds);
// Copies the score file's version 1 "CATK" chunks to `catks`, by card
// number, stopping at one numbered 0x8d or more. Returns 0, or -1 if
// `catks` is NULL.
int LoadSpellCards(void *scoreDat, Catk *catks);
// Copies the score file's last version 1 "PLST" chunk to `plst`.
// Returns 0.
int LoadPlstChunk(void *scoreDat, Plst *plst);
// Frees `dat` and its list.
void ReleaseScoreDat(ScoreDat *dat);
// Writes "score.dat" from `screen`'s file header, a "TH7K" chunk, its
// high score lists (ten entries each at most), g_GameManager's clear,
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
// The shot types' short names ("ReimuA", ...), by character * 2 +
// shotType.
extern char *g_ShotTypeNames[6];
// The characters a name can be made of, 16 to a row; the last two
// stand for a space and the end.
extern char *g_NameAlphabet;

// The difficulties' names ("     Easy", ...), right-aligned.
extern char *g_DifficultyNames[6];
// By difficulty: what the statistics' rating starts from, and what
// each spell card captured adds to it.
extern float g_DifficultyRatings[5];
extern float g_SpellCardRatings[5];

// Writes today's date as "%m/%d" to `date`. Not in th06.
void FormatTodayDate(char *date);

// The result screen's scene object (0xce6c bytes). Only the fields
// known so far.
class ResultScreen
{
public:
    // Constructs the Sprites and lists, then zeroes the whole screen and
    // sets the cursor to 1. Inline, but too large to expand: RegisterChain
    // calls the copy at 0x444a5b and keeps slots for it.
    ResultScreen()
    {
        memset(this, 0, sizeof(ResultScreen));
        this->cursor = 1;
    }
    ~ResultScreen()
    {
        MemFree(this->scoreDat);
    }

    // Sets the state. Inline: RegisterChain's three calls count towards
    // its inline budget, so it keeps 6 slots for the constructor, not 13.
    void SetState(int state)
    {
        this->state = state;
    }

    // Inserts `entry` into the high score list for `difficulty` and
    // `shot`.
    int AddScore(Hscr *entry, int difficulty, int shot);
    // The menu (states 0 and 1), the score lists (3 to 8, by
    // difficulty), the spell cards (9), after a game the name entry
    // (10), replay saving (11 to 15) and statistics (16, 17), and states
    // 0x14 to 0x16 (RunStatisticsMenu). Steps the sprites. Returns 1, or 0 once
    // it sets the scene back to the title.
    // Allocates the screen and adds it to the chains (update priority
    // 0xe, draw 0xd). With `unk` 1 it starts in state 10, or 0x12 in a
    // practice; with 2 (on exit) in state 0x13, running only
    // AddedCallback. Returns 0, or -1 if adding fails.
    static int RegisterChain(int unk);
    static int OnUpdate(ResultScreen *screen);
    // Draws the sprites, the score lists or the spell cards, the name
    // entry's alphabet, the replay slots, then DrawStatistics. Returns 1.
    static int OnDraw(ResultScreen *screen);
    // Loads the images and starts the sprites, fills the lists with the
    // default entries, then the score file's. Returns 0, or -1 if
    // loading fails.
    static int AddedCallback(ResultScreen *screen);
    // Saves and frees the score file, empties the lists, releases ANMs
    // 0x2a to 0x2d and surface 0, removes the draw element and deletes
    // the screen. Returns 0.
    static int DeletedCallback(ResultScreen *screen);

    // Lets the player enter a name for the score just made. Returns 0.
    int HandleNameEntry();
    // Offers to save the replay and lists the slots. Returns 0.
    int HandleReplaySaving();
    // After a name is entered, waits for the shot button. Returns 0.
    int WaitAfterNameEntry();
    // Runs states 0x14 to 0x16, entered from menu item 7. Returns
    // nonzero to go back to the menu.
    int RunStatisticsMenu();
    // In states 0x10 and 0x11, shows the game's statistics next to sprite
    // 0x28: score (with the continues as its last digit), difficulty,
    // progress, continues, deaths, bombs, spell cards and slowdown. It
    // also works out a rating from them that it never shows. Returns 0.
    int DrawStatistics();
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
    // The replay slot chosen, 0 to 14.
    int replaySlot;
    // The character of g_NameAlphabet selected.
    int charIndex;
    // A second cursor, moved by LoadScreenPlstChunk: the spell cards' shot type.
    int spellCursor;
    // The unk_24 that textSprites show; with spellPage.
    int shownShotType;
    int spellInfoSliding;
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
    int nameEntryAtEnd;
    // The name entered last.
    char name[0xc];
    // By shot type (and all of them): the spell cards captured.
    int capturedSpellCounts[7];
    // The runSeconds that textSprites[0] shows, in states 0x14 to 0x16.
    BYTE shownRunSeconds;
    BYTE pad_6d[0x3];
    Sprite sprites[0x29];
    // Text drawn with RenderText; the first holds the shot's name.
    Sprite textSprites[0xf];
    Sprite unk_8110;
    Sprite unk_835c;
    // The high score lists, by difficulty and shot.
    ListNode scores[6][6];
    // The entries the lists start with, by difficulty, shot and rank.
    Hscr defaultScores[6][6][10];
    // The score being entered.
    Hscr entry;
    // The "TH7K" chunk SaveScoreDat writes.
    Th7k th7k;
    Lsnm lsnm;
    ChainElem *calcChain;
    ChainElem *drawChain;
    // The headers of "./replay/th7_01.rpy" to "th7_15.rpy", where they
    // are valid.
    ReplayHeader replays[0xf];
    // The date and score of the replay being saved.
    ReplayHeader replay;
};
ASSERT_SIZE(ResultScreen, 0xce6c);
