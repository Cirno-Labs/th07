// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "chain.h"
#include "sprite.h"

// A string queued by AsciiText::AddString, as in th06.
struct AsciiString
{
    char text[0x40];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    float scaleX;
    float scaleY;
    // Drawn with the second glyph set, in white.
    int highlight;
    // Drawn inside the play area's viewport.
    int inArena;
};
ASSERT_SIZE(AsciiString, 0x60);

// The pause menu, AsciiText's at 0x74e8. th06 had one class for both
// menus; th07's two differ in size.
class PauseMenu
{
public:
    // While g_Unknown626270 is paused, as th06's. Returns 0.
    int UpdatePauseMenu();
    // Draws it while paused, as th06's, flushing SpriteEngine's batch
    // before setting the play area's viewport: the background with z
    // writes off if the back buffer is lockable (bit 1 of
    // g_App.unk_18c) and state is nonzero, then the shown sprites.
    void DrawPauseMenu();

    int state;
    // Frames since the state changed.
    int stateFrames;
    Sprite sprites[10];
    // The copy of the play area behind the menu, as in th06.
    Sprite background;
};
ASSERT_SIZE(PauseMenu, 0x194c);

// The continue menu, AsciiText's at 0x8e34.
class ContinueMenu
{
public:
    // While g_Unknown626270.isInContinueMenu is set, as th06's.
    int UpdateContinueMenu();
    // Draws it while in the continue menu, as th06's, flushing
    // SpriteEngine's batch before setting the play area's viewport: the
    // background if the back buffer is lockable and state is nonzero or
    // stateFrames above 2, sprites[4] in states 1 and 2, then those of
    // the first five sprites that are shown.
    void DrawContinueMenu();

    int state;
    int stateFrames;
    Sprite sprites[6];
    Sprite background;
};
ASSERT_SIZE(ContinueMenu, 0x101c);

// A number rising from where something scored, as in th06. OnUpdate
// moves it up and retires it after 60 frames.
struct AsciiPopup
{
    // Least significant first.
    BYTE digits[8];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    Timer timer;
    BYTE active;
    BYTE digitCount;
    unsigned char unk_26[0x2];
};
ASSERT_SIZE(AsciiPopup, 0x28);

class AsciiText
{
public:
    // Constructs the members, for g_AsciiText's static initialization.
    AsciiText();
    // Adds AsciiText's update element (priority 1) and its draw
    // elements (0x10 and 0xb), as th06's. Returns 0, or -1 if adding
    // the update element fails.
    static int RegisterChain();
    // Removes the update element and the first draw element; the
    // second draw element stays, as in th06.
    static void RemoveChain();
    // Initializes everything, where th06 calls AsciiText::Initialize:
    // zeroes the sprites, strings, menus and popups, sets the color to
    // white, the scale to 1, unk_74d8 to 14, and the sprites up, digit
    // anchored, then sends unk_498 interrupt unk_74d4.
    void Initialize();
    // Runs the scripts of unk_498, unk_6e4, unk_b7c[] and unk_930 for a
    // frame.
    void RunScripts();
    // Starts the scripts of unk_498 (4), unk_6e4 (3), unk_930 (5) and
    // unk_b7c[] (6).
    void StartScripts();
    // The update element's callback: moves the active popups along
    // unless g_Unknown626270 is paused or in the continue menu, then
    // runs the menus as th06's. Also shows unk_9e50 (script 7) while bit
    // 1 of g_Unknown626270.unk_93d8 is set. Returns 1.
    static int OnUpdate(AsciiText *text);
    // The first draw element's callback: draws the queued strings and
    // empties the queue, draws the menus, then unk_9e50 while it runs a
    // script. Returns 1.
    static int OnDraw(AsciiText *text);
    // The update element's added callback: zeroes the object, loads
    // ascii.anm (slot 1) and capture.anm (slot 4, from sprite 0x724),
    // then initializes and starts the sprites' scripts. Returns 0, or -1
    // if loading fails.
    static int AddedCallback(AsciiText *text);
    // The update element's deleted callback: frees the ANM files of
    // slots 1 to 4. Returns 0.
    static int DeletedCallback(AsciiText *text);
    // Draws the queued strings, as th06's, unk_74d8 pixels a glyph and
    // flushing SpriteEngine's batch before each viewport change. Then
    // draws those of unk_b7c[] whose x is 56 to 392, fading them in
    // from alpha 48 as they get within 64 pixels of the player, and in
    // blue while unk_14ac[i] is set.
    void DrawStrings();
    // The second draw element's callback: draws the popups. th06 picked
    // DrawPopups or DrawPopups2 by g_App.hardwareTnL. Returns 1.
    static int OnDraw2(AsciiText *text);
    // Draws the active popups, as th06's.
    void DrawPopups();

    // Queues `text` to be drawn at `pos`.
    void AddString(D3DXVECTOR3 *pos, char *text);
    // AddString with printf-style formatting.
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // Shows `value` rising from `pos` (play-area coordinates) in `color`
    // for 60 frames, in one of the first 0x2d0 popups, as th06's.
    void AddPopup(D3DXVECTOR3 *pos, int value, D3DCOLOR color);
    // Like AddPopup, in the last three popups, as th06's AddPopup2.
    void AddPopup2(D3DXVECTOR3 *pos, int value, D3DCOLOR color);

    void SetColor(D3DCOLOR color)
    {
        this->color = color;
    }
    void SetHighlight(int highlight)
    {
        this->highlight = highlight;
    }
    // Sends unk_b7c[idx] interrupt 99. Must be an inline: under /Od only
    // an inline's constant argument leaves the index multiplied out at
    // run time, as Player::DeletedCallback does.
    void InterruptUnkB7c(int idx)
    {
        this->unk_b7c[idx].pendingInterrupt = 99;
    }
    // Sends unk_b7c[idx] interrupt `interrupt`, the same way.
    void SetUnkB7cInterrupt(int idx, int interrupt)
    {
        this->unk_b7c[idx].pendingInterrupt = interrupt;
    }
    // unk_74d4, and setting it while sending unk_498 the same
    // interrupt. Inlines: each read takes a temporary in ResetHomingAndText.
    int GetUnk74d4()
    {
        return this->unk_74d4;
    }
    void SetUnk74d4(int value)
    {
        this->unk_498.pendingInterrupt = value;
        this->unk_74d4 = value;
    }

    // The glyph DrawStrings draws each character with.
    Sprite glyph;
    // The digit the popups are drawn with.
    Sprite digit;
    // Player::DeletedCallback sends these interrupt 99.
    Sprite unk_498;
    Sprite unk_6e4;
    Sprite unk_930;
    Sprite unk_b7c[4];
    // Whether unk_b7c[i] is drawn dimmed in blue by DrawStrings.
    int unk_14ac[4];
    AsciiString strings[0x100];
    // How many strings are queued.
    int stringCount;
    // The color text is queued with.
    D3DCOLOR color;
    // The scale of the characters added next.
    float scaleX;
    float scaleY;
    // The strings queued next are drawn inside the play area.
    int inArena;
    // Drawn with the second glyph set, as in th06.
    int highlight;
    // Player::DeletedCallback sets it to 99.
    int unk_74d4;
    // The width DrawStrings gives each glyph: 14, as in th06.
    int unk_74d8;
    // As th06's nextPopup and nextPopup2.
    int unk_74dc;
    int unk_74e0;
    int unk_74e4;
    PauseMenu pauseMenu;
    ContinueMenu continueMenu;
    Sprite unk_9e50;
    AsciiPopup popups[0x2d3];
};
ASSERT_SIZE(AsciiText, 0x11194);
extern AsciiText g_AsciiText;

// Adds 1 to `*count` unless it reached 999999; `max` goes unused.
// ContinueMenu::UpdateContinueMenu calls it on six counters of
// g_Unknown626270. Not in th06.
void IncrementCounterCapped(DWORD *count, DWORD max);

// AsciiText's chain elements, statically allocated as in th06: the
// update element and the two draw elements.
extern ChainElem g_AsciiCalcChain;
extern ChainElem g_AsciiDrawChain;
extern ChainElem g_AsciiDrawChain2;
