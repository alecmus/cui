//
// CCriticalSection.h - C++ wrapper for win32 critical section object
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>

/*
** C++ wrapper for win32 critical section object
** avoid calling this class directly
** use CCriticalSectionLocker instead unless you're sure
** you'll handle this class without making errors
*/
class CCriticalSection
{
public:
	CCriticalSection() { InitializeCriticalSection(&m_cs); }
	~CCriticalSection() { DeleteCriticalSection(&m_cs); }

	void Enter() { EnterCriticalSection(&m_cs); }
	void Leave() { LeaveCriticalSection(&m_cs); }

private:
	CRITICAL_SECTION m_cs;
}; // CCriticalSection

   /*
   ** CCriticalSectionLocker - Locks a critical section as RIIA
   ** Use this instead of manually calling
   ** CriticalSection::Enter() and CriticalSection::Leave().
   */
class CCriticalSectionLocker
{
public:
	CCriticalSectionLocker(CCriticalSection& cs) : m_cs(cs) { cs.Enter(); }
	~CCriticalSectionLocker() { m_cs.Leave(); }

private:
	CCriticalSection& m_cs;
}; // CCriticalSectionLocker
