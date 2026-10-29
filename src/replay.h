// Replays ("./replay/th7_%.2d.rpy"). Only what is known so far.

#pragma once

#include <windows.h>

#include "chain.h"
#include "common.h"

// One frame of a replay.
struct ReplayFrame
{
    // The buttons held.
    WORD input;
    // ReplayManager::frameEvents for the frame.
    WORD events;
};

// A stage of a replay: the game's state at its start, then its frames.
// Only the fields known so far.
struct ReplayStage
{
    // The score the stage started with, divided by 10.
    int score;
    // GameStats::pointItemsTotal.
    int pointItemsTotal;
    // GameManager's bonusPoints, bonusPointsCap and invulnerabilityBonus, less
    // GameStats::grazeCount.
    int bonusPoints;
    int bonusPointsCap;
    int invulnerabilityBonus;
    // GameStats::grazeTotal.
    int grazeTotal;
    // GameStats::checksumKeyStage and checksumKeyPoints.
    int checksumKeyStage;
    int checksumKeyPoints;
    // GameManager::rngSeed.
    WORD rngSeed;
    BYTE power;
    BYTE lives;
    BYTE bombs;
    // GameManager::rank and powerItemCombo.
    BYTE rank;
    BYTE powerItemCombo;
    // GameStats::spellCardsCaptured.
    BYTE spellCardsCaptured;
    BYTE pad_28[0x4];
    ReplayFrame frames[1];
};

// The start of a replay file. Only the fields known so far.
struct ReplayHeader
{
    // "T7RP".
    char magic[4];
    // 0x1100, 0x100 in its low 12 bits.
    WORD version;
    BYTE pad_06[0x2];
    // 0x3f000318 plus the bytes from `key` on.
    int checksum;
    BYTE secondaryKey;
    // The bytes from `size` on are stored plus key, key + 7, key + 14,
    // ...
    BYTE key;
    BYTE pad_0e[0x2];
    // The size of the file decompressed.
    int size;
    // In the file, the size of the compressed data after the first 0x54
    // bytes, and its size decompressed.
    int compressedSize;
    int dataSize;
    // Indexed by stage. In the file, the offsets of the stages' data.
    ReplayStage *stages[7];
    // The same for the frame rates kept every 30 frames.
    BYTE *extras[7];
    BYTE randomByte;
    // 0x62.
    BYTE versionByte55;
    // character * 2 + shot type.
    BYTE shotType;
    BYTE difficulty;
    // The date it was saved, "%m/%d".
    char date[6];
    // The player's name.
    char name[0xc];
    // 0x100.
    WORD subVersion;
    DWORD score;
    // The config the game was played with, an GameConfig.
    BYTE config[0x38];
    BYTE pad_a8[0x20];
    // How far the game ran below full speed, in percent, stored three
    // times: plus 1.12, as is, and plus 2.34.
    float slowdown2;
    float slowdown;
    float slowdown3;
    // 30.
    int replayFormatVersion;
    // App::exeSize and exeChecksum, and "0100b".
    DWORD exeSize;
    DWORD exeChecksum;
    char exeVersion[6];
    BYTE pad_e6[0x2];
};
ASSERT_SIZE(ReplayHeader, 0xe8);

// Checks and decodes the replay file `data` of `size` bytes: its magic,
// version and checksum, then decompresses it into a new copy, which it
// returns unless the config's scoreDisabled is set or App::CheckVersionFile rejects
// the executable it was recorded with. Frees `data`, and returns NULL
// if it is not a valid replay.
ReplayHeader *ValidateReplay(void *data, int size);
// Saves the replay being recorded to `path`, under `name`, unless it
// was played with another config than the current one (outside
// practice, Extra and Phantasm) or with the config's scoreDisabled set: the
// header's first 0x54 bytes as they are, the rest and the stages'
// frames and frame rates compressed with LzssCompress, all encoded as
// ValidateReplay decodes them. Then frees the stages and removes the
// update element.
void SaveReplay(char *path, char *name);
// Writes the replay being played back to `path` again, as SaveReplay
// does, with new keys and the current score, but its stages and name as
// they are. Then removes the update element. Not in th06.
void ResaveReplay(char *path);

// Records and plays back replays, behind the pointer at 0x4b9e48. Only
// the fields known so far.
class ReplayManager
{
public:
    // The second update callback while recording, run before the game's
    // own: clears frameEvents (marking 0x100 when GameManager::pauseRequested is
    // set, which it clears), keeps the random seed in frameSeed and clears
    // the count of random numbers. Returns 1. Not in th06.
    static int RecordOnUpdate2(ReplayManager *replay);
    // The update callback while recording: while bit 2 of
    // GameManager::stateFlags is set, plays by g_Buttons and, unless the
    // config's scoreDisabled or bit 3 of App::stateFlags is set, records them with
    // frameEvents, and every 30 frames the frame rate (0x80 set while
    // App::fastClockStreak is), then counts the frame. Returns 1.
    static int RecordOnUpdate(ReplayManager *replay);
    // The second update callback while playing: while bit 2 of
    // GameManager::stateFlags is set, returns 6 on two frames of every
    // three while g_Gui's IsMsgRunning and IsMsgSkippable hold, and
    // on four of every five while replayMenuChoice is 2 and
    // g_EnemyManager.AnyBossActive() does not; otherwise 1.
    static int PlayOnUpdate2(ReplayManager *replay);
    // The update callback while playing: while bit 2 of
    // GameManager::stateFlags is set and the config's scoreDisabled is not,
    // plays the recorded buttons for the frame, with key repeat as in
    // App::OnUpdate, and every 30 frames puts the recorded frame rate
    // back into App::roundedFps and fpsWarning, then counts the frame.
    // Returns 1.
    static int PlayOnUpdate(ReplayManager *replay);
    // The update element's added callback when recording: starts the
    // replay data the first time ("T7RP", the shot type, difficulty,
    // config and executable), otherwise keeps the score in the stage
    // before; then (re)allocates the stage's frames and frame rates and
    // keeps the game's state in the stage. Returns 0.
    static int RecordAddedCallback(ReplayManager *replay);
    // The update element's added callback when playing: loads and checks
    // the replay at `path` the first time, keeping each stage's sizes and
    // turning its offsets into pointers; then puts the stage's state back
    // into the game. Returns 0, or -1 if the replay or the stage is
    // missing.
    static int PlayAddedCallback(ReplayManager *replay);
    // Removes the chain elements but the update one, frees the replay
    // data and frameRateBuffer and deletes the ReplayManager. Returns 0.
    static int DeletedCallback(ReplayManager *replay);
    // The draw callback; does nothing. Returns 1.
    static int OnDraw(ReplayManager *replay);

    ReplayManager()
    {
    }

    // 0 when recording, 1 when playing.
    int GetMode()
    {
        return this->mode;
    }
    // Whether the replay has stage `stage`.
    BOOL HasStage(int stage)
    {
        return this->data->stages[stage] != NULL;
    }

    // Creates the ReplayManager, recording (`mode` 0) or playing (1) the
    // replay at `path`, and adds its chain elements; if it exists, runs
    // the mode's added callback again. Returns 0, or -1 if adding the
    // update element fails.
    static int RegisterChain(int mode, char *path);
    // Ends the current stage's record with an empty frame and keeps
    // where it ends.
    static void EndStage();

    // The current frame.
    int frame;
    // The replay data.
    ReplayHeader *data;
    // Each stage's size in bytes, and its frame rates', when playing.
    int stageSizes[7];
    int extraSizes[7];
    void *frameRateBuffer;
    // RegisterChain's arguments.
    int mode;
    char *path;
    unsigned char pad_4c[0x36];
    WORD clearedAtRecordStart;
    // The last frame recorded or played; [stage - 1] is where that
    // stage's frames end (stages past 7 share the last).
    ReplayFrame *cursor;
    ReplayFrame *stageEnds[7];
    // The same for the frame rates kept every 30 frames.
    BYTE *extraCursor;
    BYTE *extraEnds[7];
    // The update element, the draw element, the second update element
    // when playing and when recording.
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *calcChain2;
    ChainElem *calcChain3;
    // The random seed at the start of the frame.
    WORD frameSeed;
    // Bits set during a frame: 2 when the player is hit.
    WORD frameEvents;
};
extern ReplayManager *g_ReplayManager;
