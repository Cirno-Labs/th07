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
