#include "StdAfx.h"
#include "INtDll.h"
#include "SystemModuleInformation.h"
#include "SystemProcessInformation.h"

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemModuleInformation
//
//////////////////////////////////////////////////////////////////////////////////////

SystemModuleInformation::SystemModuleInformation(DWORD pID, BOOL bRefresh)
{
	m_processId = pID;

	if (bRefresh)
		Refresh();
}

void SystemModuleInformation::GetModuleListForProcess(DWORD processID)
{
	DWORD i = 0;
	DWORD cbNeeded = 0;
	HMODULE* hModules = NULL;
	MODULE_INFO moduleInfo;

	// Open process to read to query the module list
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	if (hProcess == NULL)
		goto cleanup;

	//Get the number of modules
	if (!(*m_EnumProcessModules)(hProcess, NULL, 0, &cbNeeded))
		goto cleanup;

	hModules = new HMODULE[cbNeeded / sizeof(HMODULE)];

	//Get module handles
	if (!(*m_EnumProcessModules)(hProcess, hModules, cbNeeded, &cbNeeded))
		goto cleanup;

	for (i = 0; i < cbNeeded / sizeof(HMODULE); i++)
	{
		moduleInfo.ProcessId = processID;
		moduleInfo.Handle = hModules[i];

		//Get module full paths
		if ((*m_GetModuleFileNameEx)(hProcess, hModules[i], moduleInfo.FullPath, _MAX_PATH))
			m_ModuleInfos.push_back(moduleInfo);
	}

cleanup:
	if (hModules != NULL)
		delete[] hModules;

	if (hProcess != NULL)
		CloseHandle(hProcess);
}

BOOL SystemModuleInformation::Refresh()
{
	BOOL rc = FALSE;
	m_EnumProcessModules = NULL;
	m_GetModuleFileNameEx = NULL;

	m_ModuleInfos.clear();

	//Load Psapi.dll
	HINSTANCE hDll = LoadLibrary("PSAPI.DLL");

	if (hDll == NULL)
	{
		rc = FALSE;
		goto cleanup;
	}

	//Get Psapi.dll functions
	m_EnumProcessModules = (PEnumProcessModules)GetProcAddress(hDll, "EnumProcessModules");

	m_GetModuleFileNameEx = (PGetModuleFileNameEx)GetProcAddress(hDll,
#ifdef UNICODE
		"GetModuleFileNameExW");
#else
		"GetModuleFileNameExA");
#endif

	if (m_GetModuleFileNameEx == NULL || m_EnumProcessModules == NULL)
	{
		rc = FALSE;
		goto cleanup;
	}

	// Everey process or just a particular one
	if (m_processId != -1)
		// For a particular one
		GetModuleListForProcess(m_processId);
	else
	{
		// Get teh process list
		DWORD pID;
		SystemProcessInformation::SYSTEM_PROCESS_INFORMATION* p = NULL;
		SystemProcessInformation pi(TRUE);

		if (pi.m_ProcessInfos.empty())
		{
			rc = FALSE;
			goto cleanup;
		}

		// Iterating through the processes and get the module list
		for (std::map<DWORD, SystemProcessInformation::SYSTEM_PROCESS_INFORMATION*>::iterator pos = pi.m_ProcessInfos.begin(); pos != pi.m_ProcessInfos.end(); pos++)
		{
			pID = pos->first;
			GetModuleListForProcess(pID);
		}
	}

	rc = TRUE;

cleanup:

	//Free psapi.dll
	if (hDll != NULL)
		FreeLibrary(hDll);

	return rc;
}
