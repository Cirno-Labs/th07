// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

class AsciiText
{
public:
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

    unsigned char unk_0[0x74c0];
    // The color text is queued with.
    D3DCOLOR color;
    // The scale of the characters added next.
    float scaleX;
    float scaleY;
    unsigned char unk_74cc[0x4];
    // Drawn with the second glyph set, as in th06.
    int highlight;
};
extern AsciiText g_AsciiText;
