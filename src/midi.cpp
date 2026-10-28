#include "midi.h"
#include "unsorted.h"

MidiDevice::MidiDevice()
{
    this->handle = 0;
    this->deviceId = 0;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

int MidiDevice::Close()
{
    if (this->handle == 0)
    {
        return -1;
    }
    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;
    return 0;
}

int MidiDevice::Open(UINT deviceId)
{
    if (this->handle != 0)
    {
        if (this->deviceId != deviceId)
        {
            this->Close();
        }
        else
        {
            return 0;
        }
    }
    this->deviceId = deviceId;
    return midiOutOpen(&this->handle, deviceId, (DWORD_PTR)g_App.hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR;
}

int MidiDevice::SendLongMsg(LPMIDIHDR header)
{
    if (this->handle == 0)
    {
        return 0;
    }
    if (midiOutPrepareHeader(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
    {
        return 1;
    }
    return midiOutLongMsg(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR;
}

int MidiDevice::SendShortMsg(BYTE status, BYTE data1, BYTE data2)
{
    MidiShortMsg msg;

    if (this->handle == 0)
    {
        return 0;
    }
    msg.bytes[0] = status;
    msg.bytes[1] = data1;
    msg.bytes[2] = data2;
    return midiOutShortMsg(this->handle, msg.dword) != MMSYSERR_NOERROR;
}

MmTimer::MmTimer()
{
    timeGetDevCaps(&this->caps, sizeof(this->caps));
    this->timerId = 0;
}

MmTimer::~MmTimer()
{
    this->Stop();
    timeEndPeriod(this->caps.wPeriodMin);
}

void MmTimer::OnTimer()
{
}

UINT MmTimer::Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user)
{
    this->Stop();
    timeBeginPeriod(this->caps.wPeriodMin);
    if (callback != NULL)
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, callback, user, TIME_PERIODIC);
    }
    else
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, DefaultCallback, (DWORD_PTR)this, TIME_PERIODIC);
    }
    return this->timerId;
}

int MmTimer::Stop()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->caps.wPeriodMin);
    this->timerId = 0;
    return 1;
}

void CALLBACK MmTimer::DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MmTimer *timer = (MmTimer *)user;
    timer->OnTimer();
}

WORD SwapBytes16(WORD value)
{
    union
    {
        WORD word;
        BYTE bytes[2];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[1];
    swapped.bytes[1] = ((BYTE *)&value)[0];
    return swapped.word;
}

DWORD ReadVarLen(BYTE **cursor)
{
    DWORD value = 0;
    BYTE byte;

    do
    {
        byte = **cursor;
        (*cursor)++;
        value = (value << 7) + (byte & 0x7f);
    } while (byte & 0x80);
    return value;
}

MidiPlayer::MidiPlayer()
{
    int i;
    int j;

    this->tracks = NULL;
    this->division = 0;
    this->tempo = 0;
    this->trackCount = 0;
    this->transpose = 0;
    this->volumeScale = 0.0f;
    this->lastVolume = 0;
    this->unk_2d0 = 0;
    this->unk_2d4 = 0;
    this->unk_2d8 = 0;
    this->unk_2dc = 0;
    this->fading = 0;
    for (i = 0; i < 0x20; i++)
    {
        this->fileData[i] = NULL;
    }
    for (j = 0; j < 0x20; j++)
    {
        this->headers[j] = NULL;
    }
    this->playingSlot = -1;
    this->nextHeader = 0;
}

void MidiPlayer::UnloadFile(int slot)
{
    MemFree(this->fileData[slot]);
    this->fileData[slot] = NULL;
}

void MidiPlayer::FreeTracks()
{
    int i;

    for (i = 0; i < this->trackCount; i++)
    {
        MemFree(this->tracks[i].data);
    }
    MemFree(this->tracks);
    this->tracks = NULL;
    this->trackCount = 0;
}

int MidiPlayer::FadeOut(int duration)
{
    this->volumeScale = 0.0f;
    this->fadeDuration = duration;
    this->fadeElapsed = 0;
    this->unk_2dc = 0;
    this->fading = 1;
    return 0;
}

void MidiPlayer::Rewind()
{
    int i;
    MidiTrack *track;

    track = this->tracks;
    this->volumeScale = 1.0f;
    this->unk_2dc = 0;
    this->fading = 0;
    this->ticks = 0;
    this->baseTime = 0;
    for (i = 0; i < this->trackCount; i++, track++)
    {
        track->cursor = track->data;
        track->loopCursor = track->cursor;
        track->active = 1;
        track->delta = ReadVarLen(&track->cursor);
    }
}

int MidiPlayer::Play()
{
    if (this->tracks == NULL)
    {
        return -1;
    }
    this->Rewind();
    this->device.Open(MIDI_MAPPER);
    this->Start(1, NULL, 0);
    DebugPrint(" midi play\n");
    return 0;
}

int MidiPlayer::ReleaseHeader(LPMIDIHDR header)
{
    MMRESULT result;

    if (header == NULL)
    {
        DebugPrint("error :\r\n");
    }
    if (this->device.handle == 0)
    {
        DebugPrint("error :\r\n");
    }
    for (int i = 0; i < 0x20; i++)
    {
        if (this->headers[i] == header)
        {
            this->headers[i] = NULL;
            goto found;
        }
    }
    return -1;

found:
    result = midiOutUnprepareHeader(this->device.handle, header, sizeof(MIDIHDR));
    if (result != MMSYSERR_NOERROR)
    {
        DebugPrint("error :\r\n");
    }
    MemFree(header->lpData);
    MemFree(header);
    return 0;
}

int MidiPlayer::Stop()
{
    int i;

    if (this->tracks == NULL)
    {
        return -1;
    }
    for (i = 0; i < 0x20; i++)
    {
        // Indexes with nextHeader, not i, in the original.
        if (this->headers[this->nextHeader] != NULL)
        {
            this->ReleaseHeader(this->headers[this->nextHeader]);
        }
    }
    MmTimer::Stop();
    this->device.Close();
    this->playingSlot = -1;
    return 0;
}

int MidiPlayer::LoadFile(int slot, char *path)
{
    if (this->playingSlot == slot)
    {
        this->Stop();
    }
    this->UnloadFile(slot);
    this->fileData[slot] = LoadFileData(path, 0);
    if (this->fileData[slot] == NULL)
    {
        // "\rv\r\n" in the original.
        LogPrintf(&g_Log, "error : MIDI File が読み込めない %s \rv\r\n", path);
        return -1;
    }
    return 0;
}

MidiPlayer::~MidiPlayer()
{
    int i;

    this->Stop();
    this->FreeTracks();
    for (i = 0; i < 0x20; i++)
    {
        this->UnloadFile(i);
    }
}

void MidiPlayer::SendVolume(int offset)
{
    int cc;
    int i;
    int chStatus;
    int data2;
    int volume;

    if (this->unk_2d4 != 0)
    {
        return;
    }
    cc = 7;
    for (i = 0; i < 0x10; i++)
    {
        chStatus = (BYTE)(i + 0xb0);
        volume = (int)(this->channels[i].volume * this->volumeScale) + offset;
        if (volume < 0)
        {
            volume = 0;
        }
        else if (volume > 0x7f)
        {
            volume = 0x7f;
        }
        data2 = (BYTE)volume;
        this->device.SendShortMsg(chStatus, cc, data2);
    }
}

int MidiPlayer::ParseFile(int slot)
{
    int i;
    BYTE *cursor;
    BYTE *file;
    BYTE *track;
    DWORD headerLength;
    MidiChunk hdr;
    BYTE *header;
    DWORD trackLen;

    this->FreeTracks();
    cursor = (BYTE *)this->fileData[slot];
    file = cursor;
    if (cursor == NULL)
    {
        DebugPrint("error : まだMIDIが読み込まれていないのに再生しようとしている\r\n");
        return -1;
    }
    memcpy(&hdr, cursor, sizeof(MidiChunk));
    cursor += sizeof(MidiChunk);
    headerLength = SwapBytes32(hdr.length);
    header = cursor;
    cursor += headerLength;
    this->format = SwapBytes16(*(WORD *)header);
    this->division = SwapBytes16(*(WORD *)(header + 4));
    this->trackCount = SwapBytes16(*(WORD *)(header + 2));
    this->tracks = (MidiTrack *)MemAlloc(this->trackCount * sizeof(MidiTrack));
    memset(this->tracks, 0, this->trackCount * sizeof(MidiTrack));
    for (i = 0; i < this->trackCount; i++)
    {
        track = cursor;
        cursor += sizeof(MidiChunk);
        trackLen = SwapBytes32(((MidiChunk *)track)->length);
        this->tracks[i].length = trackLen;
        this->tracks[i].data = (BYTE *)MemAlloc(trackLen);
        this->tracks[i].active = 1;
        memcpy(this->tracks[i].data, cursor, trackLen);
        cursor += trackLen;
    }
    this->tempo = 1000000;
    this->playingSlot = slot;
    DebugPrint(" midi open %d\n", slot);
    return 0;
}
