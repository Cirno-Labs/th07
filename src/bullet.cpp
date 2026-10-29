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
