// cflags: /Os
// Part of the /Os file starting at 0x437903, as in th06.

#include <string.h>

#include "musicscreen.h"
#include "unsorted.h"

int MusicScreen::UpdateIntro()
{
    int i;

    if (this->stateFrames == 0)
    {
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].pendingInterrupt = 1;
            }
            else
            {
                this->titles[i].pendingInterrupt = 2;
            }
        }
        for (i = 0; i < 8; i++)
        {
            this->comment[i].pendingInterrupt = 1;
        }
    }
    if (this->stateFrames >= 8)
    {
        this->state = 1;
    }
    return 0;
}

int MusicScreen::UpdateList()
{
    int prevTop = this->listTop;
    int i;
    char str[0x42];

    if (WAS_PRESSED(0x10))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor = this->tuneCount - 1;
            this->listTop = this->tuneCount - 10;
            if (this->listTop < 0)
            {
                this->listTop = 0;
            }
        }
        else if (this->listTop > this->cursor)
        {
            this->listTop = this->cursor;
        }
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].pendingInterrupt = 1;
            }
            else
            {
                this->titles[i].pendingInterrupt = 2;
            }
        }
    }
    if (WAS_PRESSED(0x20))
    {
        this->cursor++;
        if (this->cursor >= this->tuneCount)
        {
            this->cursor = 0;
            this->listTop = 0;
        }
        else if (this->listTop <= this->cursor - 10)
        {
            this->listTop = this->cursor - 9;
        }
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].pendingInterrupt = 1;
            }
            else
            {
                this->titles[i].pendingInterrupt = 2;
            }
        }
    }
    if (WAS_PRESSED(0x1001))
    {
        this->playing = this->cursor;
        if (g_App.flags.preloadBgm)
        {
            g_SoundPlayer.LoadBgm("thbgm.dat");
        }
        g_App.PlayMusic(this->tunes[this->playing].path);
        for (i = 0; i < 8; i++)
        {
            memset(str, 0, sizeof(str));
            memcpy(str, this->tunes[this->playing].comment[i], 0x40);
            if (str[0] != '\0')
            {
                this->comment[i].flags |= 2;
                g_SpriteEngine->RenderText(&this->comment[i], 0xffe0c0, 0x300000, str);
            }
            else
            {
                this->comment[i].flags &= ~2;
            }
            this->comment[i].pendingInterrupt = 1;
        }
    }
    if (WAS_PRESSED(0xa))
    {
        g_App.scene = 1;
        return 1;
    }
    return 0;
}
