// The front: the interface drawn over the game and the dialogue, as th06's.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// A loaded message file: a count, then the offsets of its scripts, made
// pointers once loaded, as th06's.
struct MsgFile
{
    int count;
    void *scripts[1];
};

// The arguments of a message instruction, as th06's: who it is for (0
// the player, 1 the boss; 2 and 3 select the text lines for opcode 5)
// and a sprite, script or line number, then the text.
struct MsgArgs
{
    short who;
    union
    {
        short index;
        BYTE value;
    };
    char text[1];
};

// One instruction of a message script, as th06's.
struct MsgInstr
{
    // The frame it runs on.
    WORD time;
    BYTE opcode;
    // The size of the arguments, in bytes.
    BYTE argsSize;
    union
    {
        MsgArgs args;
        int i;
        BYTE b;
    };
};

// The dialogue (th06's GuiMsg), GuiImpl+0x1fba4, 0xe10 bytes. Only what
// is known so far.
struct GuiMsg
{
    // The message file loaded.
    MsgFile *msgFile;
    // The next instruction of the running script.
    MsgInstr *instr;
    // The running script; negative when none runs.
    int scriptIndex;
    Timer timer;
    // How long the script has waited for the shot button (opcode 4).
    int waitFrames;
    // The player's and the boss's portraits, and the two lines of text,
    // as th06's.
    Sprite portraits[2];
    Sprite text[2];
    Sprite nameplates[2];
    // The text colors and their shadows, by speaker.
    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];
    // The text's height in pixels.
    int fontSize;
    // While it is above 0, the enemy timeline's wait for the dialogue
    // ends, as th06's.
    DWORD timelineRelease;
    // Lets the skip button fast-forward the script, as th06's.
    BYTE skippable;
    unsigned char pad_e0d[0x3];
};
ASSERT_SIZE(GuiMsg, 0xe10);

// A message Gui shows for a while at a fixed place, with a
// number, as th06's GuiPopup.
struct GuiPopup
{
    D3DXVECTOR3 pos;
    int value;
    // Nonzero while it shows.
    int shown;
    Timer timer;
};

// The bulk of Gui (th06's GuiImpl), 0x20a30 bytes. Only what
// is known so far.
struct GuiImpl
{
    Sprite sprites[0x21];
    // The boss's bar (sprites[11]): 0 hidden, 1 appearing, 2 shown, 3
    // leaving, as th06's.
    BYTE bossBarState;
    unsigned char pad_4bcd[0x3];
    Sprite trackNumberSprites[5];
    Sprite bombPortrait;
    Sprite spellPortrait;
    Sprite bombPortraitGlow;
    Sprite spellPortraitGlow;
    Sprite bombPortraitFrame;
    Sprite spellPortraitFrame;
    Sprite bombName;
    Sprite spellName;
    Sprite bombNameBar;
    Sprite spellNameBar;
    Sprite loadingIcon;
    Sprite loadingCaption;
    Sprite loadingBar;
    Sprite spellScoreDigit;
    Sprite spellScoreBar;
    Sprite backgroundTiles[0xa8];
    int backgroundTileCount;
    // Starts script `index` of msg.msgFile, as th06's.
    void StartMsgScript(int index);
    // Runs the dialogue for a frame, as th06's. New: each frame it calls
    // g_Player.EndBombState while bombState is set and collects the items
    // unless the player is dying;
    // opcode 7 shows the song name in trackNumberSprites[0], 9 records the stage
    // clear bonus, 11 picks the next scene from the game mode, 14 fades
    // the screen out.
    int RunMsg();

    // Draws the dialogue, as th06's.
    int DrawMsg();

    GuiMsg msg;
    // Set once the stage is cleared: OnDraw then shows the bonus,
    // stageClearTotal (in tens).
    int stageClearShown;
    int stageClearTotal;
    // When set, OnUpdate clears it and sets g_App.scene to 3.
    int returnToTitleRequested;
    // The bonus for bullets cleared (th06's bonusPopup), a popup that
    // also shows what kind it is in `shown`, and the spell card's bonus.
    GuiPopup bonusPopup;
    GuiPopup fullPowerPopup;
    GuiPopup spellBonusPopup;
    // The stage clear bonus's power, point items, cherry and grazes.
    int stageClearPower;
    int stageClearPoints;
    int stageClearCherry;
    int stageClearGraze;
};
ASSERT_SIZE(GuiImpl, 0x20a30);

// The in-game interface, at 0x49fbf0, as th06's Gui.
class Gui
{
public:
    // The update element's callback: unless g_GameManager.isFrozen is
    // set, sets g_App.scene to 3 if impl->returnToTitleRequested is set, runs
    // UpdateInterface and impl->RunMsg and counts frameCounter up; plays MIDI
    // slot 0 on frame 300 of stage 6, and holds g_App.redrawFrames at 8
    // or more while button 0x100 is held. Returns 1.
    static int OnUpdate(Gui *gui);
    // The draw element's callback: stops the screen shake, then draws the
    // stage clear bonus once impl->stageClearShown is set, the dialogue,
    // DrawBanners, DrawHud and the popups. Returns 1.
    static int OnDraw(Gui *gui);
    // Shows the player's bomb, spell card `name`, with sprite `sprite`
    // in bombPortrait (script 0x4a1), scripts 0x4a4 and 0x4a6 with
    // sprite 0x4ac, and bombName (script 0x704); sends bombNameBar
    // interrupt 1, plays sound 14 and sets g_App.redrawFrames to 2.
    void ShowBombName(int sprite, char *name);
    // Shows a boss's spell card, `text`: unless `sprite` is negative,
    // sprite `sprite` + 0x4ad in spellPortrait (script 0x4a3), moved left
    // by 288 or 112 when it is wider than 256 or 128; scripts 0x4a5 and
    // 0x4a7 with sprite 0x4ac, and spellName (script 0x705); sends
    // spellNameBar and spellScoreBar interrupt 1, plays sound 14 and sets
    // g_App.redrawFrames to 2.
    void ShowSpellCardName(int sprite, char *text);
    // Loads the message file at `path` into impl->msg.msgFile, turning
    // its offsets into pointers. Returns 0, or -1 after logging
    // "メッセージファイル %s が読み込めませんでした".
    int LoadMsg(char *path);
    // Frees impl->msg.msgFile, if there is one.
    void FreeMsgFile();
    // Runs impl->StartMsgScript(`index`). th06 also set
    // g_App.redrawFrames.
    void StartMsg(int index);
    // Whether impl is set, a dialogue script runs and
    // impl->msg.timelineRelease is 0.
    BOOL MsgHoldsTimeline();
    // Whether a boss fight runs, as th06's.
    BYTE IsBossPresent()
    {
        return this->bossPresent;
    }
    // As th06's DrawBanners and DrawHud.
    void DrawBanners();
    void DrawHud();
    // The update: while no dialogue runs, the boss bar (sprites[11],
    // interrupt 1 to show it, 2 to hide it, its flag 13 when done) and
    // its life; then the interface's sprites, loadingIcon and loadingBar
    // (hidden when done), the backgroundTiles sprites while impl->backgroundTileCount
    // is set (counting those still running into it), the popups, and
    // once the stage is cleared, the bonus (stage, grazes, point items,
    // cherry and, on the last stages, lives and bombs, by difficulty
    // and starting lives), added to the score in ten parts.
    void UpdateInterface();
    // Loads the interface's sprites and the stage's portraits and
    // messages, as th06's. Unless the scene is 3, 0xb or 0xc, zeroes impl
    // and loads front.anm and the character's face and loading ANMs;
    // otherwise restarts loadingBar (script 0x725), has the play area of
    // the back buffer copied to texture 4 at its sprite and lays out the
    // 14 by 12 backgroundTiles tiles. Stages 1 to 8 run CopySpriteToTexture and
    // load their face, text and message files; others return -1. Returns
    // 0, or -1 when a file fails to load.
    int LoadResources();
    // The update element's added callback: runs LoadResources.
    static int AddedCallback(Gui *gui);
    // Frees ANM slots 0x18 and 0x1c to 0x1f and the message file and,
    // unless the scene is 3, 0xb or 0xc, slots 0x15 to 0x17 and 0x19 to
    // 0x1b and impl. Returns 0.
    static int DeletedCallback(Gui *gui);
    // Unless the scene is 3, 0xb or 0xc, zeroes the interface and
    // allocates impl; then adds the update element (priority 13) and the
    // draw element (priority 12). Returns 0, or -1 if adding fails.
    static int RegisterChain();
    // Removes the update element and the draw element.
    static void RemoveChain();
    // Sets bombName's pendingInterrupt to 1 and bombNameBar's to 2, as
    // the bomb ends. th06 interrupted bombName only.
    void EndBombName();
    // Sets spellName's pendingInterrupt to 1 and spellNameBar's and
    // spellScoreBar's to 2, as the spell card ends.
    void EndSpellCardName();
    // Hides loadingIcon, loadingCaption and loadingBar (spriteIndex -1) and clears
    // impl->backgroundTileCount. Not in th06.
    void HideInterface();
    // Whether impl->loadingIcon shows a sprite with flag bit 13 set, as
    // th06's IsLoadingShown.
    BOOL IsLoadingShown();

    // Whether impl is set and impl->msg.scriptIndex is not negative, or
    // is -2. th06's IsMsgRunning.
    int IsMsgRunning();
    // Returns impl->msg.skippable, as th06's IsMsgSkippable.
    int IsMsgSkippable();
    // Shows `bonus` in bonusPopup, at (416, 48), and sets
    // g_App.redrawFrames to 2.
    void ShowBonusPopup(int bonus);
    // th06's ShowFullPowerPopup: shows `points` in impl->fullPowerPopup, at
    // (416, 168), with `kind` as its shown value, and sets
    // g_App.redrawFrames to 2. Player::EndBombState calls it with the
    // points it scores and 4.
    void ShowFullPowerPopup(int points, int kind);
    // Shows `value` in spellBonusPopup, at (224, 16), and sets
    // g_App.redrawFrames to 2.
    void ShowSpellCardBonus(int value);
    // Sets bossTimer, as th06's.
    void SetBossTimer(int seconds)
    {
        this->bossTimer = seconds;
    }
    // Sets boss bar section `idx` to end at `end` and start at `start`.
    void SetBossSection(int idx, float end, float start)
    {
        this->sectionEnds[idx] = end;
        this->sectionStarts[idx] = start;
    }
    // Sets the color of boss bar section `idx`.
    void SetBossSectionColor(int idx, int color)
    {
        this->sectionColors[idx] = color;
    }
    // Sets bossLives, the boss lives marked beside the bar, as th06's
    // SetBossLives.
    void SetBossLives(int lives)
    {
        this->bossLives = lives;
    }
    // Sets bossHealth, as th06's SetBossHealth.
    void SetBossHealth(float health)
    {
        this->bossHealth = health;
    }

    // Counts OnUpdate's frames.
    int frameCounter;
    // Set to 2 when they change, as th06's GuiFlags.
    unsigned int livesChanged : 2;
    unsigned int bombsChanged : 2;
    unsigned int powerChanged : 2;
    unsigned int grazeChanged : 2;
    unsigned int pointsChanged : 2;
    GuiImpl *impl;
    // The width of the text ShowBombName shows, from its length.
    float bombNameWidth;
    // The same for ShowSpellCardName.
    float spellNameWidth;
    // The boss bar's alpha, raised by 4 a frame up to 0xff while a boss
    // fight runs.
    DWORD bossBarAlpha;
    // How many boss lives DrawBanners marks beside the bar.
    int bossLives;
    // The seconds left on the boss's timer.
    int bossTimer;
    // bossTimer as DrawBanners last drew it: it plays sound 0x1d when
    // the timer changes under 10.
    int lastDrawnBossTimer;
    // Set while a boss fight runs.
    BYTE bossPresent;
    unsigned char pad_25[0x3];
    // The boss's life as a fraction of its maximum, and what the bar
    // shows, following it.
    float bossHealth;
    float bossHealthShown;
    unsigned char pad_30[0x4];
    // Eight sections DrawBanners draws over the boss bar: where each
    // ends and starts, as fractions of the bar (unused while the end
    // is 0), and its color.
    float sectionStarts[8];
    float sectionEnds[8];
    int sectionColors[8];
};
ASSERT_SIZE(Gui, 0x94);
extern Gui g_Gui;
extern ChainElem g_GuiCalcElem;
extern ChainElem g_GuiDrawElem;
// The colors of the boss timer: 20 seconds and more, 10, 5 and under 5.
extern D3DCOLOR g_BossTimerColors[4];
// Copies the rectangle of sprite `sprite` in texture 0x16 into sprite
// 0x609's in texture 0x15 (SpriteEngine::CopyTextureRect). Not in th06.
void CopySpriteToTexture(int sprite);

// The bonus of each spell card, by number.
extern int g_SpellCardBonuses[];

struct Enemy;
struct EnemyScriptState;
