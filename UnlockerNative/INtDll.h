// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
//////////////////////////////////////////////////////////////////////////////////////
//
// SystemInfo.h, v1.1

#ifndef SYSTEMINFO_H_INCLUDED
#define SYSTEMINFO_H_INCLUDED

#ifndef WINNT
#error You need Windows NT to use this source code. Define WINNT!
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)
#pragma warning(disable : 4200)

//////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
//////////////////////////////////////////////////////////////////////////////////////

typedef struct _UNICODE_STRING
{
	WORD  Length;
	WORD  MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING;


//////////////////////////////////////////////////////////////////////////////////////
//
// INtDll
//
//////////////////////////////////////////////////////////////////////////////////////

class INtDll
{
public:
	typedef DWORD(WINAPI* PNtQueryObject)(HANDLE, DWORD, VOID*, DWORD, VOID*);
	typedef DWORD(WINAPI* PNtQuerySystemInformation)(DWORD, VOID*, DWORD, ULONG*);
	typedef DWORD(WINAPI* PNtQueryInformationThread)(HANDLE, ULONG, PVOID, DWORD, DWORD*);
	typedef DWORD(WINAPI* PNtQueryInformationFile)(HANDLE, PVOID, PVOID, DWORD, DWORD);
	typedef DWORD(WINAPI* PNtQueryInformationProcess)(HANDLE, DWORD, PVOID, DWORD, PVOID);

public:
	static PNtQuerySystemInformation NtQuerySystemInformation;
	static PNtQueryObject    NtQueryObject;
	static PNtQueryInformationThread NtQueryInformationThread;
	static PNtQueryInformationFile  NtQueryInformationFile;
	static PNtQueryInformationProcess NtQueryInformationProcess;

	static BOOL       NtDllStatus;

	static DWORD      dwNTMajorVersion;

protected:
	static BOOL Init();
};

#endif