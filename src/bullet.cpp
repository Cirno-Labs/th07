// The file with BulletManager, from 0x4232e0, as th06's bullet.cpp.

#include <string.h>

#include "bullet.h"
#include "unsorted.h"

BulletManager::BulletManager()
{
    this->Reset();
}

void BulletManager::Reset()
{
    memset(this, 0, sizeof(BulletManager));
    this->nextBullet = this->bullets;
    this->bullets[0x400].state = 6;
    this->unk_37a160 = 6;
}

int BulletManager::DeletedCallback(BulletManager *mgr)
{
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_SpriteEngine->ReleaseAnm(0xb);
        g_SpriteEngine->ReleaseAnm(0xc);
        g_SpriteEngine->ReleaseAnm(0xd);
        g_SpriteEngine->ReleaseAnm(0xe);
    }
    return 0;
}

int BulletManager::RegisterChain(char *anmPath)
{
    BulletManager *mgr = &g_BulletManager;

    g_EffectColors = g_EffectColorsFull;
    mgr->Reset();
    mgr->anmPath = anmPath;
    g_BulletCalcChain.SetCallback((ChainCallback)BulletManager::OnUpdate);
    g_BulletCalcChain.addedCallback = (ChainCallback)BulletManager::AddedCallback;
    g_BulletCalcChain.deletedCallback = (ChainCallback)BulletManager::DeletedCallback;
    g_BulletCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_BulletCalcChain, 0xc) != 0)
    {
        return -1;
    }
    g_BulletDrawChain.SetCallback((ChainCallback)BulletManager::OnDraw);
    g_BulletDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_BulletDrawChain, 10);
    return 0;
}

void BulletManager::RemoveChain()
{
    g_Chain.Remove(&g_BulletCalcChain);
    g_Chain.Remove(&g_BulletDrawChain);
    memset(&g_BulletManager, 0, sizeof(BulletManager));
}

void BulletManager::StopAllBullets()
{
    Bullet *bullet;
    int i;

    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        bullet->unk_ba4 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        bullet->unk_bb8 = 0.0f;
        bullet->unk_bb4 = 0.0f;
        bullet->speed = 0.0f;
        bullet->color = 0;
        g_SpriteEngine->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex + bullet->color);
    }
}

BulletCommand *Bullet::RestartCommand(int idx, int unk_14, DWORD flag)
{
    BulletCommand *cmd = &this->commands[idx];

    cmd->flag = flag;
    cmd->unk_14 = unk_14;
    this->flags |= flag;
    this->commandIndex = 0;
    return cmd;
}

BulletCommand *BulletShooter::StartCommand(int idx, int unk_14, DWORD flag)
{
    BulletCommand *cmd = &this->commands[idx];

    cmd->flag = flag;
    cmd->unk_14 = unk_14;
    this->flags |= flag;
    return cmd;
}

void Bullet::SetTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0)
{
    BulletCommand *cmd = this->RestartCommand(idx, unk_14, 0x20);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void Bullet::SetAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->RestartCommand(idx, unk_14, 0x10);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetShooterTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x20);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetStopTurnCommand(int idx, int unk_14, int unk_8, int unk_c, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x80);

    cmd->unk_8 = unk_8;
    cmd->unk_c = unk_c;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetShooterAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x10);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetHiddenCommand(int idx, int unk_14, int unk_8)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x2000);

    cmd->unk_8 = unk_8;
}
