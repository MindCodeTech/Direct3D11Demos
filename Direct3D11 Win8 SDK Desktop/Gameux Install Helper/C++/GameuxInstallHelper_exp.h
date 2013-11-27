
#pragma once


#if defined(LIB_EXPORT) || defined(GAMEUXINSTALLHELPERLIB_EXPORT)
#ifndef GAMEUXINSTALLHELPERLIB_IMPORT
#ifndef GAMEUXINSTALLHELPERLIB_EXPORT
#define GAMEUXINSTALLHELPERLIB_EXPORT 1
#endif
#endif
#endif

#if defined(LIB_IMPORT) || defined(GAMEUXINSTALLHELPERLIB_IMPORT)
#if defined(GAMEUXINSTALLHELPERLIB_EXPORT)
#error ("!!!You are bulding GAMEUXINSTALLHELPER export and import simultaniously")
#else
#ifndef GAMEUXINSTALLHELPERLIB_IMPORT
#define GAMEUXINSTALLHELPERLIB_IMPORT 1
#endif
#endif
#endif

#if defined(LIB_STATIC) || defined(GAMEUXINSTALLHELPERLIB_STATIC)
#ifndef GAMEUXINSTALLHELPERLIB_DLL
#ifndef GAMEUXINSTALLHELPERLIB_STATIC 
#define GAMEUXINSTALLHELPERLIB_STATIC 1
#endif
#endif
#endif

#if (defined(_DLL) || defined(_USRDLL) || defined(_WINDLL) || defined(LIB_DYNAMIC) || defined(GAMEUXINSTALLHELPERLIB_DLL)) && !defined(_LIB) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#ifndef GAMEUXINSTALLHELPERLIB_STATIC
#ifndef GAMEUXINSTALLHELPERLIB_DLL
#define GAMEUXINSTALLHELPERLIB_DLL 1
#endif
#endif
#endif

#if !defined(GAMEUXINSTALLHELPERLIB_DLL) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#error ("!!!Your D3D11INSTALLHELPER lib type static or dll aren't defined")
#endif

#ifdef GAMEUXINSTALLHELPERLIB_IMPORT
#ifdef GAMEUXINSTALLHELPERLIB_DLL
#ifdef _DEBUG
#pragma comment( lib, "GameuxInstallHelper_d.lib" )
#else
#pragma comment( lib, "GameuxInstallHelper.lib" )
#endif
#elif GAMEUXINSTALLHELPERLIB_STATIC
#ifdef _DEBUG
#pragma comment( lib, "GameuxInstallHelpers_d.lib" )
#else
#pragma comment( lib, "GameuxInstallHelpers.lib" )
#endif
#else
#pragma warning ("GAMEUXINSTALLHELPERLIB_IMPORT import librarys aren't defined")
#endif
#endif


#if defined(_WINDOWS) || defined(_WIN32)
/* If building or using GAMEUXINSTALLHELPERlib as a DLL, define GAMEUXINSTALLHELPERLIB_DLL.
* This is not mandatory, but it offers a little performance increase.
*/

#if defined(LIB_EXPORT) || defined(GAMEUXINSTALLHELPERLIB_EXPORT)
#ifndef GAMEUXINSTALLHELPERLIB_IMPORT
#ifndef GAMEUXINSTALLHELPERLIB_EXPORT
#define GAMEUXINSTALLHELPERLIB_EXPORT 1
#endif
#endif
#endif

#if defined(LIB_IMPORT) || defined(GAMEUXINSTALLHELPERLIB_IMPORT)
#if defined(GAMEUXINSTALLHELPERLIB_EXPORT)
#error ("!!!You are bulding GAMEUXINSTALLHELPER export and import simultaniously")
#else
#ifndef GAMEUXINSTALLHELPERLIB_IMPORT
#define GAMEUXINSTALLHELPERLIB_IMPORT 1
#endif
#endif
#endif

#if defined(LIB_STATIC) || defined(GAMEUXINSTALLHELPERLIB_STATIC)
#ifndef GAMEUXINSTALLHELPERLIB_DLL
#ifndef GAMEUXINSTALLHELPERLIB_STATIC 
#define GAMEUXINSTALLHELPERLIB_STATIC 1
#endif
#endif
#endif

#if (defined(_DLL) || defined(_USRDLL) || defined(_WINDLL) || defined(LIB_DYNAMIC) || defined(GAMEUXINSTALLHELPERLIB_DLL)) && !defined(_LIB) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#ifndef GAMEUXINSTALLHELPERLIB_STATIC
#ifndef GAMEUXINSTALLHELPERLIB_DLL
#define GAMEUXINSTALLHELPERLIB_DLL 1
#endif
#endif
#endif

#if !defined(GAMEUXINSTALLHELPERLIB_DLL) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#error ("!!!Your GAMEUXINSTALLHELPER lib type static or dll aren't defined")
#endif

#define extern_cplus

// Defined for Templates functions export
//#define extern_cplusplus

#ifdef extern_cplus
#define _EXTERN_C_START extern "C" {
#define _EXTERN_C_END  }
#endif

#ifdef extern_cplusplus
#define _EXTERN_CPP_START extern "C++" {
#define _EXTERN_CPP_END  }
#endif

#define _EXTERN extern

#  ifdef GAMEUXINSTALLHELPERLIB_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      if defined(GAMEUXINSTALLHELPERLIB_EXPORT) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#        define GAMEUXIHAPI __declspec(dllexport)
#      elif defined (GAMEUXINSTALLHELPERLIB_IMPORT) && (defined(_DLL) || defined(_WINDLL) || defined(_USRDLL)) && !defined(GAMEUXINSTALLHELPERLIB_STATIC)
#        define GAMEUXIHAPI __declspec(dllimport)
#      else // GAMEUXINSTALLHELPERLIB_STATIC  _LIB
#        define GAMEUXIHAPI
#      endif
#    endif
#  else  // GAMEUXINSTALLHELPERLIB_STATIC
#      define GAMEUXIHAPI
#  endif  /* ! GAMEUXINSTALLHELPERLIB_DLL */

#else
#	define GAMEUXIHAPI
#endif // _WINDOWS
