// The DirectSound helper classes of the DirectX 8 SDK's samples
// (dsutil), as the game modified them. Only what is known so far.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "common.h"

struct BgmFormat;
class CStreamingSound;

class CWaveFile
{
public:
    // Switches the stream to the track `format` describes. Returns
    // E_FAIL while (+0x7c) is set or without a file. Not in th06.
    HRESULT Func45e130(BgmFormat *format);
};

class CSoundManager
{
public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                       DWORD dwPrimaryBitRate);
    // As th06's, streaming the track `format` describes out of
    // `strWaveFileName` (thbgm.dat).
    HRESULT CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                            GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent,
                            BgmFormat *format);
    // CreateStreaming for a track read into memory: `size` bytes at
    // `data`, in the format `format` describes. Not in th06.
    HRESULT Func45cc30(CStreamingSound **ppStreamingSound, void *data, DWORD size, BgmFormat *format,
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

    LPDIRECTSOUNDBUFFER *m_apDSBuffer;
    DWORD m_dwDSBufferSize;
    CWaveFile *m_pWaveFile;
};

class CStreamingSound : public CSound
{
public:
    ~CStreamingSound();
};
