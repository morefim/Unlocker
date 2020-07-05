// ForceDel.cpp : Defines the entry point for the console application.
//
// Modified by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
///////////////////////////////////////////////////////////////////////////////
//
// Thanks Felix, you have a great site. www.mvps.org/win32
//
//
// History:
// 
// 11/6/00		Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ForceDel.h"
#include "INtDll.h"
#include "SystemHandleInformation.h"
#include "SystemProcessInformation.h"
#include "SystemInfoUtils.h"


CForceDel::CForceDel()
{

}

CForceDel::~CForceDel()
{

}

int CForceDel::Run(LPCTSTR lpPath, BOOL bUsage, BOOL bSoft)
{
	int rc = 0;	

	if (bUsage)
	{
		ShowUsage();		
		return -1;
	}

	//Enable debug privilege
	EnableDebugPriv();

	//Close every handle in the system for this file
	if (!bSoft)
		CloseRemoteFileHandles(lpPath);
	
	//Try to delete it
	rc = DeleteTheFile(lpPath);
		
	return rc ? 0 : 1;
}

//Show the help
void CForceDel::ShowUsage()
{
	//Usage information
	SystemInfoUtils::print_log(_T("ForceDelete 1.0 for WinNT/Win2k"));
	SystemInfoUtils::print_log(_T(""));
	SystemInfoUtils::print_log(_T("Deletes a given file even if it is locked by another process."));
	SystemInfoUtils::print_log(_T("Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com"));
	SystemInfoUtils::print_log(_T(""));
	SystemInfoUtils::print_log(_T("Usage: FORCEDEL.EXE [/S] filename"));
	SystemInfoUtils::print_log(_T(""));
	SystemInfoUtils::print_log(_T("          /S             Soft delete. Like the \"del\" command"));
	SystemInfoUtils::print_log(_T("          filename       File name you want to delete"));
	SystemInfoUtils::print_log(_T(""));
	SystemInfoUtils::print_log(_T("Examples:"));
	SystemInfoUtils::print_log(_T(""));
	SystemInfoUtils::print_log(_T("       FORCEDEL.EXE C:\\TEST.TXT"));
}

//Enable DEBUG privilege
void CForceDel::EnableDebugPriv(void)
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	// enable the SeDebugPrivilege
	if (! OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		SystemInfoUtils::print_log(_T("OpenProcessToken() failed, Error = %d SeDebugPrivilege is not available."), GetLastError());
		return;
	}

	if (! LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		SystemInfoUtils::print_log(_T("LookupPrivilegeValue() failed, Error = %d SeDebugPrivilege is not available."), GetLastError());
		CloseHandle(hToken);
		return;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (! AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof tkp, NULL, NULL))
		SystemInfoUtils::print_log(_T("AdjustTokenPrivileges() failed, Error = %d SeDebugPrivilege is not available."), GetLastError());
		
	CloseHandle(hToken);
}

//Close a handle in a remote process
DWORD CForceDel::CloseRemoteHandle(LPCTSTR lpProcessName, DWORD processID, HANDLE handle)
{
	HANDLE ht = 0;
	DWORD rc = 0;
	
	SystemInfoUtils::print_log(_T("Closing handle in process #%d (%s) ... "), processID, lpProcessName);

	// open the process
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
			PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processID);
	
	if (hProcess == NULL)
	{
		rc = GetLastError();
		SystemInfoUtils::print_log(_T("OpenProcess() failed"));
		return rc;
	}

	// load kernel32.dll
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));

	// CreateRemoteThread()
	ht = CreateRemoteThread(
			hProcess, 
			0, 
			0, 
			(DWORD (__stdcall *)(void *))GetProcAddress(hKernel32, "CloseHandle"),
			handle, 
			0, 
			&rc);
	
	if (ht == NULL)
	{
		//Something is wrong with the privileges, or the process doesn't like us
		rc = GetLastError();
		SystemInfoUtils::print_log(_T("CreateRemoteThread() failed"));
		goto cleanup;
	}

	switch (WaitForSingleObject(ht, 2000))
	{
	case WAIT_OBJECT_0:
		//Well done
		rc = 0;
		SystemInfoUtils::print_log(_T("Ok 0x%d"), rc);
		
		break;
	
	default:
		//Oooops, shouldn't be here
		rc = GetLastError();
		SystemInfoUtils::print_log(_T("WaitForSingleObject() failed"));
		goto cleanup;

		break;
	}

cleanup:

	//Closes the remote thread handle
	CloseHandle(ht);

	//Free up the kernel32.dll
	if (hKernel32 != NULL)
		FreeLibrary(hKernel32);

	//Close the process handle
	CloseHandle(hProcess);
		
	return rc;
}

//Closes the file handles in the processes which are using this file
void CForceDel::CloseRemoteFileHandles(LPCTSTR lpFileName)
{
	CString deviceFileName;
	CString fsFilePath;
	CString name;
	CString processName;
	SystemHandleInformation hi;
	SystemProcessInformation pi;
	SystemProcessInformation::SYSTEM_PROCESS_INFORMATION* pPi;
	
	//Convert it to device file name
	if (!SystemInfoUtils::GetDeviceFileName(lpFileName, deviceFileName))
	{
		SystemInfoUtils::print_log(_T("GetDeviceFileName() failed."));
		return;
	}
	
	//Query every file handle (system wide)
	if (!hi.SetFilter(_T("File"), TRUE))
	{
		SystemInfoUtils::print_log(_T("SystemHandleInformation::SetFilter() failed."));
		return;
	}

	//if (!pi.Refresh())
	//{
	//	SystemInfoUtils::print_log(_T("SystemProcessInformation::Refresh() failed."));
	//	return;
	//}

	//Iterate through the found file handles
	for (std::list<SystemHandleInformation::SYSTEM_HANDLE>::iterator pos = hi.m_HandleInfos.begin(); pos != hi.m_HandleInfos.end(); pos++)
	{
		SystemHandleInformation::SYSTEM_HANDLE& h = *pos;

		std::map<DWORD, SystemProcessInformation::SYSTEM_PROCESS_INFORMATION*>::iterator foundPI = pi.m_ProcessInfos.find(h.ProcessID);
		if (foundPI == pi.m_ProcessInfos.end())
			continue;

		pPi = foundPI->second;
		if (pPi == NULL)
			continue;

		//Get the process name
		SystemInfoUtils::Unicode2CString(&pPi->usName, processName);

		//NT4 Stupid thing if I query the name of a file in services.exe
		//Messengr service brings up a message dialog ??? :(
		if (INtDll::dwNTMajorVersion == 4 && _tcsicmp(processName, _T("services.exe")) == 0)
			continue;
		
		//what's the file name for this given handle?
		hi.GetName((HANDLE)h.HandleNumber, name, h.ProcessID);

		//This is what we want to delete, so close the handle
		if (_tcsicmp(name, deviceFileName) == 0)
			CloseRemoteHandle(processName, h.ProcessID, (HANDLE)h.HandleNumber);
	}
}

//Deletes the file
BOOL CForceDel::DeleteTheFile(LPCTSTR lpFileName)
{
	//Deletes the file
	BOOL rc = ::DeleteFile(lpFileName);
	
	if (rc)
		SystemInfoUtils::print_log(_T("Successfully deleted."));
	else
		SystemInfoUtils::print_log(_T("Couldn't delete. Error = %d"), GetLastError());

	return rc;
}
