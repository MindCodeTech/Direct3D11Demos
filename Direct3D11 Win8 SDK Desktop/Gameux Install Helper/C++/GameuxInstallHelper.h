//--------------------------------------------------------------------------------------
// File: GameuxInstallHelper.h
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
#include <gameux.h>
#include <msi.h>
#include <msiquery.h>

#include "GameuxInstallHelper_exp.h"


#ifdef extern_cplus
extern "C" {
#endif

#ifdef extern_cplusplus
	extern "C++" {
#endif

//--------------------------------------------------------------------------------------
// UNICODE/ANSI define mappings
//--------------------------------------------------------------------------------------
#ifdef UNICODE
    #define GameExplorerInstall GameExplorerInstallW
    #define GameExplorerUninstall GameExplorerUninstallW
    #define GameExplorerUpdate GameExplorerUpdateW
#else
    #define GameExplorerInstall GameExplorerInstallA
    #define GameExplorerUninstall GameExplorerUninstallA
    #define GameExplorerUpdate GameExplorerUpdateA
#endif

//--------------------------------------------------------------------------------------
// Given a game path to GDF binary, registers the game with Game Explorer
//
// [in] strGDFBinPath: the full path to the GDF binary 
// [in] strGameInstallPath: the full path to the folder where the game is installed.  
//                          This folder will be under the protection of parental controls after this call
// [in] InstallScope: if the game is being installed for all users or just the current user 
//--------------------------------------------------------------------------------------
STDAPI GameExplorerInstallW(_In_z_ WCHAR* strGDFBinPath, _In_z_ WCHAR* strGameInstallPath, _In_ GAME_INSTALL_SCOPE InstallScope);
STDAPI GameExplorerInstallA(_In_z_ CHAR* strGDFBinPath, _In_z_ CHAR* strGameInstallPath, _In_ GAME_INSTALL_SCOPE InstallScope);

//--------------------------------------------------------------------------------------
// Given a game path to GDF binary, unregisters the game with Game Explorer
//
// [in] strGDFBinPath: the full path to the GDF binary 
//--------------------------------------------------------------------------------------
STDAPI GameExplorerUninstallW(_In_z_ WCHAR* strGDFBinPath);
STDAPI GameExplorerUninstallA(_In_z_ CHAR* strGDFBinPath);

//--------------------------------------------------------------------------------------
// Given a game path to GDF binary, updates a registered game with Game Explorer
//
// [in] strGDFBinPath: the full path to the GDF binary 
//--------------------------------------------------------------------------------------
STDAPI GameExplorerUpdateW(_In_z_ WCHAR* strGDFBinPath);
STDAPI GameExplorerUpdateA(_In_z_ CHAR* strGDFBinPath);

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This sets up the CustomActionData properties for the deferred custom actions. 
//--------------------------------------------------------------------------------------
UINT WINAPI GameExplorerSetMSIProperties(_In_ MSIHANDLE hModule);

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This adds the game to the Game Explorer
//--------------------------------------------------------------------------------------
UINT WINAPI GameExplorerInstallUsingMSI(_In_ MSIHANDLE hModule);

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This removes the game to the Game Explorer
//--------------------------------------------------------------------------------------
UINT WINAPI GameExplorerUninstallUsingMSI(_In_ MSIHANDLE hModule);


#if defined(extern_cplus) && defined(extern_cplusplus)
	}
}
#elif defined(extern_cplus) && !defined(extern_cplusplus)
	}
#elif defined(extern_cplusplus) && !defined(extern_cplus)
	}
#endif

