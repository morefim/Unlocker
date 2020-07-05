#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemHandleInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemHandleInformation : public INtDll
{
public:
	enum {
		OB_TYPE_UNKNOWN = 0,
		OB_TYPE_TYPE = 1,
		OB_TYPE_DIRECTORY,
		OB_TYPE_SYMBOLIC_LINK,
		OB_TYPE_TOKEN,
		OB_TYPE_PROCESS,
		OB_TYPE_THREAD,
		OB_TYPE_UNKNOWN_7,
		OB_TYPE_EVENT,
		OB_TYPE_EVENT_PAIR,
		OB_TYPE_MUTANT,
		OB_TYPE_UNKNOWN_11,
		OB_TYPE_SEMAPHORE,
		OB_TYPE_TIMER,
		OB_TYPE_PROFILE,
		OB_TYPE_WINDOW_STATION,
		OB_TYPE_DESKTOP,
		OB_TYPE_SECTION,
		OB_TYPE_KEY,
		OB_TYPE_PORT,
		OB_TYPE_WAITABLE_PORT,
		OB_TYPE_UNKNOWN_21,
		OB_TYPE_UNKNOWN_22,
		OB_TYPE_UNKNOWN_23,
		OB_TYPE_UNKNOWN_24,
		//OB_TYPE_CONTROLLER,
		//OB_TYPE_DEVICE,
		//OB_TYPE_DRIVER,
		OB_TYPE_IO_COMPLETION,
		OB_TYPE_FILE
	} SystemHandleType;

public:
	typedef struct _SYSTEM_HANDLE
	{
		DWORD ProcessID;
		WORD HandleType;
		WORD HandleNumber;
		DWORD KernelAddress;
		DWORD Flags;
	} SYSTEM_HANDLE;

	typedef struct _SYSTEM_HANDLE_INFORMATION
	{
		DWORD   Count;
		SYSTEM_HANDLE Handles[1];
	} SYSTEM_HANDLE_INFORMATION;

protected:
	typedef struct _GetFileNameThreadParam
	{
		HANDLE  hFile;
		CString* pName;
		ULONG  rc;
	} GetFileNameThreadParam;

public:
	SystemHandleInformation(DWORD pID = (DWORD)-1, BOOL bRefresh = FALSE, LPCTSTR lpTypeFilter = NULL);
	~SystemHandleInformation();

	BOOL SetFilter(LPCTSTR lpTypeFilter, BOOL bRefresh = TRUE);
	const CString& GetFilter();

private:
	BOOL Refresh();

public:
	//Information functions
	static BOOL GetType(HANDLE, WORD&, DWORD processId = GetCurrentProcessId());
	static BOOL GetTypeToken(HANDLE, CString&, DWORD processId = GetCurrentProcessId());
	static BOOL GetTypeFromTypeToken(LPCTSTR typeToken, WORD& type);
	static BOOL GetNameByType(HANDLE, WORD, CString& str, DWORD processId = GetCurrentProcessId());
	static BOOL GetName(HANDLE, CString&, DWORD processId = GetCurrentProcessId());

	//Thread related functions
	static BOOL GetThreadId(HANDLE, DWORD&, DWORD processId = GetCurrentProcessId());

	//Process related functions
	static BOOL GetProcessId(HANDLE, DWORD&, DWORD processId = GetCurrentProcessId());
	static BOOL GetProcessPath(HANDLE h, CString& strPath, DWORD processId = GetCurrentProcessId());

	//File related functions
	static BOOL GetFileName(HANDLE, CString&, DWORD processId = GetCurrentProcessId());

public:
	//For remote handle support
	static HANDLE OpenProcess(DWORD processId);
	static HANDLE DuplicateHandle(HANDLE hProcess, HANDLE hRemote);

protected:
	static void GetFileNameThread(PVOID /* GetFileNameThreadParam* */);
	BOOL IsSupportedHandle(SYSTEM_HANDLE& handle);

public:
	std::list<SYSTEM_HANDLE> m_HandleInfos;
	DWORD m_processId;

protected:
	CString m_strTypeFilter;
};

