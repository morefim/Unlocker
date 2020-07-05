// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
//////////////////////////////////////////////////////////////////////////////////////
//
// SystemInfo.cpp v1.1
//
// History:
// 
// Date      Version     Description
// --------------------------------------------------------------------------------
// 10/16/00	 1.0	     Initial version
// 11/09/00  1.1         NT4 doesn't like if we bother the System process fix :)
//                       SystemInfoUtils::GetDeviceFileName() fix (subst drives added)
//                       NT Major version added to INtDLL class
//
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "INtDll.h"
#include "SystemInfoUtils.h"

#ifndef WINNT
#error You need Windows NT to use this source code. Define WINNT!
#endif

//////////////////////////////////////////////////////////////////////////////////////
//
// INtDll
//
//////////////////////////////////////////////////////////////////////////////////////

INtDll::PNtQuerySystemInformation INtDll::NtQuerySystemInformation = NULL;
INtDll::PNtQueryObject INtDll::NtQueryObject = NULL;
INtDll::PNtQueryInformationThread	INtDll::NtQueryInformationThread = NULL;
INtDll::PNtQueryInformationFile	INtDll::NtQueryInformationFile = NULL;
INtDll::PNtQueryInformationProcess INtDll::NtQueryInformationProcess = NULL;
DWORD INtDll::dwNTMajorVersion = SystemInfoUtils::GetNTMajorVersion();

BOOL INtDll::NtDllStatus = INtDll::Init();

BOOL INtDll::Init()
{
	// Get the NtDll function pointers
	NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), _T("NtQuerySystemInformation"));
	NtQueryObject = (PNtQueryObject)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), _T("NtQueryObject"));
	NtQueryInformationThread = (PNtQueryInformationThread)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), _T("NtQueryInformationThread"));
	NtQueryInformationFile = (PNtQueryInformationFile)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), _T("NtQueryInformationFile"));
	NtQueryInformationProcess = (PNtQueryInformationProcess)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), _T("NtQueryInformationProcess"));

	return  NtQuerySystemInformation != NULL && NtQueryObject != NULL && NtQueryInformationThread != NULL && NtQueryInformationFile != NULL && NtQueryInformationProcess != NULL;
}
