// The DirectSound helper classes of the DirectX 8 SDK's samples
// (dsutil), as the game modified them. Only what is known so far.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "common.h"

struct BgmFormat;
class CStreamingSound;

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
};

class CStreamingSound : public CSound
{
public:
    ~CStreamingSound();
};
