#include "StdAfx.h"
#include "INtDll.h"
#include "SystemProcessInformation.h"
#include "SystemInfoUtils.h"

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemProcessInformation
//
//////////////////////////////////////////////////////////////////////////////////////

SystemProcessInformation::SystemProcessInformation(BOOL bRefresh)
{
	//m_pBuffer = (UCHAR*)VirtualAlloc((void*)0x100000, BufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_pBuffer = (UCHAR*)VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);
	if (m_pBuffer == NULL)
	{
		std::string error = SystemInfoUtils::GetLastErrorStdStr();
		OutputDebugString(error.c_str());
	}

	if (bRefresh)
		Refresh();
}

SystemProcessInformation::~SystemProcessInformation()
{
	VirtualFree(m_pBuffer, 0, MEM_RELEASE);
}

BOOL SystemProcessInformation::Refresh()
{
	m_ProcessInfos.clear();
	m_pCurrentProcessInfo = NULL;

	if (!NtDllStatus || m_pBuffer == NULL)
		return FALSE;

	// query the process information
	if (INtDll::NtQuerySystemInformation(5, m_pBuffer, BufferSize, NULL) != 0)
		//if (INtDll::NtQueryInformationProcess(5, m_pBuffer, BufferSize, NULL) != 0)
	{
		std::string error = SystemInfoUtils::GetLastErrorStdStr();
		OutputDebugString(error.c_str());
		return FALSE;
	}

	DWORD currentProcessID = GetCurrentProcessId(); //Current Process ID

	SYSTEM_PROCESS_INFORMATION* pSysProcess = (SYSTEM_PROCESS_INFORMATION*)m_pBuffer;
	do
	{
		// fill the process information map
		m_ProcessInfos[pSysProcess->dUniqueProcessId] = pSysProcess;

		// we found this process
		if (pSysProcess->dUniqueProcessId == currentProcessID)
			m_pCurrentProcessInfo = pSysProcess;

		// get the next process information block
		if (pSysProcess->dNext != 0)
			pSysProcess = (SYSTEM_PROCESS_INFORMATION*)((UCHAR*)pSysProcess + pSysProcess->dNext);
		else
			pSysProcess = NULL;

	} while (pSysProcess != NULL);

	return TRUE;
}