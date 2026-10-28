// cflags: /Os -/Oi
// The /Os file with the result screen: the score file and its sorted
// lists, th06's resultscreen.cpp.

#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ascii.h"
#include "lzss.h"
#include "resultscreen.h"
#include "unsorted.h"

int InsertScore(ListNode *head, Hscr *entry)
{
    int index = 0;
    ListNode *next;

    while (head->next != NULL)
    {
        if (head->next->data != NULL && head->next->data->score <= entry->score)
        {
            break;
        }
        head = head->next;
        index++;
    }
    next = head->next;
    head->next = (ListNode *)MemAlloc(sizeof(ListNode));
    head->next->prev = head;
    head = head->next;
    head->data = entry;
    head->next = next;
    return index;
}

void FreeListNodes(ListNode *head)
{
    ListNode *next;

    head = head->next;
    while (head != NULL)
    {
        next = head->next;
        free(head);
        head = next;
    }
}

ScoreDat *LoadScoreDat(char *path)
{
    Vrsm *vrsm2;
    BOOL found2;
    Th7k *th7k1;
    int rest;
    Th7k *chunk;
    int remaining0;
    BYTE *p2;
    WORD checksumVal0;
    BYTE key;
    int n;
    ScoreDat *dat;
    ScoreDat *unpacked;

    DebugPrint2("info : score load\r\n");
    dat = (ScoreDat *)LoadFileData(path, 1);
    if (dat == NULL)
    {
    fail:
        DebugPrint2("info : score recreate\r\n");
        if (dat != NULL)
        {
            free(dat);
        }
        dat = (ScoreDat *)MemAlloc(sizeof(ScoreDat));
        dat->dataOffset = sizeof(ScoreDat);
        dat->fileSize = sizeof(ScoreDat);
        goto done;
    }
    if (g_FileSize < sizeof(ScoreDat))
    {
        DebugPrint2("warning : score.dat size is short\r\n");
        free(dat);
        goto fail;
    }
    remaining0 = g_FileSize - 2;
    checksumVal0 = 0;
    key = 0;
    n = 0;
    p2 = (BYTE *)dat + 1;
    while (remaining0 > 0)
    {
        key += *p2;
        key = ((key & 0xe0) >> 5) | ((key & 0x1f) << 3);
        p2[1] ^= key;
        if (n >= 2)
        {
            checksumVal0 += p2[1];
        }
        p2++;
        remaining0--;
        n++;
    }
    if (dat->checksum != checksumVal0)
    {
        DebugPrint2("warning : score.dat chksum error\r\n");
        goto fail;
    }
    if (dat->dataOffset != sizeof(ScoreDat))
    {
        DebugPrint2("warning : header size is mismatch\r\n");
        goto fail;
    }
    if (dat->version != 0xb)
    {
        DebugPrint2("warning : score.dat version mismatch\r\n");
        goto fail;
    }
    unpacked = (ScoreDat *)MemAlloc(sizeof(ScoreDat) + 0xa0000);
    memcpy(unpacked, dat, sizeof(ScoreDat));
    LzssDecompress((BYTE *)dat + sizeof(ScoreDat), dat->packedSize, (BYTE *)unpacked + sizeof(ScoreDat),
               dat->dataSize);
    free(dat);
    dat = unpacked;
    rest = dat->fileSize;
    found2 = FALSE;
    chunk = (Th7k *)((BYTE *)dat + dat->dataOffset);
    rest -= dat->dataOffset;
    while (rest > 0)
    {
        if (chunk->magic == 0x4b374854)
        {
            found2 = TRUE;
            th7k1 = chunk;
        }
        if (chunk->magic == 0x4d535256 && chunk->version == 1)
        {
            vrsm2 = (Vrsm *)chunk;
            if (g_App.CheckVersionFile(vrsm2->gameVersion, vrsm2->exeSize, vrsm2->exeChecksum) != 0)
            {
                DebugPrint2("warning : score.dat exesumcheck error\r\n");
                goto fail;
            }
        }
        if (chunk->size == 0)
        {
            DebugPrint2("warning : score.dat chapter size is ZERO\r\n");
            goto fail;
        }
        rest -= chunk->size;
        chunk = (Th7k *)((BYTE *)chunk + chunk->size);
    }
    if (!found2 || th7k1->version != 1)
    {
        DebugPrint2("warning : score.dat version mismatch\r\n");
        goto fail;
    }
done:
    dat->scores = (ListNode *)MemAlloc(sizeof(ListNode));
    dat->scores->next = NULL;
    dat->scores->data = NULL;
    dat->scores->prev = NULL;
    return dat;
}

DWORD LoadHighScores(void *scoreDat, ListNode *list, int shot, int difficulty, BYTE *unk)
{
    Hscr *chunk;
    int left;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    if (list == NULL)
    {
        FreeListNodes(dat1->scores);
        dat1->scores->next = NULL;
        dat1->scores->data = NULL;
        dat1->scores->prev = NULL;
    }
    left = dat1->fileSize;
    chunk = (Hscr *)((BYTE *)dat1 + dat1->dataOffset);
    left -= dat1->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x52435348 && chunk->version == 1 && chunk->shot == shot &&
            chunk->difficulty == difficulty)
        {
            if (list != NULL)
            {
                InsertScore(list, chunk);
            }
            else
            {
                InsertScore(dat1->scores, chunk);
            }
        }
        left -= chunk->size;
        chunk = (Hscr *)((BYTE *)chunk + chunk->size);
    }
    if (unk != NULL)
    {
        *unk = dat1->scores->next != NULL ? (char)dat1->scores->next->data->unk_26 : 0;
    }
    return dat1->scores->next != NULL
               ? (dat1->scores->next->data->score > 100000 ? dat1->scores->next->data->score : 100000)
               : 100000;
}

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
    g_App.AddPlayTime();
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
    packed7 = LzssCompress(buf + sizeof(ScoreDat), dat10->dataSize, &dat10->packedSize);
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
                if (g_Unknown626270.catks[i].attempts[6] == 0)
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
        if (screen->RunStatisticsMenu())
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

    if (g_App.IsScoreDisabled() || (g_App.unk_18c >> 3 & 1))
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
        this->entry.unk_26 = g_Unknown626270.stats->continuesUsed;
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
            if (g_App.IsScoreDisabled() || (g_App.unk_18c >> 3 & 1))
            {
                interrupt = 0x13;
            }
            else if (g_Unknown626270.stats->continuesUsed != 0)
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

int ResultScreen::WaitAfterNameEntry()
{
    Sprite *sprite;

    switch (this->state)
    {
    case 0x10:
        if (this->frame <= 30)
        {
            sprite = &this->sprites[0x28];
            sprite->pendingInterrupt = 0x12;
        }
        if (this->frame >= 90 && WAS_PRESSED(0x1001))
        {
            sprite = &this->sprites[0x28];
            sprite->pendingInterrupt = 2;
            this->frame = 0;
            this->state = 0x11;
        }
        break;
    case 0x11:
        if (this->frame >= 30)
        {
            this->frame = 59;
            this->state = 0xb;
        }
        break;
    }
    return 0;
}

int ResultScreen::RunStatisticsMenu()
{
    int k;
    int j1;
    int i;
    D3DXVECTOR3 pos2;
    Sprite *sprite0;

    switch (this->state)
    {
    case 0x14:
        if (this->frame == 1)
        {
            pos2.x = 56.0f;
            pos2.y = 128.0f;
            pos2.z = 0.0f;
            sprite0 = &this->textSprites[0];
            sprite0->pos = pos2;
            g_App.AddPlayTime();
            g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "総起動時間   %.2d:%.2d:%.2d", g_Unknown626270.plst.runHours,
                                       g_Unknown626270.plst.runMinutes, g_Unknown626270.plst.runSeconds);
            g_App.AddPlayTime();
            this->unk_6c = g_Unknown626270.plst.runSeconds;
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "総プレイ時間 %.2d:%.2d:%.2d", g_Unknown626270.plst.playHours,
                                       g_Unknown626270.plst.playMinutes, g_Unknown626270.plst.playSeconds);
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0,
                                           "プレイ回数　　　 　Easy 　Norm 　Hard 　Luna  Extra Phants  Total");
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "プレイ回数　　　 　Easy 　Norm 　Hard 　Luna  Extra  Total");
            }
            for (i = 0; i < 6; i++)
            {
                sprite0++;
                pos2.y += 17.0f;
                sprite0->pos = pos2;
                if (g_Unknown626270.Func42f94c())
                {
                    g_SpriteEngine->RenderText(
                        sprite0, 0xffffff, 0, "%s %6d %6d %6d %6d %6d %6d %6d", g_ShotNames[i],
                        g_Unknown626270.plst.counts[0].plays[i + 1], g_Unknown626270.plst.counts[1].plays[i + 1],
                        g_Unknown626270.plst.counts[2].plays[i + 1], g_Unknown626270.plst.counts[3].plays[i + 1],
                        g_Unknown626270.plst.counts[4].plays[i + 1], g_Unknown626270.plst.counts[5].plays[i + 1],
                        g_Unknown626270.plst.counts[6].plays[i + 1]);
                }
                else
                {
                    g_SpriteEngine->RenderText(
                        sprite0, 0xffffff, 0, "%s %6d %6d %6d %6d %6d %6d", g_ShotNames[i],
                        g_Unknown626270.plst.counts[0].plays[i + 1], g_Unknown626270.plst.counts[1].plays[i + 1],
                        g_Unknown626270.plst.counts[2].plays[i + 1], g_Unknown626270.plst.counts[3].plays[i + 1],
                        g_Unknown626270.plst.counts[4].plays[i + 1], g_Unknown626270.plst.counts[6].plays[i + 1]);
                }
            }
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "%s %6d %6d %6d %6d %6d %6d %6d", g_ShotNames[6],
                                           g_Unknown626270.plst.counts[0].plays[0], g_Unknown626270.plst.counts[1].plays[0],
                                           g_Unknown626270.plst.counts[2].plays[0], g_Unknown626270.plst.counts[3].plays[0],
                                           g_Unknown626270.plst.counts[4].plays[0], g_Unknown626270.plst.counts[5].plays[0],
                                           g_Unknown626270.plst.counts[6].plays[0]);
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "%s %6d %6d %6d %6d %6d %6d", g_ShotNames[6],
                                           g_Unknown626270.plst.counts[0].plays[0], g_Unknown626270.plst.counts[1].plays[0],
                                           g_Unknown626270.plst.counts[2].plays[0], g_Unknown626270.plst.counts[3].plays[0],
                                           g_Unknown626270.plst.counts[4].plays[0], g_Unknown626270.plst.counts[6].plays[0]);
            }
            sprite0++;
            pos2.y += 34.0f;
            sprite0->pos = pos2;
            g_Unknown626270.plst.counts[6].clears =
                g_Unknown626270.plst.counts[0].clears + g_Unknown626270.plst.counts[1].clears +
                g_Unknown626270.plst.counts[2].clears + g_Unknown626270.plst.counts[3].clears +
                g_Unknown626270.plst.counts[4].clears + g_Unknown626270.plst.counts[5].clears;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "クリア回数  　　 %6d %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].clears, g_Unknown626270.plst.counts[1].clears, g_Unknown626270.plst.counts[2].clears, g_Unknown626270.plst.counts[3].clears, g_Unknown626270.plst.counts[4].clears, g_Unknown626270.plst.counts[5].clears, g_Unknown626270.plst.counts[6].clears);
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "クリア回数  　　 %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].clears, g_Unknown626270.plst.counts[1].clears, g_Unknown626270.plst.counts[2].clears, g_Unknown626270.plst.counts[3].clears, g_Unknown626270.plst.counts[4].clears, g_Unknown626270.plst.counts[6].clears);
            }
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "コンティニュー   %6d %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].continues, g_Unknown626270.plst.counts[1].continues, g_Unknown626270.plst.counts[2].continues, g_Unknown626270.plst.counts[3].continues, g_Unknown626270.plst.counts[4].continues, g_Unknown626270.plst.counts[5].continues, g_Unknown626270.plst.counts[6].continues);
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "コンティニュー   %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].continues, g_Unknown626270.plst.counts[1].continues, g_Unknown626270.plst.counts[2].continues, g_Unknown626270.plst.counts[3].continues, g_Unknown626270.plst.counts[4].continues, g_Unknown626270.plst.counts[6].continues);
            }
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "プラクティス　   %6d %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].practices, g_Unknown626270.plst.counts[1].practices, g_Unknown626270.plst.counts[2].practices, g_Unknown626270.plst.counts[3].practices, g_Unknown626270.plst.counts[4].practices, g_Unknown626270.plst.counts[5].practices, g_Unknown626270.plst.counts[6].practices);
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "プラクティス　   %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].practices, g_Unknown626270.plst.counts[1].practices, g_Unknown626270.plst.counts[2].practices, g_Unknown626270.plst.counts[3].practices, g_Unknown626270.plst.counts[4].practices, g_Unknown626270.plst.counts[6].practices);
            }
            sprite0++;
            pos2.y += 17.0f;
            sprite0->pos = pos2;
            if (g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "リトライ回数  　 %6d %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].retries, g_Unknown626270.plst.counts[1].retries, g_Unknown626270.plst.counts[2].retries, g_Unknown626270.plst.counts[3].retries, g_Unknown626270.plst.counts[4].retries, g_Unknown626270.plst.counts[5].retries, g_Unknown626270.plst.counts[6].retries);
            }
            else
            {
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "リトライ回数  　 %6d %6d %6d %6d %6d %6d", g_Unknown626270.plst.counts[0].retries, g_Unknown626270.plst.counts[1].retries, g_Unknown626270.plst.counts[2].retries, g_Unknown626270.plst.counts[3].retries, g_Unknown626270.plst.counts[4].retries, g_Unknown626270.plst.counts[6].retries);
            }
        }
        if (this->frame < 40)
        {
            sprite0 = &this->textSprites[0];
            for (j1 = 0; j1 < 0xe; j1++, sprite0++)
            {
                sprite0->alpha = this->frame * 0xff / 40;
            }
        }
        else
        {
            this->state = 0x15;
        }
        break;
    case 0x15:
        if (this->frame % 60 == 0)
        {
            g_App.AddPlayTime();
            if (g_Unknown626270.plst.runSeconds != this->unk_6c)
            {
                sprite0 = &this->textSprites[0];
                g_SpriteEngine->RenderText(sprite0, 0xffffff, 0, "総起動時間   %.2d:%.2d:%.2d",
                                           g_Unknown626270.plst.runHours, g_Unknown626270.plst.runMinutes,
                                           g_Unknown626270.plst.runSeconds);
                this->unk_6c = g_Unknown626270.plst.runSeconds;
            }
        }
        if (WAS_PRESSED(0x100b))
        {
            this->state = 0x16;
            this->frame = 0;
        }
        break;
    case 0x16:
        if (this->frame < 20)
        {
            sprite0 = &this->textSprites[0];
            for (k = 0; k < 0xe; k++, sprite0++)
            {
                sprite0->alpha = 0xff - this->frame * 0xff / 20;
            }
        }
        else
        {
            this->state = 0;
            this->frame = 0;
            return 1;
        }
        break;
    }
    return 0;
}

int ResultScreen::DrawStatistics()
{
    Sprite *spr;
    D3DXVECTOR3 cursorPos;
    float progress;
    float rating;
    float slowdown;

    switch (this->state)
    {
    case 0x10:
    case 0x11:
        spr = &this->sprites[0x28];
        {
            D3DCOLOR color = spr->color;

            g_AsciiText.color = color;
        }
        rating = 0.0f;
        progress = g_Unknown626270.difficulty < 4
                       ? g_Unknown626270.progressFrames / 180621.0f
                       : progress = g_Unknown626270.difficulty == 4 ? g_Unknown626270.progressFrames / 80000.0f
                                                                    : g_Unknown626270.progressFrames / 85000.0f;
        cursorPos = spr->pos;
        cursorPos.x += 210.0f;
        cursorPos.y += 32.0f;
        g_AsciiText.AddFormat(&cursorPos, "%9d", g_Unknown626270.stats->guiScore);
        cursorPos.x += 126.0f;
        g_AsciiText.AddFormat(&cursorPos, "%1d", g_Unknown626270.stats->continuesUsed);
        cursorPos.x -= 126.0f;
        if (g_Unknown626270.stats->guiScore < 2000000)
        {
            rating -= 20.0f;
        }
        else if (g_Unknown626270.stats->guiScore < 200000000)
        {
            rating += (g_Unknown626270.stats->guiScore - 2000000) / 198000000.0f * 60.0f - 20.0f;
        }
        else
        {
            rating += 40.0f;
        }
        cursorPos.y += 22.0f;
        g_AsciiText.AddString(&cursorPos, g_DifficultyNames[g_Unknown626270.difficulty]);
        rating += g_DifficultyRatings[g_Unknown626270.difficulty];
        cursorPos.x += 14.0f;
        cursorPos.y += 22.0f;
        if ((g_Unknown626270.unk_93d8 >> 4 & 1) == 0)
        {
            if (progress >= 1.0f)
            {
                progress = 0.99f;
            }
            g_AsciiText.AddFormat(&cursorPos, "    %3.2f%%", progress * 100.0f);
            rating += progress * 70.0f;
        }
        else
        {
            g_AsciiText.AddFormat(&cursorPos, "      100%%");
            rating += 70.0f;
        }
        cursorPos.y += 22.0f;
        g_AsciiText.AddFormat(&cursorPos, "%9d", g_Unknown626270.stats->continuesUsed);
        rating -= g_Unknown626270.stats->continuesUsed * 10.0f;
        cursorPos.y += 22.0f;
        g_AsciiText.AddFormat(&cursorPos, "%9d", (int)g_Unknown626270.stats->deaths);
        rating -= (int)g_Unknown626270.stats->deaths * 5.0f - 10.0f;
        cursorPos.y += 22.0f;
        g_AsciiText.AddFormat(&cursorPos, "%9d", (int)g_Unknown626270.stats->bombsUsed);
        rating -= (int)g_Unknown626270.stats->bombsUsed * 2.0f - 10.0f;
        cursorPos.y += 22.0f;
        g_AsciiText.AddFormat(&cursorPos, "%9d", g_Unknown626270.stats->spellCardsCaptured);
        rating += g_Unknown626270.stats->spellCardsCaptured * g_SpellCardRatings[g_Unknown626270.difficulty];
        slowdown = (g_App.unk_180 / g_App.unk_184 - 0.5f) * 2.0f;
        if (slowdown < 0.0f)
        {
            slowdown = 0.0f;
        }
        else if (slowdown >= 1.0f)
        {
            slowdown = 1.0f;
        }
        slowdown = (1.0f - slowdown) * 100.0f;
        cursorPos.y += 22.0f;
        g_AsciiText.AddFormat(&cursorPos, "    %3.2f%%", slowdown);
        if (slowdown < 50.0f)
        {
            rating -= 70.0f * slowdown / 100.0f;
        }
        else
        {
            rating = -999.0f;
        }
        if (g_Unknown626270.stats->pointItemsTotal < 800)
        {
            rating += 0.01f * g_Unknown626270.stats->pointItemsTotal;
        }
        else
        {
            rating += 8.0f;
        }
        if (g_Unknown626270.stats->grazeTotal < 5000)
        {
            rating += 0.0025f * g_Unknown626270.stats->grazeTotal;
        }
        else
        {
            rating += 12.5f;
        }
        g_AsciiText.color = 0xffffffff;
        break;
    }
    return 0;
}

int ResultScreen::OnDraw(ResultScreen *screen)
{
    char str[2];
    D3DXVECTOR3 unused_;
    float shift;
    float scale3;
    int card;
    float x0;
    D3DXVECTOR3 pos3;
    int col0;
    ListNode *node3;
    Sprite *sprite;
    char name[9];
    int i;
    D3DXVECTOR3 charPos;

    sprite = screen->sprites;
    g_SpriteEngine->Func44f5c0();
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    for (i = 0; i < 0x29; i++, sprite++)
    {
        pos3 = sprite->pos;
        sprite->pos += sprite->posOffset;
        g_SpriteEngine->Func44f770(sprite);
        sprite->pos = pos3;
    }
    sprite = &screen->sprites[0x10];
    if (sprite->pos.x < 640.0f)
    {
        if (screen->listState != 9)
        {
            pos3 = sprite->pos;
            screen->textSprites[0].pos = pos3;
            screen->textSprites[0].pos.x += 64.0f;
            g_SpriteEngine->Func44f770(&screen->textSprites[0]);
            pos3[1] += 18.0f;
            pos3[0] += 24.0f;
            g_AsciiText.color = 0xffe0e0ef;
            g_AsciiText.AddFormat(&pos3, "No  Name      Score(Stage)  Date   Slow");
            pos3[1] += 18.0f;
            node3 = screen->scores[screen->difficulty][screen->character].next;
            for (i = 0; i < 10; i++)
            {
                if (screen->state == 10)
                {
                    if (node3->data->isNew != 0)
                    {
                        g_AsciiText.color = 0xfff0f0ff;
                    }
                    else
                    {
                        g_AsciiText.color = 0xc0ffc0c0;
                    }
                }
                else
                {
                    g_AsciiText.color = 0xffffc0c0;
                }
                g_AsciiText.AddFormat(&pos3, "%2d", i + 1);
                pos3.x += 48.0f;
                if (screen->state == 10 && node3->data->isNew != 0)
                {
                    *(DWORD *)&name[0] = *(DWORD *)"    ";
                    *(DWORD *)&name[4] = *(DWORD *)"    ";
                    name[8] = 0;
                    name[screen->cursor >= 8 ? 7 : screen->cursor] = '_';
                    g_AsciiText.AddFormat(&pos3, "%8s", name);
                }
                if (node3->data->stage <= 6)
                {
                    g_AsciiText.AddFormat(&pos3, "%8s %9d%1d(%d)", node3->data->name, node3->data->score,
                                          (char)node3->data->unk_26, node3->data->stage);
                }
                else if (node3->data->stage == 7 || node3->data->stage == 8)
                {
                    g_AsciiText.AddFormat(&pos3, "%8s %9d%1d(1)", node3->data->name, node3->data->score,
                                          (char)node3->data->unk_26);
                }
                else
                {
                    g_AsciiText.AddFormat(&pos3, "%8s %9d%1d(C)", node3->data->name, node3->data->score,
                                          (char)node3->data->unk_26);
                }
                pos3.x += 320.0f;
                g_AsciiText.AddFormat(&pos3, " %5s   %3.2f", node3->data->date, node3->data->slowdown);
                pos3[1] += 18.0f;
                pos3[0] -= 368.0f;
                node3 = node3->next;
            }
        }
        else
        {
            pos3 = sprite->pos;
            screen->textSprites[10].pos = pos3;
            g_SpriteEngine->Func44f770(&screen->textSprites[10]);
            pos3[1] += 16.0f;
            for (i = 0; i < 10; i++)
            {
                card = screen->spellPage * 10 + i;
                if (card >= 0x8d)
                {
                    break;
                }
                x0 = pos3.x;
                pos3[0] += 320.0f;
                pos3[1] += 16.0f;
                screen->unk_835c.pos = pos3;
                screen->unk_835c.scaleY = 2.375f;
                g_SpriteEngine->Func44f770(&screen->unk_835c);
                pos3[1] -= 16.0f;
                pos3.x = x0;
                screen->textSprites[i].pos = pos3;
                if (g_Unknown626270.catks[card].attempts[screen->unk_28] == 0)
                {
                    g_AsciiText.color = 0xc0c0c0ff;
                }
                else if (g_Unknown626270.catks[card].captures[screen->unk_28] == 0)
                {
                    g_AsciiText.color = 0xffc0a0a0;
                }
                else
                {
                    g_AsciiText.color = 0xfff0f0ff - i * 0x80800;
                }
                g_AsciiText.AddFormat(&pos3, "No.%.2d", card + 1);
                screen->textSprites[i].pos[0] += 96.0f;
                g_SpriteEngine->Func44f770(&screen->textSprites[i]);
                pos3[0] += 496.0f;
                if (g_Unknown626270.catks[card].attempts[screen->unk_28] == 0)
                {
                    g_AsciiText.AddFormat(&pos3, "---/---", g_Unknown626270.catks[card].captures[screen->unk_28],
                                          g_Unknown626270.catks[card].attempts[screen->unk_28]);
                }
                else
                {
                    g_AsciiText.AddFormat(&pos3, "%3d/%3d", g_Unknown626270.catks[card].captures[screen->unk_28],
                                          g_Unknown626270.catks[card].attempts[screen->unk_28]);
                }
                pos3[0] -= 496.0f;
                pos3[0] += 424.0f;
                pos3[1] -= 13.0f;
                g_AsciiText.color = 0xffa08090;
                g_AsciiText.scaleX = 0.8f;
                g_AsciiText.scaleY = 0.8f;
                if (g_Unknown626270.catks[card].attempts[screen->unk_28] != 0)
                {
                    g_AsciiText.AddFormat(&pos3, "MaxBonus %8d", g_Unknown626270.catks[card].maxBonus[screen->unk_28]);
                }
                pos3[0] -= 424.0f;
                pos3[1] += 13.0f;
                g_AsciiText.scaleX = 1.0f;
                g_AsciiText.scaleY = 1.0f;
                if (screen->unk_2c == 0)
                {
                    pos3[1] += 33.0f;
                }
                else if (screen->frame < 20)
                {
                    pos3[1] += (20 - screen->frame) * 33 / 20;
                }
                else
                {
                    pos3[1] += (screen->frame - 20) * 33 / 20;
                }
            }
            if (screen->frame >= 40)
            {
                screen->unk_2c = 0;
            }
        }
    }
    if (screen->state == 10 || screen->state == 14)
    {
        pos3 = D3DXVECTOR3(160.0f, 356.0f, 0.0f);
        for (i = 0; i < 6; i++)
        {
            for (col0 = 0; col0 < 0x10; col0++)
            {
                scale3 = 0.0f;
                shift = 0.0f;
                if (screen->charIndex == i * 16 + col0)
                {
                    g_AsciiText.color = 0xffffffc0;
                    if (screen->frame % 64 < 32)
                    {
                        scale3 = 0.8f * (screen->frame % 32) / 32.0f + 1.2f;
                    }
                    else
                    {
                        scale3 = 2.0f - 0.8f * (screen->frame % 32) / 32.0f;
                    }
                    g_AsciiText.scaleX = scale3;
                    g_AsciiText.scaleY = scale3;
                    shift = scale3 = -(scale3 - 1.0f) * 8.0f;
                }
                else
                {
                    g_AsciiText.color = 0xc0c0c0c0;
                    g_AsciiText.scaleX = 1.0f;
                    g_AsciiText.scaleY = 1.0f;
                }
                charPos = pos3;
                charPos.x += scale3;
                charPos.y += shift;
                str[0] = g_NameAlphabet[i * 16 + col0];
                str[1] = 0;
                if (i == 5)
                {
                    if (col0 == 14)
                    {
                        str[0] = 0x80;
                    }
                    else if (col0 == 15)
                    {
                        str[0] = 0x81;
                    }
                }
                g_AsciiText.AddString(&charPos, str);
                pos3[0] += 20.0f;
            }
            pos3[0] -= col0 * 20;
            pos3[1] += 18.0f;
        }
    }
    g_AsciiText.scaleX = 1.0f;
    g_AsciiText.scaleY = 1.0f;
    if (screen->state >= 11 && screen->state <= 15)
    {
        sprite = &screen->sprites[0x12];
        for (i = 0; i < 6; i++, sprite++)
        {
            g_SpriteEngine->Func44f770(sprite);
        }
        sprite = &screen->sprites[0x18];
        pos3 = sprite->pos;
        sprite++;
        g_AsciiText.AddFormat(&pos3, "No.   Name     Date   Player Score");
        for (i = 0; i < 0xf; i++)
        {
            pos3 = sprite->pos;
            sprite++;
            if (i == screen->replaySlot)
            {
                g_AsciiText.color = 0xffff8080;
            }
            else
            {
                g_AsciiText.color = 0xff808080;
            }
            if (screen->state == 14)
            {
                g_AsciiText.AddFormat(&pos3, "No.%.2d %8s %5s  %7s %9d0", i + 1, screen->name, screen->replay.date,
                                      g_ShotTypeNames[g_Unknown626270.character * 2 + g_Unknown626270.shotType],
                                      screen->replay.score);
                g_AsciiText.color = 0xfff0f0ff;
                *(DWORD *)&name[0] = *(DWORD *)"    ";
                *(DWORD *)&name[4] = *(DWORD *)"    ";
                name[8] = 0;
                name[screen->cursor >= 8 ? 7 : screen->cursor] = '_';
                g_AsciiText.AddFormat(&pos3, "      %8s", name);
            }
            else if (*(DWORD *)screen->replays[i].magic != *(DWORD *)"T7RP" ||
                     (screen->replays[i].version & 0xfff) != 0x100)
            {
                g_AsciiText.AddFormat(&pos3, "No.%.2d -------- --/--  -------          0", i + 1);
            }
            else
            {
                g_AsciiText.AddFormat(&pos3, "No.%.2d %8s %5s  %7s %9d0", i + 1, screen->replays[i].name,
                                      screen->replays[i].date, g_ShotTypeNames[screen->replays[i].shotType],
                                      screen->replays[i].score);
            }
        }
    }
    g_AsciiText.color = 0xffffffff;
    screen->DrawStatistics();
    if (screen->state == 0x14 || screen->state == 0x15 || screen->state == 0x16)
    {
        sprite = screen->textSprites;
        for (i = 0; i < 0xe; i++, sprite++)
        {
            g_SpriteEngine->Func44f770(sprite);
        }
    }
    return 1;
}

int ResultScreen::AddedCallback(ResultScreen *screen)
{
    int i;
    Sprite *sprite;
    int shot;
    int k;
    Catk *card;
    int j;

    g_Unknown626270.Func42f94c();
    for (i = 0; i < 6; i++)
    {
        for (shot = 0; shot < 6; shot++)
        {
            for (k = 0; k < 10; k++)
            {
                screen->defaultScores[i][shot][k].score = 100000 - k * 10000;
                screen->defaultScores[i][shot][k].slowdown = 0.0f;
                screen->defaultScores[i][shot][k].magic = *(DWORD *)"DMYS";
                screen->defaultScores[i][shot][k].difficulty = i;
                screen->defaultScores[i][shot][k].version = 1;
                screen->defaultScores[i][shot][k].size2 = 0x28;
                screen->defaultScores[i][shot][k].size = 0x28;
                screen->defaultScores[i][shot][k].stage = 1;
                screen->defaultScores[i][shot][k].isNew = 0;
                screen->defaultScores[i][shot][k].unk_26 = 0;
                screen->AddScore(&screen->defaultScores[i][shot][k], i, shot);
                strcpy(screen->defaultScores[i][shot][k].name, "--------");
                strcpy(screen->defaultScores[i][shot][k].date, "--/--");
            }
        }
    }
    if (screen->state != 0x13)
    {
        if (g_SpriteEngine->LoadSurface(0, "data/result/result.jpg") != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x2a, "data/result00.anm", 0x900) != 0)
        {
            return -1;
        }
        sprite = screen->sprites;
        for (i = 0; i < 0x29; i++, sprite++)
        {
            sprite->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            sprite->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            g_SpriteEngine->StartScript(sprite, i + 0x900);
        }
        g_SpriteEngine->InitSprite(&screen->unk_835c, 0x910);
        sprite = screen->textSprites;
        for (i = 0; i < 0xf; i++, sprite++)
        {
            g_SpriteEngine->InitSprite(sprite, i + 0x715);
            sprite->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            sprite->flags |= 0xc00;
            sprite->fontWidth = 15;
            sprite->fontHeight = 15;
        }
    }
    screen->savedCursor = 0;
    screen->scoreDat = LoadScoreDat("score.dat");
    for (i = 0; i < 6; i++)
    {
        for (shot = 0; shot < 6; shot++)
        {
            LoadHighScores(screen->scoreDat, &screen->scores[i][shot], shot, i, NULL);
        }
    }
    screen->lsnm.magic = 0x4d4e534c;
    screen->lsnm.version = 1;
    screen->lsnm.size2 = 0x18;
    screen->lsnm.size = 0x18;
    strcpy(screen->lsnm.name, "        ");
    screen->unk_40 = Func445110(screen->scoreDat, &screen->lsnm);
    if (screen->state != 10 && screen->state != 0x12)
    {
        Func445069(screen->scoreDat, g_Unknown626270.catks);
        LoadClears(screen->scoreDat, g_Unknown626270.clrds);
        g_Unknown626270.Func42f94c();
        LoadPracticeScores(screen->scoreDat, &g_Unknown626270.pscrs[0][0][0]);
    }
    if (screen->state == 0x12)
    {
        if (g_Unknown626270
                .pscrs[g_Unknown626270.character * 2 + g_Unknown626270.shotType][g_Unknown626270.stage - 1]
                      [g_Unknown626270.difficulty]
                .score < g_Unknown626270.stats->score)
        {
            g_Unknown626270
                .pscrs[g_Unknown626270.character * 2 + g_Unknown626270.shotType][g_Unknown626270.stage - 1]
                      [g_Unknown626270.difficulty]
                .score = g_Unknown626270.stats->score;
        }
        screen->state = 0xb;
        strcpy(screen->name, screen->lsnm.name);
    }
    for (i = 0; i < 7; i++)
    {
        card = g_Unknown626270.catks;
        screen->unk_50[i] = 0;
        for (j = 0; j < 0x8d; j++, card++)
        {
            if (card->magic != 0x4b544143 || card->version != 1)
            {
                continue;
            }
            if (card->captures[i] != 0)
            {
                screen->unk_50[i]++;
            }
        }
    }
    screen->unk_24 = 6;
    screen->unk_28 = 6;
    screen->unk_2c = 0;
    screen->unk_8110.spriteIndex = -1;
    if (screen->state == 0x13)
    {
        ResultScreen::DeletedCallback(screen);
        return -1;
    }
    return 0;
}

int ResultScreen::DeletedCallback(ResultScreen *screen)
{
    int i;
    int j;

    if (screen->scoreDat != NULL)
    {
        SaveScoreDat(screen);
        ReleaseScoreDat(screen->scoreDat);
    }
    screen->scoreDat = NULL;
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 6; j++)
        {
            screen->ReleaseScores(i, j);
        }
    }
    g_SpriteEngine->ReleaseAnm(0x2a);
    g_SpriteEngine->ReleaseAnm(0x2b);
    g_SpriteEngine->ReleaseAnm(0x2c);
    g_SpriteEngine->ReleaseAnm(0x2d);
    g_SpriteEngine->ReleaseSurface(0);
    g_Chain.Remove(screen->drawChain);
    screen->drawChain = NULL;
    delete screen;
    screen = NULL;
    return 0;
}

int ResultScreen::RegisterChain(int unk)
{
    ResultScreen *screen = new ResultScreen;

    DebugPrint2("Stg.PlayTimeAll = %d\r\n", g_Unknown626270.progressFrames);
    if (unk == 1)
    {
        if (!(g_Unknown626270.unk_93d8 & 1))
        {
            screen->state = 10;
        }
        else
        {
            screen->state = 0x12;
        }
    }
    else if (unk == 2)
    {
        screen->state = 0x13;
        ResultScreen::AddedCallback(screen);
        return 0;
    }
    screen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    screen->calcChain->addedCallback = (ChainCallback)ResultScreen::AddedCallback;
    screen->calcChain->deletedCallback = (ChainCallback)ResultScreen::DeletedCallback;
    screen->calcChain->arg = screen;
    if (g_Chain.AddToCalcChain(screen->calcChain, 0xe) != 0)
    {
        return -1;
    }
    screen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    screen->drawChain->arg = screen;
    g_Chain.AddToDrawChain(screen->drawChain, 0xd);
    return 0;
}
