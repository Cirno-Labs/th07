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
    // The game's reset limits the data to the track's loopEnd bytes
    // and, when `bLoop` is set, skips its first loopStart bytes, as
    // th06's.
    HRESULT ResetFile(bool bLoop);
    DWORD GetSize();
    HRESULT Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead);
    // Closes a file opened for reading.
    HRESULT Close();
    // Switches the stream to the track `format` describes. Returns
    // E_FAIL when reading from memory or without a file. Not in th06.
    HRESULT SwitchTrack(BgmFormat *format);

    BgmFormat *GetFormat()
    {
        return this->format;
    }

    // The SDK's fields, without th06's HMMIO: the wave format is the
    // BgmFormat's, and only m_ck.cksize is used, as the bytes left.
    WAVEFORMATEX *m_pwfx;
    MMCKINFO m_ck;
    MMCKINFO m_ckRiff;
    DWORD m_dwSize;
    MMIOINFO m_mmioinfoOut;
    DWORD m_dwFlags;
    BOOL m_bIsReadingFromMemory;
    BYTE *m_pbData;
    BYTE *m_pbDataCur;
    ULONG m_ulDataSize;
    // Not in the SDK: the file streamed from, in place of th06's HMMIO.
    HANDLE hFile;
    // The track streamed, in place of th06's loopStart and loopEnd.
    BgmFormat *format;
};
ASSERT_SIZE(CWaveFile, 0x94);

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
    CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile);
    virtual ~CSound();

    HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
    HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored);
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);
    HRESULT Play(DWORD dwPriority, DWORD dwFlags);
    HRESULT Stop();
    // Stops the first buffer, keeping its position. Not in th06.
    HRESULT PauseBuffer();
    // Plays the first buffer again. Not in th06.
    HRESULT ResumeBuffer();
    HRESULT Reset();

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
    // Not in th06: Play's priority and flags, for ResumeBuffer.
    DWORD dwPlayPriority;
    DWORD dwPlayFlags;
    DWORD unk_28;
    // Cleared by Play.
    DWORD unk_2c;
    // Set while the first buffer plays: ResumeBuffer sets it, PauseBuffer
    // clears it. Not in th06.
    BOOL unk_30;
};

class CStreamingSound : public CSound
{
public:
    CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize);
    ~CStreamingSound();

    // Not in the SDK: steps a fade begun with `fading`, run once a frame.
    // Stops the buffer and returns 1 when the fade ends, otherwise 0.
    int UpdateFade();

    // Not in th06: releases the buffers and creates them again from
    // dsbd, with 16 notifications every m_dwNotifySize bytes.
    HRESULT RecreateBuffers();

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
