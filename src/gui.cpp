// cflags: /Os
// The /Os file with Unknown49fbf0, th06's Gui, from 0x427b21 to
// Unknown626270 at 0x42d560.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "item.h"
#include "replay.h"
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

int Unknown49fbf0::OnDraw(Unknown49fbf0 *gui)
{
    D3DXVECTOR3 cursor;
    char buf[32];

    g_SpriteEngine->shakeY = 0.0f;
    g_SpriteEngine->shakeX = 0.0f;
    if (gui->impl->stageClearShown != 0)
    {
        cursor.x = 144.0f;
        cursor.y = 128.0f;
        cursor.z = 0.0f;
        g_AsciiText.color = 0xffffff40;
        if (g_Unknown626270.stage < 6)
        {
            g_AsciiText.AddFormat(&cursor, "Stage Clear");
        }
        else
        {
            g_AsciiText.AddFormat(&cursor, "All Clear!");
        }
        cursor.y += 32.0f;
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&cursor, "Clear  = %8d", g_Unknown626270.stage * 1000000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffe0e0ff;
        g_AsciiText.AddFormat(&cursor, "Point  = %8d", gui->impl->stageClearPoints * 50000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Graze  = %8d", gui->impl->stageClearGraze * 500);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Cherry = %8d", gui->impl->stageClearCherry * 10);
        if (g_Unknown626270.stage >= 7 ||
            (g_Unknown626270.stage == 6 && !(g_Unknown626270.unk_93d8 & 1) &&
             (!g_Unknown626270.unk_93d8Bit3 || g_ReplayManager->data->stages[4] != NULL)))
        {
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Player =%9d", (int)g_Unknown626270.stats->lives * 20000000);
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Bomb   = %8d", (int)g_Unknown626270.stats->bombs * 4000000);
        }
        cursor.y += 32.0f;
        switch (g_Unknown626270.difficulty)
        {
        case 0:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Easy Rank    *0.5");
            break;
        case 1:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Normal Rank  *1.0");
            break;
        case 2:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Hard Rank    *1.2");
            break;
        case 3:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Lunatic Rank *1.5");
            break;
        case 4:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Extra Rank   *2.0");
            break;
        case 5:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Phantasm Rank*2.0");
            break;
        }
        cursor.y += 16.0f;
        if (g_Unknown626270.difficulty < 4 && !(g_Unknown626270.unk_93d8 & 1))
        {
            switch (g_Unknown626270.unk_4->lifeCount)
            {
            case 3:
                g_AsciiText.color = 0xffff8080;
                g_AsciiText.AddFormat(&cursor, "Player Penalty*0.5");
                cursor.y += 16.0f;
                break;
            case 4:
                g_AsciiText.color = 0xffff8080;
                g_AsciiText.AddFormat(&cursor, "Player Penalty*0.2");
                cursor.y += 16.0f;
                break;
            }
        }
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&cursor, "Total = %8d0", gui->impl->stageClearTotal);
        g_AsciiText.color = 0xffffffff;
    }
    gui->impl->DrawMsg();
    gui->DrawBanners();
    gui->DrawHud();
    g_AsciiText.inArena = 1;
    if (gui->impl->bonusPopup.shown)
    {
        g_AsciiText.color = 0xffffff80;
        g_AsciiText.AddFormat(&gui->impl->bonusPopup.pos, "BONUS %8d", gui->impl->bonusPopup.value);
        g_AsciiText.color = 0xffffffff;
    }
    switch (gui->impl->unk_209e0.shown)
    {
    case 1:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Full Power Mode!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 2:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Supernatural Border!!");
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 14;
        break;
    case 3:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "CherryPoint Max!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 4:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Border Bonus %7d", gui->impl->unk_209e0.value);
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 14;
        break;
    }
    if (gui->impl->spellBonusPopup.shown)
    {
        g_AsciiText.color = 0xffff0000;
        gui->impl->spellBonusPopup.pos.x = (384.0f - strlen("Spell Card Bonus!") * 16.0f) / 2.0f + 32.0f;
        gui->impl->spellBonusPopup.pos.y = 80.0f;
        g_AsciiText.AddFormat(&gui->impl->spellBonusPopup.pos, "Spell Card Bonus!");
        gui->impl->spellBonusPopup.pos.y += 16.0f;
        sprintf(buf, "+%d", gui->impl->spellBonusPopup.value);
        gui->impl->spellBonusPopup.pos.x = (384.0f - strlen(buf) * 32.0f) / 2.0f + 32.0f;
        g_AsciiText.scaleX = 2.0f;
        g_AsciiText.scaleY = 2.0f;
        g_AsciiText.color = 0xffff8080;
        g_AsciiText.AddString(&gui->impl->spellBonusPopup.pos, buf);
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.color = 0xffffffff;
    }
    g_AsciiText.inArena = 0;
    return 1;
}

void Unknown49fbf0::ShowBombName(int sprite, char *name)
{
    g_SpriteEngine->StartScript(&this->impl->bombPortrait, 0x4a1);
    g_SpriteEngine->SetSprite(&this->impl->bombPortrait, sprite);
    g_SpriteEngine->StartScript(&this->impl->unk_5be4, 0x4a4);
    g_SpriteEngine->SetSprite(&this->impl->unk_5be4, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->unk_607c, 0x4a6);
    g_SpriteEngine->SetSprite(&this->impl->unk_607c, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->bombName, 0x704);
    g_SpriteEngine->RenderText(&this->impl->bombName, 0xf0f0ff, 0, name);
    this->bombNameWidth = strlen(name) * 15 / 2.0f + 16.0f;
    this->impl->bombNameBar.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(14, 0);
    g_App.redrawFrames = 2;
}

void Unknown49fbf0::ShowSpellCardName(int sprite, char *text)
{
    if (sprite >= 0)
    {
        g_SpriteEngine->StartScript(&this->impl->spellPortrait, 0x4a3);
        g_SpriteEngine->SetSprite(&this->impl->spellPortrait, sprite + 0x4ad);
        if (this->impl->spellPortrait.spriteData->width > 256.0f)
        {
            this->impl->spellPortrait.posOffset.x = -288.0f;
        }
        else if (this->impl->spellPortrait.spriteData->width > 128.0f)
        {
            this->impl->spellPortrait.posOffset.x = -112.0f;
        }
        else
        {
            this->impl->spellPortrait.posOffset.x = 0.0f;
        }
    }
    g_SpriteEngine->StartScript(&this->impl->unk_5e30, 0x4a5);
    g_SpriteEngine->SetSprite(&this->impl->unk_5e30, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->unk_62c8, 0x4a7);
    g_SpriteEngine->SetSprite(&this->impl->unk_62c8, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->spellName, 0x705);
    g_SpriteEngine->RenderTextRightAligned(&this->impl->spellName, 0xfff0f0, 0, text);
    this->spellNameWidth = strlen(text) * 15 / 2.0f + 16.0f;
    this->impl->spellNameBar.SetPendingInterrupt(1);
    this->impl->unk_7774.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(14, 0);
    g_App.redrawFrames = 2;
}

int Unknown49fbf0::LoadMsg(char *path)
{
    int i;

    this->FreeMsgFile();
    this->impl->msg.msgFile = (MsgFile *)LoadFileData(path, 0);
    if (this->impl->msg.msgFile == NULL)
    {
        LogPrintf(&g_Log, "error : メッセージファイル %s が読み込めませんでした\r\n", path);
        return -1;
    }
    this->impl->msg.scriptIndex = -1;
    this->impl->msg.instr = NULL;
    for (i = 0; i < this->impl->msg.msgFile->count; i++)
    {
        this->impl->msg.msgFile->scripts[i] = (char *)this->impl->msg.msgFile->scripts[i] + (int)this->impl->msg.msgFile;
    }
    return 0;
}

void Unknown49fbf0::FreeMsgFile()
{
    if (this->impl->msg.msgFile != NULL)
    {
        MemFree(this->impl->msg.msgFile);
        this->impl->msg.msgFile = NULL;
    }
}

void Unknown49fbf0::StartMsg(int index)
{
    this->impl->StartMsgScript(index);
}

void GuiImpl::StartMsgScript(int index)
{
    MsgFile *msgFile;

    if (this->msg.msgFile->count <= index)
    {
        return;
    }
    msgFile = this->msg.msgFile;
    memset(&this->msg, 0, sizeof(GuiMsg));
    this->msg.scriptIndex = index;
    this->msg.msgFile = msgFile;
    this->msg.instr = (MsgInstr *)this->msg.msgFile->scripts[index];
    this->msg.text[0].scriptIndex = -1;
    this->msg.text[1].scriptIndex = -1;
    this->msg.fontSize = 15;
    this->msg.textColors[0] = 0xe8f0ff;
    this->msg.textColors[1] = 0xffe8f0;
    this->msg.shadowColors[0] = 0;
    this->msg.shadowColors[1] = 0;
    this->msg.skippable = 1;
    g_BulletManager.ClearAll(1);
    g_Unknown9a9b00.Func423090(0, 0);
    g_ItemManager.CollectAll();
    if (index % 10 == 0)
    {
        switch (g_Unknown626270.stage)
        {
        case 1:
            CopySpriteToTexture(0x60f);
            break;
        case 2:
            CopySpriteToTexture(0x611);
            break;
        case 3:
            CopySpriteToTexture(0x613);
            break;
        case 4:
            CopySpriteToTexture(0x615);
            break;
        case 5:
            CopySpriteToTexture(0x617);
            break;
        case 6:
            CopySpriteToTexture(0x619);
            g_Unknown1347b00.unk_524 = 2;
            g_Unknown9a9ab8 = 9;
            break;
        case 7:
            CopySpriteToTexture(0x61b);
            g_Unknown1347b00.unk_524 = 1;
            g_Unknown1347b00.unk_520 = 2;
            g_Unknown9a9ab8 = 9;
            break;
        case 8:
            CopySpriteToTexture(0x61d);
            g_Unknown1347b00.unk_524 = 2;
            g_Unknown9a9ab8 = 9;
            break;
        }
    }
}

BOOL Unknown49fbf0::MsgHoldsTimeline()
{
    if (this->impl == NULL)
    {
        return FALSE;
    }
    if (this->impl->msg.timelineRelease > 0)
    {
        return FALSE;
    }
    return this->impl->msg.scriptIndex >= 0;
}

int Unknown49fbf0::Func42ad66()
{
    if (this->impl == NULL)
    {
        return 0;
    }
    return this->impl->msg.scriptIndex >= 0 || this->impl->msg.scriptIndex == -2 ? 1 : 0;
}

int Unknown49fbf0::AddedCallback(Unknown49fbf0 *gui)
{
    return gui->LoadResources();
}

int Unknown49fbf0::DeletedCallback(Unknown49fbf0 *gui)
{
    g_SpriteEngine->ReleaseAnm(0x18);
    g_SpriteEngine->ReleaseAnm(0x1c);
    g_SpriteEngine->ReleaseAnm(0x1d);
    g_SpriteEngine->ReleaseAnm(0x1e);
    g_SpriteEngine->ReleaseAnm(0x1f);
    gui->FreeMsgFile();
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_SpriteEngine->ReleaseAnm(0x15);
        g_SpriteEngine->ReleaseAnm(0x17);
        g_SpriteEngine->ReleaseAnm(0x19);
        g_SpriteEngine->ReleaseAnm(0x1a);
        g_SpriteEngine->ReleaseAnm(0x1b);
        g_SpriteEngine->ReleaseAnm(0x16);
        delete gui->impl;
        gui->impl = NULL;
    }
    return 0;
}

int Unknown49fbf0::RegisterChain()
{
    Unknown49fbf0 *gui = &g_Unknown49fbf0;

    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        memset(gui, 0, sizeof(Unknown49fbf0));
        gui->impl = new GuiImpl;
    }
    g_Unknown49fbf0CalcChain.SetCallback((ChainCallback)Unknown49fbf0::OnUpdate);
    g_Unknown49fbf0CalcChain.addedCallback = (ChainCallback)Unknown49fbf0::AddedCallback;
    g_Unknown49fbf0CalcChain.deletedCallback = (ChainCallback)Unknown49fbf0::DeletedCallback;
    g_Unknown49fbf0CalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_Unknown49fbf0CalcChain, 13) != 0)
    {
        return -1;
    }
    g_Unknown49fbf0DrawChain.SetCallback((ChainCallback)Unknown49fbf0::OnDraw);
    g_Unknown49fbf0DrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_Unknown49fbf0DrawChain, 12);
    return 0;
}

void Unknown49fbf0::RemoveChain()
{
    g_Chain.Remove(&g_Unknown49fbf0CalcChain);
    g_Chain.Remove(&g_Unknown49fbf0DrawChain);
}

int GuiImpl::DrawMsg()
{
    float height;
    D3DXVECTOR3 pos;

    if (this->msg.scriptIndex < 0)
    {
        return -1;
    }
    if (g_Unknown626270.stage == 6 && (this->msg.scriptIndex == 1 || this->msg.scriptIndex == 11))
    {
        return 0;
    }
    if (this->msg.timer < 60)
    {
        height = this->msg.timer.AsFloat() * 48.0f / 60.0f;
    }
    else
    {
        height = 48.0f;
    }
    VertexDiffuseXyzrwh vertices[4];
    vertices[0].pos = D3DXVECTOR3(g_Unknown626270.arena[0] + 16.0f, 384.0f, 0.0f);
    vertices[1].pos = D3DXVECTOR3(g_Unknown626270.arena[0] + 384.0f - 16.0f, 384.0f, 0.0f);
    vertices[2].pos = D3DXVECTOR3(g_Unknown626270.arena[0] + 16.0f, 384.0f + height, 0.0f);
    vertices[3].pos = D3DXVECTOR3(g_Unknown626270.arena[0] + 384.0f - 16.0f, 384.0f + height, 0.0f);
    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    g_SpriteEngine->DrawUnrotated(&this->msg.portraits[0]);
    pos = this->msg.portraits[1].pos;
    this->msg.portraits[1].pos += this->msg.portraits[1].posOffset;
    g_SpriteEngine->DrawUnrotated(&this->msg.portraits[1]);
    this->msg.portraits[1].pos = pos;
    g_SpriteEngine->FlushQuadBatch();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.flags.noDepthTest)
    {
        g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_SpriteEngine->DrawUnrotated(&this->msg.text[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.text[1]);
    g_SpriteEngine->DrawUnrotated(&this->msg.unk_94c[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.unk_94c[1]);
    return 0;
}
