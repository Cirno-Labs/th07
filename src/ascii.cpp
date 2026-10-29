// The file starting at 0x401000: text in the ASCII font, as th06's
// ascii.cpp.

#include "ascii.h"
#include "unsorted.h"

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

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    AsciiPopup *entry;

    if (!g_Unknown626270.unk_93dc && !g_Unknown626270.isInContinueMenu)
    {
        for (entry = text->popups, i = 0; i < 0x2d3; i++, entry++)
        {
            if (!entry->active)
            {
                continue;
            }
            entry->pos.y -= 0.5f * g_App.unk_178;
            entry->timer.Tick();
            if ((entry->timer.current > 60) != 0)
            {
                entry->active = 0;
            }
        }
    }
    else if (g_Unknown626270.unk_93dc)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_Unknown626270.isInContinueMenu)
    {
        text->continueMenu.UpdateContinueMenu();
    }
    text->RunScripts();
    if ((g_Unknown626270.unk_93d8 >> 1) & 1)
    {
        if (text->unk_9e50.scriptIndex == 0)
        {
            g_SpriteEngine->StartScript(&text->unk_9e50, 7);
        }
        g_SpriteEngine->Step(&text->unk_9e50);
    }
    else
    {
        text->unk_9e50.scriptIndex = 0;
    }
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->stringCount = 0;
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    if (text->unk_9e50.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->unk_9e50);
    }
    return 1;
}
