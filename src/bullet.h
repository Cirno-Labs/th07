// The enemies' bullets. Only what is known so far.

#pragma once

// The bullets, at 0x62f958. Only what is known so far.
class BulletManager
{
public:
    // Removes the bullets (Player's respawn step calls it with 0).
    void ClearAll(int unk);
};
extern BulletManager g_BulletManager;
