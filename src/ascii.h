// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "sprite.h"

class AsciiText
{
public:
    // Adds the text's chain elements. Returns 0, or nonzero on failure.
    static int RegisterChain();
    // Removes them.
    static void RemoveChain();

    // Queues `text` to be drawn at `pos`.
    void AddString(D3DXVECTOR3 *pos, char *text);
    // AddString with printf-style formatting.
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);

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

    unsigned char unk_0[0x498];
    // Player::DeletedCallback sends these interrupt 99.
    Sprite unk_498;
    unsigned char unk_6e4[0x498];
    Sprite unk_b7c[3];
    unsigned char unk_1260[0x6260];
    // The color text is queued with.
    D3DCOLOR color;
    // The scale of the characters added next.
    float scaleX;
    float scaleY;
    unsigned char unk_74cc[0x4];
    // Drawn with the second glyph set, as in th06.
    int highlight;
    // Player::DeletedCallback sets it to 99.
    int unk_74d4;
};
extern AsciiText g_AsciiText;
