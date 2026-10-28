// cflags: /Os
// Part of the /Os file starting at 0x437903, as in th06.

#include <stdlib.h>
#include <string.h>

#include "ascii.h"
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

int MusicScreen::OnUpdate(MusicScreen *screen)
{
    int oldState = screen->state;
    int i;

again:
    switch (screen->state)
    {
    case 0:
        if (screen->UpdateIntro() == 0)
        {
            break;
        }
        // The state changed: run the new one this frame.
        goto again;
    case 1:
        if (screen->UpdateList() != 0)
        {
            return 0;
        }
        break;
    }
    if (oldState != screen->state)
    {
        screen->stateFrames = 0;
    }
    else
    {
        screen->stateFrames++;
    }
    g_SpriteEngine->Step(&screen->unk_0024[0]);
    for (i = 0; i < 0x1f; i++)
    {
        g_SpriteEngine->Step(&screen->titles[i]);
    }
    for (i = 0; i < 8; i++)
    {
        g_SpriteEngine->Step(&screen->comment[i]);
    }
    return 1;
}

int MusicScreen::OnDraw(MusicScreen *screen)
{
    char arrow[2] = {0x7f, 0};
    D3DXVECTOR3 pos;
    int i;

    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    g_SpriteEngine->Func44f770(&screen->unk_0024[0]);
    for (i = screen->listTop; i < screen->listTop + 10; i++)
    {
        if (i >= screen->tuneCount)
        {
            break;
        }
        g_AsciiText.SetColor(screen->titles[i].color);
        screen->titles[i].pos.x = 93.0f;
        screen->titles[i].pos.y = 104.0f + (i + 1 - screen->listTop) * 18 - 20.0f;
        screen->titles[i].pos.z = 0.0f;
        g_SpriteEngine->Func44f770(&screen->titles[i]);
        // The number left of the title, and the arrow left of that.
        pos = screen->titles[i].pos;
        pos.x -= 60.0f;
        if (screen->cursor == i)
        {
            g_AsciiText.AddString(&pos, arrow);
        }
        pos.x += 15.0f;
        g_AsciiText.AddFormat(&pos, "%2d.", i + 1);
    }
    // Has no effect, but the original increments here.
    i++;
    for (i = 0; i < 8; i++)
    {
        g_SpriteEngine->Func44f770(&screen->comment[i]);
    }
    g_AsciiText.color = 0xffffffff;
    return 1;
}

// musiccmt.txt lists the tunes. Each starts with a line "@path", then
// its title on the next line and up to 8 lines of comment.
int MusicScreen::AddedCallback(MusicScreen *screen)
{
    int i;
    int row;
    char *rp;
    int k;
    char *data;
    char str[0x42];

    if (g_SpriteEngine->LoadSurface(0, "data/result/music.jpg") != 0)
    {
        return -1;
    }
    if (g_SpriteEngine->LoadAnm(0x2e, "data/music00.anm", 0x900) != 0)
    {
        return -1;
    }
    g_SpriteEngine->StartScript(&screen->unk_0024[0], 0x900);
    screen->stateFrames = 0;
    data = rp = (char *)LoadFileData("data/musiccmt.txt", 0);
    if (rp == NULL)
    {
        return -1;
    }
    screen->tunes = new MusicTune[0x20];
    i = -1;
    while ((DWORD)(rp - data) < (DWORD)g_FileSize)
    {
        if (*rp == '@')
        {
            rp++;
            i++;
            k = 0;
            while (*rp != '\n' && *rp != '\r')
            {
                screen->tunes[i].path[k] = *rp;
                rp++;
                k++;
                if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                {
                    goto done;
                }
            }
            while (*rp == '\n' || *rp == '\r')
            {
                rp++;
                if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                {
                    goto done;
                }
            }
            k = 0;
            while (*rp != '\n' && *rp != '\r')
            {
                screen->tunes[i].title[k] = *rp;
                rp++;
                k++;
                if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                {
                    goto done;
                }
            }
            // Never true, so the row break after the title is skipped
            // as an empty comment row.
            while (*rp == '\n' && *rp == '\r')
            {
                rp++;
                if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                {
                    goto done;
                }
            }
            for (row = 0; row < 8; row++)
            {
                if (*rp == '@')
                {
                    break;
                }
                memset(screen->tunes[i].comment[row], 0, sizeof(screen->tunes[i].comment[row]));
                k = 0;
                while (*rp != '\n' && *rp != '\r')
                {
                    screen->tunes[i].comment[row][k] = *rp;
                    rp++;
                    k++;
                    if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                    {
                        goto done;
                    }
                }
                while (*rp == '\n' || *rp == '\r')
                {
                    rp++;
                    if ((DWORD)(rp - data) >= (DWORD)g_FileSize)
                    {
                        goto done;
                    }
                }
            }
        }
        else
        {
            rp++;
        }
    }

done:
    screen->tuneCount = i + 1;
    for (i = 0; i < screen->tuneCount; i++)
    {
        g_SpriteEngine->StartScript(&screen->titles[i], i + 0x901);
        g_SpriteEngine->RenderText(&screen->titles[i], 0xc0e0ff, 0x302080, screen->tunes[i].title);
        screen->titles[i].pos.x = 93.0f;
        screen->titles[i].pos.y = 104.0f + (i + 1) * 18 - 20.0f;
        screen->titles[i].pos.z = 0.0f;
        screen->titles[i].flags |= 0xc00;
    }
    // The comment of the tune playing, as UpdateList shows it.
    for (i = 0; i < 8; i++)
    {
        g_SpriteEngine->StartScript(&screen->comment[i], i + 0x707);
        memset(str, 0, sizeof(str));
        memcpy(str, screen->tunes[screen->playing].comment[i], 0x40);
        if (str[0] != '\0')
        {
            screen->comment[i].flags |= 2;
            g_SpriteEngine->RenderText(&screen->comment[i], 0xffe0c0, 0x300000, str);
        }
        else
        {
            screen->comment[i].flags &= ~2;
        }
    }
    free(data);
    return 0;
}

int MusicScreen::DeletedCallback(MusicScreen *screen)
{
    delete screen->tunes;
    screen->tunes = NULL;
    g_SpriteEngine->ReleaseSurface(0);
    g_SpriteEngine->ReleaseAnm(0x2e);
    g_SpriteEngine->ReleaseAnm(0x2f);
    g_Chain.Remove(screen->drawChain);
    screen->drawChain = NULL;
    return 0;
}
