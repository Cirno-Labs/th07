// The DirectSound helper classes of the DirectX 8 SDK's samples
// (dsutil), as the game modified them. Only what is known so far.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "common.h"

#define WAVEFILE_READ 1

struct BgmFormat;
class CStreamingSound;

class CWaveFile
{
public:
    CWaveFile();
    ~CWaveFile();

    // Opens `strFileName` (thbgm.dat) to stream the track `format`
    // describes, where th06 opened a wave file with mmio.
    HRESULT Open(LPTSTR strFileName, BgmFormat *format, DWORD dwFlags);
    // As the SDK's, with a BgmFormat for the wave format. th06 dropped
    // it.
    HRESULT OpenFromMemory(BYTE *pbData, ULONG ulDataSize, BgmFormat *format, DWORD dwFlags);
    // Switches the stream to the track `format` describes. Returns
    // E_FAIL while (+0x7c) is set or without a file. Not in th06.
    HRESULT Func45e130(BgmFormat *format);

    BgmFormat *GetFormat()
    {
        return this->format;
    }

    unsigned char unk_0[0x90];
    // The track streamed.
    BgmFormat *format;
};

class CSoundManager
{
public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                       DWORD dwPrimaryBitRate);
    HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);
    // As th06's, streaming the track `format` describes out of
    // `strWaveFileName` (thbgm.dat).
    HRESULT CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                            GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent,
                            BgmFormat *format);
    // CreateStreaming for a track read into memory: `size` bytes at
    // `data`, in the format `format` describes. Not in th06.
    HRESULT CreateStreamingFromMemory(CStreamingSound **ppStreamingSound, void *data, DWORD size, BgmFormat *format,
                       DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize,
                       HANDLE hNotifyEvent);

    inline LPDIRECTSOUND8 GetDirectSound()
    {
        return m_pDS;
    }

    LPDIRECTSOUND8 m_pDS;
};

class CSound
{
public:
    virtual ~CSound();

    HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
    LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);
    // Recreates the buffers. Not in th06.
    HRESULT Func45d060();
    HRESULT Play(DWORD dwPriority, DWORD dwFlags);
    HRESULT Stop();
    // Stops the first buffer, keeping its position. Not in th06.
    HRESULT Func45d910();
    // Plays the first buffer again. Not in th06.
    HRESULT Func45d960();

    CWaveFile *GetWaveFile()
    {
        return this->m_pWaveFile;
    }

    LPDIRECTSOUNDBUFFER *m_apDSBuffer;
    DWORD m_dwDSBufferSize;
    CWaveFile *m_pWaveFile;
    DWORD m_dwNumBuffers;
    // As in th06: while `fading`, each frame counts `fadeFrames` down
    // and lowers the volume.
    int fadeFrames;
    int fadeTotal;
    int fading;
    unsigned char unk_20[0x10];
    // Set while the first buffer plays: Func45d960 sets it, Func45d910
    // clears it. Not in th06.
    BOOL unk_30;
};

class CStreamingSound : public CSound
{
public:
    CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize);
    ~CStreamingSound();

    HRESULT HandleWaveStreamNotification(BOOL bLoopedPlay);
    HRESULT Reset();

    // Not in th06: the buffer's description, and the manager that
    // created it, to create it again with.
    DSBUFFERDESC dsbd;
    CSoundManager *manager;
    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNextWriteOffset;
    BOOL m_bFillNextNotificationWithSilence;
    DWORD m_dwNotifySize;
    // Not in th06: the event the buffer signals.
    HANDLE hNotifyEvent;
    // Set by BgmNotificationProc while it fills the buffer.
    int unk_74;
};
