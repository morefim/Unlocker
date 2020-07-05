#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemModuleInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemModuleInformation
{
public:
	typedef struct _MODULE_INFO
	{
		DWORD ProcessId;
		TCHAR FullPath[_MAX_PATH];
		HMODULE Handle;
	} MODULE_INFO;

public:
	typedef DWORD(WINAPI* PEnumProcessModules)(
		HANDLE hProcess,      // handle to process
		HMODULE* lphModule,   // array of module handles
		DWORD cb,             // size of array
		LPDWORD lpcbNeeded    // number of bytes required
		);

	typedef DWORD(WINAPI* PGetModuleFileNameEx)(
		HANDLE hProcess,    // handle to process
		HMODULE hModule,    // handle to module
		LPTSTR lpFilename,  // path buffer
		DWORD nSize         // maximum characters to retrieve
		);

public:
	SystemModuleInformation(DWORD pID = (DWORD)-1, BOOL bRefresh = FALSE);

private:
	BOOL Refresh();

protected:
	void GetModuleListForProcess(DWORD processID);

public:
	DWORD m_processId;
	std::list<MODULE_INFO> m_ModuleInfos;

protected:
	PEnumProcessModules  m_EnumProcessModules;
	PGetModuleFileNameEx m_GetModuleFileNameEx;
};