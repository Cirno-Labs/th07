// The file with ReplayManager, around 0x442c60.

#include <string.h>

#include "enemy.h"
#include "lzss.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

int ReplayManager::RecordOnUpdate2(ReplayManager *replay)
{
    replay->frameEvents = 0;
    replay->frameSeed = g_Rng.seed;
    g_Rng.generated = 0;
    if (g_GameManager.pauseRequested)
    {
        replay->frameEvents |= 0x100;
    }
    g_GameManager.pauseRequested = 0;
    return 1;
}

int ReplayManager::RecordOnUpdate(ReplayManager *replay)
{
    int idx;
    WORD buttons;

    if (!(g_GameManager.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    g_Unknown4b9e58 = g_Unknown4b9e50;
    g_Unknown4b9e50 = g_Buttons;
    if (g_GameManager.unk_4->scoreDisabled != 0)
    {
        return 1;
    }
    if (g_App.stateFlags >> 3 & 1)
    {
        return 1;
    }
    idx = g_GameManager.stage - 1;
    if (idx >= 7)
    {
        idx = 6;
    }
    buttons = g_Buttons;
    g_Unknown4b9e50 = buttons;
    replay->cursor++;
    replay->stageEnds[idx] = replay->cursor + 1;
    replay->cursor->input = buttons;
    replay->cursor->events = replay->frameEvents;
    if (replay->frame % 30 == 0)
    {
        *replay->extraCursor = (BYTE)g_App.roundedFps | (g_App.fastClockStreak != 0 ? 0x80 : 0);
        replay->extraCursor[1] = (BYTE)g_App.roundedFps;
        replay->extraEnds[idx] = replay->extraCursor + 2;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}

int ReplayManager::PlayOnUpdate2(ReplayManager *replay)
{
    if (!(g_GameManager.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    if (g_Gui.IsMsgRunning() && g_Gui.IsMsgSkippable() && replay->frame % 3 != 2)
    {
        return 6;
    }
    if (g_GameManager.replayMenuChoice == 2 && !g_EnemyManager.AnyBossActive() && replay->frame % 5 != 4)
    {
        return 6;
    }
    return 1;
}

int ReplayManager::PlayOnUpdate(ReplayManager *replay)
{
    int unused;

    if (!(g_GameManager.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    if (g_GameManager.unk_4->scoreDisabled != 0)
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
        g_App.roundedFps = (char)replay->extraCursor[1] & 0x7f;
        g_App.fpsWarning = (char)replay->extraCursor[1] >> 7;
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
        replay->data->shotType = g_GameManager.shotIndex;
        replay->data->version = 0x1100;
        replay->data->unk_6a = 0x100;
        replay->data->unk_55 = 0x62;
        strcpy(replay->data->exeVersion, "0100b");
        replay->data->exeSize = g_App.exeSize;
        replay->data->exeChecksum = g_App.exeChecksum;
        replay->data->difficulty = g_GameManager.difficulty;
        *(DWORD *)replay->data->name = *(DWORD *)"NO NAME";
        *(Unknown626270Unk4 *)replay->data->config = *g_GameManager.unk_4;
        for (idx = 0; idx < 7; idx++)
        {
            replay->data->stages[idx] = NULL;
            replay->data->extras[idx] = NULL;
        }
    }
    else if (g_GameManager.stage - 2 >= 0)
    {
        earlier = replay->data->stages[g_GameManager.stage - 2];
        if (earlier != NULL)
        {
            earlier->score = g_GameManager.stats->score;
        }
    }
    idx = g_GameManager.stage - 1;
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
    cur->grazeTotal = g_GameManager.stats->grazeTotal;
    cur->bombs = g_GameManager.stats->bombs;
    cur->lives = g_GameManager.stats->lives;
    cur->power = g_GameManager.stats->power;
    cur->unk_25 = g_GameManager.rank;
    cur->pointItemsTotal = g_GameManager.stats->pointItemsTotal;
    cur->rngSeed = g_GameManager.rngSeed;
    cur->unk_26 = g_GameManager.powerItemCombo;
    cur->unk_08 = g_GameManager.bonusPoints - g_GameManager.stats->grazeCount;
    cur->unk_0c = g_GameManager.bonusPointsCap - g_GameManager.stats->grazeCount;
    cur->unk_10 = g_GameManager.invulnerabilityBonus - g_GameManager.stats->grazeCount;
    cur->spellCardsCaptured = g_GameManager.stats->spellCardsCaptured;
    cur->unk_18 = g_GameManager.stats->checksumKeyStage;
    cur->unk_1c = g_GameManager.stats->checksumKeyPoints;
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

int ReplayManager::PlayAddedCallback(ReplayManager *replay)
{
    ReplayStage *cur;
    int idx;
    BYTE *extras;

    replay->frame = 0;
    if (replay->data == NULL)
    {
        replay->data = (ReplayHeader *)LoadFileData(replay->path, !(g_GameManager.unk_93d8 >> 1 & 1));
        replay->data = ValidateReplay(replay->data, g_FileSize);
        if (replay->data == NULL)
        {
            return -1;
        }
        replay->unk_40 = NULL;
        for (idx = 0; idx < 7; idx++)
        {
            replay->stageSizes[idx] = 0;
            replay->extraSizes[idx] = 0;
            if (replay->data->stages[idx] != NULL)
            {
                if (idx < 6 && replay->data->stages[idx + 1] != NULL)
                {
                    replay->stageSizes[idx] = (int)replay->data->stages[idx + 1] - (int)replay->data->stages[idx];
                }
                else
                {
                    replay->stageSizes[idx] = (int)replay->data->extras[idx] - (int)replay->data->stages[idx];
                }
                if (idx < 6 && replay->data->extras[idx + 1] != NULL)
                {
                    replay->extraSizes[idx] = (int)replay->data->extras[idx + 1] - (int)replay->data->extras[idx];
                }
                else
                {
                    replay->extraSizes[idx] = replay->data->dataSize + 0x54 - (int)replay->data->extras[idx];
                }
            }
            if (replay->data->stages[idx] != NULL)
            {
                replay->data->stages[idx] = (ReplayStage *)((int)replay->data->stages[idx] + (int)replay->data);
            }
            if (replay->data->extras[idx] != NULL)
            {
                replay->data->extras[idx] = (BYTE *)((int)replay->data->extras[idx] + (int)replay->data);
            }
        }
    }
    idx = g_GameManager.stage - 1;
    if (idx >= 7)
    {
        idx = 6;
    }
    if (replay->data->stages[idx] == NULL)
    {
        return -1;
    }
    cur = replay->data->stages[idx];
    extras = replay->data->extras[idx];
    g_GameManager.character = replay->data->shotType / 2;
    g_GameManager.shotType = replay->data->shotType % 2;
    g_GameManager.shotIndex = replay->data->shotType;
    g_GameManager.difficulty = replay->data->difficulty;
    g_GameManager.stats->pointItemsTotal = cur->pointItemsTotal;
    g_GameManager.rank = cur->unk_25;
    g_GameManager.SetLives(cur->lives);
    g_GameManager.RerollChecksumKeys();
    g_GameManager.SetBombs(cur->bombs);
    g_GameManager.SetPower(cur->power);
    g_GameManager.RerollChecksumKeys();
    g_GameManager.stats->grazeTotal = cur->grazeTotal;
    replay->cursor = cur->frames;
    g_GameManager.powerItemCombo = cur->unk_26;
    g_GameManager.bonusPoints = cur->unk_08 + g_GameManager.stats->grazeCount;
    g_GameManager.bonusPointsCap = cur->unk_0c + g_GameManager.stats->grazeCount;
    g_GameManager.invulnerabilityBonus = cur->unk_10 + g_GameManager.stats->grazeCount;
    if (g_GameManager.invulnerabilityBonus >= g_GameManager.stats->grazeCount + 50000)
    {
        g_GameManager.invulnerabilityBonus = g_GameManager.stats->grazeCount + 50000;
        StartBombState(&g_Player);
    }
    *g_GameManager.unk_4 = *(Unknown626270Unk4 *)replay->data->config;
    g_Rng.SetSeed(cur->rngSeed);
    g_GameManager.stats->spellCardsCaptured = cur->spellCardsCaptured;
    g_GameManager.stats->checksumKeyStage = cur->unk_18;
    g_GameManager.stats->checksumKeyPoints = cur->unk_1c;
    replay->extraCursor = extras;
    if (g_GameManager.stage >= 2 && g_GameManager.stage <= 6 &&
        replay->data->stages[g_GameManager.stage - 2] != NULL)
    {
        g_GameManager.stats->score = replay->data->stages[g_GameManager.stage - 2]->score;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
    }
    return 0;
}

int ReplayManager::DeletedCallback(ReplayManager *replay)
{
    g_Chain.Remove(replay->drawChain);
    replay->drawChain = NULL;
    if (replay->calcChain2 != NULL)
    {
        g_Chain.Remove(replay->calcChain2);
        replay->calcChain2 = NULL;
    }
    if (replay->calcChain3 != NULL)
    {
        g_Chain.Remove(replay->calcChain3);
        replay->calcChain3 = NULL;
    }
    MemFree(g_ReplayManager->data);
    if (replay->unk_40 != NULL)
    {
        MemFree(replay->unk_40);
    }
    delete g_ReplayManager;
    g_ReplayManager = NULL;
    g_ReplayManager = NULL;
    return 0;
}

int ReplayManager::OnDraw(ReplayManager *replay)
{
    return 1;
}

int ReplayManager::RegisterChain(int mode, char *path)
{
    ReplayManager *replay;

    g_Unknown4b9e58 = 0;
    g_Unknown4b9e50 = 0;
    if (g_ReplayManager == NULL)
    {
        replay = new ReplayManager;
        g_ReplayManager = replay;
        replay->data = NULL;
        replay->mode = mode;
        replay->path = path;
        switch (mode)
        {
        case 0:
            replay->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate);
            replay->calcChain->addedCallback = (ChainCallback)ReplayManager::RecordAddedCallback;
            replay->calcChain->deletedCallback = (ChainCallback)ReplayManager::DeletedCallback;
            replay->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnDraw);
            replay->calcChain->arg = replay;
            if (g_Chain.AddToCalcChain(replay->calcChain, 0x10) != 0)
            {
                return -1;
            }
            replay->calcChain2 = NULL;
            replay->calcChain3 = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate2);
            replay->calcChain3->arg = replay;
            g_Chain.AddToCalcChain(replay->calcChain3, 6);
            break;
        case 1:
            replay->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate);
            replay->calcChain->addedCallback = (ChainCallback)ReplayManager::PlayAddedCallback;
            replay->calcChain->deletedCallback = (ChainCallback)ReplayManager::DeletedCallback;
            replay->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnDraw);
            replay->calcChain->arg = replay;
            if (g_Chain.AddToCalcChain(replay->calcChain, 5) != 0)
            {
                return -1;
            }
            replay->calcChain2 = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate2);
            replay->calcChain2->arg = replay;
            g_Chain.AddToCalcChain(replay->calcChain2, 0x11);
            replay->calcChain3 = NULL;
            break;
        }
        replay->drawChain->arg = replay;
        g_Chain.AddToDrawChain(replay->drawChain, 0xe);
    }
    else
    {
        switch (mode)
        {
        case 0:
            ReplayManager::RecordAddedCallback(g_ReplayManager);
            break;
        case 1:
            ReplayManager::PlayAddedCallback(g_ReplayManager);
            break;
        }
    }
    return 0;
}

void ReplayManager::EndStage()
{
    int stageIdx;
    ReplayManager *replay = g_ReplayManager;

    if (replay != NULL)
    {
        replay->cursor++;
        replay->cursor->input = 0;
        stageIdx = g_GameManager.stage - 1;
        if (stageIdx >= 7)
        {
            stageIdx = 6;
        }
        replay->stageEnds[stageIdx] = replay->cursor + 1;
    }
}

void SaveReplay(char *path, char *name)
{
    BYTE *dp2;
    BYTE kbVal;
    BYTE *bpCur;
    int checksum;
    int offset;
    HANDLE file;
    ReplayHeader hX;
    BYTE *rpyBytes;
    int chunk;
    int compSize;
    float slowdown2;
    BYTE *compressedData;
    DWORD writtenSize;
    ReplayManager *replayTmp;
    int i;

    if (g_ReplayManager != NULL)
    {
        replayTmp = g_ReplayManager;
        if (replayTmp->GetMode() == 0)
        {
            if (!(g_GameManager.unk_93d8 & 1) && g_GameManager.difficulty < 4)
            {
                if (memcmp(&g_App.controllerMapping, replayTmp->data->config, sizeof(replayTmp->data->config)) != 0)
                {
                    goto freeStages;
                }
            }
            if (replayTmp->data->config[0x25] != 0)
            {
                goto freeStages;
            }
            if (path != NULL)
            {
                DebugPrint("info : Replay File write %s\r\n", path);
                rpyBytes = (BYTE *)MemAlloc(0x100000);
                hX = *replayTmp->data;
                ReplayManager::EndStage();
                i = g_GameManager.stage - 1;
                if (i >= 7)
                {
                    i = 6;
                }
                replayTmp->data->stages[i]->score = g_GameManager.stats->score;
                offset = 0x54;
                offset += 0x94;
                for (i = 0; i < 7; i++)
                {
                    if (replayTmp->data->stages[i] != NULL)
                    {
                        chunk = (int)replayTmp->stageEnds[i] - (int)replayTmp->data->stages[i];
                        memcpy(rpyBytes + offset - 0x54, replayTmp->data->stages[i], chunk);
                        hX.stages[i] = (ReplayStage *)offset;
                        offset += chunk;
                    }
                }
                for (i = 0; i < 7; i++)
                {
                    if (replayTmp->data->extras[i] != NULL)
                    {
                        chunk = (int)replayTmp->extraEnds[i] - (int)replayTmp->data->extras[i];
                        memcpy(rpyBytes + offset - 0x54, replayTmp->data->extras[i], chunk);
                        hX.extras[i] = (BYTE *)offset;
                        offset += chunk;
                    }
                }
                hX.score = g_GameManager.stats->guiScore;
                slowdown2 = (g_App.achievedFrames / g_App.expectedFrames - 0.5f) * 2.0f;
                if (slowdown2 < 0.0f)
                {
                    slowdown2 = 0.0f;
                }
                else if (slowdown2 >= 1.0f)
                {
                    slowdown2 = 1.0f;
                }
                hX.slowdown = (1.0f - slowdown2) * 100.0f;
                hX.size = offset;
                strcpy(hX.name, name);
                FormatTodayDate(hX.date);
                hX.key = g_Rng.GetRandomU16InRange(128) + 64;
                hX.unk_54 = g_Rng.GetRandomU16InRange(256);
                hX.unk_0c = g_Rng.GetRandomU16InRange(256);
                hX.slowdown2 = hX.slowdown + 1.12f;
                hX.slowdown3 = hX.slowdown + 2.34f;
                hX.unk_d4 = 30;
                memcpy(rpyBytes, &hX.unk_54, sizeof(ReplayHeader) - 0x54);
                DebugPrint("info : original size %d\r\n", offset);
                hX.dataSize = offset - 0x54;
                compressedData = (BYTE *)LzssCompress(rpyBytes, hX.dataSize, &hX.compressedSize);
                free(rpyBytes);
                compSize = hX.compressedSize;
                bpCur = &hX.key;
                checksum = 0x3f000318;
                for (i = 0; i < 0x47U; i++, bpCur++)
                {
                    checksum += *bpCur;
                }
                bpCur = compressedData;
                for (i = 0; i < compSize; i++, bpCur++)
                {
                    checksum += *bpCur;
                }
                hX.checksum = checksum;
                dp2 = (BYTE *)&hX.size;
                kbVal = hX.key;
                for (i = 0; i < 0x44U; i++, dp2++)
                {
                    *dp2 += kbVal;
                    kbVal += 7;
                }
                dp2 = compressedData;
                for (i = 0; i < compSize; i++, dp2++)
                {
                    *dp2 += kbVal;
                    kbVal += 7;
                }
                file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (file == INVALID_HANDLE_VALUE)
                {
                    goto freeStages;
                }
                WriteFile(file, &hX, 0x54, &writtenSize, NULL);
                WriteFile(file, compressedData, compSize, &writtenSize, NULL);
                CloseHandle(file);
                DebugPrint("info : Size %d -> %d\r\n", offset, compSize + 0x54);
                GlobalFree(compressedData);
            }
        freeStages:
            for (i = 0; i < 7; i++)
            {
                if (g_ReplayManager->data->stages[i] != NULL)
                {
                    MemFree(g_ReplayManager->data->stages[i]);
                }
                if (replayTmp->data->extras[i] != NULL)
                {
                    MemFree(g_ReplayManager->data->extras[i]);
                }
            }
        }
        g_Chain.Remove(g_ReplayManager->calcChain);
    }
}

void ResaveReplay(char *path)
{
    BYTE *dp2;
    BYTE kbVal;
    BYTE *bpCur;
    int checksum;
    int offset;
    HANDLE file;
    ReplayHeader hX;
    BYTE *rpyBytes;
    int chunk;
    int compSize;
    BYTE *compressedData;
    DWORD writtenSize;
    ReplayManager *replayTmp;
    int i;

    if (g_ReplayManager != NULL)
    {
        replayTmp = g_ReplayManager;
        if (!(g_GameManager.unk_93d8 & 1) && g_GameManager.difficulty < 4)
        {
            if (memcmp(&g_App.controllerMapping, replayTmp->data->config, sizeof(replayTmp->data->config)) != 0)
            {
                goto done;
            }
        }
        if (replayTmp->data->config[0x25] != 0)
        {
            goto done;
        }
        if (path != NULL)
        {
            DebugPrint("info : Replay File rewrite %s\r\n", path);
            rpyBytes = (BYTE *)MemAlloc(0x100000);
            hX = *replayTmp->data;
            i = g_GameManager.stage - 1;
            if (i >= 7)
            {
                i = 6;
            }
            replayTmp->data->stages[i]->score = g_GameManager.stats->score;
            offset = 0x54;
            offset += 0x94;
            for (i = 0; i < 7; i++)
            {
                if (replayTmp->data->stages[i] != NULL)
                {
                    chunk = replayTmp->stageSizes[i];
                    memcpy(rpyBytes + offset - 0x54, replayTmp->data->stages[i], chunk);
                    hX.stages[i] = (ReplayStage *)offset;
                    offset += chunk;
                }
            }
            for (i = 0; i < 7; i++)
            {
                if (replayTmp->data->extras[i] != NULL)
                {
                    chunk = replayTmp->extraSizes[i];
                    memcpy(rpyBytes + offset - 0x54, replayTmp->data->extras[i], chunk);
                    hX.extras[i] = (BYTE *)offset;
                    offset += chunk;
                }
            }
            hX.score = g_GameManager.stats->guiScore;
            hX.size = offset;
            hX.key = g_Rng.GetRandomU16InRange(128) + 64;
            hX.unk_54 = g_Rng.GetRandomU16InRange(256);
            hX.unk_0c = g_Rng.GetRandomU16InRange(256);
            hX.slowdown2 = hX.slowdown + 1.12f;
            hX.slowdown3 = hX.slowdown + 2.34f;
            hX.unk_d4 = 30;
            memcpy(rpyBytes, &hX.unk_54, sizeof(ReplayHeader) - 0x54);
            DebugPrint("info : original size %d\r\n", offset);
            hX.dataSize = offset - 0x54;
            compressedData = (BYTE *)LzssCompress(rpyBytes, hX.dataSize, &hX.compressedSize);
            free(rpyBytes);
            compSize = hX.compressedSize;
            bpCur = &hX.key;
            checksum = 0x3f000318;
            for (i = 0; i < 0x47U; i++, bpCur++)
            {
                checksum += *bpCur;
            }
            bpCur = compressedData;
            for (i = 0; i < compSize; i++, bpCur++)
            {
                checksum += *bpCur;
            }
            hX.checksum = checksum;
            dp2 = (BYTE *)&hX.size;
            kbVal = hX.key;
            for (i = 0; i < 0x44U; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            dp2 = compressedData;
            for (i = 0; i < compSize; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file == INVALID_HANDLE_VALUE)
            {
                goto done;
            }
            WriteFile(file, &hX, 0x54, &writtenSize, NULL);
            WriteFile(file, compressedData, compSize, &writtenSize, NULL);
            CloseHandle(file);
            DebugPrint("info : Size %d -> %d\r\n", offset, compSize + 0x54);
            GlobalFree(compressedData);
        }
    done:
        g_Chain.Remove(g_ReplayManager->calcChain);
    }
}
