// cflags: /Os
// The /Os file with GameManager, probably th07's GameManager, from
// 0x42d560 to the Chain code at 0x42fa60.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "midi.h"
#include "player.h"
#include "replay.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

GameManager::GameManager()
{
    memset(this, 0, sizeof(GameManager));
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->demoIndex = 2;
    this->phantasmWasOpen = 1;
}

void GameManager::AddLives(int lives)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->lives += lives;
    this->RerollChecksumKeys();
}

void GameManager::AddBombs(int bombs)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombs += bombs;
    this->RerollChecksumKeys();
}

void GameManager::SetPlayArea()
{
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->arena2[0] = 8.0f;
    this->arena2[1] = 16.0f;
    this->arena2[2] = 368.0f;
    this->arena2[3] = 416.0f;
}

BOOL GameManager::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (x - width / 2.0f > 384.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (y - height / 2.0f > 448.0f)
    {
        return FALSE;
    }
    return TRUE;
}

int ChecksumBytes(BYTE *data, int size)
{
    int i;
    int sum;

    sum = 0;
    for (i = 0; i < size; i++, data++)
    {
        sum += *data;
        g_Unknown626270.stats->unk_ac += g_Unknown626270.stats->unk_b4[2];
    }
    return sum;
}

int GameManager::ComputeChecksum()
{
    int sum;

    sum = ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_34, (BYTE *)&this->stats->unk_ac - (BYTE *)this->stats->unk_34);
    sum += ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_b4, sizeof(g_Unknown626270.stats->unk_b4));
    sum += ChecksumBytes((BYTE *)g_Unknown626270.unk_4, 0x38);
    sum += ChecksumBytes((BYTE *)&g_App.controllerMapping, 0x38);
    return sum;
}

void GameManager::AddLifeOrBomb()
{
    if ((int)this->stats->lives < 8)
    {
        this->AddLives(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Unknown49fbf0.livesChanged = 2;
    }
    else if ((int)this->stats->bombs < 8)
    {
        this->AddBombs(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Unknown49fbf0.bombsChanged = 2;
    }
}

int GameManager::OnUpdate(GameManager *mgr)
{
    int check;
    DWORD i;
    DWORD step;

    if (!mgr->isInContinueMenu && !mgr->isPaused && !((mgr->unk_93d8 >> 1) & 1) && !mgr->skipFpsMeasure && WAS_PRESSED(8))
    {
        mgr->isPaused = 1;
        g_Unknown626270.arena[0] = 32.0f;
        g_Unknown626270.arena[1] = 16.0f;
        g_Unknown626270.arena[2] = 384.0f;
        g_Unknown626270.arena[3] = 448.0f;
        mgr->pauseRequested = 1;
        if (g_Unknown626270.stage != 6 || g_Unknown49fbf0.frameCounter >= 300)
        {
            g_SoundPlayer.QueueSoundCommand(6, 0, "Pause");
        }
        g_SoundPlayer.QueueSound(0x25, 0);
        g_App.AddTotalPlayTime();
    }
    g_App.viewport.X = mgr->arena[0];
    g_App.viewport.Y = mgr->arena[1];
    g_App.viewport.Width = mgr->arena[2];
    g_App.viewport.Height = mgr->arena[3];
    g_App.viewport.MinZ = 0.0f;
    g_App.viewport.MaxZ = 1.0f;
    g_SpriteEngine->ClearUnk2e4d4();
    if ((g_Unknown626270.unk_93d8 >> 3) & 1 && g_Unknown626270.replayMenuChoice == 1 && !g_Unknown49fbf0.Func42ad66())
    {
        mgr->slowdownCounter++;
        if ((g_App.roundedFps < 20 && mgr->slowdownCounter % 3 != 0) ||
            (g_App.roundedFps >= 20 && g_App.roundedFps < 30 && mgr->slowdownCounter % 2 != 0) ||
            (g_App.roundedFps >= 30 && g_App.roundedFps < 40 && mgr->slowdownCounter % 3 == 0) ||
            (g_App.roundedFps >= 40 && g_App.roundedFps < 50 && mgr->slowdownCounter % 6 == 0))
        {
            return 3;
        }
    }
    if ((mgr->unk_93d8 >> 1) & 1)
    {
        if (WAS_PRESSED(0xffff))
        {
            g_App.scene = 1;
        }
        mgr->demoFrames++;
        if ((mgr->demoIndex == 0 && mgr->demoFrames == 8100) || (mgr->demoIndex == 1 && mgr->demoFrames == 7020) ||
            (mgr->demoIndex == 2 && mgr->demoFrames == 4620))
        {
            ScreenEffect::StartEffect(2, 120, 0, 0, 0);
            g_App.FadeOutMusic(3.0f);
        }
        if ((mgr->demoIndex == 0 && mgr->demoFrames >= 8220) || (mgr->demoIndex == 1 && mgr->demoFrames >= 7140) ||
            (mgr->demoIndex == 2 && mgr->demoFrames >= 4740))
        {
            g_App.scene = 1;
            return 3;
        }
    }
    g_Unknown626270.stats->unk_ac = g_Unknown626270.stats->unk_34[2];
    check = mgr->ComputeChecksum();
    g_Unknown626270.checksumValue = (float)check + g_Unknown626270.stats->unk_8c[3];
    for (i = 0; i < 7; i++)
    {
        if ((int)mgr->stats->unk_34[i] < 6543 || (int)mgr->stats->unk_34[i] > 106543)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->unk_60[i] < 6543.0f || mgr->stats->unk_60[i] > 106543.0f)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    mgr->isPlaying = !mgr->isInContinueMenu && !mgr->isPaused ? 1 : 0;
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->unk_54[i] < 6543.0f || mgr->stats->unk_54[i] > 106543.0f)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 8; i++)
    {
        if (mgr->stats->unk_8c[i] < 6543 || mgr->stats->unk_8c[i] > 106543)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_Unknown1347b00.fog.color, 1.0f, 0);
    if (mgr->isPaused == 1 || mgr->isPaused == 2 || mgr->isInContinueMenu)
    {
        return 3;
    }
    if (mgr->stats->score >= 1000000000)
    {
        mgr->stats->score = 999999999;
    }
    if (mgr->stats->guiScore != mgr->stats->score)
    {
        if (mgr->stats->score < mgr->stats->guiScore)
        {
            mgr->stats->score = mgr->stats->guiScore;
        }
        step = (mgr->stats->score - mgr->stats->guiScore) >> 5;
        if (step >= 0x8d55e)
        {
            step = 0x8d55e;
        }
        else if (step == 0)
        {
            step = 1;
        }
        if (mgr->stats->unk_8 < step)
        {
            mgr->stats->unk_8 = step;
        }
        if (mgr->stats->guiScore + mgr->stats->unk_8 > mgr->stats->score)
        {
            mgr->stats->unk_8 = mgr->stats->score - mgr->stats->guiScore;
        }
        mgr->stats->guiScore += mgr->stats->unk_8;
        if (mgr->stats->guiScore >= mgr->stats->score)
        {
            mgr->stats->unk_8 = 0;
            mgr->stats->guiScore = mgr->stats->score;
        }
        if (mgr->stats->highScore < mgr->stats->guiScore)
        {
            mgr->stats->highScore = mgr->stats->guiScore;
            mgr->stats->unk_10 = mgr->stats->continuesUsed;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (mgr->stats->unk_70[i] < 6543.0f || mgr->stats->unk_70[i] > 106543.0f)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->unk_80[i] < 6543.0f || mgr->stats->unk_80[i] > 106543.0f)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 5; i++)
    {
        if ((int)mgr->stats->unk_b4[i] < 6543 || (int)mgr->stats->unk_b4[i] > 106543)
        {
            g_Unknown626270.checksumValue = -9999.0f;
        }
    }
    if (g_Unknown626270.unk_4->unk_25)
    {
        g_Unknown626270.skipFpsMeasure = 0;
        mgr->slowdownCounter++;
        if ((g_BulletManager.bulletCount >= 320 && mgr->slowdownCounter % 3 == 0) ||
            (g_BulletManager.bulletCount < 320 && g_BulletManager.bulletCount >= 224 && mgr->slowdownCounter % 4 == 0) ||
            (g_BulletManager.bulletCount < 224 && g_BulletManager.bulletCount >= 128 && mgr->slowdownCounter % 5 == 0))
        {
            g_Unknown626270.skipFpsMeasure = 1;
            return 3;
        }
        if (g_BulletManager.bulletCount < 128)
        {
            mgr->slowdownCounter = 0;
        }
    }
    mgr->stageFrames++;
    return 1;
}

int GameManager::OnDraw(GameManager *mgr)
{
    if (mgr->isPaused)
    {
        mgr->isPaused = 2;
    }
    return 1;
}

void DrawLoadingScreen()
{
    Sprite sprite;
    float rect[4];

    rect[0] = 0.0f;
    rect[1] = 0.0f;
    rect[2] = 640.0f;
    rect[3] = 480.0f;
    g_SpriteEngine->InitSprite(&sprite, 0x10c);
    sprite.pos = D3DXVECTOR3(528.0f, 448.0f, 0.0f);
    g_App.d3dDevice->BeginScene();
    DrawColorRect(rect, 0xa0000000);
    g_SpriteEngine->DrawUnrotated(&sprite);
    g_SpriteEngine->FlushQuadBatch();
    g_App.d3dDevice->EndScene();
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    g_App.d3dDevice->BeginScene();
    DrawColorRect(rect, 0xa0000000);
    g_SpriteEngine->DrawUnrotated(&sprite);
    g_SpriteEngine->FlushQuadBatch();
    g_App.d3dDevice->EndScene();
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
}

void GameManager::SetRankForDifficulty()
{
    this->rank = g_Ranks[g_Unknown626270.difficulty].rank;
    this->minRank = g_Ranks[g_Unknown626270.difficulty].minRank;
    this->maxRank = g_Ranks[g_Unknown626270.difficulty].maxRank;
}

void RerollStatsChecksumKeys()
{
    int sum;
    int j;

    g_Unknown626270.stats->unk_88 = g_Rng.GetRandomU32InRange(100000) + 6543;
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_34); j++)
    {
        g_Unknown626270.stats->unk_34[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_8c); j++)
    {
        g_Unknown626270.stats->unk_8c[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_54); j++)
    {
        g_Unknown626270.stats->unk_54[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_60); j++)
    {
        g_Unknown626270.stats->unk_60[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_70); j++)
    {
        g_Unknown626270.stats->unk_70[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_80); j++)
    {
        g_Unknown626270.stats->unk_80[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_Unknown626270.stats->unk_b4); j++)
    {
        g_Unknown626270.stats->unk_b4[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    g_Unknown626270.stats->unk_ac = g_Unknown626270.stats->unk_34[2];
    sum = g_Unknown626270.ComputeChecksum();
    g_Unknown626270.stats->unk_b0 = sum;
    g_Unknown626270.checksumValue = (float)sum + g_Unknown626270.stats->unk_8c[3];
}

int LoadScoreFile()
{
    Catk *catks;
    int i;
    ScoreDat *data;
    int j;

    catks = g_Unknown626270.catks;
    ResultScreen::RegisterChain(2);
    memset(g_Unknown626270.catks, 0, sizeof(g_Unknown626270.catks));
    for (i = 0; i < 0x8d; i++, catks++)
    {
        catks->magic = 0x4b544143;
        catks->size2 = sizeof(Catk);
        catks->size = sizeof(Catk);
        catks->version = 1;
        catks->index = i;
        for (j = 0; j < 7; j++)
        {
            catks->attempts[j] = 0;
            catks->captures[j] = 0;
            catks->maxBonus[j] = 0;
        }
    }
    data = LoadScoreDat("score.dat");
    if (data == NULL)
    {
        LogPrintf(&g_Log, "error : スコアファイルの読み取りに失敗しました\r\n");
        return -1;
    }
    g_Unknown626270.stats->highScore = LoadHighScores(data, NULL, g_Unknown626270.shotIndex, g_Unknown626270.difficulty, &g_Unknown626270.stats->unk_10);
    LoadSpellCards(data, g_Unknown626270.catks);
    LoadClears(data, g_Unknown626270.clrds);
    LoadPracticeScores(data, &g_Unknown626270.pscrs[0][0][0]);
    if (g_Unknown626270.unk_93d8 & 1)
    {
        g_Unknown626270.stats->highScore = g_Unknown626270.pscrs[g_Unknown626270.shotIndex][g_Unknown626270.stage][g_Unknown626270.difficulty].score;
        g_Unknown626270.pscrs[g_Unknown626270.shotIndex][g_Unknown626270.stage][g_Unknown626270.difficulty].playCount++;
        g_Unknown626270.stats->unk_10 = 0;
    }
    ReleaseScoreDat(data);
    memcpy(g_Unknown626270.savedCatks, g_Unknown626270.catks, sizeof(g_Unknown626270.catks));
    return 0;
}

void IncrementStatCounter(DWORD *count, DWORD max)
{
    if (*count < 999999)
    {
        (*count)++;
    }
}

int GameManager::AddedCallback(GameManager *mgr)
{
    int size;
    int shot;
    WORD seed;

    g_App.clockMeasurementPending = 0;
    mgr->difficultyMask = 1 << mgr->difficulty;
    mgr->shotIndex = mgr->character * 2 + mgr->shotType;
    g_App.lastTotalPlayTimeTick = timeGetTime();
    g_App.gameSpeed = 1.0f;
    if (g_App.scene != 3)
    {
        DrawLoadingScreen();
        if (mgr->unk_4 != NULL)
        {
            delete mgr->unk_4;
            mgr->unk_4 = NULL;
        }
        if (mgr->stats != NULL)
        {
            delete mgr->stats;
            mgr->stats = NULL;
        }
        size = g_Rng.GetRandomU32InRange(0xffff) + 0x10;
        mgr->tempBuffer = malloc(size);
        mgr->unk_4 = new Unknown626270Unk4;
        mgr->stats = new Unknown626270Stats;
        RerollStatsChecksumKeys();
        *mgr->unk_4 = *(Unknown626270Unk4 *)&g_App.controllerMapping;
        MemFree(mgr->tempBuffer);
        mgr->powerItemCombo = 0;
        mgr->bonusPoints = mgr->stats->unk_88;
        mgr->invulnerabilityBonus = mgr->stats->unk_88;
        if (g_Unknown626270.difficulty >= 4)
        {
            mgr->unk_4->lifeCount = 2;
        }
        if (g_Unknown626270.unk_93d8 & 1)
        {
            mgr->unk_4->lifeCount = 8;
        }
        if (Player::RegisterChain(0) != 0)
        {
            LogPrintf(&g_Log, "error : プレイヤーの初期化に失敗しました\r\n");
            return -1;
        }
        if (!((g_Unknown626270.unk_93d8 >> 3) & 1))
        {
            g_Unknown626270.SetLives(mgr->unk_4->lifeCount);
            g_Unknown626270.RerollChecksumKeys();
            g_Unknown626270.SetBombs(g_Player.shotType->bombs);
        }
        mgr->SetPlayArea();
        mgr->stats->power = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->progressFrames = 0;
        mgr->stats->guiScore = 0;
        mgr->stats->score = 0;
        mgr->stats->unk_8 = 0;
        mgr->stats->highScore = 100000;
        mgr->stats->continuesUsed = 0;
        mgr->stats->grazeTotal = 0;
        mgr->stats->pointItemsTotal = 0;
        if (mgr->difficulty < 4)
        {
            mgr->stats->unk_30 = 50;
        }
        else
        {
            mgr->stats->unk_30 = 200;
            mgr->unk_4->unk_25 = 0;
        }
        mgr->stats->unk_2c = 0;
        if (LoadScoreFile() != 0)
        {
            return -1;
        }
        mgr->SetRankForDifficulty();
        mgr->stats->deaths = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->stats->bombsUsed = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->stats->spellCardsCaptured = 0;
        if (!(g_Unknown626270.unk_93d8 & 1))
        {
            switch (mgr->difficulty)
            {
            case 0:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 200000;
                break;
            case 1:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 200000;
                break;
            case 2:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 250000;
                break;
            case 3:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 300000;
                break;
            case 4:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 400000;
                mgr->bonusPoints = mgr->stats->unk_88 + 200000;
                break;
            case 5:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 400000;
                mgr->bonusPoints = mgr->stats->unk_88 + 300000;
                break;
            }
        }
        else
        {
            switch (mgr->difficulty)
            {
            case 0:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 200000;
                break;
            case 1:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 200000;
                break;
            case 2:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 250000;
                break;
            case 3:
                mgr->bonusPointsCap = mgr->stats->unk_88 + 300000;
                break;
            }
            switch (mgr->stage + 1)
            {
            case 2:
                mgr->bonusPoints = mgr->bonusPointsCap;
                break;
            case 3:
                mgr->bonusPointsCap += 50000;
                mgr->bonusPoints = mgr->bonusPointsCap;
                break;
            case 4:
                mgr->bonusPointsCap += 100000;
                mgr->bonusPoints = mgr->bonusPointsCap;
                break;
            case 5:
                mgr->bonusPointsCap += 150000;
                mgr->bonusPoints = mgr->bonusPointsCap;
                break;
            case 6:
                mgr->bonusPointsCap += 200000;
                mgr->bonusPoints = mgr->bonusPointsCap;
                break;
            }
        }
        if (!((g_Unknown626270.unk_93d8 >> 3) & 1))
        {
            if (!mgr->unk_4->unk_25)
            {
                IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].plays[0], 999999);
                IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[6].plays[0], 999999);
                IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].plays[mgr->shotIndex + 1], 999999);
                IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[6].plays[mgr->shotIndex + 1], 999999);
                if (g_App.scene == 10)
                {
                    IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].retries, 999999);
                    IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[6].retries, 999999);
                }
                if (g_Unknown626270.unk_93d8 & 1)
                {
                    IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].practices, 999999);
                    IncrementStatCounter((DWORD *)&g_Unknown626270.plst.counts[6].practices, 999999);
                }
            }
        }
        else
        {
            mgr->unk_4->unk_25 = 0;
        }
    }
    else
    {
        mgr->stats->guiScore = mgr->stats->score;
        mgr->stats->unk_8 = 0;
        if (Player::RegisterChain(0) != 0)
        {
            LogPrintf(&g_Log, "error : プレイヤーの初期化に失敗しました\r\n");
            return -1;
        }
    }
    mgr->subRank = 0;
    mgr->stats->unk_24 = 0;
    mgr->stats->graze = 0;
    mgr->isPaused = 0;
    mgr->stage++;
    if (!((g_Unknown626270.unk_93d8 >> 3) & 1))
    {
        shot = g_Unknown626270.shotIndex;
        if (!mgr->stats->continuesUsed && mgr->clrds[shot].stagesClearedNoContinue[g_Unknown626270.difficulty] < mgr->stage - 1)
        {
            mgr->clrds[shot].stagesClearedNoContinue[g_Unknown626270.difficulty] = mgr->stage - 1;
        }
        if (mgr->clrds[shot].stagesCleared[g_Unknown626270.difficulty] < mgr->stage - 1)
        {
            mgr->clrds[shot].stagesCleared[g_Unknown626270.difficulty] = mgr->stage - 1;
        }
    }
    if (mgr->unk_93d8 & 1)
    {
        switch (mgr->stage)
        {
        case 1:
            break;
        default:
            mgr->stats->power = 128.0f;
            mgr->RerollChecksumKeys();
            break;
        }
    }
    if ((g_Unknown626270.unk_93d8 >> 3) & 1)
    {
        mgr->SetRankForDifficulty();
        ReplayManager::RegisterChain(1, g_Unknown626270.replayFile);
        seed = g_Rng.seed;
        mgr->RerollChecksumKeys();
        g_Rng.seed = seed;
    }
    mgr->rngSeed = g_Rng.seed;
    if (Stage::RegisterChain(mgr->stage) != 0)
    {
        LogPrintf(&g_Log, "error : 背景データの初期化に失敗しました\r\n");
        return -1;
    }
    if (BulletManager::RegisterChain("data/etama.anm") != 0)
    {
        LogPrintf(&g_Log, "error : 敵弾の初期化に失敗しました\r\n");
        return -1;
    }
    if (EnemyManager::RegisterChain(g_EnemyAnmPaths[mgr->stage].anmPath, g_EnemyAnmPaths[mgr->stage].anmPath2) != 0)
    {
        LogPrintf(&g_Log, "error : 敵の初期化に失敗しました\r\n");
        return -1;
    }
    if (g_Unknown1347938.Load(g_EclPaths[mgr->stage]) != 0)
    {
        LogPrintf(&g_Log, "error : 敵頭脳の初期化に失敗しました\r\n");
        return -1;
    }
    if (EffectManager::RegisterChain() != 0)
    {
        LogPrintf(&g_Log, "error : エフェクトの初期化に失敗しました\r\n");
        return -1;
    }
    if (Gui::RegisterChain() != 0)
    {
        LogPrintf(&g_Log, "error : 2D表示の初期化に失敗しました\r\n");
        return -1;
    }
    if (!((g_Unknown626270.unk_93d8 >> 3) & 1))
    {
        ReplayManager::RegisterChain(0, "replay/th7_00.rpy");
    }
    g_App.LoadMidi(0, g_Unknown1347b00.data->songPaths[0]);
    g_App.LoadMidi(1, g_Unknown1347b00.data->songPaths[1]);
    if (mgr->stage != 6)
    {
        g_App.PlayMidi(0);
    }
    else
    {
        g_App.StopMusic();
        g_App.LoadMidi(2, "bgm/th07_13b.mid");
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    mgr->isInContinueMenu = 0;
    mgr->unk_93d8 |= 4;
    if (g_App.scene != 3)
    {
        g_App.achievedFrames = 0.0f;
        g_App.expectedFrames = 0.0f;
    }
    mgr->isFrozen = 0;
    mgr->stats->score = 0;
    mgr->unk_93d8 &= ~0x10;
    g_AsciiText.Initialize();
    g_Unknown626270.skipFpsMeasure = 0;
    DrawFps(0);
    DebugPrint2("random seed %d %d\r\n", g_Rng.seed, g_Rng.GetGenerated());
    return 0;
}

int GameManager::DeletedCallback(GameManager *mgr)
{
    g_App.StopMusic();
    if (g_App.musicMode == 2 && g_App.midi != NULL)
    {
        g_App.midi->Restart(0x1e);
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    Stage::RemoveChain();
    BulletManager::RemoveChain();
    Player::RemoveChain();
    EnemyManager::RemoveChain();
    g_Unknown1347938.Release();
    EffectManager::RemoveChain();
    Gui::RemoveChain();
    ReplayManager::EndStage();
    if (!g_Unknown626270.unk_93d8Bit3)
    {
        g_App.AddTotalPlayTime();
    }
    g_App.lastTotalPlayTimeTick = 0;
    g_App.AddPlayTime();
    mgr->unk_93d8 &= ~4;
    g_AsciiText.Initialize();
    g_Unknown626270.skipFpsMeasure = 0;
    g_Unknown626270.stageFrames = 0;
    return 0;
}

int RegisterGameManagerChain()
{
    GameManager *mgr = &g_Unknown626270;

    g_Unknown626270CalcChain.SetCallback((ChainCallback)GameManager::OnUpdate);
    g_Unknown626270CalcChain.addedCallback = (ChainCallback)GameManager::AddedCallback;
    g_Unknown626270CalcChain.deletedCallback = (ChainCallback)GameManager::DeletedCallback;
    g_Unknown626270CalcChain.arg = mgr;
    mgr->stageFrames = 0;
    if (g_Chain.AddToCalcChain(&g_Unknown626270CalcChain, 2) != 0)
    {
        return -1;
    }
    g_Unknown626270DrawChain.SetCallback((ChainCallback)GameManager::OnDraw);
    g_Unknown626270DrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_Unknown626270DrawChain, 2);
    return 0;
}

void RemoveGameManagerChain()
{
    g_Chain.Remove(&g_Unknown626270CalcChain);
    g_Chain.Remove(&g_Unknown626270DrawChain);
    if (g_Unknown626270.stats->score >= 1000000000)
    {
        g_Unknown626270.stats->score = 999999999;
    }
    g_Unknown626270.stats->guiScore = g_Unknown626270.stats->score;
}

void GameManager::AddSubRank(int amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank)
    {
        this->rank = this->maxRank;
    }
}

void GameManager::SubSubRank(int amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank)
    {
        this->rank = this->minRank;
    }
}

void GameManager::AddBonusPoints(int amount)
{
    int prev;

    prev = this->bonusPoints;
    this->bonusPoints += amount;
    if (this->bonusPoints > this->bonusPointsCap)
    {
        this->bonusPoints = this->bonusPointsCap;
    }
    if (amount > 0 && !g_Player.bombState)
    {
        this->invulnerabilityBonus += amount;
        if (this->invulnerabilityBonus >= this->stats->unk_88 + 50000)
        {
            this->invulnerabilityBonus = this->stats->unk_88 + 50000;
            StartBombState(&g_Player);
        }
    }
    if (this->bonusPoints >= this->bonusPointsCap && prev != this->bonusPoints)
    {
        g_Unknown49fbf0.Func427c81(this->bonusPoints - this->stats->unk_88, 3);
    }
}

void GameManager::AddBonusPointsNoTrack(int amount)
{
    int prev;

    prev = this->bonusPoints;
    this->bonusPoints += amount;
    if (this->bonusPoints > this->bonusPointsCap)
    {
        this->bonusPoints = this->bonusPointsCap;
    }
    if (this->bonusPoints >= this->bonusPointsCap && prev != this->bonusPoints)
    {
        g_Unknown49fbf0.Func427c81(this->bonusPoints - this->stats->unk_88, 3);
    }
}

void GameManager::AddBonusPointsCapped(int amount)
{
    int prev;

    prev = this->bonusPoints;
    this->bonusPoints += amount;
    if (this->bonusPoints > this->bonusPointsCap)
    {
        this->bonusPoints = this->bonusPointsCap;
    }
}

void GameManager::IncreaseBonusCap(int amount)
{
    this->bonusPointsCap += amount;
    if (this->bonusPointsCap >= this->stats->unk_88 + 9999990)
    {
        this->bonusPointsCap = this->stats->unk_88 + 9999990;
    }
}

int GameManager::HasExtraOpen(int shot)
{
    return this->clrds[shot].stagesClearedNoContinue[0] == 99 || this->clrds[shot].stagesClearedNoContinue[1] == 99 ||
                   this->clrds[shot].stagesClearedNoContinue[2] == 99 || this->clrds[shot].stagesClearedNoContinue[3] == 99
               ? TRUE
               : FALSE;
}

int GameManager::HasPhantasmOpen(int shot)
{
    int count;
    int i;

    count = 0;
    for (i = 0; i < 0x8d; i++)
    {
        if (this->catks[i].captures[6] > 0)
        {
            count++;
        }
    }
    if (count >= 60 && this->clrds[shot].stagesClearedNoContinue[4] == 99)
    {
        this->clrds[shot].stagesClearedNoContinue[5] = 99;
    }
    return this->clrds[shot].stagesClearedNoContinue[5] == 99;
}

int GameManager::AnyShotHasExtraOpen()
{
    return this->HasExtraOpen(0) || this->HasExtraOpen(1) || this->HasExtraOpen(2) || this->HasExtraOpen(3) || this->HasExtraOpen(4) ||
                   this->HasExtraOpen(5)
               ? TRUE
               : FALSE;
}

int GameManager::IsPhantasmOpen()
{
    int count;
    int i;
    int shot;

    count = 0;
    for (i = 0; i < 0x8d; i++)
    {
        if (this->catks[i].captures[6] > 0)
        {
            count++;
        }
    }
    if (count >= 60)
    {
        for (shot = 0; shot < 6; shot++)
        {
            if (this->clrds[shot].stagesClearedNoContinue[4] == 99)
            {
                this->clrds[shot].stagesClearedNoContinue[5] = 99;
            }
        }
    }
    if (this->clrds[0].stagesClearedNoContinue[5] == 99)
    {
        count = 60;
    }
    return this->clrds[0].stagesClearedNoContinue[5] == 99 || this->clrds[1].stagesClearedNoContinue[5] == 99 ||
                   this->clrds[2].stagesClearedNoContinue[5] == 99 || this->clrds[3].stagesClearedNoContinue[5] == 99 ||
                   this->clrds[4].stagesClearedNoContinue[5] == 99 || this->clrds[5].stagesClearedNoContinue[5] == 99
               ? TRUE
               : FALSE;
}
