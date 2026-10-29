// The file with ReplayManager, around 0x442c60.

#include <string.h>

#include "lzss.h"
#include "replay.h"
#include "unsorted.h"

int ReplayManager::RecordOnUpdate2(ReplayManager *replay)
{
    replay->unk_d6 = 0;
    replay->unk_d4 = g_Rng.seed;
    g_Rng.generated = 0;
    if (g_Unknown626270.unk_93d0)
    {
        replay->unk_d6 |= 0x100;
    }
    g_Unknown626270.unk_93d0 = 0;
    return 1;
}

int ReplayManager::RecordOnUpdate(ReplayManager *replay)
{
    int idx;
    WORD buttons;

    if (!(g_Unknown626270.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    g_Unknown4b9e58 = g_Unknown4b9e50;
    g_Unknown4b9e50 = g_Buttons;
    if (g_Unknown626270.unk_4->unk_25 != 0)
    {
        return 1;
    }
    if (g_App.unk_18c >> 3 & 1)
    {
        return 1;
    }
    idx = g_Unknown626270.stage - 1;
    if (idx >= 7)
    {
        idx = 6;
    }
    buttons = g_Buttons;
    g_Unknown4b9e50 = buttons;
    replay->cursor++;
    replay->stageEnds[idx] = replay->cursor + 1;
    replay->cursor->input = buttons;
    replay->cursor->events = replay->unk_d6;
    if (replay->frame % 30 == 0)
    {
        *replay->extraCursor = (BYTE)g_App.unk_188 | (g_App.unk_2a4 != 0 ? 0x80 : 0);
        replay->extraCursor[1] = (BYTE)g_App.unk_188;
        replay->extraEnds[idx] = replay->extraCursor + 2;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}

int ReplayManager::PlayOnUpdate2(ReplayManager *replay)
{
    if (!(g_Unknown626270.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    if (g_Unknown49fbf0.Func42ad66() && g_Unknown49fbf0.Func427be2() && replay->frame % 3 != 2)
    {
        return 6;
    }
    if (g_Unknown626270.unk_93df == 2 && !g_Unknown9a9b00.Func4232a0() && replay->frame % 5 != 4)
    {
        return 6;
    }
    return 1;
}

int ReplayManager::PlayOnUpdate(ReplayManager *replay)
{
    int unused;

    if (!(g_Unknown626270.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    if (g_Unknown626270.unk_4->unk_25 != 0)
    {
        return 1;
    }
    unused = 0;
    g_Unknown4b9e58 = g_Unknown4b9e50;
    g_Unknown4b9e50 = replay->cursor->input;
    replay->cursor++;
    g_ButtonRepeat = 0;
    if (g_Unknown4b9e58 == g_Unknown4b9e50)
    {
        if (g_ButtonFrames >= 30)
        {
            if (g_ButtonFrames % 8 == 0)
            {
                g_ButtonRepeat = 1;
            }
            if (g_ButtonFrames >= 38)
            {
                g_ButtonFrames = 30;
            }
        }
        g_ButtonFrames++;
    }
    else
    {
        g_ButtonFrames = 0;
    }
    if (replay->frame % 30 == 0)
    {
        g_App.unk_188 = (char)replay->extraCursor[1] & 0x7f;
        g_App.unk_2a8 = (char)replay->extraCursor[1] >> 7;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}

int ReplayManager::RecordAddedCallback(ReplayManager *replay)
{
    ReplayStage *earlier;
    BYTE *extras;
    int idx;
    ReplayStage *cur;

    replay->frame = 0;
    replay->unk_40 = NULL;
    if (replay->data == NULL)
    {
        replay->data = new ReplayHeader;
        *(DWORD *)replay->data->magic = *(DWORD *)"T7RP";
        replay->data->shotType = g_Unknown626270.unk_93d7;
        replay->data->version = 0x1100;
        replay->data->unk_6a = 0x100;
        replay->data->unk_55 = 0x62;
        strcpy(replay->data->exeVersion, "0100b");
        replay->data->exeSize = g_App.exeSize;
        replay->data->exeChecksum = g_App.exeChecksum;
        replay->data->difficulty = g_Unknown626270.difficulty;
        *(DWORD *)replay->data->name = *(DWORD *)"NO NAME";
        *(Unknown626270Unk4 *)replay->data->config = *g_Unknown626270.unk_4;
        for (idx = 0; idx < 7; idx++)
        {
            replay->data->stages[idx] = NULL;
            replay->data->extras[idx] = NULL;
        }
    }
    else if (g_Unknown626270.stage - 2 >= 0)
    {
        earlier = replay->data->stages[g_Unknown626270.stage - 2];
        if (earlier != NULL)
        {
            earlier->score = g_Unknown626270.stats->score;
        }
    }
    idx = g_Unknown626270.stage - 1;
    if (idx >= 7)
    {
        idx = 6;
    }
    if (replay->data->stages[idx] != NULL)
    {
        MemFree(replay->data->stages[idx]);
    }
    if (replay->data->extras[idx] != NULL)
    {
        MemFree(replay->data->extras[idx]);
    }
    replay->data->stages[idx] = (ReplayStage *)MemAlloc(0x70800);
    replay->data->extras[idx] = (BYTE *)MemAlloc(0x70800);
    cur = replay->data->stages[idx];
    extras = replay->data->extras[idx];
    cur->grazeTotal = g_Unknown626270.stats->grazeTotal;
    cur->bombs = g_Unknown626270.stats->bombs;
    cur->lives = g_Unknown626270.stats->lives;
    cur->power = g_Unknown626270.stats->power;
    cur->unk_25 = g_Unknown626270.unk_9634;
    cur->pointItemsTotal = g_Unknown626270.stats->pointItemsTotal;
    cur->unk_20 = g_Unknown626270.unk_95e4;
    cur->unk_26 = g_Unknown626270.unk_93d4;
    cur->unk_08 = g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88;
    cur->unk_0c = g_Unknown626270.unk_9618 - g_Unknown626270.stats->unk_88;
    cur->unk_10 = g_Unknown626270.unk_9620 - g_Unknown626270.stats->unk_88;
    cur->spellCardsCaptured = g_Unknown626270.stats->spellCardsCaptured;
    cur->unk_18 = g_Unknown626270.stats->unk_2c;
    cur->unk_1c = g_Unknown626270.stats->unk_30;
    replay->cursor = cur->frames;
    replay->extraCursor = extras;
    replay->cursor->input = 0;
    replay->unk_82 = 0;
    return 0;
}

ReplayHeader *ValidateReplay(void *data, int size)
{
    BYTE *ptr2;
    int sum;
    BYTE *bytes;
    BYTE b;
    ReplayHeader *file;
    int i;
    ReplayHeader *decoded;

    file = (ReplayHeader *)data;
    if (file == NULL)
    {
        goto fail;
    }
    if (*(DWORD *)file->magic != *(DWORD *)"T7RP")
    {
        goto fail;
    }
    if (file->version != 0x1100)
    {
        goto fail;
    }
    bytes = (BYTE *)file + 0x10;
    b = file->key;
    for (i = 0; i < size - 0x10; i++, bytes++)
    {
        *bytes -= b;
        b += 7;
    }
    ptr2 = &file->key;
    sum = 0x3f000318;
    for (i = 0; i < size - 0xd; i++, ptr2++)
    {
        sum += *ptr2;
    }
    if (sum != file->checksum)
    {
        goto fail;
    }
    decoded = (ReplayHeader *)MemAlloc(file->dataSize + 0x54);
    memcpy(decoded, data, 0x54);
    LzssDecompress((BYTE *)file + 0x54, file->compressedSize, (BYTE *)decoded + 0x54, file->dataSize);
    file = decoded;
    if (file->config[0x25] != 0)
    {
        goto fail;
    }
    if (g_App.CheckVersionFile(file->exeVersion, file->exeSize, file->exeChecksum) != 0)
    {
        goto fail;
    }
    free(data);
    return decoded;

fail:
    free(data);
    return NULL;
}
