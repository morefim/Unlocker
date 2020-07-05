#include "StdAfx.h"
#include "INtDll.h"
#include "SystemWindowInformation.h"


//////////////////////////////////////////////////////////////////////////////////////
//
// SystemWindowInformation
//
//////////////////////////////////////////////////////////////////////////////////////

SystemWindowInformation::SystemWindowInformation(DWORD pID, BOOL bRefresh)
{
	m_processId = pID;

	if (bRefresh)
		Refresh();
}

BOOL SystemWindowInformation::Refresh()
{
	m_WindowInfos.clear();

	// Enumerating the windows
	EnumWindows(EnumerateWindows, (LPARAM)this);

	return TRUE;
}

BOOL CALLBACK SystemWindowInformation::EnumerateWindows(HWND hwnd, LPARAM lParam)
{
	SystemWindowInformation* _this = (SystemWindowInformation*)lParam;
	WINDOW_INFO wi;

	wi.hWnd = hwnd;
	GetWindowThreadProcessId(hwnd, &wi.ProcessId);

	// Filtering by process ID
	if (_this->m_processId == -1 || _this->m_processId == wi.ProcessId)
	{
		GetWindowText(hwnd, wi.Caption, MaxCaptionSize);

		// That is we are looking for
		if (GetLastError() == 0)
			_this->m_WindowInfos.push_back(wi);
	}

	return TRUE;
};