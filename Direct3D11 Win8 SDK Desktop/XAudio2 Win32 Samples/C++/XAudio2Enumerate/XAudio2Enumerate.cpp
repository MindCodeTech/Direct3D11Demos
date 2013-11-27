//--------------------------------------------------------------------------------------
// File: XAudio2Enumerate.cpp
//
// Demonstrates enumerating audio devices and creating a XAudio2 mastering voice for them
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include <string>
#include <vector>

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#pragma comment(lib,"runtimeobject.lib")
#include <Windows.Devices.Enumeration.h>
#include <wrl.h>
#else
#include <c:\dxsdk\Include\comdecl.h>
#include <c:\dxsdk\Include\xaudio2.h>
#endif

//--------------------------------------------------------------------------------------
// Helper macros
//--------------------------------------------------------------------------------------
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
#endif


//--------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------
struct AudioDevice
{
    std::wstring deviceId;
    std::wstring description;
};

HRESULT EnumerateAudio( _In_ IXAudio2* pXaudio2, _Inout_ std::vector<AudioDevice>& list );


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
    // Enumerate and display audio devices on the system
    //
    std::vector<AudioDevice> list;
    hr = EnumerateAudio( pXAudio2, list );
    if( FAILED( hr ) )
    {
        wprintf( L"Failed to enumerate audio devices: %#X\n", hr );
        CoUninitialize();
        return 0;
    }

    if ( hr == S_FALSE )
    {
        wprintf( L"No audio devices found\n");
        CoUninitialize();
        return 0;
    }

    UINT32 devcount = 0;
    UINT32 devindex = -1;
    for( auto it = list.cbegin(); it != list.cend(); ++it, ++devcount )
    {
        wprintf( L"\nDevice %u\n\tID = \"%s\"\n\tDescription = \"%s\"\n",
                 devcount,
                 it->deviceId.c_str(),
                 it->description.c_str() );

        // Simple selection criteria of just picking the first one
        if ( devindex == -1 )
        {
            devindex = devcount;
        }
    }

    wprintf( L"\n" );

    //
    // Create a mastering voice
    //
    IXAudio2MasteringVoice* pMasteringVoice = nullptr;

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    if( FAILED( hr = pXAudio2->CreateMasteringVoice( &pMasteringVoice,
                                                     XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0,
                                                     list[ devindex ].deviceId.c_str() ) ) )
#else
    if( FAILED( hr = pXAudio2->CreateMasteringVoice( &pMasteringVoice,
                                                     XAUDIO2_DEFAULT_CHANNELS,
                                                     XAUDIO2_DEFAULT_SAMPLERATE, 0,
                                                     devindex ) ) )
#endif
    {
        wprintf( L"Failed creating mastering voice: %#X\n", hr );
        SAFE_RELEASE( pXAudio2 );
        CoUninitialize();
        return 0;
    }

    XAUDIO2_VOICE_DETAILS details;
    pMasteringVoice->GetVoiceDetails( &details );

    wprintf( L"Mastering voice created with %u input channels, %u sample rate\n", details.InputChannels, details.InputSampleRate );

    //
    // Cleanup XAudio2
    //
    // All XAudio2 interfaces are released when the engine is destroyed, but being tidy
    pMasteringVoice->DestroyVoice();

    SAFE_RELEASE( pXAudio2 );
    CoUninitialize();
}


//--------------------------------------------------------------------------------------
// Enumerate audio end-points
//--------------------------------------------------------------------------------------
HRESULT EnumerateAudio( _In_ IXAudio2* pXaudio2, _Inout_ std::vector<AudioDevice>& list )
{
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)

    UNREFERENCED_PARAMETER( pXaudio2 );

#if defined(__cplusplus_winrt )

    // Enumerating with WinRT using C++/CX
    using Windows::Devices::Enumeration::DeviceClass;
    using Windows::Devices::Enumeration::DeviceInformation;
    using Windows::Devices::Enumeration::DeviceInformationCollection;
 
    auto operation = DeviceInformation::FindAllAsync(DeviceClass::AudioRender);
    while (operation->Status != Windows::Foundation::AsyncStatus::Completed)
        ;
 
    DeviceInformationCollection^ devices = operation->GetResults();

    if ( !devices->Size )
        return S_FALSE;
 
    for( unsigned i=0; i < devices->Size; ++i )
    {
        using Windows::Devices::Enumeration::DeviceInformation;
 
        DeviceInformation^ d = devices->GetAt(i);

        AudioDevice device;
        device.deviceId = d->Id->Data();
        device.description = d->Name->Data();
        list.emplace_back( device );
    }

#else

    // Enumerating with WinRT using WRL
    using namespace Microsoft::WRL;
    using namespace Microsoft::WRL::Wrappers;
    using namespace ABI::Windows::Foundation;
    using namespace ABI::Windows::Foundation::Collections;
    using namespace ABI::Windows::Devices::Enumeration;

    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    HRESULT hr = initialize;
    if ( FAILED(hr) )
        return hr;

    Microsoft::WRL::ComPtr<IDeviceInformationStatics> diFactory;
    hr = ABI::Windows::Foundation::GetActivationFactory( HStringReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &diFactory );
    if ( FAILED(hr) )
        return hr;

    Event findCompleted( CreateEventEx( nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, WRITE_OWNER | EVENT_ALL_ACCESS ) );
    if ( !findCompleted.IsValid() )
        return HRESULT_FROM_WIN32( GetLastError() );

    auto callback = Callback<IAsyncOperationCompletedHandler<DeviceInformationCollection*>>(
        [&findCompleted,list]( IAsyncOperation<DeviceInformationCollection*>* aDevices, AsyncStatus status ) -> HRESULT
    {
        SetEvent( findCompleted.Get() );
        return S_OK;
    });

    ComPtr<IAsyncOperation<DeviceInformationCollection*>> operation;
    hr = diFactory->FindAllAsyncDeviceClass( DeviceClass_AudioRender, operation.GetAddressOf() );
    if ( FAILED(hr) )
        return hr;

    operation->put_Completed( callback.Get() );

    WaitForSingleObject( findCompleted.Get(), INFINITE );

    ComPtr<IVectorView<DeviceInformation*>> devices;
    operation->GetResults( devices.GetAddressOf() );

    unsigned int count = 0;
    hr = devices->get_Size( &count );
    if ( FAILED(hr) )
        return hr;

    if ( !count )
        return S_FALSE;

    for( unsigned int j = 0; j < count; ++j )
    {
        ComPtr<IDeviceInformation> deviceInfo;
        hr = devices->GetAt( j, deviceInfo.GetAddressOf() );
        if ( SUCCEEDED(hr) )
        {
            HString id;
            deviceInfo->get_Id( id.GetAddressOf() );

            HString name;
            deviceInfo->get_Name( name.GetAddressOf() );

            AudioDevice device;
            device.deviceId = id.GetRawBuffer( nullptr );
            device.description = name.GetRawBuffer( nullptr );
            list.emplace_back( device );
        }
    }

    return S_OK;

#endif 

#else // _WIN32_WINNT < 0x0602

    // Enumerating with XAudio 2.7
    UINT32 count = 0;
    HRESULT hr = pXaudio2->GetDeviceCount( &count );
    if ( FAILED(hr) )
        return hr;

    if ( !count )
        return S_FALSE;

    list.reserve( count );

    for( UINT32 j = 0; j < count; ++j )
    {
        XAUDIO2_DEVICE_DETAILS details;
        hr = pXaudio2->GetDeviceDetails( j, &details );
        if ( FAILED(hr) )
            return hr;

        AudioDevice device;
        device.deviceId = details.DeviceID;
        device.description = details.DisplayName;
        list.emplace_back( device );
    }

#endif

    return S_OK;
}
