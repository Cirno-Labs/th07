// The game's state across stages, as th06's GameManager.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// The player's numbers, behind GameManager::stats. Only the fields
// known so far.
struct GameStats
{
    // The score shown, and the score.
    DWORD guiScore;
    DWORD score;
    // Cleared when a game continues.
    DWORD scoreIncrement;
    // The high score to beat, from the score file.
    DWORD highScore;
    // What LoadHighScores stores from the best entry; cleared in a
    // practice.
    BYTE bestContinuesUsed;
    unsigned char pad_11[0x3];
    // The stage's grazes, up to 9999, and the game's, up to 999999.
    int graze;
    int grazeTotal;
    int spellCardsCaptured;
    BYTE continuesUsed;
    unsigned char pad_21[0x3];
    // Cleared when a game continues.
    int stageClearPoints;
    int pointItemsTotal;
    int checksumKeyStage;
    int checksumKeyPoints;
    // Set to 6543 plus a random number under 100000 by
    // GameManager::RerollLifeChecksumKeys.
    DWORD lifeChecksumKeys[7];
    float deaths;
    // Set to random floats from 6543 up by RerollStatsChecksumKeys, as statsChecksumKeysB,
    // lifeChecksumKeysF and statsChecksumKeysC.
    float statsChecksumKeysA[2];
    // The lives left, a float behind checksums, as the other counts.
    float lives;
    float statsChecksumKeysB[2];
    // The bombs left, the same way.
    float bombs;
    float bombsUsed;
    // Set to 6543 plus a random float under 100000 by
    // GameManager::RerollLifeChecksumKeys.
    float lifeChecksumKeysF[3];
    // The shot power, up to 128.
    float power;
    float statsChecksumKeysC[2];
    // Player::Graze subtracts it from GameManager::bonusPoints.
    int grazeCount;
    int checksumKeysD[8];
    // ChecksumBytes adds checksumKeysE[2] to it for every byte it sums.
    int checksum;
    // The checksum, from ComputeChecksum.
    int computedChecksum;
    DWORD checksumKeysE[5];
};
ASSERT_SIZE(GameStats, 0xc8);

// What GameManager::savedConfig points at.
struct GameConfig
{
    unsigned char pad_0[0x1c];
    // The lives a game starts with, as App's lives (+0x1c of the
    // config).
    BYTE lives;
    unsigned char pad_1d[0x8];
    // App::IsScoreDisabled tests it.
    BYTE scoreDisabled;
    unsigned char pad_26[0x12];
};

// The game's state and the player's numbers (at 0x626270), with checksums
// against tampering, as th06's GameManager.
class GameManager
{
public:
    GameManager();
    // The update chain callback and its added and deleted callbacks,
    // as th06's GameManager's.
    static int OnUpdate(GameManager *mgr);
    static int AddedCallback(GameManager *mgr);
    static int DeletedCallback(GameManager *mgr);
    // The draw chain callback, as th06's: moves isPaused on from 1 to
    // 2. Returns 1.
    static int OnDraw(GameManager *mgr);
    // Whether the checksummed fields of `stats` no longer agree.
    int ChecksumMismatch();
    // Whether Phantasm is open: a shot type cleared Extra (or captured
    // 60 spell cards, which marks those that cleared Extra as such).
    int IsPhantasmOpen();
    // Whether a shot type has Extra open (HasExtraOpen for any of them).
    int AnyShotHasExtraOpen();
    // Whether shot type `shot` (character * 2 + shot) cleared one of
    // the four main difficulties, which opens Extra for it.
    int HasExtraOpen(int shot);
    // Whether shot type `shot` has Phantasm open (IsPhantasmOpen for one
    // shot type).
    int HasPhantasmOpen(int shot);
    // The checksum: ChecksumBytes over stats from lifeChecksumKeys to checksum, over
    // stats->checksumKeysE, over what savedConfig points at and over g_App's config
    // (0x38 bytes each).
    int ComputeChecksum();
    // Sets the rank and its bounds from g_Ranks for the difficulty.
    void SetRankForDifficulty();
    // Rerolls the checksum keys and recomputes the checksums.
    void RerollChecksumKeys();
    // Adds `amount` hundredths to the rank, as th06's.
    void AddSubRank(int amount);
    // Add `amount` to bonusPointsCap and to bonusPoints, each up to a limit.
    void IncreaseBonusCap(int amount);
    void AddBonusPointsCapped(int amount);
    // Adds `amount` to bonusPoints, up to bonusPointsCap. A gain outside
    // Player::bombState also adds to invulnerabilityBonus and, once that reaches
    // stats->grazeCount + 50000, caps it there and calls StartBombState. When
    // bonusPoints reaches bonusPointsCap and changed, calls Gui's
    // ShowFullPowerPopup with bonusPoints less stats->grazeCount and 3.
    void AddBonusPoints(int amount);
    // The same without invulnerabilityBonus.
    void AddBonusPointsNoTrack(int amount);
    // Takes `amount` from bonusPoints, down to stats->grazeCount.
    void SubBonusPoints(int amount)
    {
        if (this->bonusPoints - this->stats->grazeCount >= amount)
        {
            this->bonusPoints -= amount;
        }
        else
        {
            this->bonusPoints = this->stats->grazeCount;
        }
    }
    // Adds `points` to stats->score, which counts in tens.
    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th06's GameManager::IsInPlayArea.
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Sets arena to th06's play area (32, 16, 384, 448) and playerArea
    // to (8, 16, 368, 416).
    void SetPlayArea();
    // Adds `power` to stats->power and recomputes the checksums; wipes
    // g_App with 0xff first if the checksums were already wrong.
    void AddPower(int power);
    // Sets stats->lifeChecksumKeys and lifeChecksumKeysF to random numbers from 6543 up.
    // Not in th06.
    void RerollLifeChecksumKeys();
    // Adds `lives` to stats->lives, checked like AddPower.
    void AddLives(int lives);
    // Adds `bombs` to stats->bombs, checked like AddPower.
    void AddBombs(int bombs);
    // Gives a life, or a bomb when the lives are full (8). Either one
    // plays sound 0x1c, adds 200 to the subrank and marks the Gui's
    // count changed. Called for the 1up items.
    void AddLifeOrBomb();
    // Sets stats->bombs to `bombs` and updates the fields that follow
    // from it.
    void SetBombs(int bombs);
    // Set stats->lives and stats->power, without the checksums.
    void SetLives(int lives)
    {
        this->stats->lives = lives;
    }
    void SetPower(int power)
    {
        this->stats->power = power;
    }
    // Takes `amount` hundredths from the rank, as th06's SubSubRank.
    void SubSubRank(int amount);
    // Adds `deaths` to stats->deaths, checked like AddPower.
    void AddDeaths(int deaths);
    // Adds `bombs` to stats->bombsUsed, checked like AddPower.
    void AddBombsUsed(int bombs);
    // Sets bit 3 (0x8) of stateFlags to `value`.
    void SetIsInReplay(int value)
    {
        this->isInReplay = value;
    }

    // A block of random size that AddedCallback allocates and frees
    // before it allocates savedConfig and stats.
    void *tempBuffer;
    GameConfig *savedConfig;
    GameStats *stats;
    // Set while the game is paused: the screen shake holds still. As
    // th06's GameManager::isFrozen, probably.
    char isFrozen;
    // DrawFps measures nothing while it is set.
    char skipFpsMeasure;
    unsigned char pad_e[0x2];
    int difficulty;
    // 1 << difficulty, set by AddedCallback.
    int difficultyMask;
    // The spell card chunks of the score file, by card.
    Catk catks[0x8d];
    // A copy of catks, put back when no score is recorded.
    Catk savedCatks[0x8d];
    // The clear chunks, by shot type.
    Clrd clrds[6];
    // The practice scores, by shot type, stage and difficulty.
    Pscr pscrs[6][6][4];
    // The "PLST" chunk. When its magic is set on exit, WinMain calls
    // ResultScreen::RegisterChain(2) to save the score file.
    Plst plst;
    // ReplayManager::RecordOnUpdate2 marks 0x100 in the frame's replay bits
    // while it is set, and clears it.
    int pauseRequested;
    // The power items collected in a row at full power, as th06's
    // powerItemCombo: picks the points from g_PowerItemBonus. Cleared by
    // Player's UpdateDyingState when the player dies.
    char powerItemCombo;
    BYTE character;
    BYTE shotType;
    // character * 2 + shotType, as a replay stores it.
    BYTE shotIndex;
    // Bit 1: the game is a practice; 0x10: it was cleared.
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits93d80 : 2;
            // Set by OnUpdate while the game is neither paused nor in
            // the continue menu, as th06's isPlaying.
            unsigned int isPlaying : 1;
            unsigned int isInReplay : 1;
            unsigned int isGameCleared : 1;
        };
    };
    // Cleared by SetUpTitleScene. OnDraw moves it on to 2, as th06's
    // isPaused.
    BYTE isPaused;
    // Set while the continue menu shows, as th06's.
    BYTE isInContinueMenu;
    // The demo replay played last, 0 to 2.
    BYTE demoIndex;
    // Set from the replay menu's last choice when a replay starts.
    BYTE replayMenuChoice;
    // The frames a demo replay has played; OnUpdate fades out at 8100,
    // 7020 or 4620 by demoIndex and returns to the title soon after.
    int demoFrames;
    // The replay to play.
    char replayFile[0x200];
    // A replay stage keeps it.
    WORD rngSeed;
    unsigned char pad_95e6[0x2];
    // The screen shake ends at once while this is 1 or less. Cleared
    // by RegisterGameManagerChain, as th06's stageFrames.
    int stageFrames;
    int stage;
    unsigned char pad_95f0[0x4];
    // The play area: x, y, width and height. Its neighbours (a 0x200
    // byte replay file name, the stage) are th06's GameManager's, so
    // this is probably th07's GameManager.
    float arena[4];
    // A second rectangle, set with arena by SetPlayArea: x, y, width
    // and height.
    float playerArea[4];
    // RerollStatsChecksumKeys sets it to the checksum plus stats->checksumKeysD[3].
    float checksumValue;
    int bonusPointsCap;
    // Added to (up to bonusPointsCap) by AddBonusPointsCapped.
    int bonusPoints;
    // UpdateInvulnerability sets it while Player::playerState is 4.
    int invulnerabilityBonus;
    // Whether Phantasm was open when the title menu last came up.
    int phantasmWasOpen;
    // The frames played, which the result screen shows as progress.
    int progressFrames;
    // Counts frames for OnUpdate's slowdowns: of a replay played slowly
    // (replayMenuChoice 1) and, while savedConfig->scoreDisabled is set, by bullet count.
    DWORD slowdownCounter;
    // Set by the title menu from the play time: 3 under 7 hours, 4
    // under 14, else 5.
    int maxContinues;
    // A replay stage keeps it. The rank, as th06's: SetRankForDifficulty sets it
    // and its bounds from g_Ranks.
    int rank;
    int maxRank;
    int minRank;
    // Hundredths of a rank, moved by AddSubRank and SubSubRank.
    int subRank;
};
ASSERT_SIZE(GameManager, 0x9644);
extern GameManager g_GameManager;

// The update and draw chain elements of g_GameManager, after it.
extern ChainElem g_GameManagerCalcChainElem;
extern ChainElem g_GameManagerDrawChainElem;

// The rank a game starts at and its bounds, as th06's.
struct RankSettings
{
    int rank;
    int minRank;
    int maxRank;
};
// By difficulty.
extern RankSettings g_Ranks[6];

// The enemy ANM files of a stage, as th06's.
struct EnemyAnmPaths
{
    char *anmPath;
    char *anmPath2;
};
// By stage, from "dummy" for stage 0.
extern EnemyAnmPaths g_EnemyAnmPaths[];
// The enemy script files, by stage.
extern char *g_EclPaths[];

// The sum of the `size` bytes at `data`. Adds
// g_GameManager.stats->checksumKeysE[2] to its checksum for every byte.
int ChecksumBytes(BYTE *data, int size);
// Fills the checksum keys in g_GameManager.stats with random numbers
// from 6543 up, then stores the checksum. Not in th06.
void RerollStatsChecksumKeys();
// th06's score loading from GameManager::AddedCallback: saves the score
// file (ResultScreen::RegisterChain(2)), resets the spell cards, then
// loads the high score, the spell cards, the clears and the practice
// scores from "score.dat" (a practice takes its high score from the
// practice scores and adds one to its playCount) and keeps a copy of the
// spell cards in savedCatks. Returns 0, or -1 if the file cannot be
// read. Not in th06 as a function of its own.
int LoadScoreFile();
// Adds one to `*count` while it is under 999999. Its callers pass
// 999999 as `max`, which it ignores.
void IncrementStatCounter(DWORD *count, DWORD max);
