// The ending's file, 0x41d180 to 0x41e915, as th06's ending.cpp.

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "unsorted.h"

int Ending::OnUpdate(Ending *ending)
{
    int i;
    int extra = 0;

loop:
    if (ending->RunScript() != 0)
    {
        return 0;
    }
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Step(&ending->sprites[i]);
    }
    if (ending->canSkip != 0 && (g_Buttons & 0x100) != 0 && extra < 4)
    {
        extra++;
        goto loop;
    }
    return 1;
}

int Ending::OnDraw(Ending *ending)
{
    int i;

    g_SpriteEngine->CopySurfaceRectToBackBuffer(0, 0, 0, ending->scrollX, ending->scrollY, 640, 480);
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Draw(&ending->sprites[i]);
    }
    ending->DrawFade();
    return 1;
}

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}

void Ending::DrawFade()
{
    int a;
    float rect[4];

    rect[0] = 0.0f;
    rect[1] = 0.0f;
    rect[2] = 640.0f;
    rect[3] = 480.0f;
    switch (this->fadeType)
    {
    case 1:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 2:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xff000000;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 3:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 4:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xffffffff;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 0:
        this->fadeColor = 0;
        break;
    }
    if ((this->fadeColor & 0xff000000) != 0)
    {
        DrawColorRect(rect, this->fadeColor);
    }
}

#define SKIP_LINE()                                                                                                    \
    while (*this->cursor != '\n' && *this->cursor != '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }                                                                                                                  \
    while (*this->cursor == '\n' || *this->cursor == '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }

int Ending::RunScript()
{
    int half = 0;
    float secs4;
    int m;
    int j;
    int i2;
    int bottom2;
    int num4;
    int spr2;
    int a2;
    int sprite2;
    int k2;
    int lineLen3 = 0;
    char text3[0x44];

    memset(text3, 0, sizeof(text3));
    if (this->waitTimer > 0)
    {
        this->waitTimer.Sub(1);
        if (this->waitMinFrames != 0)
        {
            this->waitMinFrames--;
        }
        else if (WAS_PRESSED(0x1001) || (this->canSkip != 0 && (g_Buttons & 0x100) != 0))
        {
            this->waitTimer.Set(0);
        }
        if (this->waitTimer <= 0)
        {
            for (k2 = 0; k2 < 0xf; k2++)
            {
                this->sprites[k2].pendingInterrupt = 2;
            }
            this->line = 0;
        }
        else
        {
            goto done;
        }
    }
    if (this->lineTimer > 0)
    {
        this->lineTimer.Sub(1);
        if (this->lineMinFrames != 0)
        {
            this->lineMinFrames--;
        }
        else if (WAS_PRESSED(0x1001) || (this->canSkip != 0 && (g_Buttons & 0x100) != 0))
        {
            this->lineTimer.Set(0);
        }
        goto done;
    }
    while (TRUE)
    {
        switch (*this->cursor)
        {
        case '@':
            this->cursor++;
            switch (*this->cursor)
            {
            case 'b':
                if (g_SpriteEngine->LoadSurface(0, this->cursor + 1) != 0)
                {
                    return -1;
                }
                break;
            case 'a':
                this->cursor++;
                a2 = this->ReadInt();
                sprite2 = this->ReadInt();
                spr2 = this->ReadInt();
                g_SpriteEngine->StartScriptAtOrigin(&this->sprites[a2], sprite2 + 0x600);
                g_SpriteEngine->SetSprite(&this->sprites[a2], spr2 + 0x600);
                break;
            case 'V':
                this->cursor++;
                num4 = this->ReadInt();
                bottom2 = this->ReadInt();
                this->scrollSpeed = (float)num4 / (float)bottom2;
                break;
            case 'v':
                this->cursor++;
                this->scrollY = this->ReadInt();
                break;
            case 'F':
                if (this->Load(this->cursor + 1) != 0)
                {
                    return -1;
                }
                lineLen3 = 0;
                half = 0;
                for (i2 = 0; i2 < 6; i2++)
                {
                    for (j = 0; j < 4; j++)
                    {
                        if (g_GameManager.clrds[i2].stagesClearedNoContinue[j] == 'c' ||
                            g_GameManager.clrds[i2].stagesCleared[j] == 'c')
                        {
                            this->canSkip = 1;
                            break;
                        }
                    }
                }
            case 'R':
                for (m = 0; m < 0x10; m++)
                {
                    this->sprites[m].scriptIndex = 0;
                }
                break;
            case 'm':
                g_App.LoadMidi(0, this->cursor + 1);
                g_App.PlayMidi(0);
                break;
            case 'M':
                this->cursor++;
                secs4 = this->ReadInt();
                g_App.FadeOutMusic(secs4);
                break;
            case 's':
                this->cursor++;
                this->lineDelay = this->ReadInt();
                this->lineDelayFast = this->ReadInt();
                break;
            case 'c':
                this->cursor++;
                this->textColor = this->ReadInt();
                break;
            case 'r':
                this->cursor++;
                this->waitTimer.Set(this->ReadInt());
                this->waitMinFrames = this->ReadInt();
                SKIP_LINE();
                goto done;
            case 'w':
                this->cursor++;
                this->lineTimer.Set(this->ReadInt());
                this->lineMinFrames = this->ReadInt();
                SKIP_LINE();
                goto done;
            case '0':
                this->cursor++;
                this->fadeType = 1;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '1':
                this->cursor++;
                this->fadeType = 2;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '2':
                this->cursor++;
                this->fadeType = 3;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '3':
                this->cursor++;
                this->fadeType = 4;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case 'z':
                return -1;
            }
            SKIP_LINE();
            break;
        case '\0':
        case '\n':
        case '\r':
            if (lineLen3 != 0)
            {
                g_SpriteEngine->RenderText(&this->sprites[this->line], this->textColor, 0xffffffff, text3);
                this->sprites[this->line].SetPendingInterrupt(1);
            }
            while (*this->cursor == '\n' || *this->cursor == '\0' || *this->cursor == '\r')
            {
                this->cursor++;
            }
            if ((g_Buttons & 0x1001) != 0)
            {
                this->lineTimer.Set(this->lineDelayFast);
                this->lineMinFrames = this->lineDelayFast;
            }
            else
            {
                this->lineTimer.Set(this->lineDelay);
                this->lineMinFrames = this->lineDelay;
            }
            this->line++;
            goto done;
        default:
            text3[lineLen3] = this->cursor[0];
            text3[lineLen3 + 1] = this->cursor[1];
            lineLen3 += 2;
            this->cursor += 2;
            break;
        }
    }
done:
    this->timer.Tick();
    this->scrollY -= this->scrollSpeed;
    if (this->scrollY <= 0.0f)
    {
        this->scrollY = 0.0f;
        this->scrollSpeed = 0.0f;
    }
    return 0;
}

int Ending::Load(char *path)
{
    char *oldFile;

    oldFile = this->file;
    this->file = (char *)LoadFileData(path, 0);
    if (this->file == NULL)
    {
        LogPrintf(&g_Log, "error : エンディングファイルが読み込めない、ファイルが破壊されています\r\n");
        return -1;
    }
    this->cursor = this->file;
    this->lineDelay = 8;
    this->lineTimer.Set(0);
    this->timer.Set(0);
    if (oldFile != NULL)
    {
        free(oldFile);
    }
    return 0;
}

int Ending::AddedCallback(Ending *ending)
{
    char *script;
    int j;
    int si;
    // Unused.
    int unused = g_GameManager.shotIndex;

    g_GameManager.unk_93d8 |= 0x10;
    g_App.hideFps = 1;
    g_SpriteEngine->LoadAnm(0x31, "data/staff01.anm", 0x600);
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearVertexFormat();
    si = g_GameManager.shotIndex;
    ending->canSkip = 0;
    if (g_GameManager.stats->continuesUsed == 0)
    {
        if (g_GameManager.clrds[si].stagesClearedNoContinue[g_GameManager.difficulty] == 'c')
        {
            ending->canSkip = 1;
        }
        g_GameManager.clrds[si].stagesClearedNoContinue[g_GameManager.difficulty] = 'c';
    }
    else if (g_GameManager.clrds[si].stagesCleared[g_GameManager.difficulty] == 'c')
    {
        ending->canSkip = 1;
    }
    g_GameManager.clrds[si].stagesCleared[g_GameManager.difficulty] = 'c';
    for (j = 0; j < 0xf; j++)
    {
        g_SpriteEngine->StartScriptAtOrigin(&ending->sprites[j], j + 0x70f);
        ending->sprites[j].pos = D3DXVECTOR3(64.0f, (float)j * 16.0f + 392.0f, 0.0f);
    }
    if (g_GameManager.stats->continuesUsed != 0)
    {
        script = g_BadEndingFiles[g_GameManager.character];
    }
    else
    {
        script = g_EndingFiles[g_GameManager.shotIndex];
    }
    if (ending->Load(script) != 0)
    {
        return -1;
    }
    return 0;
}

int Ending::DeletedCallback(Ending *ending)
{
    g_SpriteEngine->ReleaseAnm(0x31);
    g_App.scene = 6;
    g_SpriteEngine->ReleaseSurface(0);
    MemFree(ending->file);
    g_Chain.Remove(ending->drawChain);
    ending->drawChain = NULL;
    delete ending;
    ending = NULL;
    g_App.hideFps = 0;
    return 0;
}

int Ending::RegisterChain()
{
    Ending *ending = new Ending();

    ending->calcChain = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainCallback)Ending::AddedCallback;
    ending->calcChain->deletedCallback = (ChainCallback)Ending::DeletedCallback;
    if (g_Chain.AddToCalcChain(ending->calcChain, 4) != 0)
    {
        return -1;
    }
    ending->drawChain = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawChain->arg = ending;
    g_Chain.AddToDrawChain(ending->drawChain, 1);
    return 0;
}
