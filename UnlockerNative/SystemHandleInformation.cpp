#include "StdAfx.h"
#include "INtDll.h"
#include "SystemHandleInformation.h"
#include "SystemInfoUtils.h"
#include "SystemThreadInformation.h"
#include "SystemProcessInformation.h"


//////////////////////////////////////////////////////////////////////////////////////
//
// SystemHandleInformation
//
//////////////////////////////////////////////////////////////////////////////////////

SystemHandleInformation::SystemHandleInformation(DWORD pID, BOOL bRefresh, LPCTSTR lpTypeFilter)
{
	m_processId = pID;

	// Set the filter
	SetFilter(lpTypeFilter, bRefresh);
}

SystemHandleInformation::~SystemHandleInformation()
{
}

BOOL SystemHandleInformation::SetFilter(LPCTSTR lpTypeFilter, BOOL bRefresh)
{
	// Set the filter ( default = all objects )
	m_strTypeFilter = lpTypeFilter == NULL ? _T("") : lpTypeFilter;

	return bRefresh ? Refresh() : TRUE;
}

const CString& SystemHandleInformation::GetFilter()
{
	return m_strTypeFilter;
}

BOOL SystemHandleInformation::IsSupportedHandle(SYSTEM_HANDLE& handle)
{
	//Here you can filter the handles you don't want in the Handle list

	// Windows 2000 supports everything :)
	if (dwNTMajorVersion >= 5)
		return TRUE;

	//NT4 System process doesn't like if we bother his internal security :)
	if (handle.ProcessID == 2 && handle.HandleType == 16)
		return FALSE;

	return TRUE;
}

BOOL SystemHandleInformation::Refresh()
{
	DWORD size = 0x2000;
	DWORD needed = 0;
	DWORD i = 0;
	BOOL  ret = TRUE;
	CString strType;

	m_HandleInfos.clear();

	if (!INtDll::NtDllStatus)
		return FALSE;

	// Allocate the memory for the buffer
	SYSTEM_HANDLE_INFORMATION* pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
		VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

	if (pSysHandleInformation == NULL)
		return FALSE;

	// Query the needed buffer size for the objects ( system wide )
	if (INtDll::NtQuerySystemInformation(16, pSysHandleInformation, size, &needed) != 0)
	{
		if (needed == 0)
		{
			ret = FALSE;
			goto cleanup;
		}

		// The size was not enough
		VirtualFree(pSysHandleInformation, 0, MEM_RELEASE);

		pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
			VirtualAlloc(NULL, size = needed + 256, MEM_COMMIT, PAGE_READWRITE);
	}

	if (pSysHandleInformation == NULL)
		return FALSE;

	// Query the objects ( system wide )
	if (INtDll::NtQuerySystemInformation(16, pSysHandleInformation, size, NULL) != 0)
	{
		ret = FALSE;
		goto cleanup;
	}

	// Iterating through the objects
	for (i = 0; i < pSysHandleInformation->Count; i++)
	{
		if (!IsSupportedHandle(pSysHandleInformation->Handles[i]))
			continue;

		// ProcessId filtering check
		if (pSysHandleInformation->Handles[i].ProcessID == m_processId || m_processId == (DWORD)-1)
		{
			BOOL bAdd = FALSE;

			if (m_strTypeFilter == _T(""))
				bAdd = TRUE;
			else
			{
				if (pSysHandleInformation->Handles[i].HandleNumber == 0 && pSysHandleInformation->Handles[i].ProcessID == 0) continue;

				// Type filtering
				BOOL tokenGot = GetTypeToken((HANDLE)pSysHandleInformation->Handles[i].HandleNumber, strType, pSysHandleInformation->Handles[i].ProcessID);
				bAdd = strType == m_strTypeFilter;
				SystemInfoUtils::print_log("Type: '%s', ProcessID: 0x%X, Handle: 0x%X", 
					strType, pSysHandleInformation->Handles[i].ProcessID, pSysHandleInformation->Handles[i].HandleNumber);
			}

			// That's it. We found one.
			if (bAdd)
			{
				pSysHandleInformation->Handles[i].HandleType = (WORD)(pSysHandleInformation->Handles[i].HandleType % 256);

				m_HandleInfos.push_back(pSysHandleInformation->Handles[i]);
			}
		}
	}

cleanup:

	if (pSysHandleInformation != NULL)
		VirtualFree(pSysHandleInformation, 0, MEM_RELEASE);

	return ret;
}

HANDLE SystemHandleInformation::OpenProcess(DWORD processId)
{
	// Open the process for handle duplication
	return ::OpenProcess(PROCESS_DUP_HANDLE, TRUE, processId);
}

HANDLE SystemHandleInformation::DuplicateHandle(HANDLE hProcess, HANDLE hRemote)
{
	HANDLE hDup = NULL;

	// Duplicate the remote handle for our process
	::DuplicateHandle(hProcess, hRemote, GetCurrentProcess(), &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS);

	return hDup;
}

//Information functions
BOOL SystemHandleInformation::GetTypeToken(HANDLE h, CString& str, DWORD processId)
{
	ULONG size = 0x2000;
	UCHAR* lpBuffer = NULL;
	BOOL ret = FALSE;

	HANDLE handle;
	HANDLE hRemoteProcess = NULL;
	BOOL remote = processId != GetCurrentProcessId();

	if (!NtDllStatus)
		return FALSE;

	if (remote)
	{
		// Open the remote process
		hRemoteProcess = OpenProcess(processId);

		if (hRemoteProcess == NULL)
			return FALSE;

		// Duplicate the handle
		handle = DuplicateHandle(hRemoteProcess, h);
	}
	else
		handle = h;

	// Query the info size
	INtDll::NtQueryObject(handle, 2, NULL, 0, &size);

	lpBuffer = new UCHAR[size];

	// Query the info size ( type )
	if (INtDll::NtQueryObject(handle, 2, lpBuffer, size, NULL) == 0)
	{
		str = _T("");
		SystemInfoUtils::LPCWSTR2CString((LPCWSTR)(lpBuffer + 0x60), str);

		ret = TRUE;
	}

	if (remote)
	{
		if (hRemoteProcess != NULL)
			CloseHandle(hRemoteProcess);

		if (handle != NULL)
			CloseHandle(handle);
	}

	if (lpBuffer != NULL)
		delete[] lpBuffer;

	return ret;
}

BOOL SystemHandleInformation::GetType(HANDLE h, WORD& type, DWORD processId)
{
	CString strType;

	type = OB_TYPE_UNKNOWN;

	if (!GetTypeToken(h, strType, processId))
		return FALSE;

	return GetTypeFromTypeToken(strType, type);
}

BOOL SystemHandleInformation::GetTypeFromTypeToken(LPCTSTR typeToken, WORD& type)
{
	const WORD count = 27;
	CString constStrTypes[count] = {
		_T(""), _T(""), _T("Directory"), _T("SymbolicLink"), _T("Token"),
		_T("Process"), _T("Thread"), _T("Unknown7"), _T("Event"), _T("EventPair"), _T("Mutant"),
		_T("Unknown11"), _T("Semaphore"), _T("Timer"), _T("Profile"), _T("WindowStation"),
		_T("Desktop"), _T("Section"), _T("Key"), _T("Port"), _T("WaitablePort"),
		_T("Unknown21"), _T("Unknown22"), _T("Unknown23"), _T("Unknown24"),
		_T("IoCompletion"), _T("File") };

	type = OB_TYPE_UNKNOWN;

	for (WORD i = 1; i < count; i++)
		if (constStrTypes[i] == typeToken)
		{
			type = i;
			return TRUE;
		}

	return FALSE;
}

BOOL SystemHandleInformation::GetName(HANDLE handle, CString& str, DWORD processId)
{
	WORD type = 0;

	if (!GetType(handle, type, processId))
		return FALSE;

	return GetNameByType(handle, type, str, processId);
}

BOOL SystemHandleInformation::GetNameByType(HANDLE h, WORD type, CString& str, DWORD processId)
{
	ULONG size = 0x2000;
	UCHAR* lpBuffer = NULL;
	BOOL ret = FALSE;

	HANDLE handle;
	HANDLE hRemoteProcess = NULL;
	BOOL remote = processId != GetCurrentProcessId();
	DWORD dwId = 0;

	if (!NtDllStatus)
		return FALSE;

	if (remote)
	{
		hRemoteProcess = OpenProcess(processId);

		if (hRemoteProcess == NULL)
			return FALSE;

		handle = DuplicateHandle(hRemoteProcess, h);
	}
	else
		handle = h;

	// let's be happy, handle is in our process space, so query the infos :)
	switch (type)
	{
	case OB_TYPE_PROCESS:
		GetProcessId(handle, dwId);

		str.Format(_T("PID: 0x%X"), dwId);

		ret = TRUE;
		goto cleanup;
		break;

	case OB_TYPE_THREAD:
		GetThreadId(handle, dwId);

		str.Format(_T("TID: 0x%X"), dwId);

		ret = TRUE;
		goto cleanup;
		break;

	case OB_TYPE_FILE:
		ret = GetFileName(handle, str);

		// access denied :(
		if (ret && str == _T(""))
			goto cleanup;
		break;

	};

	INtDll::NtQueryObject(handle, 1, NULL, 0, &size);

	// let's try to use the default
	if (size == 0)
		size = 0x2000;

	lpBuffer = new UCHAR[size];

	if (INtDll::NtQueryObject(handle, 1, lpBuffer, size, NULL) == 0)
	{
		SystemInfoUtils::Unicode2CString((UNICODE_STRING*)lpBuffer, str);
		ret = TRUE;
	}

cleanup:

	if (remote)
	{
		if (hRemoteProcess != NULL)
			CloseHandle(hRemoteProcess);

		if (handle != NULL)
			CloseHandle(handle);
	}

	if (lpBuffer != NULL)
		delete[] lpBuffer;

	return ret;
}

//Thread related functions
BOOL SystemHandleInformation::GetThreadId(HANDLE h, DWORD& threadID, DWORD processId)
{
	SystemThreadInformation::BASIC_THREAD_INFORMATION ti;
	HANDLE handle;
	HANDLE hRemoteProcess = NULL;
	BOOL remote = processId != GetCurrentProcessId();

	if (!NtDllStatus)
		return FALSE;

	if (remote)
	{
		// Open process
		hRemoteProcess = OpenProcess(processId);

		if (hRemoteProcess == NULL)
			return FALSE;

		// Duplicate handle
		handle = DuplicateHandle(hRemoteProcess, h);
	}
	else
		handle = h;

	// Get the thread information
	if (INtDll::NtQueryInformationThread(handle, 0, &ti, sizeof(ti), NULL) == 0)
		threadID = ti.ThreadId;

	if (remote)
	{
		if (hRemoteProcess != NULL)
			CloseHandle(hRemoteProcess);

		if (handle != NULL)
			CloseHandle(handle);
	}

	return TRUE;
}

//Process related functions
BOOL SystemHandleInformation::GetProcessPath(HANDLE h, CString& strPath, DWORD remoteProcessId)
{
	h; strPath; remoteProcessId;

	strPath.Format(_T("%d"), remoteProcessId);

	return TRUE;
}

BOOL SystemHandleInformation::GetProcessId(HANDLE h, DWORD& processId, DWORD remoteProcessId)
{
	BOOL ret = FALSE;
	HANDLE handle;
	HANDLE hRemoteProcess = NULL;
	BOOL remote = remoteProcessId != GetCurrentProcessId();
	SystemProcessInformation::PROCESS_BASIC_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	processId = 0;

	if (!NtDllStatus)
		return FALSE;

	if (remote)
	{
		// Open process
		hRemoteProcess = OpenProcess(remoteProcessId);

		if (hRemoteProcess == NULL)
			return FALSE;

		// Duplicate handle
		handle = DuplicateHandle(hRemoteProcess, h);
	}
	else
		handle = h;

	// Get the process information
	if (INtDll::NtQueryInformationProcess(handle, 0, &pi, sizeof(pi), NULL) == 0)
	{
		processId = pi.UniqueProcessId;
		ret = TRUE;
	}

	if (remote)
	{
		if (hRemoteProcess != NULL)
			CloseHandle(hRemoteProcess);

		if (handle != NULL)
			CloseHandle(handle);
	}

	return ret;
}

//File related functions
void SystemHandleInformation::GetFileNameThread(PVOID pParam)
{
	// This thread function for getting the filename
	// if access denied, we hang up in this function, 
	// so if it times out we just kill this thread
	GetFileNameThreadParam* p = (GetFileNameThreadParam*)pParam;

	UCHAR lpBuffer[0x1000];
	DWORD iob[2];

	p->rc = INtDll::NtQueryInformationFile(p->hFile, iob, lpBuffer, sizeof(lpBuffer), 9);

	if (p->rc == 0)
		*p->pName = lpBuffer;
}

BOOL SystemHandleInformation::GetFileName(HANDLE h, CString& str, DWORD processId)
{
	BOOL ret = FALSE;
	HANDLE hThread = NULL;
	GetFileNameThreadParam tp;
	HANDLE handle;
	HANDLE hRemoteProcess = NULL;
	BOOL remote = processId != GetCurrentProcessId();

	if (!NtDllStatus)
		return FALSE;

	if (remote)
	{
		// Open process
		hRemoteProcess = OpenProcess(processId);

		if (hRemoteProcess == NULL)
			return FALSE;

		// Duplicate handle
		handle = DuplicateHandle(hRemoteProcess, h);
	}
	else
		handle = h;

	tp.hFile = handle;
	tp.pName = &str;
	tp.rc = 0;

	// Let's start the thread to get the file name
	hThread = (HANDLE)_beginthread(GetFileNameThread, 0, &tp);

	if (hThread == NULL)
	{
		ret = FALSE;
		goto cleanup;
	}

	// Wait for finishing the thread
	if (WaitForSingleObject(hThread, 100) == WAIT_TIMEOUT)
	{
		// Access denied
		// Terminate the thread
		TerminateThread(hThread, 0);

		str = _T("");

		ret = TRUE;
	}
	else
		ret = (tp.rc == 0);

cleanup:

	if (remote)
	{
		if (hRemoteProcess != NULL)
			CloseHandle(hRemoteProcess);

		if (handle != NULL)
			CloseHandle(handle);
	}

	return ret;
}
