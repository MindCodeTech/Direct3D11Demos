//--------------------------------------------------------------------------------------
// File: audio.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib")
#else
#include <c:\dxsdk\Include\comdecl.h>
#include <c:\dxsdk\Include\xaudio2.h>
#include <c:\dxsdk\Include\xaudio2fx.h>
#pragma warning(push)
#pragma warning( disable : 4005 )
#include <c:\dxsdk\Include\x3daudio.h>
#pragma warning(pop)
#pragma comment(lib,"x3daudio.lib")
#endif

//-----------------------------------------------------------------------------
// Global defines
//-----------------------------------------------------------------------------
#define INPUTCHANNELS 1  // number of source channels
#define OUTPUTCHANNELS 8 // maximum number of destination channels supported in this sample

#define NUM_PRESETS 30

// Constants to define our world space
const INT           XMIN = -10;
const INT           XMAX = 10;
const INT           ZMIN = -10;
const INT           ZMAX = 10;

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
    IXAudio2SubmixVoice* pSubmixVoice;
    IUnknown* pReverbEffect;
    std::unique_ptr<uint8_t> waveData;

    // 3D
    X3DAUDIO_HANDLE x3DInstance;
    int nFrameToApply3DAudio;

    DWORD dwChannelMask;
    UINT32 nChannels;

    X3DAUDIO_DSP_SETTINGS dspSettings;
    X3DAUDIO_LISTENER listener;
    X3DAUDIO_EMITTER emitter;
    X3DAUDIO_CONE emitterCone;

    DirectX::XMFLOAT3 vListenerPos;
    DirectX::XMFLOAT3 vEmitterPos;
    float fListenerAngle;
    bool  fUseListenerCone;
    bool  fUseInnerRadius;
    bool  fUseRedirectToLFE;

    FLOAT32 emitterAzimuths[INPUTCHANNELS];
    FLOAT32 matrixCoefficients[INPUTCHANNELS * OUTPUTCHANNELS];
};


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern AUDIO_STATE  g_audioState;


//--------------------------------------------------------------------------------------
// External functions
//--------------------------------------------------------------------------------------
HRESULT InitAudio();
HRESULT PrepareAudio( _In_z_ const LPWSTR wavname );
HRESULT UpdateAudio( float fElapsedTime );
HRESULT SetReverb( int nReverb );
VOID PauseAudio( bool resume );
VOID CleanupAudio();
