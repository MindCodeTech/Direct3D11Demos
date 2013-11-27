//--------------------------------------------------------------------------------------
// File: D3D11InstallHelper.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

// Exclude rarely-used stuff from Windows headers
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>

#if defined(LIB_EXPORT) || defined(D3D11INSTALLHELPERLIB_EXPORT)
#ifndef D3D11INSTALLHELPERLIB_IMPORT
#ifndef D3D11INSTALLHELPERLIB_EXPORT
#define D3D11INSTALLHELPERLIB_EXPORT 1
#endif
#endif
#endif

#if defined(LIB_IMPORT) || defined(D3D11INSTALLHELPERLIB_IMPORT)
#if defined(D3D11INSTALLHELPERLIB_EXPORT)
#error ("!!!You are bulding D3D11INSTALLHELPER export and import simultaniously")
#else
#ifndef D3D11INSTALLHELPERLIB_IMPORT
#define D3D11INSTALLHELPERLIB_IMPORT 1
#endif
#endif
#endif

#if defined(LIB_STATIC) || defined(D3D11INSTALLHELPERLIB_STATIC)
#ifndef D3D11INSTALLHELPERLIB_DLL
#ifndef D3D11INSTALLHELPERLIB_STATIC 
#define D3D11INSTALLHELPERLIB_STATIC 1
#endif
#endif
#endif

#if (defined(_DLL) || defined(_USRDLL) || defined(_WINDLL) || defined(LIB_DYNAMIC) || defined(D3D11INSTALLHELPERLIB_DLL)) && !defined(_LIB) && !defined(D3D11INSTALLHELPERLIB_STATIC)
#ifndef D3D11INSTALLHELPERLIB_STATIC
#ifndef D3D11INSTALLHELPERLIB_DLL
#define D3D11INSTALLHELPERLIB_DLL 1
#endif
#endif
#endif

#if !defined(D3D11INSTALLHELPERLIB_DLL) && !defined(D3D11INSTALLHELPERLIB_STATIC)
#error ("!!!Your D3D11INSTALLHELPER lib type static or dll aren't defined")
#endif

#ifdef D3D11INSTALLHELPERLIB_IMPORT
#ifdef D3D11INSTALLHELPERLIB_DLL
#ifdef _DEBUG
#pragma comment( lib, "D3D11InstallHelper_d.lib" )
#else
#pragma comment( lib, "D3D11InstallHelper.lib" )
#endif
#elif D3D11INSTALLHELPERLIB_STATIC
#ifdef _DEBUG
#pragma comment( lib, "D3D11InstallHelpers_d.lib" )
#else
#pragma comment( lib, "D3D11InstallHelpers.lib" )
#endif
#else
#pragma warning ("D3D11INSTALLHELPERLIB_IMPORT import librarys aren't defined")
#endif
#endif


enum D3D11IH_STATUS
{
   D3D11IH_STATUS_INSTALLED = 0,
       // Direct3D 11 is already installed

   D3D11IH_STATUS_NOT_SUPPORTED = 1,
       // Direct3D 11 not supported on this OS

   D3D11IH_STATUS_REQUIRES_UPDATE = 2,
       // Direct3D 11 is not yet installed, needs update package applied

   D3D11IH_STATUS_NEED_LATEST_SP = 3,
       // Direct3D 11 cannot be installed on this system without a Service Pack update
};

enum D3D11IH_RESULT
{
   D3D11IH_RESULT_SUCCESS = 0,
       // Direct3D 11 update applied succesfully (or already present)

   D3D11IH_RESULT_SUCCESS_REBOOT = 1,
       // Direct3D 11 update applied succesfully, needs a reboot to complete

   D3D11IH_RESULT_NOT_SUPPORTED = 2,
       // Direct3D 11 update not supported for this OS

   D3D11IH_RESULT_UPDATE_NOT_FOUND = 3,
       // Direct3D 11 update not found on Windows Update

   D3D11IH_RESULT_UPDATE_DOWNLOAD_FAILED = 4,
       // Direct3D 11 update failed to download from Windows Update

   D3D11IH_RESULT_UPDATE_INSTALL_FAILED = 5,
       // Direct3D 11 update failed to install through Windows Update

   D3D11IH_RESULT_WU_SERVICE_ERROR = 6,
       // Windows Update error related to service, server, or online connection
};

enum D3D11IH_PROGRESS
{
    D3D11IH_PROGRESS_BEGIN = 0,
        // Called once to allow progress callback to initialize

    D3D11IH_PROGRESS_SEARCHING = 1,
        // Searching for update (progress is 0 during search, 100 when complete)

    D3D11IH_PROGRESS_DOWNLOADING = 2,
        // Downloading update  (progress as percentage, 100 when complete)

    D3D11IH_PROGRESS_INSTALLING = 3,
        // Installing update (progress as percentage, 100 when complete)

    D3D11IH_PROGRESS_END = 4,
        // Called once to allow progress callback to clean up
};

#define D3D11IH_QUIET           0x1
	// Install quietly (if possible)

#define D3D11IH_WINDOWS_UPDATE  0x2
	// Use Microsoft Windows Update server rather than default (possibly managed WSUS) server


//--------------------------------------------------------------------------------------
// Desc: Callback prototype for DoUpdateForDirect3D11 progress notifications
//
// Params: [in] phase - see D3D11IH_PROGRESS
//         [in] progress - Current progress value
//         [in] pContext - User context value provided on call tO DoUpdateForDirect3D11
//--------------------------------------------------------------------------------------
typedef void (*D3D11UPDATEPROGRESSCB)( _In_ UINT phase, _In_ UINT progress, _In_z_ void *pContext );


//--------------------------------------------------------------------------------------
// Desc: Checks the system for the current status of Direct3D 11.
// 
// Params: [out] pStatus - see D3D11IH_STATUS enumeration for values
//
// Remark: If Direct3D 11 is available, you can assume that DXGI 1.1, 
//         10level9, WARP10, and the updated Direct3D 10.1 should also be present.
//
// Returns: S_OK, E_INVALIDARG, or Win32 Error HRESULT.
//          On S_OK, see pStatus.
//--------------------------------------------------------------------------------------
STDAPI CheckDirect3D11Status( _In_z_ UINT *pStatus );


//--------------------------------------------------------------------------------------
// Desc: Performs Windows Update operations to apply the Direct3D 11 update
//       if available.
//
// Params:  [in] dwFlags - D3D11IH_QUIET, D3D11IH_WINDOWS_UPDATE, and/or 0
//          [in optional] pfnProgress - Callback function for progress updates
//          [in optional] pContext - User context pointer for callback function
//          [out] pResult - see D3D11IH_RESULT enumeration for values
//
// Returns: S_OK, E_INVALIDARG, E_FAIL, Win32 Error HRESULT, or Windows Update Agent API HRESULT
//          On S_OK, see pResult.
//--------------------------------------------------------------------------------------
STDAPI DoUpdateForDirect3D11( _In_ DWORD dwFlags, _In_z_ D3D11UPDATEPROGRESSCB pfnProgress,
                              _In_z_ void *pContext, _In_z_ UINT *pResult );