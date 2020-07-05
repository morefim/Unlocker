#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemThreadInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemThreadInformation : public INtDll
{
public:
	typedef struct _THREAD_INFORMATION
	{
		DWORD  ProcessId;
		DWORD  ThreadId;
		HANDLE  ThreadHandle;
	} THREAD_INFORMATION;


	typedef struct _BASIC_THREAD_INFORMATION {
		DWORD u1;
		DWORD u2;
		DWORD u3;
		DWORD ThreadId;
		DWORD u5;
		DWORD u6;
		DWORD u7;
	} BASIC_THREAD_INFORMATION;

public:
	SystemThreadInformation(DWORD pID = (DWORD)-1, BOOL bRefresh = FALSE);

private:
	BOOL Refresh();

public:
	std::list<THREAD_INFORMATION> m_ThreadInfos;
	DWORD m_processId;
};

