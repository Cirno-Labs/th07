// The file starting at 0x401000: text in the ASCII font, as th06's
// ascii.cpp.

#include "ascii.h"

void AsciiText::RunScripts()
{
    g_SpriteEngine->Step(&this->unk_498);
    g_SpriteEngine->Step(&this->unk_6e4);
    g_SpriteEngine->Step(&this->unk_b7c[0]);
    g_SpriteEngine->Step(&this->unk_b7c[1]);
    g_SpriteEngine->Step(&this->unk_b7c[2]);
    g_SpriteEngine->Step(&this->unk_b7c[3]);
    g_SpriteEngine->Step(&this->unk_930);
}
