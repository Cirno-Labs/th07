// cflags: /Os
// The /Os file with Unknown49fbf0, th06's Gui, from 0x427b21 to
// Unknown626270 at 0x42d560.

#include "unsorted.h"

void Unknown49fbf0::EndBombName()
{
    this->impl->bombName.pendingInterrupt = 1;
    this->impl->bombNameBar.SetPendingInterrupt(2);
}

void Unknown49fbf0::EndSpellCardName()
{
    this->impl->spellName.pendingInterrupt = 1;
    this->impl->spellNameBar.SetPendingInterrupt(2);
    this->impl->unk_7774.SetPendingInterrupt(2);
}

void Unknown49fbf0::HideInterface()
{
    this->impl->unk_6e44.spriteIndex = -1;
    this->impl->unk_7090.spriteIndex = -1;
    this->impl->unk_72dc.spriteIndex = -1;
    this->impl->unk_1fba0 = 0;
}

int Unknown49fbf0::Func427be2()
{
    return this->impl->msg.skippable;
}

void Unknown49fbf0::ShowBonusPopup(int bonus)
{
    this->impl->bonusPopup.pos = D3DXVECTOR3(416.0f, 48.0f, 0.0f);
    this->impl->bonusPopup.shown = 1;
    this->impl->bonusPopup.timer.Set(0);
    this->impl->bonusPopup.value = bonus;
    g_App.redrawFrames = 2;
}

void Unknown49fbf0::Func427c81(int points, int unk)
{
    this->impl->unk_209e0.pos = D3DXVECTOR3(416.0f, 168.0f, 0.0f);
    this->impl->unk_209e0.shown = unk;
    this->impl->unk_209e0.timer.Set(0);
    this->impl->unk_209e0.value = points;
    g_App.redrawFrames = 2;
}

void Unknown49fbf0::ShowSpellCardBonus(int value)
{
    this->impl->spellBonusPopup.pos = D3DXVECTOR3(224.0f, 16.0f, 0.0f);
    this->impl->spellBonusPopup.shown = 1;
    this->impl->spellBonusPopup.timer.Set(0);
    this->impl->spellBonusPopup.value = value;
    g_App.redrawFrames = 2;
}

// The rectangle of sprite `sprite` in its texture, in pixels.
__forceinline void GetSpriteRect(RECT *rect, int sprite)
{
    rect->left = g_SpriteEngine->sprites[sprite].startX;
    rect->top = g_SpriteEngine->sprites[sprite].startY;
    rect->right = g_SpriteEngine->sprites[sprite].endX;
    rect->bottom = g_SpriteEngine->sprites[sprite].endY;
}

void CopySpriteToTexture(int sprite)
{
    RECT dstRect;
    RECT spriteRect;

    GetSpriteRect(&dstRect, 0x609);
    GetSpriteRect(&spriteRect, sprite);
    g_SpriteEngine->CopyTextureRect(0x15, 0x16, &dstRect, &spriteRect);
}

int Unknown49fbf0::OnUpdate(Unknown49fbf0 *gui)
{
    if (g_Unknown626270.isFrozen)
    {
        return 1;
    }
    if (gui->impl->unk_209bc != 0)
    {
        g_App.scene = 3;
        gui->impl->unk_209bc = 0;
    }
    gui->UpdateInterface();
    gui->impl->RunMsg();
    gui->unk_0++;
    if (g_Unknown626270.stage == 6 && gui->unk_0 == 300)
    {
        g_App.PlayMidi(0);
    }
    if ((g_Buttons & 0x100) && g_App.redrawFrames < 8)
    {
        g_App.redrawFrames = 8;
    }
    return 1;
}
