#include "StdAfx.h"
#include "INtDll.h"
#include "SystemThreadInformation.h"
#include "SystemHandleInformation.h"

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemThreadInformation
//
//////////////////////////////////////////////////////////////////////////////////////

SystemThreadInformation::SystemThreadInformation(DWORD pID, BOOL bRefresh)
{
	m_processId = pID;

	if (bRefresh)
		Refresh();
}

BOOL SystemThreadInformation::Refresh()
{
	// Get the Thread objects ( set the filter to "Thread" )
	SystemHandleInformation hi(m_processId);
	BOOL rc = hi.SetFilter(_T("Thread"), TRUE);

	m_ThreadInfos.clear();

	if (!rc)
		return FALSE;

	THREAD_INFORMATION ti;

	// Iterating through the found Thread objects
	for (std::list<SystemHandleInformation::SYSTEM_HANDLE>::iterator pos = hi.m_HandleInfos.begin(); pos != hi.m_HandleInfos.end(); pos++)
	{
		SystemHandleInformation::SYSTEM_HANDLE& h = *pos;

		ti.ProcessId = h.ProcessID;
		ti.ThreadHandle = (HANDLE)h.HandleNumber;

		// This is one of the threads we are lokking for
		if (SystemHandleInformation::GetThreadId(ti.ThreadHandle, ti.ThreadId, ti.ProcessId))
			m_ThreadInfos.push_back(ti);
	}

	return TRUE;
}