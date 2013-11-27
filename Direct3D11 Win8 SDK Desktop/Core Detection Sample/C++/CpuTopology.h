//-------------------------------------------------------------------------------------
// CpuTopology.h
// 
// CpuToplogy class declaration.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------
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

class ICpuTopology;

//---------------------------------------------------------------------------------
// Name: CpuToplogy
// Desc: This class constructs a supported cpu topology implementation object on
//       initialization and forwards calls to it.  This is the Abstraction class
//       in the traditional Bridge Pattern.
//---------------------------------------------------------------------------------
class CpuTopology
{
public:
                CpuTopology( bool bForceCpuid = false );
                ~CpuTopology();

    bool        IsDefaultImpl() const;
    DWORD       NumberOfProcessCores() const;
    DWORD       NumberOfSystemCores() const;
    DWORD_PTR   CoreAffinityMask( DWORD coreIdx ) const;

    void        ForceCpuid( bool bForce );
private:
    void        Destroy_();

    ICpuTopology* m_pImpl;
};
