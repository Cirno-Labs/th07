// The file starting at 0x401000: text in the ASCII font, as th06's
// ascii.cpp.

#include <string.h>

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

int AsciiText::OnDraw2(AsciiText *text)
{
    text->DrawPopups();
    return 1;
}

void AsciiText::Initialize()
{
    memset(&this->digit, 0, sizeof(this->digit));
    memset(&this->glyph, 0, sizeof(this->glyph));
    memset(this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(this->pauseMenu));
    memset(&this->continueMenu, 0, sizeof(this->continueMenu));
    memset(this->popups, 0, sizeof(this->popups));
    this->stringCount = 0;
    this->inArena = 0;
    this->highlight = 0;
    this->unk_74dc = 0;
    this->unk_74e0 = 0;
    this->unk_74e4 = 0;
    this->color = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->digit.flags |= 0xc00;
    g_SpriteEngine->InitSprite(&this->digit, 0);
    g_SpriteEngine->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->unk_74d8 = 14;
    this->SetUnk74d4(this->unk_74d4);
}

void AsciiText::StartScripts()
{
    g_SpriteEngine->StartScript(&this->unk_498, 4);
    g_SpriteEngine->StartScript(&this->unk_6e4, 3);
    g_SpriteEngine->StartScript(&this->unk_930, 5);
    g_SpriteEngine->StartScript(&this->unk_b7c[0], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[1], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[2], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[3], 6);
}

int AsciiText::AddedCallback(AsciiText *text)
{
    memset(text, 0, sizeof(AsciiText));
    if (g_SpriteEngine->LoadAnm(1, "data/ascii.anm", 0) != 0)
    {
        return -1;
    }
    if (g_SpriteEngine->LoadAnm(4, "data/capture.anm", 0x724) != 0)
    {
        return -1;
    }
    text->Initialize();
    text->StartScripts();
    return 0;
}

int AsciiText::DeletedCallback(AsciiText *text)
{
    g_SpriteEngine->ReleaseAnm(1);
    g_SpriteEngine->ReleaseAnm(2);
    g_SpriteEngine->ReleaseAnm(4);
    g_SpriteEngine->ReleaseAnm(3);
    return 0;
}

int AsciiText::RegisterChain()
{
    AsciiText *text = &g_AsciiText;

    g_AsciiCalcChain.SetCallback((ChainCallback)AsciiText::OnUpdate);
    g_AsciiCalcChain.addedCallback = (ChainCallback)AsciiText::AddedCallback;
    g_AsciiCalcChain.deletedCallback = (ChainCallback)AsciiText::DeletedCallback;
    g_AsciiCalcChain.arg = text;
    if (g_Chain.AddToCalcChain(&g_AsciiCalcChain, 1) != 0)
    {
        return -1;
    }
    g_AsciiDrawChain.SetCallback((ChainCallback)AsciiText::OnDraw);
    g_AsciiDrawChain.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain, 0x10);
    g_AsciiDrawChain2.SetCallback((ChainCallback)AsciiText::OnDraw2);
    g_AsciiDrawChain2.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain2, 0xb);
    return 0;
}

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcChain);
    g_Chain.Remove(&g_AsciiDrawChain);
}

void AsciiText::AddString(D3DXVECTOR3 *pos, char *text)
{
    AsciiString *str;

    if (this->stringCount >= 0x100)
    {
        return;
    }
    str = &this->strings[this->stringCount];
    this->stringCount++;
    strcpy(str->text, text);
    str->pos = *pos;
    str->color = this->color;
    str->scaleX = this->scaleX;
    str->scaleY = this->scaleY;
    str->inArena = this->inArena;
    if (g_App.HasFlag0Or8())
    {
        str->highlight = this->highlight;
    }
    else
    {
        str->highlight = 0;
    }
}
