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

    unsigned char unk_0[0x74c0];
    // The color text is queued with.
    D3DCOLOR color;
};
extern AsciiText g_AsciiText;
