// The file with SoundPlayer, the sound effects and wave music, from
// 0x44b510.

#include "dsutil.h"
#include "unsorted.h"

char *FindRiffChunk(char *data, char *name, int *chunkSize, unsigned int size);

SoundPlayer::SoundPlayer()
{
    int i;

    memset(this, 0, sizeof(*this));
    for (i = 0; i < 0x80; i++)
    {
        this->unk_408[i] = -1;
    }
}

int SoundPlayer::Initialize(HWND hwnd)
{
    DSBUFFERDESC params;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    WAVEFORMATEX formatInfo;
    int j;

    memset(this, 0, sizeof(*this));
    for (j = 0; j < 0x80; j++)
    {
        this->unk_408[j] = -1;
    }
    this->manager = new CSoundManager();
    if (this->manager->Initialize(hwnd, DSSCL_PRIORITY, 2, 44100, 16) < 0)
    {
        LogPrintf(&g_Log, "DirectSound オブジェクトの初期化が失敗したよ\r\n");
        if (this->manager != NULL)
        {
            delete this->manager;
            this->manager = NULL;
        }
        return -1;
    }
    this->dsound = this->manager->GetDirectSound();
    this->notifyThread = NULL;
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    params.dwBufferBytes = 0x8000;
    memset(&formatInfo, 0, sizeof(formatInfo));
    formatInfo.cbSize = 0;
    formatInfo.wFormatTag = WAVE_FORMAT_PCM;
    formatInfo.nChannels = 2;
    formatInfo.nSamplesPerSec = 44100;
    formatInfo.nAvgBytesPerSec = 44100 * 4;
    formatInfo.nBlockAlign = 4;
    formatInfo.wBitsPerSample = 16;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->silence, NULL) < 0)
    {
        return -1;
    }
    if (this->silence->Lock(0, 0x8000, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        return -1;
    }
    memset(audioPtr_1, 0, 0x8000);
    this->silence->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    this->silence->Play(0, 0, DSBPLAY_LOOPING);
    SetTimer(hwnd, 0, 250, NULL);
    this->hwnd = hwnd;
    LogPrintf(&g_Log, "DirectSound は正常に初期化されました\r\n");
    return 0;
}

int SoundPlayer::Release()
{
    int i;

    if (!this->manager)
    {
        return 0;
    }
    for (i = 0; i < 0x80; i++)
    {
        if (this->sounds[i] != NULL)
        {
            this->sounds[i]->Release();
            this->sounds[i] = NULL;
        }
        if (this->soundBuffers[i] != NULL)
        {
            this->soundBuffers[i]->Release();
            this->soundBuffers[i] = NULL;
        }
    }
    KillTimer(this->hwnd, 1);
    this->StopBgm();
    this->dsound = NULL;
    this->silence->Stop();
    if (this->silence != NULL)
    {
        this->silence->Release();
        this->silence = NULL;
    }
    if (this->bgm != NULL)
    {
        delete this->bgm;
        this->bgm = NULL;
    }
    if (this->manager != NULL)
    {
        delete this->manager;
        this->manager = NULL;
    }
    for (i = 0; i < 0x10; i++)
    {
        if (this->bgmPreloadData[i] != NULL)
        {
            MemFree(this->bgmPreloadData[i]);
            this->bgmPreloadData[i] = NULL;
        }
    }
    if (this->bgmFormat != NULL)
    {
        MemFree(this->bgmFormat);
    }
    return 0;
}

// Finds chunk `name` among the RIFF chunks at `data`, `size` bytes of
// them. Returns its contents and sets *chunkSize, or returns NULL.
char *FindRiffChunk(char *data, char *name, int *chunkSize, unsigned int size)
{
    while (size > 0)
    {
        *chunkSize = *(int *)(data + 4);
        if (strncmp(data, name, 4) == 0)
        {
            return data + 8;
        }
        size -= *chunkSize + 8;
        data += *chunkSize + 8;
    }
    return NULL;
}

int SoundPlayer::FindTrackIndex(char *path)
{
    char name[0x80];
    int i;
    char *p;

    i = 0;
    p = strrchr(path, '/');
    if (p == NULL)
    {
        p = strrchr(path, '\\');
    }
    if (p == NULL)
    {
        strcpy(name, path);
    }
    else
    {
        strcpy(name, p + 1);
    }
    while (this->bgmFormat[i].name[0] != '\0')
    {
        if (strcmp(this->bgmFormat[i].name, name) == 0)
        {
            break;
        }
        i++;
    }
    if (this->bgmFormat[i].name[0] == '\0')
    {
        i = 0;
    }
    return i;
}

int SoundPlayer::LoadSound(int index, char *path)
{
    char *cursor;
    DSBUFFERDESC params;
    char *chunk;
    int size;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    char *wavBuf;
    WAVEFORMATEX formatInfo;
    int totalSize2;

    if (!this->manager)
    {
        return 0;
    }
    if (this->soundBuffers[index] != NULL)
    {
        this->soundBuffers[index]->Release();
        this->soundBuffers[index] = NULL;
    }
    wavBuf = (char *)LoadFileData(path, 0);
    cursor = wavBuf;
    if (cursor == NULL)
    {
        return -1;
    }
    if (strncmp(cursor, "RIFF", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない %s\r\n", path);
        MemFree(wavBuf);
        return -1;
    }
    cursor += 4;
    totalSize2 = *(int *)cursor;
    cursor += 4;
    if (strncmp(cursor, "WAVE", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        MemFree(wavBuf);
        return -1;
    }
    cursor += 4;
    chunk = FindRiffChunk(cursor, "fmt ", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        MemFree(wavBuf);
        return -1;
    }
    formatInfo = *(WAVEFORMATEX *)chunk;
    chunk = FindRiffChunk(cursor, "data", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        MemFree(wavBuf);
        return -1;
    }
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
    params.dwBufferBytes = size;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->soundBuffers[index], NULL) < 0)
    {
        MemFree(wavBuf);
        return -1;
    }
    if (this->soundBuffers[index]->Lock(0, size, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        MemFree(wavBuf);
        return -1;
    }
    memcpy(audioPtr_1, chunk, sizeOne);
    if (size2 != 0)
    {
        memcpy(audioPtrTwo, chunk + sizeOne, size2);
    }
    this->soundBuffers[index]->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    MemFree(wavBuf);
    return 0;
}

int SoundPlayer::LoadBgmFormat(char *path)
{
    this->bgmFormat = (BgmFormat *)LoadFileData(path, 0);
    return this->bgmFormat != NULL ? 0 : -1;
}

int SoundPlayer::LoadBgm(char *path)
{
    DWORD notifySize;
    BgmFormat *format;
    HRESULT hr;
    INT nSamplesPerSec;
    DWORD nBlockAlign;

    strcpy(this->bgmPath, path);
    if (!this->manager)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return -1;
    }
    DebugPrint("Streming BGM Start\r\n");
    this->StopBgm();
    format = this->bgmFormat;
    nBlockAlign = format->format.nBlockAlign;
    nSamplesPerSec = format->format.nSamplesPerSec;
    notifySize = nSamplesPerSec * 4 * nBlockAlign / 16;
    notifySize -= notifySize % nBlockAlign;
    this->notifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->notifyThread = CreateThread(NULL, 0, BgmNotificationProc, g_App.hwnd, 0, &this->notifyThreadId);
    hr = this->manager->CreateStreaming(&this->bgm, path, DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2,
                                        GUID_NULL, 0x10, notifySize, this->notifyEvent, format);
    if (hr < 0)
    {
        DebugPrint("error : ストリーミング用サウンドバッファを作成出来ませんでした\r\n");
        return -1;
    }
    return 0;
}

int SoundPlayer::SwitchBgmTrack(char *path)
{
    int i;
    CWaveFile *waveFile;

    if (this->bgm == NULL)
    {
        return -1;
    }
    i = this->FindTrackIndex(path);
    waveFile = this->bgm->m_pWaveFile;
    waveFile->Func45e130(&this->bgmFormat[i]);
    DebugPrint("Streming BGM Reopen %d\r\n", i);
    return 0;
}

int SoundPlayer::PreloadBgm(int slot, char *path)
{
    int i;
    DWORD sizeRead;
    HANDLE file;
    void *data;

    if (this->bgmPreloadData[slot] != NULL && strcmp(path, this->bgmNames[slot]) == 0)
    {
        return 0;
    }
    strcpy(g_SoundPlayer.bgmNames[slot], path);
    if (!g_App.flags.preloadBgm)
    {
        return 0;
    }
    if (!this->manager)
    {
        return 0;
    }
    if (this->bgmPreloadData[slot] != NULL)
    {
        MemFree(this->bgmPreloadData[slot]);
        this->bgmPreloadData[slot] = NULL;
    }
    DebugPrint("Streming BGM PreLoad %d\r\n", slot);
    file = CreateFileA(this->bgmPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        DebugPrint("error : bgmfile is not find %s\r\n", this->bgmPath);
        return -1;
    }
    i = this->FindTrackIndex(path);
    SetFilePointer(file, this->bgmFormat[i].offset, NULL, FILE_BEGIN);
    data = MemAlloc(this->bgmFormat[i].size);
    if (data == NULL)
    {
        CloseHandle(file);
        DebugPrint("error : bgmfile is not find %s\r\n", this->bgmPath);
        return -1;
    }
    ReadFile(file, data, this->bgmFormat[i].size, &sizeRead, NULL);
    CloseHandle(file);
    this->bgmEntries[slot] = &this->bgmFormat[i];
    this->bgmPreloadData[slot] = data;
    this->bgmPreloadCursor[slot] = data;
    this->bgmPreloadSize[slot] = this->bgmEntries[slot]->size;
    return 0;
}

int SoundPlayer::PlayPreloadedBgm(int slot)
{
    DWORD notifySize;
    HRESULT hr;
    INT nSamplesPerSec;
    DWORD nBlockAlign;

    if (!this->manager)
    {
        return -1;
    }
    if (!g_App.musicMode)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return -1;
    }
    if (!g_App.flags.preloadBgm)
    {
        return this->SwitchBgmTrack(this->bgmNames[slot]);
    }
    if (this->bgmPreloadData[slot] == NULL)
    {
        return -1;
    }
    DebugPrint("Streming BGM Load no %d\r\n", slot);
    nBlockAlign = this->bgmEntries[slot]->format.nBlockAlign;
    nSamplesPerSec = this->bgmEntries[slot]->format.nSamplesPerSec;
    notifySize = nSamplesPerSec * 4 * nBlockAlign / 16;
    notifySize -= notifySize % nBlockAlign;
    this->notifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->notifyThread = CreateThread(NULL, 0, BgmNotificationProc, g_App.hwnd, 0, &this->notifyThreadId);
    hr = this->manager->Func45cc30(&this->bgm, this->bgmPreloadCursor[slot], this->bgmPreloadSize[slot],
                                   this->bgmEntries[slot], DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2,
                                   GUID_NULL, 0x10, notifySize, this->notifyEvent);
    if (hr < 0)
    {
        DebugPrint("error : ストリーミング用サウンドバッファを作成出来ませんでした\r\n");
        return -1;
    }
    DebugPrint("load comp\r\n");
    this->bgmPlaying = slot;
    return 0;
}

void SoundPlayer::StopBgm()
{
    if (this->bgm != NULL)
    {
        DebugPrint("Streming BGM stop\r\n");
        this->bgm->Stop();
        if (this->notifyThread != NULL)
        {
            PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
            DebugPrint("stop m_dwNotifyThreadID\r\n");
            while (WaitForSingleObject(this->notifyThread, 0x100) != WAIT_OBJECT_0)
            {
                PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
            }
            DebugPrint("stop comp\r\n");
            CloseHandle(this->notifyThread);
            CloseHandle(this->notifyEvent);
            this->notifyThread = NULL;
        }
        if (this->bgm != NULL)
        {
            delete this->bgm;
            this->bgm = NULL;
        }
    }
}

int SoundPlayer::LoadSounds()
{
    int i;

    if (!this->manager)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return 0;
    }
    for (i = 0; i < 5; i++)
    {
        this->queue[i] = -1;
    }
    for (i = 0; i < 0x1e; i++)
    {
        if (this->LoadSound(i, g_SoundPaths[i]) != 0)
        {
            LogPrintf(&g_Log, "error : Sound ファイルが読み込めない データを確認 %s\r\n", g_SoundPaths[i]);
            return -1;
        }
    }
    for (i = 0; i < ARRAY_SIZE(g_SoundInfos); i++)
    {
        this->dsound->DuplicateSoundBuffer(this->soundBuffers[g_SoundInfos[i].buffer], &this->sounds[i]);
        this->sounds[i]->SetCurrentPosition(0);
        this->sounds[i]->SetVolume(g_SoundInfos[i].volume);
    }
    return 0;
}

void SoundPlayer::QueueSound(int idx, int unused)
{
    int i;
    int unk;

    unk = g_SoundInfos[idx].unk_6;
    for (i = 0; i < 5; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        if (this->queue[i] == idx)
        {
            return;
        }
    }
    if (i >= 5)
    {
        return;
    }
    this->queue[i] = idx;
    this->unk_408[idx] = unk;
}

inline void SoundPlayer::FadeOutBgm(float seconds)
{
    if (this->bgm != NULL)
    {
        CStreamingSound *bgm = this->bgm;
        bgm->fading = 1;
        bgm->fadeFrames = seconds * 60.0f;
        bgm->fadeTotal = bgm->fadeFrames;
    }
}

int SoundPlayer::PlayQueuedSounds()
{
    int sound;
    SoundCommand *command;
    int i;
    int advance;

    if (!this->manager)
    {
        return 0;
    }
    command = this->commands;
again:
    advance = 0;
    switch (command->type)
    {
    case 1:
        if (g_App.flags.preloadBgm)
        {
            DebugPrint("Sound : PreLoad Stage\r\n");
            if (command->stage == 0)
            {
                this->StopBgm();
                this->PreloadBgm(command->arg, command->path);
                advance = 1;
                break;
            }
        }
        else
        {
            DebugPrint("Sound : PreLoad Stage\r\n");
            this->PreloadBgm(command->arg, command->path);
            advance = 1;
            break;
        }
        command->stage++;
        goto done;
    case 2:
        if (g_App.flags.preloadBgm && command->arg >= 0)
        {
            if (command->stage == 0)
            {
                DebugPrint("Sound : Load Stage\r\n");
                if (this->PlayPreloadedBgm(command->arg) != 0)
                {
                    break;
                }
            }
            else if (command->stage == 2)
            {
                DebugPrint("Sound : Reset Stage\r\n");
                if (this->bgm != NULL)
                {
                    if (this->bgm->Reset() < 0)
                    {
                        break;
                    }
                }
            }
            else if (command->stage == 5)
            {
                DebugPrint("Sound : Fill Buffer Stage\r\n");
                LPDIRECTSOUNDBUFFER buffer = this->bgm->GetBuffer(0);
                command->arg = this->bgm->GetWaveFile()->GetFormat()->Loops();
                if (this->bgm->FillBufferWithSound(buffer, command->arg) < 0)
                {
                    break;
                }
            }
            else if (command->stage == 7)
            {
                DebugPrint("Sound : Play Stage\r\n");
                this->bgm->Play(0, DSBPLAY_LOOPING);
            }
            else if (command->stage >= 0x14)
            {
                break;
            }
        }
        else
        {
            if (this->bgm == NULL)
            {
                break;
            }
            if (command->stage == 0)
            {
                DebugPrint("Sound : Stop Stage\r\n");
                this->bgm->Stop();
            }
            else if (command->stage == 1)
            {
                if (this->bgm->unk_74 != 0)
                {
                    goto done;
                }
                DebugPrint("Sound : Recreate Stage\r\n");
                this->bgm->Func45d060();
            }
            else if (command->stage == 2)
            {
                DebugPrint("Sound : ReOpen Stage\r\n");
                char *name = command->arg >= 0 ? this->bgmNames[command->arg] : command->path;
                int entry = this->FindTrackIndex(name);
                this->bgm->GetWaveFile()->Func45e130(&this->bgmFormat[entry]);
            }
            else if (command->stage == 3)
            {
                DebugPrint("Sound : Fill Buffer Stage\r\n");
                LPDIRECTSOUNDBUFFER buffer = this->bgm->GetBuffer(0);
                this->bgm->Reset();
                command->arg = this->bgm->GetWaveFile()->GetFormat()->Loops();
                if (this->bgm->FillBufferWithSound(buffer, command->arg) < 0)
                {
                    break;
                }
            }
            else if (command->stage == 4)
            {
                DebugPrint("Sound : Play Stage\r\n");
                this->bgm->Play(0, DSBPLAY_LOOPING);
            }
            else if (command->stage >= 7)
            {
                break;
            }
        }
        command->stage++;
        goto done;
    case 4:
        if (this->bgm == NULL)
        {
            break;
        }
        if (command->stage == 0)
        {
            DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (command->stage == 1)
        {
            DebugPrint("Sound : Thread Stop Stage\r\n");
            if (this->notifyThread == NULL)
            {
                break;
            }
            PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
        }
        else if (command->stage == 2)
        {
            if (WaitForSingleObject(this->notifyThread, 0x100) != WAIT_OBJECT_0)
            {
                DebugPrint("Sound : Thread Stop Wait Stage\r\n");
                PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
                command->stage--;
            }
            else
            {
                this->notifyThread = NULL;
            }
        }
        else if (command->stage == 3)
        {
            DebugPrint("Sound : Handle Close Stage\r\n");
            CloseHandle(this->notifyThread);
            CloseHandle(this->notifyEvent);
            this->notifyThread = NULL;
            if (this->bgm != NULL)
            {
                delete this->bgm;
                this->bgm = NULL;
            }
        }
        else if (command->stage == 10)
        {
            break;
        }
        command->stage++;
        goto done;
    case 3:
        if (this->bgm == NULL)
        {
            break;
        }
        if (command->stage == 0)
        {
            DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (command->stage == 1)
        {
            break;
        }
        command->stage++;
        goto done;
    case 5:
        DebugPrint("Sound : Fade Out Stage %d\r\n", command->arg);
        g_SoundPlayer.FadeOutBgm(command->arg);
        break;
    case 6:
        if (g_App.musicMode == 1)
        {
            if (this->bgm->unk_74 != 0)
            {
                DebugPrint("locked\n");
                goto done;
            }
            if (this->bgm != NULL)
            {
                this->bgm->Func45d910();
            }
        }
        break;
    case 7:
        if (g_App.musicMode == 1)
        {
            if (this->bgm->unk_74 != 0)
            {
                goto done;
            }
            if (this->bgm != NULL)
            {
                this->bgm->Func45d960();
            }
        }
        break;
    default:
        goto done;
    }
    for (i = 0; i < 0x1f; i++, command++)
    {
        if (command->type == 0)
        {
            break;
        }
        *command = *(command + 1);
    }
    if (advance)
    {
        goto again;
    }
done:
    if (!g_App.playSounds)
    {
        return this->commands[0].type;
    }
    for (i = 0; i < 5; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        sound = this->queue[i];
        this->queue[i] = -1;
        if (this->sounds[sound] == NULL)
        {
            continue;
        }
        this->sounds[sound]->Stop();
        this->sounds[sound]->SetCurrentPosition(0);
        this->sounds[sound]->Play(0, 0, 0);
    }
    return this->commands[0].type;
}

DWORD WINAPI BgmNotificationProc(LPVOID lpParameter)
{
    HRESULT hr;
    HWND hDlg = (HWND)lpParameter;
    MSG msg;
    DWORD dwResult;
    BOOL bDone = FALSE;
    BOOL bLooped = TRUE;

    while (!bDone)
    {
        dwResult = MsgWaitForMultipleObjects(1, &g_SoundPlayer.notifyEvent, FALSE, INFINITE, QS_ALLEVENTS);
        if (g_SoundPlayer.bgm == NULL)
        {
            bDone = TRUE;
        }
        switch (dwResult)
        {
        case WAIT_OBJECT_0 + 0:
            if (g_SoundPlayer.bgm != NULL && g_SoundPlayer.bgm->unk_30)
            {
                g_SoundPlayer.bgm->unk_74 = 1;
                hr = g_SoundPlayer.bgm->HandleWaveStreamNotification(bLooped);
                g_SoundPlayer.bgm->unk_74 = 0;
            }
            break;
        case WAIT_OBJECT_0 + 1:
            while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    bDone = TRUE;
                }
            }
            break;
        }
    }
    DebugPrint("atention : ストリーミング用スレッドは終了しました。\r\n");
    return 0;
}
