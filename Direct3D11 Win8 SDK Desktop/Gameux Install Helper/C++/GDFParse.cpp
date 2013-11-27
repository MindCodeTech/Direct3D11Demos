//--------------------------------------------------------------------------------------
// File: GDFParse.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _WIN32_DCOM

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
#endif

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
#include <rpcsal.h>
#include <gameux.h>
#include <crtdbg.h>
#include <stdio.h>
#include <assert.h>
#include <shellapi.h>
#include <shlobj.h>

#include "GDFParse.h"


#ifdef extern_cplus
extern "C" {
#endif

#ifdef extern_cplusplus
	extern "C++" {
#endif

//--------------------------------------------------------------------------------------
CGDFParse::CGDFParse( void )
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    m_bCleanupCOM = SUCCEEDED( hr );
    m_pRootNode = nullptr;
}


//--------------------------------------------------------------------------------------
CGDFParse::~CGDFParse( void )
{
    SAFE_RELEASE( m_pRootNode );
    if( m_bCleanupCOM )
        CoUninitialize();
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ExtractXML( WCHAR* strGDFBinPath )
{
    SAFE_RELEASE( m_pRootNode );

    // Extract the GDF XML from the GDF binary 
    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {
        // Find resource will pick the right ID_GDF_XML_STR based on the current language
        HRSRC hrsrc = FindResource( hGDFDll, ID_GDF_XML_STR, L"DATA" );
        if( hrsrc )
        {
            HGLOBAL hgResource = LoadResource( hGDFDll, hrsrc );
            if( hgResource )
            {
                BYTE* pResourceBuffer = ( BYTE* )LockResource( hgResource );
                if( pResourceBuffer )
                {
                    DWORD dwGDFXMLSize = SizeofResource( hGDFDll, hrsrc );
                    if( dwGDFXMLSize )
                    {
                        // HGLOBAL from LoadResource() needs to be copied for CreateStreamOnHGlobal() to work
                        HGLOBAL hgResourceCopy = GlobalAlloc( GMEM_MOVEABLE, dwGDFXMLSize );
                        if( hgResourceCopy )
                        {
                            LPVOID pCopy = GlobalLock( hgResourceCopy );
                            if( pCopy )
                            {
                                CopyMemory( pCopy, pResourceBuffer, dwGDFXMLSize );
                                GlobalUnlock( hgResource );

                                IStream* piStream = nullptr;
                                CreateStreamOnHGlobal( hgResourceCopy, TRUE, &piStream );
                                if( piStream )
                                {
                                    IXMLDOMDocument* pDoc = nullptr;
                                    HRESULT hr;

                                    // Load the XML into a IXMLDOMDocument object
                                    hr = CoCreateInstance( CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER,
                                                           IID_IXMLDOMDocument, ( void** )&pDoc );
                                    if( SUCCEEDED( hr ) )
                                    {
                                        IPersistStreamInit* pPersistStreamInit = nullptr;
                                        hr = pDoc->QueryInterface( IID_IPersistStreamInit,
                                                                   ( void** )&pPersistStreamInit );
                                        if( SUCCEEDED( hr ) )
                                        {
                                            hr = pPersistStreamInit->Load( piStream );
                                            if( SUCCEEDED( hr ) )
                                            {
                                                // Get the root node to the XML doc and store it 
                                                pDoc->QueryInterface( IID_IXMLDOMNode, ( void** )&m_pRootNode );
                                            }
                                            SAFE_RELEASE( pPersistStreamInit );
                                        }
                                        SAFE_RELEASE( pDoc );
                                    }
                                    SAFE_RELEASE( piStream );
                                }
                            }
                            GlobalFree( hgResourceCopy );
                        }
                    }
                }
            }
        }
    }

    if( m_pRootNode )
        return S_OK;
    else
        return E_FAIL;
}


//--------------------------------------------------------------------------------------
// Various get methods
//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetName( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Name", strDest, cchDest );
}
HRESULT CGDFParse::GetDescription( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Description", strDest, cchDest );
}
HRESULT CGDFParse::GetReleaseDate( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/ReleaseDate", strDest, cchDest );
}
HRESULT CGDFParse::GetGenre( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Genres/Genre", strDest, cchDest );
}
HRESULT CGDFParse::GetDeveloper( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Developers/Developer", strDest, cchDest );
}
HRESULT CGDFParse::GetPublisher( WCHAR* strDest, int cchDest )
{
    return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Publishers/Publisher", strDest, cchDest );
}
HRESULT CGDFParse::GetGameID( WCHAR* strDest, int cchDest )
{
    return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition", L"gameID", strDest, cchDest );
}
HRESULT CGDFParse::GetWinSPR( int* pnMin, int* pnRecommended )
{
    WCHAR strDest[256];

    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/WindowsSystemPerformanceRating", L"minimum", strDest, 256 );
    *pnMin = _wtoi( strDest );

    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/WindowsSystemPerformanceRating", L"recommended", strDest,
                  256 );
    *pnRecommended = _wtoi( strDest );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ExtractGDFThumbnail( WCHAR* strGDFBinPath, WCHAR* strDestFilePath )
{
    HGLOBAL hgResource = nullptr;
    HRSRC hrsrc = nullptr;
    BYTE* pGDFThumbnailBuffer = nullptr;
    DWORD dwGDFThumbnailSize = 0;

    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {
        // Extract GDF thumbnail 
        hrsrc = FindResource( hGDFDll, ID_GDF_THUMBNAIL_STR, L"DATA" );
        if( hrsrc )
        {
            hgResource = LoadResource( hGDFDll, hrsrc );
            if( hgResource )
            {
                BYTE* pResourceBuffer = ( BYTE* )LockResource( hgResource );
                if( pResourceBuffer )
                {
                    dwGDFThumbnailSize = SizeofResource( hGDFDll, hrsrc );
                    if( dwGDFThumbnailSize )
                    {
                        pGDFThumbnailBuffer = new BYTE[dwGDFThumbnailSize];
                        memcpy( pGDFThumbnailBuffer, pResourceBuffer, dwGDFThumbnailSize );

                        HANDLE hFileThumbnail = CreateFile( strDestFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                                            FILE_ATTRIBUTE_NORMAL, nullptr );
                        if( hFileThumbnail != INVALID_HANDLE_VALUE )
                        {
                            DWORD dwWritten;
                            WriteFile( hFileThumbnail, pGDFThumbnailBuffer, dwGDFThumbnailSize, &dwWritten, nullptr );
                            CloseHandle( hFileThumbnail );
                        }

                        SAFE_DELETE_ARRAY( pGDFThumbnailBuffer );
                    }
                }
            }
        }
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetXMLValue( WCHAR* strXPath, WCHAR* strValue, int cchValue )
{
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL;

    IXMLDOMNode* pNode = nullptr;
    m_pRootNode->selectSingleNode( strXPath, &pNode );
    if( pNode == nullptr )
        return E_FAIL;

    VARIANT v;
    IXMLDOMNode* pChild = nullptr;
    pNode->get_firstChild( &pChild );
    if( pChild )
    {
        HRESULT hr = pChild->get_nodeTypedValue( &v );
        if( SUCCEEDED( hr ) && v.vt == VT_BSTR )
            wcscpy_s( strValue, cchValue, v.bstrVal );
        VariantClear( &v );
        SAFE_RELEASE( pChild );
    }
    SAFE_RELEASE( pNode );

    return S_OK;
}

//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetXMLAttrib( WCHAR* strXPath, WCHAR* strAttribName, WCHAR* strValue, int cchValue )
{
    bool bFound = false;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL;

    IXMLDOMNode* pNode = nullptr;
    m_pRootNode->selectSingleNode( strXPath, &pNode );
    if( pNode == nullptr )
        return E_FAIL;

    IXMLDOMNamedNodeMap* pIXMLDOMNamedNodeMap = nullptr;
    BSTR bstrAttributeName = ::SysAllocString( strAttribName );
    IXMLDOMNode* pIXMLDOMNode = nullptr;

    HRESULT hr;
    VARIANT v;
    hr = pNode->get_attributes( &pIXMLDOMNamedNodeMap );
    if( SUCCEEDED( hr ) && pIXMLDOMNamedNodeMap )
    {
        hr = pIXMLDOMNamedNodeMap->getNamedItem( bstrAttributeName, &pIXMLDOMNode );
        if( SUCCEEDED( hr ) && pIXMLDOMNode )
        {
            pIXMLDOMNode->get_nodeValue( &v );
            if( SUCCEEDED( hr ) && v.vt == VT_BSTR )
            {
                wcscpy_s( strValue, cchValue, v.bstrVal );
                bFound = true;
            }
            VariantClear( &v );
            SAFE_RELEASE( pIXMLDOMNode );
        }
        SAFE_RELEASE( pIXMLDOMNamedNodeMap );
    }

    ::SysFreeString( bstrAttributeName );
    bstrAttributeName = nullptr;

    SAFE_RELEASE( pNode );

    if( !bFound )
        return E_FAIL;
    else
        return S_OK;
}

HRESULT CGDFParse::GetXMLRootNode(IXMLDOMNode** ppRootNode)
{
    assert(ppRootNode);
    *ppRootNode = m_pRootNode;
    return S_OK;
}


#if defined(extern_cplus) && defined(extern_cplusplus)
	}
	}
#elif defined(extern_cplus) && !defined(extern_cplusplus)
}
#elif defined(extern_cplusplus) && !defined(extern_cplus)
}
#endif

