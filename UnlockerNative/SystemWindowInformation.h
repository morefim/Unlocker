#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemWindowInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemWindowInformation
{
public:
	enum { MaxCaptionSize = 1024 };

	typedef struct _WINDOW_INFO
	{
		DWORD ProcessId;
		TCHAR Caption[MaxCaptionSize];
		HWND hWnd;
	} WINDOW_INFO;

public:
	SystemWindowInformation(DWORD pID = (DWORD)-1, BOOL bRefresh = FALSE);

private:
	BOOL Refresh();

protected:
	static BOOL CALLBACK EnumerateWindows(HWND hwnd, LPARAM lParam);

public:
	DWORD m_processId;
	std::list<WINDOW_INFO> m_WindowInfos;
};
