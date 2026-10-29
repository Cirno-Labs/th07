// cflags: /Os
// The /Os file with Unknown626270, probably th07's GameManager, from
// 0x42d560 to the Chain code at 0x42fa60.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "midi.h"
#include "player.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

Unknown626270::Unknown626270()
{
    memset(this, 0, sizeof(Unknown626270));
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->demoIndex = 2;
    this->unk_9624 = 1;
}

void Unknown626270::AddLives(int lives)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->lives += lives;
    this->RerollChecksumKeys();
}

void Unknown626270::AddBombs(int bombs)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombs += bombs;
    this->RerollChecksumKeys();
}

void Unknown626270::SetPlayArea()
{
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->unk_9604[0] = 8.0f;
    this->unk_9604[1] = 16.0f;
    this->unk_9604[2] = 368.0f;
    this->unk_9604[3] = 416.0f;
}

BOOL Unknown626270::IsInPlayArea(float x, float y, float width, float height)
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

int Unknown626270::ComputeChecksum()
{
    int sum;

    sum = ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_34, (BYTE *)&this->stats->unk_ac - (BYTE *)this->stats->unk_34);
    sum += ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_b4, sizeof(g_Unknown626270.stats->unk_b4));
    sum += ChecksumBytes((BYTE *)g_Unknown626270.unk_4, 0x38);
    sum += ChecksumBytes((BYTE *)&g_App.controllerMapping, 0x38);
    return sum;
}

void Unknown626270::AddLifeOrBomb()
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

int Unknown626270::OnDraw(Unknown626270 *mgr)
{
    if (mgr->unk_93dc)
    {
        mgr->unk_93dc = 2;
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

void Unknown626270::SetRankForDifficulty()
{
    this->unk_9634 = g_Ranks[g_Unknown626270.difficulty].rank;
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
    g_Unknown626270.unk_9614 = (float)sum + g_Unknown626270.stats->unk_8c[3];
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
    g_Unknown626270.stats->highScore = LoadHighScores(data, NULL, g_Unknown626270.unk_93d7, g_Unknown626270.difficulty, &g_Unknown626270.stats->unk_10);
    LoadSpellCards(data, g_Unknown626270.catks);
    LoadClears(data, g_Unknown626270.clrds);
    LoadPracticeScores(data, &g_Unknown626270.pscrs[0][0][0]);
    if (g_Unknown626270.unk_93d8 & 1)
    {
        g_Unknown626270.stats->highScore = g_Unknown626270.pscrs[g_Unknown626270.unk_93d7][g_Unknown626270.stage][g_Unknown626270.difficulty].score;
        g_Unknown626270.pscrs[g_Unknown626270.unk_93d7][g_Unknown626270.stage][g_Unknown626270.difficulty].unk_0c++;
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

int Unknown626270::DeletedCallback(Unknown626270 *mgr)
{
    g_App.StopMusic();
    if (g_App.musicMode == 2 && g_App.midi != NULL)
    {
        g_App.midi->Restart(0x1e);
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    Unknown1347b00::RemoveChain();
    BulletManager::RemoveChain();
    Player::RemoveChain();
    Unknown9a9b00::RemoveChain();
    g_Unknown1347938.Release();
    EffectManager::RemoveChain();
    Unknown49fbf0::RemoveChain();
    ReplayManager::EndStage();
    if (!g_Unknown626270.unk_93d8Bit3)
    {
        g_App.AddTotalPlayTime();
    }
    g_App.unk_194 = 0;
    g_App.AddPlayTime();
    mgr->unk_93d8 &= ~4;
    g_AsciiText.Initialize();
    g_Unknown626270.unk_d = 0;
    g_Unknown626270.unk_95e8 = 0;
    return 0;
}

int RegisterGameManagerChain()
{
    Unknown626270 *mgr = &g_Unknown626270;

    g_Unknown626270CalcChain.SetCallback((ChainCallback)Unknown626270::OnUpdate);
    g_Unknown626270CalcChain.addedCallback = (ChainCallback)Unknown626270::AddedCallback;
    g_Unknown626270CalcChain.deletedCallback = (ChainCallback)Unknown626270::DeletedCallback;
    g_Unknown626270CalcChain.arg = mgr;
    mgr->unk_95e8 = 0;
    if (g_Chain.AddToCalcChain(&g_Unknown626270CalcChain, 2) != 0)
    {
        return -1;
    }
    g_Unknown626270DrawChain.SetCallback((ChainCallback)Unknown626270::OnDraw);
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

void Unknown626270::AddSubRank(int amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->unk_9634++;
        this->subRank -= 100;
    }
    if (this->unk_9634 > this->maxRank)
    {
        this->unk_9634 = this->maxRank;
    }
}

void Unknown626270::SubSubRank(int amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->unk_9634--;
        this->subRank += 100;
    }
    if (this->unk_9634 < this->minRank)
    {
        this->unk_9634 = this->minRank;
    }
}

void Unknown626270::AddBonusPoints(int amount)
{
    int prev;

    prev = this->unk_961c;
    this->unk_961c += amount;
    if (this->unk_961c > this->unk_9618)
    {
        this->unk_961c = this->unk_9618;
    }
    if (amount > 0 && !g_Player.unk_240d)
    {
        this->unk_9620 += amount;
        if (this->unk_9620 >= this->stats->unk_88 + 50000)
        {
            this->unk_9620 = this->stats->unk_88 + 50000;
            StartBombState(&g_Player);
        }
    }
    if (this->unk_961c >= this->unk_9618 && prev != this->unk_961c)
    {
        g_Unknown49fbf0.Func427c81(this->unk_961c - this->stats->unk_88, 3);
    }
}

void Unknown626270::AddBonusPointsNoTrack(int amount)
{
    int prev;

    prev = this->unk_961c;
    this->unk_961c += amount;
    if (this->unk_961c > this->unk_9618)
    {
        this->unk_961c = this->unk_9618;
    }
    if (this->unk_961c >= this->unk_9618 && prev != this->unk_961c)
    {
        g_Unknown49fbf0.Func427c81(this->unk_961c - this->stats->unk_88, 3);
    }
}

void Unknown626270::AddBonusPointsCapped(int amount)
{
    int prev;

    prev = this->unk_961c;
    this->unk_961c += amount;
    if (this->unk_961c > this->unk_9618)
    {
        this->unk_961c = this->unk_9618;
    }
}

void Unknown626270::IncreaseBonusCap(int amount)
{
    this->unk_9618 += amount;
    if (this->unk_9618 >= this->stats->unk_88 + 9999990)
    {
        this->unk_9618 = this->stats->unk_88 + 9999990;
    }
}

int Unknown626270::HasExtraOpen(int shot)
{
    return this->clrds[shot].stagesClearedNoContinue[0] == 99 || this->clrds[shot].stagesClearedNoContinue[1] == 99 ||
                   this->clrds[shot].stagesClearedNoContinue[2] == 99 || this->clrds[shot].stagesClearedNoContinue[3] == 99
               ? TRUE
               : FALSE;
}

int Unknown626270::HasPhantasmOpen(int shot)
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

int Unknown626270::AnyShotHasExtraOpen()
{
    return this->HasExtraOpen(0) || this->HasExtraOpen(1) || this->HasExtraOpen(2) || this->HasExtraOpen(3) || this->HasExtraOpen(4) ||
                   this->HasExtraOpen(5)
               ? TRUE
               : FALSE;
}

int Unknown626270::IsPhantasmOpen()
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
