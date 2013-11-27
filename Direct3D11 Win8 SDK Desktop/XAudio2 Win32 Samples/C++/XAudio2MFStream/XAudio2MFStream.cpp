//--------------------------------------------------------------------------------------
// File: XAudio2MFStream.cpp
//
// Streaming from a media file, using Media Foundation to decompress the data
//
// Note: This sample will only run on N or KN editions of Windows if the appropriate
//       Windows Media Feature Pack has been installed.
//
// http://blogs.msdn.com/b/chuckw/archive/2010/08/13/quot-who-moved-my-windows-media-cheese-quot.aspx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include <initguid.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfreadwrite.lib")

#include <wrl.h> // for Microsoft::WRL::ComPtr

#include <memory>

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#elif (_WIN32_WINNT < _WIN32_WINNT_WIN7 )
#error This code needs _WIN32_WINNT set to 0x0601 or higher. It should be compatible with Windows Vista with KB 2117917 installed
#else
#include <c:\dxsdk\Include\comdecl.h>
#include <c:\dxsdk\Include\xaudio2.h>
#endif

using Microsoft::WRL::ComPtr;


//--------------------------------------------------------------------------------------
#define MAX_BUFFER_COUNT 3


//--------------------------------------------------------------------------------------
// Helper macros
//--------------------------------------------------------------------------------------
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
#endif


//--------------------------------------------------------------------------------------
// Callback structure
//--------------------------------------------------------------------------------------
struct StreamingVoiceContext : public IXAudio2VoiceCallback
{
    STDMETHOD_( void, OnVoiceProcessingPassStart )( UINT32 ) override
    {
    }
    STDMETHOD_( void, OnVoiceProcessingPassEnd )() override
    {
    }
    STDMETHOD_( void, OnStreamEnd )() override
    {
    }
    STDMETHOD_( void, OnBufferStart )( void* ) override
    {
    }
    STDMETHOD_( void, OnBufferEnd )( void* ) override
    {
        SetEvent( hBufferEndEvent );
    }
    STDMETHOD_( void, OnLoopEnd )( void* ) override
    {
    }
    STDMETHOD_( void, OnVoiceError )( void*, HRESULT ) override
    {
    }

    HANDLE hBufferEndEvent;

    StreamingVoiceContext() :
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
        hBufferEndEvent( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) )
#else
        hBufferEndEvent( CreateEvent( nullptr, FALSE, FALSE, nullptr ) )
#endif
    {
    }
    virtual ~StreamingVoiceContext()
    {
        CloseHandle( hBufferEndEvent );
    }
};


//--------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------
HRESULT CreateMFReader( _In_z_ const WCHAR* mediaFile, _Outptr_ IMFSourceReader ** reader, _Out_ WAVEFORMATEX* wfx, _In_ size_t maxwfx );

HRESULT FindMediaFileCch( _Out_writes_(cchDest) WCHAR* strDestPath, _In_ int cchDest, _In_z_ LPCWSTR strFilename );


//--------------------------------------------------------------------------------------
// Entry point to the program
//--------------------------------------------------------------------------------------
int main()
{
    //
    // Initialize XAudio2
    //
    CoInitializeEx( nullptr, COINIT_MULTITHREADED );

    IXAudio2* pXAudio2 = nullptr;

    UINT32 flags = 0;
 #if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    flags |= XAUDIO2_DEBUG_ENGINE;
 #endif
    HRESULT hr = XAudio2Create( &pXAudio2, flags );
    if( FAILED( hr ) )
    {
        wprintf( L"Failed to init XAudio2 engine: %#X\n", hr );
        CoUninitialize();
        return 0;
    }

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    // To see the trace output, you need to view ETW logs for this application:
    //    Go to Control Panel, Administrative Tools, Event Viewer.
    //    View->Show Analytic and Debug Logs.
    //    Applications and Services Logs / Microsoft / Windows / XAudio2. 
    //    Right click on Microsoft Windows XAudio2 debug logging, Properties, then Enable Logging, and hit OK 
    XAUDIO2_DEBUG_CONFIGURATION debug ={0};
    debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    debug.BreakMask = XAUDIO2_LOG_ERRORS;
    pXAudio2->SetDebugConfiguration( &debug, 0 );
#endif

    //
    // Create a mastering voice
    //
    IXAudio2MasteringVoice* pMasteringVoice = nullptr;

    if( FAILED( hr = pXAudio2->CreateMasteringVoice( &pMasteringVoice ) ) )
    {
        wprintf( L"Failed creating mastering voice: %#X\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }

    //
    // Find our media file
    //
    WCHAR mediaFile[ MAX_PATH ];

    if( FAILED( hr = FindMediaFileCch( mediaFile, MAX_PATH, L"Media\\Wavs\\becky.wma" ) ) )
    {
        wprintf( L"Failed to find media file (%#X)\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }

    //
    // Start up Media Foundation
    //
    hr = MFStartup( MF_VERSION );
    if ( FAILED(hr) )
    {
        wprintf( L"Failed to initialize Media Foundation (%#X)\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }

    //
    // Create MF reader for our media file
    //
    ComPtr<IMFSourceReader> reader;
    WAVEFORMATEX wfx;
    hr = CreateMFReader( mediaFile, reader.GetAddressOf(), &wfx, sizeof(wfx) );
    if( FAILED(hr) )
    {
        wprintf( L"Failed to create media reader (%#X)\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }

    //
    // Create the source voice
    //
    StreamingVoiceContext voiceContext;

    IXAudio2SourceVoice* pSourceVoice;
    hr = pXAudio2->CreateSourceVoice( &pSourceVoice, &wfx, 0, 1.0f, &voiceContext );
    if( FAILED( hr ) )
    {
        wprintf( L"Error %#X creating source voice\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }
    pSourceVoice->Start( 0, 0 );

    //
    // This loop continously updates the audio stream. Within an application, this should be handled
    // on a worker thread rather than the main thread.
    //
    wprintf( L"Press <ESC> to exit.\n" );
    wprintf( L"Now playing %s", mediaFile );

    DWORD currentStreamBuffer = 0;

    hr = S_OK;

    size_t bufferSize[MAX_BUFFER_COUNT]={0};
    std::unique_ptr<uint8_t[]> buffers[MAX_BUFFER_COUNT];

    for(;;)
    {
        wprintf( L"." );

        if( GetAsyncKeyState( VK_ESCAPE ) )
        {
            while( GetAsyncKeyState( VK_ESCAPE ) )
                Sleep( 10 );

            break;
        }

        DWORD flags = 0;
        ComPtr<IMFSample> sample;
        hr = reader->ReadSample( MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, sample.GetAddressOf() );
        if ( FAILED(hr) )
            break;

        // TODO - Wait for ReadSample async to complete

        if ( flags & MF_SOURCE_READERF_ENDOFSTREAM )
        {
            wprintf( L"restart streaming.." );

            // Restart loop
            PROPVARIANT var={0};
            var.vt = VT_I8;

            hr = reader->SetCurrentPosition( GUID_NULL, var );
            if ( SUCCEEDED(hr) )
                continue;
            else
                break;
        }

        ComPtr<IMFMediaBuffer> mediaBuffer;
        hr = sample->ConvertToContiguousBuffer( mediaBuffer.GetAddressOf() );
        if ( FAILED(hr) )
            return hr;

        BYTE* audioData = nullptr;
        DWORD sampleBufferLength = 0;

        hr = mediaBuffer->Lock( &audioData, nullptr, &sampleBufferLength );
        if ( FAILED(hr) )
            return hr;

        if ( bufferSize[ currentStreamBuffer ] < sampleBufferLength )
        {
            buffers[ currentStreamBuffer ].reset( new uint8_t[ sampleBufferLength ] );
            bufferSize[ currentStreamBuffer ] = sampleBufferLength;
        }

        memcpy_s( buffers[ currentStreamBuffer ].get(), sampleBufferLength, audioData, sampleBufferLength );

        hr = mediaBuffer->Unlock();
        if ( FAILED(hr) )
            return hr;

        //
        // Wait to se if our XAudio2 source voice has played enough data for us to give
        // it another buffer full of audio. We'd like to keep no more than MAX_BUFFER_COUNT - 1
        // buffers on the queue, so that one buffer is always free for the MF streamer
        //
        XAUDIO2_VOICE_STATE state;
        for( ;; )
        {
            pSourceVoice->GetState( &state );
            if( state.BuffersQueued < MAX_BUFFER_COUNT - 1 )
                break;

            WaitForSingleObject( voiceContext.hBufferEndEvent, INFINITE );
        }

        XAUDIO2_BUFFER buf = {0};
        buf.AudioBytes = sampleBufferLength;
        buf.pAudioData = buffers[ currentStreamBuffer ].get();
        pSourceVoice->SubmitSourceBuffer( &buf );

        currentStreamBuffer++;
        currentStreamBuffer %= MAX_BUFFER_COUNT;
    }

    if ( FAILED(hr) )
    {
        wprintf( L"\nError %#X during playback!\n", hr );
    }

    reader->Flush( MF_SOURCE_READER_FIRST_AUDIO_STREAM );

    //
    // Cleanup XAudio2
    //

    // All XAudio2 interfaces are released when the engine is destroyed, but being tidy
    pSourceVoice->DestroyVoice();
    pMasteringVoice->DestroyVoice();

    //
    // Cleanup Media Foundation
    //
    reader = nullptr;
    MFShutdown();

    SAFE_RELEASE( pXAudio2 );
    CoUninitialize();
}


//--------------------------------------------------------------------------------------
// Helper for setting up the MF source reader
//--------------------------------------------------------------------------------------
HRESULT CreateMFReader( _In_z_ const WCHAR* mediaFile, _Outptr_ IMFSourceReader ** reader, _Out_ WAVEFORMATEX* wfx, _In_ size_t maxwfx  )
{
    if ( !mediaFile || !reader || !wfx )
        return E_INVALIDARG;

    HRESULT hr;
    ComPtr<IMFAttributes> lowLatency;
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    hr = MFCreateAttributes( lowLatency.GetAddressOf(), 1 );
    if ( FAILED(hr) )
        return hr;

    hr = lowLatency->SetUINT32( MF_LOW_LATENCY, TRUE );
    if ( FAILED(hr) )
        return hr;
#endif
    
    hr = MFCreateSourceReaderFromURL( mediaFile, lowLatency.Get(), reader );
    if ( FAILED(hr) )
        return hr;

    //
    // Make the output from Media Foundation PCM so XAudio2 can consume it
    //

    ComPtr<IMFMediaType> mediaType;
    hr = MFCreateMediaType( mediaType.GetAddressOf() );
    if ( FAILED(hr) )
        return hr;

    hr = mediaType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Audio );
    if ( FAILED(hr) )
        return hr;

    hr = mediaType->SetGUID( MF_MT_SUBTYPE, MFAudioFormat_PCM );
    if ( FAILED(hr) )
        return hr;

    hr = (*reader)->SetCurrentMediaType( MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, mediaType.Get() );
    if ( FAILED(hr) )
        return hr;

    //
    // Get the wave format
    //

    ComPtr<IMFMediaType> outputMediaType;
    hr = (*reader)->GetCurrentMediaType( MF_SOURCE_READER_FIRST_AUDIO_STREAM, outputMediaType.GetAddressOf() );
    if ( FAILED(hr) )
        return hr;

    UINT32 waveFormatSize = 0;
    WAVEFORMATEX* waveFormat = nullptr;
    hr = MFCreateWaveFormatExFromMFMediaType( outputMediaType.Get(), &waveFormat, &waveFormatSize );
    if ( FAILED(hr) )
        return hr;

    memcpy_s( wfx, maxwfx, waveFormat, waveFormatSize );
    CoTaskMemFree( waveFormat );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper function to try to find the location of a media file
//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT FindMediaFileCch( WCHAR* strDestPath, int cchDest, LPCWSTR strFilename )
{
    bool bFound = false;

    if( !strFilename || strFilename[0] == 0 || !strDestPath || cchDest < 10 )
        return E_INVALIDARG;

    // Get the exe name, and exe path
    WCHAR strExePath[MAX_PATH] = {0};
    WCHAR strExeName[MAX_PATH] = {0};
    WCHAR* strLastSlash = nullptr;
    GetModuleFileName( nullptr, strExePath, MAX_PATH );
    strExePath[MAX_PATH - 1] = 0;
    strLastSlash = wcsrchr( strExePath, TEXT( '\\' ) );
    if( strLastSlash )
    {
        wcscpy_s( strExeName, MAX_PATH, &strLastSlash[1] );

        // Chop the exe name from the exe path
        *strLastSlash = 0;

        // Chop the .exe from the exe name
        strLastSlash = wcsrchr( strExeName, TEXT( '.' ) );
        if( strLastSlash )
            *strLastSlash = 0;
    }

    wcscpy_s( strDestPath, cchDest, strFilename );
    if( GetFileAttributes( strDestPath ) != 0xFFFFFFFF )
        return S_OK;

    // Search all parent directories starting at .\ and using strFilename as the leaf name
    WCHAR strLeafName[MAX_PATH] = {0};
    wcscpy_s( strLeafName, MAX_PATH, strFilename );

    WCHAR strFullPath[MAX_PATH] = {0};
    WCHAR strFullFileName[MAX_PATH] = {0};
    WCHAR strSearch[MAX_PATH] = {0};
    WCHAR* strFilePart = nullptr;

    GetFullPathName( L".", MAX_PATH, strFullPath, &strFilePart );
    if( !strFilePart )
        return E_FAIL;

    while( strFilePart && *strFilePart != '\0' )
    {
        swprintf_s( strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName );
        if( GetFileAttributes( strFullFileName ) != 0xFFFFFFFF )
        {
            wcscpy_s( strDestPath, cchDest, strFullFileName );
            bFound = true;
            break;
        }

        swprintf_s( strFullFileName, MAX_PATH, L"%s\\%s\\%s", strFullPath, strExeName, strLeafName );
        if( GetFileAttributes( strFullFileName ) != 0xFFFFFFFF )
        {
            wcscpy_s( strDestPath, cchDest, strFullFileName );
            bFound = true;
            break;
        }

        swprintf_s( strSearch, MAX_PATH, L"%s\\..", strFullPath );
        GetFullPathName( strSearch, MAX_PATH, strFullPath, &strFilePart );
    }
    if( bFound )
        return S_OK;

    // On failure, return the file as the path but also return an error code
    wcscpy_s( strDestPath, cchDest, strFilename );

    return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
}
