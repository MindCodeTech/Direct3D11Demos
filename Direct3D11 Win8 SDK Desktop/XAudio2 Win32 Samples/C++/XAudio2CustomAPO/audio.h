//--------------------------------------------------------------------------------------
// File: audio.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xaudio2.h>
#include <xaudio2fx.h>
#pragma comment(lib,"xaudio2.lib")
#else
#include <c:\dxsdk\Include\comdecl.h>
#include <c:\dxsdk\Include\xaudio2.h>
#include <c:\dxsdk\Include\xaudio2fx.h>
#endif

#include "SimpleAPO.h"
#include "MonitorAPO.h"

//-----------------------------------------------------------------------------
// Global defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Struct to hold audio game state
//-----------------------------------------------------------------------------
struct AUDIO_STATE
{
    bool bInitialized;

    // XAudio2
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasteringVoice;
    IXAudio2SourceVoice* pSourceVoice;
    std::unique_ptr<uint8_t> waveData;

    // APOs
    SimpleAPOParams simpleParams;
    MonitorAPOPipe *pPipePre;
    MonitorAPOPipe *pPipePost;
};


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern AUDIO_STATE  g_audioState;


//--------------------------------------------------------------------------------------
// External functions
//--------------------------------------------------------------------------------------
HRESULT InitAudio();
HRESULT PrepareAudio( const LPWSTR wavname );
VOID SetSimpleGain( float gain );
VOID PauseAudio( bool resume );
VOID CleanupAudio();
