// cflags: /Os -/Oi
// The /Os file with the result screen: the score file and its sorted
// lists, th06's resultscreen.cpp.

#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "resultscreen.h"
#include "unsorted.h"

int LoadPracticeScores(void *scoreDat, Pscr *pscrs)
{
    Pscr *p;
    Pscr *chunk;
    int i3;
    int j0;
    int rest3;
    int k1;
    ScoreDat *dat10 = (ScoreDat *)scoreDat;

    if (pscrs == NULL)
    {
        return -1;
    }
    p = pscrs;
    for (i3 = 0; i3 < 6; i3++)
    {
        for (j0 = 0; j0 < 6; j0++)
        {
            for (k1 = 0; k1 < 4; k1++, p++)
            {
                memset(p, 0, sizeof(Pscr));
                p->magic = 0x52435350;
                p->size2 = sizeof(Pscr);
                p->size = sizeof(Pscr);
                p->version = 1;
                p->shot = i3;
                p->difficulty = k1;
                p->stage = j0;
                p->unk_0c = 0;
            }
        }
    }
    chunk = (Pscr *)((BYTE *)dat10 + dat10->dataOffset);
    rest3 = dat10->fileSize - dat10->dataOffset;
    while (rest3 > 0)
    {
        if (chunk->magic == 0x52435350 && chunk->version == 1)
        {
            p = chunk;
            if (p->shot >= 6 || p->difficulty >= 5 || p->stage >= 7)
            {
                break;
            }
            pscrs[p->shot * 6 * 4 + p->stage * 4 + p->difficulty] = *p;
        }
        rest3 -= chunk->size;
        chunk = (Pscr *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadPlstChunk(void *scoreDat, Plst *plst)
{
    Plst *chunk;
    int rest3;
    ScoreDat *dat10 = (ScoreDat *)scoreDat;

    chunk = (Plst *)((BYTE *)dat10 + dat10->dataOffset);
    rest3 = dat10->fileSize - dat10->dataOffset;
    while (rest3 > 0)
    {
        if (chunk->magic == 0x54534c50 && chunk->version == 1)
        {
            *plst = *chunk;
        }
        rest3 -= chunk->size;
        chunk = (Plst *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

void ReleaseScoreDat(ScoreDat *dat)
{
    FreeListNodes(dat->scores);
    MemFree(dat->scores);
    free(dat);
}

void SaveScoreDat(ResultScreen *screen)
{
    ScoreDat *header;
    BYTE *p24;
    BYTE key26;
    int m;
    BYTE prev3;
    ScoreDat *dat10;
    void *packed7;
    Vrsm vrsm13;
    int l2;
    int j;
    Pscr *pscr0;
    Catk *catk1;
    Clrd *clrd26;
    int s14;
    ListNode *node;
    int size0 = 0;
    BYTE *buf;
    int n;
    int i;

    buf = (BYTE *)MemAlloc(0xa0000);
    memcpy(buf + size0, screen->scoreDat, sizeof(ScoreDat));
    size0 += sizeof(ScoreDat);
    screen->th7k.magic = 0x4b374854;
    screen->th7k.size2 = sizeof(Th7k);
    screen->th7k.size = sizeof(Th7k);
    screen->th7k.version = 1;
    memcpy(buf + size0, &screen->th7k, sizeof(Th7k));
    size0 += sizeof(Th7k);
    for (i = 0; i < 6; i++)
    {
        for (s14 = 0; s14 < 6; s14++)
        {
            node = screen->scores[i][s14].next;
            n = 0;
            while (node != NULL)
            {
                if (node->data->magic == 0x52435348)
                {
                    node->data->shot = s14;
                    node->data->difficulty = i;
                    node->data->size2 = sizeof(Hscr);
                    node->data->size = sizeof(Hscr);
                    node->data->version = 1;
                    node->data->isNew = 0;
                    memcpy(buf + size0, node->data, sizeof(Hscr));
                    size0 += sizeof(Hscr);
                }
                node = node->next;
                n++;
                if (n >= 10)
                {
                    break;
                }
            }
        }
    }
    clrd26 = g_Unknown626270.clrds;
    for (i = 0; i < 6; i++, clrd26++)
    {
        clrd26->magic = 0x44524c43;
        clrd26->size2 = sizeof(Clrd);
        clrd26->size = sizeof(Clrd);
        clrd26->version = 1;
        memcpy(buf + size0, clrd26, sizeof(Clrd));
        size0 += sizeof(Clrd);
    }
    catk1 = g_Unknown626270.catks;
    for (i = 0; i < 0x8d; i++, catk1++)
    {
        if (catk1->magic == 0x4b544143)
        {
            catk1->index = i;
            catk1->size2 = sizeof(Catk);
            catk1->size = sizeof(Catk);
            catk1->version = 1;
            memcpy(buf + size0, catk1, sizeof(Catk));
            size0 += sizeof(Catk);
        }
    }
    pscr0 = &g_Unknown626270.pscrs[0][0][0];
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 6; j++)
        {
            for (l2 = 0; l2 < 4; l2++, pscr0++)
            {
                if (pscr0->score != 0)
                {
                    memcpy(buf + size0, pscr0, sizeof(Pscr));
                    size0 += sizeof(Pscr);
                }
            }
        }
    }
    memcpy(buf + size0, &screen->lsnm, sizeof(Lsnm));
    size0 += sizeof(Lsnm);
    g_App.Func43a27f();
    memcpy(buf + size0, &g_Unknown626270.plst, sizeof(Plst));
    size0 += sizeof(Plst);
    vrsm13.magic = 0x4d535256;
    vrsm13.version = 1;
    vrsm13.size2 = sizeof(Vrsm);
    vrsm13.size = sizeof(Vrsm);
    vrsm13.unk_09 = 0;
    strcpy(vrsm13.gameVersion, "0100b");
    vrsm13.exeSize = g_App.exeSize;
    vrsm13.exeChecksum = g_App.exeChecksum;
    memcpy(buf + size0, &vrsm13, sizeof(Vrsm));
    size0 += sizeof(Vrsm);
    dat10 = (ScoreDat *)buf;
    dat10->dataSize = size0 - sizeof(ScoreDat);
    dat10->fileSize = size0;
    packed7 = Func45ead0(buf + sizeof(ScoreDat), dat10->dataSize, &dat10->packedSize);
    memcpy(buf + sizeof(ScoreDat), packed7, dat10->packedSize);
    GlobalFree(packed7);
    size0 = dat10->packedSize + sizeof(ScoreDat);
    header = (ScoreDat *)buf;
    header->dataOffset = sizeof(ScoreDat);
    header->checksum = 0;
    header->key = g_Rng.GetRandomU16InRange(256);
    header->unk_06 = g_Rng.GetRandomU16InRange(256);
    header->version = 0xb;
    for (m = 4; m < size0; m++)
    {
        header->checksum += buf[m];
    }
    key26 = 0;
    prev3 = 0;
    p24 = (BYTE *)header + 1;
    m = size0;
    m -= 2;
    key26 = *p24;
    while (m > 0)
    {
        prev3 = p24[1];
        key26 = ((key26 & 0xe0) >> 5) | ((key26 & 0x1f) << 3);
        p24[1] ^= key26;
        key26 += prev3;
        p24++;
        m--;
    }
    WriteToFile("score.dat", buf, size0);
    free(buf);
}

int MoveCursorVertical(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int LoadScreenPlstChunk(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->unk_24--;
        if (screen->unk_24 < 0)
        {
            screen->unk_24 += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->unk_24++;
        if (screen->unk_24 >= count)
        {
            screen->unk_24 -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int MoveCursorHorizontal(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int ResultScreen::AddScore(Hscr *entry, int difficulty, int shot)
{
    return InsertScore(&this->scores[difficulty][shot], entry);
}

void ResultScreen::ReleaseScores(int difficulty, int shot)
{
    FreeListNodes(&this->scores[difficulty][shot]);
}

int ResultScreen::OnUpdate(ResultScreen *screen)
{
    int i;
    Sprite *sprite;
    int shot;
    int k;

    switch (screen->state)
    {
    case 0x12:
        g_App.scene = 1;
        return 0;
    case 0x13:
        return 0;
    case 0:
    menu:
        if (screen->frame == 0)
        {
            sprite = screen->sprites;
            for (i = 0; i < 0x29; i++, sprite++)
            {
                sprite->pendingInterrupt = 1;
                sprite->flags |= 0x20;
                sprite->color = sprite->color;
            }
            sprite = screen->sprites;
            for (i = 0; i <= 8; i++, sprite++)
            {
                if (i == screen->cursor)
                {
                    sprite->color = 0xffffffff;
                    sprite->posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
                }
                else
                {
                    sprite->color = 0xb0ffffff;
                    sprite->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                }
            }
            if (!g_Unknown626270.Func42f94c())
            {
                screen->sprites[5].flags &= ~2;
                screen->sprites[6].posOffset.y += -32.0f;
                screen->sprites[7].posOffset.y += -32.0f;
                screen->sprites[8].posOffset.y += -32.0f;
            }
            else
            {
                screen->sprites[5].flags |= 2;
            }
        }
        if (screen->frame < 20)
        {
            break;
        }
        screen->state++;
        screen->frame = 0;
    case 1:
        i = MoveCursorVertical(screen, 9);
        if (screen->cursor == 5 && !g_Unknown626270.Func42f94c())
        {
            screen->cursor += i;
        }
        sprite = screen->sprites;
        for (i = 0; i <= 8; i++, sprite++)
        {
            if (i == screen->cursor)
            {
                sprite->color = 0xffffffff;
                sprite->posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
            }
            else
            {
                sprite->color = 0xb0ffffff;
                sprite->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            }
        }
        if (!g_Unknown626270.Func42f94c())
        {
            screen->sprites[5].flags &= ~2;
            screen->sprites[6].posOffset.y += -32.0f;
            screen->sprites[7].posOffset.y += -32.0f;
            screen->sprites[8].posOffset.y += -32.0f;
        }
        if (WAS_PRESSED(0xa))
        {
            sprite = screen->sprites;
            if (screen->cursor == 8)
            {
                goto quit;
            }
            screen->cursor = 8;
            g_SoundPlayer.QueueSound(0xb, 0);
        }
        if (WAS_PRESSED(0x1001))
        {
            sprite = screen->sprites;
            switch (screen->cursor)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                for (i = 0; i < 0x29; i++, sprite++)
                {
                    sprite->pendingInterrupt = screen->cursor + 3;
                }
                screen->difficulty = screen->cursor;
                screen->state = screen->cursor + 3;
                screen->listState = screen->state;
                screen->frame = 0;
                screen->cursor = screen->savedCursor;
                screen->character = -1;
                screen->spellPage = -1;
                break;
            case 6:
                for (i = 0; i < 0x29; i++, sprite++)
                {
                    sprite->pendingInterrupt = 10;
                }
                screen->difficulty = screen->cursor;
                screen->state = 9;
                screen->listState = screen->state;
                screen->frame = 0;
                screen->character = -1;
                screen->cursor = screen->savedSpellPage;
                screen->spellPage = -1;
                break;
            case 7:
                for (i = 0; i < 0x29; i++, sprite++)
                {
                    sprite->pendingInterrupt = 9;
                }
                screen->difficulty = screen->cursor;
                screen->state = 0x14;
                screen->listState = screen->state;
                screen->frame = 0;
                screen->character = -1;
                break;
            case 8:
            quit:
                for (i = 0; i < 0x29; i++, sprite++)
                {
                    sprite->pendingInterrupt = 2;
                }
                screen->state = 2;
                g_SoundPlayer.QueueSound(0xb, 0);
                screen->frame = 0;
                break;
            }
        }
        break;
    case 2:
        if (screen->frame < 60)
        {
            break;
        }
        g_App.scene = 1;
        return 0;
    case 5:
        if ((g_Buttons & 4) || (g_Buttons & 0x100))
        {
            if (screen->cheatProgress < 3)
            {
                if (WAS_PRESSED(0x10))
                {
                    screen->cheatProgress++;
                }
                else if (WAS_PRESSED(0x160b))
                {
                    screen->cheatProgress = 0;
                }
            }
            else if (screen->cheatProgress < 5)
            {
                if (WAS_PRESSED(0x2000))
                {
                    screen->cheatProgress++;
                }
                else if (WAS_PRESSED(0x160b))
                {
                    screen->cheatProgress = 0;
                }
            }
            else if (screen->cheatProgress < 7)
            {
                if (WAS_PRESSED(0x20))
                {
                    screen->cheatProgress++;
                }
                else if (WAS_PRESSED(0x160b))
                {
                    screen->cheatProgress = 0;
                }
            }
            else if (screen->cheatProgress < 10)
            {
                if (WAS_PRESSED(0x200))
                {
                    screen->cheatProgress++;
                }
                else if (WAS_PRESSED(0x160b))
                {
                    screen->cheatProgress = 0;
                }
            }
            else
            {
                for (shot = 0; shot < 6; shot++)
                {
                    for (k = 0; k < 6; k++)
                    {
                        g_Unknown626270.clrds[shot].stagesClearedNoContinue[k] = 99;
                        g_Unknown626270.clrds[shot].stagesCleared[k] = 99;
                    }
                }
                screen->cheatProgress = 0;
                g_SoundPlayer.QueueSound(0x1c, 0);
            }
        }
        else
        {
            screen->cheatProgress = 0;
        }
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
        if (screen->character != screen->cursor && screen->frame == 20)
        {
            screen->character = screen->cursor;
            g_SpriteEngine->RenderTextCentered(&screen->textSprites[0], 0xffffff, 0, g_ShotNames[screen->character]);
            screen->textSprites[0].alpha = 0xff;
        }
        if (screen->frame < 30)
        {
            break;
        }
        if (MoveCursorHorizontal(screen, 6))
        {
            screen->frame = 0;
            sprite = screen->sprites;
            for (i = 0; i < 0x29; i++, sprite++)
            {
                sprite->pendingInterrupt = screen->difficulty + 3;
            }
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(0xb, 0);
            screen->state = 0;
            screen->frame = 0;
            sprite = screen->sprites;
            for (i = 0; i < 0x29; i++, sprite++)
            {
                sprite->pendingInterrupt = 1;
            }
            screen->savedCursor = screen->cursor;
            screen->cursor = screen->difficulty;
            goto menu;
        }
        break;
    case 9:
        if ((screen->spellPage != screen->cursor || screen->unk_28 != screen->unk_24) && screen->frame == 20)
        {
            screen->spellPage = screen->cursor;
            screen->unk_28 = screen->unk_24;
            for (i = screen->spellPage * 10; i < screen->spellPage * 10 + 10; i++)
            {
                if (i >= 0x8d)
                {
                    break;
                }
                if (g_Unknown626270.catks[i].unk_68 == 0)
                {
                    g_SpriteEngine->RenderText(&screen->textSprites[i % 10], 0xffffff, 0, "？？？？？");
                }
                else
                {
                    g_SpriteEngine->RenderText(&screen->textSprites[i % 10], 0xffffff, 0,
                                               g_Unknown626270.catks[i].name);
                }
                screen->textSprites[i % 10].alpha = 0xff;
            }
            g_SpriteEngine->RenderText(&screen->textSprites[10], 0xffffff, 0,
                                       "%s %3d枚中%3d枚取得（キャラ切り替え↓↑）", g_ShotNames[screen->unk_28], 0x8d,
                                       screen->unk_50[screen->unk_24]);
            screen->textSprites[10].alpha = 0xff;
        }
        if (screen->frame < 30)
        {
            break;
        }
        if (MoveCursorHorizontal(screen, 15))
        {
            screen->frame = 0;
            sprite = screen->sprites;
            for (i = 0; i < 0x29; i++, sprite++)
            {
                sprite->pendingInterrupt = 10;
            }
        }
        else if (LoadScreenPlstChunk(screen, 7))
        {
            screen->frame = 0;
            screen->unk_2c = 1;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(0xb, 0);
            screen->state = 0;
            screen->frame = 0;
            sprite = screen->sprites;
            for (i = 0; i < 0x29; i++, sprite++)
            {
                sprite->pendingInterrupt = 1;
            }
            screen->savedSpellPage = screen->cursor;
            screen->cursor = screen->difficulty;
            goto menu;
        }
        break;
    case 10:
        screen->HandleNameEntry();
        break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        screen->HandleReplaySaving();
        break;
    case 16:
    case 17:
        screen->WaitAfterNameEntry();
        break;
    case 0x14:
    case 0x15:
    case 0x16:
        if (screen->Func448098())
        {
            goto menu;
        }
        break;
    }
    sprite = screen->sprites;
    for (i = 0; i < 0x29; i++, sprite++)
    {
        g_SpriteEngine->Step(sprite);
    }
    screen->frame++;
    return 1;
}

int ResultScreen::HandleNameEntry()
{
    Sprite *sprite;
    int i;
    float rate;

    if (g_App.Func43a18d() || (g_App.unk_18c >> 3 & 1))
    {
        this->state = 0x10;
        this->frame = 0;
        memcpy(g_Unknown626270.catks, g_Unknown626270.savedCatks, sizeof(g_Unknown626270.catks));
        return 0;
    }
    if (this->frame == 0)
    {
        this->character = g_Unknown626270.character * 2 + g_Unknown626270.shotType;
        this->difficulty = g_Unknown626270.difficulty;
        sprite = this->sprites;
        for (i = 0; i < 0x29; i++, sprite++)
        {
            sprite->pendingInterrupt = this->difficulty + 3;
        }
        g_SpriteEngine->RenderTextCentered(&this->textSprites[0], 0xffffff, 0, g_ShotNames[this->character]);
        this->textSprites[0].alpha = 0xff;
        this->entry.shot = this->character;
        this->entry.difficulty = this->difficulty;
        this->entry.score = g_Unknown626270.stats->score;
        this->entry.unk_26 = g_Unknown626270.stats->unk_20;
        this->entry.version = 1;
        this->entry.magic = *(DWORD *)"HSCR";
        if ((g_Unknown626270.unk_93d8 >> 4 & 1) == 0)
        {
            this->entry.stage = g_Unknown626270.stage;
        }
        else
        {
            this->entry.stage = 99;
        }
        this->entry.isNew = 1;
        strcpy(this->entry.name, this->lsnm.name);
        FormatTodayDate(this->entry.date);
        rate = (g_App.unk_180 / g_App.unk_184 - 0.5f) * 2.0f;
        if (rate < 0.0f)
        {
            rate = 0.0f;
        }
        else if (rate >= 1.0f)
        {
            rate = 1.0f;
        }
        this->entry.slowdown = (1.0f - rate) * 100.0f;
        if (this->AddScore(&this->entry, this->difficulty, this->character) >= 10)
        {
            goto done;
        }
        this->cursor = 0;
        if (this->unk_40)
        {
            this->charIndex = 0x5f;
        }
        strcpy(this->name, "");
    }
    if (this->frame < 30)
    {
        return 0;
    }
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        for (;;)
        {
            this->charIndex -= 16;
            if (this->charIndex < 0)
            {
                this->charIndex += 0x60;
            }
            if (g_NameAlphabet[this->charIndex] == ' ')
            {
                continue;
            }
            break;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        for (;;)
        {
            this->charIndex += 16;
            if (this->charIndex >= 0x60)
            {
                this->charIndex -= 0x60;
            }
            if (g_NameAlphabet[this->charIndex] == ' ')
            {
                continue;
            }
            break;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
    }
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        for (;;)
        {
            this->charIndex--;
            if (this->charIndex % 16 == 15)
            {
                this->charIndex += 16;
            }
            if (this->charIndex < 0)
            {
                this->charIndex = 15;
            }
            if (g_NameAlphabet[this->charIndex] == ' ')
            {
                continue;
            }
            break;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        for (;;)
        {
            this->charIndex++;
            if (this->charIndex % 16 == 0)
            {
                this->charIndex -= 16;
            }
            if (g_NameAlphabet[this->charIndex] == ' ')
            {
                continue;
            }
            break;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
    }
    if (WAS_PRESSED_OR_HELD(0x1001))
    {
        int pos = this->cursor >= 8 ? 7 : this->cursor;

        if (this->charIndex < 0x5e)
        {
            this->entry.name[pos] = g_NameAlphabet[this->charIndex];
        }
        else if (this->charIndex == 0x5e)
        {
            this->entry.name[pos] = ' ';
        }
        else
        {
            goto end;
        }
        if (this->cursor < 8)
        {
            this->cursor++;
            if (this->cursor == 8)
            {
                this->charIndex = 0x5f;
            }
        }
        g_SoundPlayer.QueueSound(10, 0);
    }
    if (WAS_PRESSED_OR_HELD(0xa))
    {
        int pos2 = this->cursor >= 8 ? 7 : this->cursor;

        if (this->cursor > 0)
        {
            this->cursor--;
            this->entry.name[pos2] = ' ';
            this->entry.name[this->cursor] = ' ';
        }
        g_SoundPlayer.QueueSound(0xb, 0);
    }
    if (WAS_PRESSED(8))
    {
    end:
        g_SoundPlayer.QueueSound(0xb, 0);
    done:
        this->state = 0x10;
        this->frame = 0;
        sprite = this->sprites;
        for (i = 0; i < 0x29; i++, sprite++)
        {
            sprite->pendingInterrupt = 2;
        }
        strcpy(this->name, this->entry.name);
        strcpy(this->lsnm.name, this->name);
    }
    return 0;
}

void FormatTodayDate(char *date)
{
    time_t t;
    struct tm *tm;

    time(&t);
    tm = localtime(&t);
    strftime(date, 6, "%m/%d", tm);
}

int ResultScreen::HandleReplaySaving()
{
    Sprite *spr;
    int interrupt;
    int k;
    ReplayHeader *replay;
    char path[0x40];

    switch (this->state)
    {
    case 0xb:
        if (this->frame == 60)
        {
            if (g_App.Func43a18d() || (g_App.unk_18c >> 3 & 1))
            {
                interrupt = 0x13;
            }
            else if (g_Unknown626270.stats->unk_20 != 0)
            {
                interrupt = 0xe;
            }
            else
            {
                interrupt = 0xb;
            }
            spr = this->sprites;
            for (k = 0; k < 0x29; k++, spr++)
            {
                spr->pendingInterrupt = interrupt;
            }
            if (interrupt != 0xb)
            {
                this->state = 0xc;
            }
            this->cursor = 0;
        }
        spr = &this->sprites[0x13];
        if (this->cursor == 0)
        {
            spr->color = spr->color & 0xff000000 | 0xff6060;
            spr[1].color = spr[1].color & 0xff000000 | 0x606060;
        }
        else
        {
            spr->color = spr->color & 0xff000000 | 0x606060;
            spr[1].color = spr[1].color & 0xff000000 | 0xff6060;
        }
        if (this->frame < 80)
        {
            return 0;
        }
        MoveCursorHorizontal(this, 2);
        if (WAS_PRESSED(0xa) || WAS_PRESSED(8))
        {
            goto cancel;
        }
        if (WAS_PRESSED(0x1001))
        {
            if (this->cursor == 0)
            {
            chooseSlot:
                g_SoundPlayer.QueueSound(10, 0);
                this->state = 0xd;
                spr = this->sprites;
                for (k = 0; k < 0x29; k++, spr++)
                {
                    spr->pendingInterrupt = 0xc;
                }
                this->frame = 0;
                goto listSlots;
            }
        cancel:
            this->frame = 0;
            g_SoundPlayer.QueueSound(0xb, 0);
            this->state = 2;
            spr = this->sprites;
            for (k = 0; k < 0x29; k++, spr++)
            {
                spr->pendingInterrupt = 2;
            }
        }
        break;
    case 0xc:
        if (this->frame < 20)
        {
            return 0;
        }
        if (WAS_PRESSED(0x1001) || WAS_PRESSED(0xa))
        {
            this->frame = 0;
            g_SoundPlayer.QueueSound(0xb, 0);
            this->state = 2;
            spr = this->sprites;
            for (k = 0; k < 0x29; k++, spr++)
            {
                spr->pendingInterrupt = 2;
            }
        }
        break;
    case 0xd:
    listSlots:
        if (this->frame == 0)
        {
            _mkdir("replay");
            for (k = 0; k < 0xf; k++)
            {
                sprintf(path, "./replay/th7_%.2d.rpy", k + 1);
                replay = (ReplayHeader *)LoadFileData(path, 1);
                if (replay == NULL)
                {
                    continue;
                }
                replay = ValidateReplay(replay, g_FileSize);
                if (replay != NULL)
                {
                    this->replays[k] = *replay;
                    free(replay);
                }
            }
        }
        if (this->frame < 20)
        {
            return 0;
        }
        MoveCursorVertical(this, 0xf);
        this->replaySlot = this->cursor;
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            this->replaySlot = this->cursor;
            this->frame = 0;
            FormatTodayDate(this->replay.date);
            this->replay.score = g_Unknown626270.stats->score;
            if (*(DWORD *)this->replays[this->cursor].magic != *(DWORD *)"T7RP" ||
                (this->replays[this->cursor].version & 0xfff) != 0x100)
            {
                spr = this->sprites;
                for (k = 0; k < 0x29; k++, spr++)
                {
                    spr->pendingInterrupt = 0x11;
                }
                spr = &this->sprites[this->replaySlot + 0x19];
                spr->pendingInterrupt = 0x10;
                this->state = 0xe;
            }
            else
            {
                spr = this->sprites;
                for (k = 0; k < 0x29; k++, spr++)
                {
                    spr->pendingInterrupt = 0xd;
                }
                spr = &this->sprites[this->replaySlot + 0x19];
                spr->pendingInterrupt = 0x10;
                this->state = 0xf;
            }
            this->cursor = 0;
            this->charIndex = 0;
            if (this->unk_40)
            {
                this->charIndex = 0x5f;
            }
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(0xb, 0);
            this->state = 0xb;
            spr = this->sprites;
            for (k = 0; k < 0x29; k++, spr++)
            {
                spr->pendingInterrupt = 2;
            }
            this->frame = 0;
        }
        break;
    case 0xe:
        if (this->frame < 30)
        {
            return 0;
        }
        if (WAS_PRESSED_OR_HELD(0x10))
        {
            for (;;)
            {
                this->charIndex -= 16;
                if (this->charIndex < 0)
                {
                    this->charIndex += 0x60;
                }
                if (g_NameAlphabet[this->charIndex] == ' ')
                {
                    continue;
                }
                break;
            }
            g_SoundPlayer.QueueSound(0xc, 0);
        }
        if (WAS_PRESSED_OR_HELD(0x20))
        {
            for (;;)
            {
                this->charIndex += 16;
                if (this->charIndex >= 0x60)
                {
                    this->charIndex -= 0x60;
                }
                if (g_NameAlphabet[this->charIndex] == ' ')
                {
                    continue;
                }
                break;
            }
            g_SoundPlayer.QueueSound(0xc, 0);
        }
        if (WAS_PRESSED_OR_HELD(0x40))
        {
            for (;;)
            {
                this->charIndex--;
                if (this->charIndex % 16 == 15)
                {
                    this->charIndex += 16;
                }
                if (this->charIndex < 0)
                {
                    this->charIndex = 15;
                }
                if (g_NameAlphabet[this->charIndex] == ' ')
                {
                    continue;
                }
                break;
            }
            g_SoundPlayer.QueueSound(0xc, 0);
        }
        if (WAS_PRESSED_OR_HELD(0x80))
        {
            for (;;)
            {
                this->charIndex++;
                if (this->charIndex % 16 == 0)
                {
                    this->charIndex -= 16;
                }
                if (g_NameAlphabet[this->charIndex] == ' ')
                {
                    continue;
                }
                break;
            }
            g_SoundPlayer.QueueSound(0xc, 0);
        }
        if (WAS_PRESSED_OR_HELD(0x1001))
        {
            int pos = this->cursor >= 8 ? 7 : this->cursor;

            if (this->charIndex < 0x5e)
            {
                this->name[pos] = g_NameAlphabet[this->charIndex];
            }
            else if (this->charIndex == 0x5e)
            {
                this->name[pos] = ' ';
            }
            else
            {
                char replayPath[0x40];

                sprintf(replayPath, "./replay/th7_%.2d.rpy", this->replaySlot + 1);
                SaveReplay(replayPath, this->name);
                this->frame = 0;
                this->state = 2;
                spr = this->sprites;
                for (k = 0; k < 0x29; k++, spr++)
                {
                    spr->pendingInterrupt = 2;
                }
                strcpy(this->lsnm.name, this->name);
            }
            if (this->cursor < 8)
            {
                this->cursor++;
                if (this->cursor == 8)
                {
                    this->charIndex = 0x5f;
                }
            }
            g_SoundPlayer.QueueSound(10, 0);
        }
        if (WAS_PRESSED_OR_HELD(0xa))
        {
            int pos2 = this->cursor >= 8 ? 7 : this->cursor;

            if (this->cursor > 0)
            {
                this->cursor--;
                this->name[pos2] = ' ';
                this->name[this->cursor] = ' ';
            }
            g_SoundPlayer.QueueSound(0xb, 0);
        }
        if (WAS_PRESSED(8))
        {
            goto chooseSlot;
        }
        break;
    case 0xf:
        spr = &this->sprites[0x13];
        if (this->cursor == 0)
        {
            spr->color = spr->color & 0xff000000 | 0xff6060;
            spr[1].color = spr[1].color & 0xff000000 | 0x606060;
        }
        else
        {
            spr->color = spr->color & 0xff000000 | 0x606060;
            spr[1].color = spr[1].color & 0xff000000 | 0xff6060;
        }
        if (this->frame < 20)
        {
            return 0;
        }
        MoveCursorHorizontal(this, 2);
        if (WAS_PRESSED(0xa) || WAS_PRESSED(8))
        {
            goto chooseSlot;
        }
        if (WAS_PRESSED(0x1001))
        {
            this->frame = 0;
            if (this->cursor == 0)
            {
                spr = this->sprites;
                for (k = 0; k < 0x29; k++, spr++)
                {
                    spr->pendingInterrupt = 0x11;
                }
                spr = &this->sprites[this->replaySlot + 0x19];
                spr->pendingInterrupt = 0x10;
                this->state = 0xe;
            }
            else
            {
                goto chooseSlot;
            }
        }
        break;
    }
    return 0;
}
