#include "StdAfx.h"
#include "INtDll.h"
#include "SystemInfoUtils.h"

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemInfoUtils
//
//////////////////////////////////////////////////////////////////////////////////////

// From wide char string to CString
void SystemInfoUtils::LPCWSTR2CString(LPCWSTR strW, CString& str)
{
#ifdef UNICODE
	// if it is already UNICODE, no problem
	str = strW;
#else
	str = _T("");

	TCHAR* actChar = (TCHAR*)strW;

	if (actChar == _T('\0'))
		return;

	ULONG len = wcslen(strW) + 1;
	TCHAR* pBuffer = new TCHAR[len];
	TCHAR* pNewStr = pBuffer;

	while (len--)
	{
		*(pNewStr++) = *actChar;
		actChar += 2;
	}

	str = pBuffer;

	delete[] pBuffer;
#endif
}

// From wide char string to unicode
void SystemInfoUtils::Unicode2CString(UNICODE_STRING* strU, CString& str)
{
	if (*(DWORD*)strU != 0)
		LPCWSTR2CString((LPCWSTR)strU->Buffer, str);
	else
		str = _T("");
}

// From device file name to DOS filename
BOOL SystemInfoUtils::GetFsFileName(LPCTSTR lpDeviceFileName, CString& fsFileName)
{
	BOOL rc = FALSE;

	TCHAR lpDeviceName[0x1000];
	TCHAR lpDrive[3] = _T("A:");

	// Iterating through the drive letters
	for (TCHAR actDrive = _T('A'); actDrive <= _T('Z'); actDrive++)
	{
		lpDrive[0] = actDrive;

		// Query the device for the drive letter
		if (QueryDosDevice(lpDrive, lpDeviceName, 0x1000) != 0)
		{
			// Network drive?
			if (_tcsnicmp(_T("\\Device\\LanmanRedirector\\"), lpDeviceName, 25) == 0)
			{
				//Mapped network drive 

				DWORD dwParam;
				char cDriveLetter;
				TCHAR lpSharedName[0x1000];

				if (_stscanf(lpDeviceName, _T("\\Device\\LanmanRedirector\\;%c:%d\\%s"), &cDriveLetter, &dwParam, lpSharedName) != 3)
					continue;

				_tcscpy(lpDeviceName, _T("\\Device\\LanmanRedirector\\"));
				_tcscat(lpDeviceName, lpSharedName);
			}

			// Is this the drive letter we are looking for?
			if (_tcsnicmp(lpDeviceName, lpDeviceFileName, _tcslen(lpDeviceName)) == 0)
			{
				fsFileName = lpDrive;
				fsFileName += (LPCTSTR)(lpDeviceFileName + _tcslen(lpDeviceName));

				rc = TRUE;

				break;
			}
		}
	}

	return rc;
}

// From DOS file name to device file name
BOOL SystemInfoUtils::GetDeviceFileName(LPCTSTR lpFsFileName, CString& deviceFileName)
{
	BOOL rc = FALSE;
	TCHAR lpDrive[3];

	// Get the drive letter 
	// unfortunetaly it works only with DOS file names
	_tcsncpy(lpDrive, lpFsFileName, 2);
	lpDrive[2] = _T('\0');

	TCHAR lpDeviceName[0x1000];

	// Query the device for the drive letter
	if (QueryDosDevice(lpDrive, lpDeviceName, 0x1000) != 0)
	{
		// Subst drive?
		if (_tcsnicmp(_T("\\??\\"), lpDeviceName, 4) == 0)
		{
			deviceFileName = lpDeviceName + 4;
			deviceFileName += lpFsFileName + 2;

			return TRUE;
		}
		else
			// Network drive?
			if (_tcsnicmp(_T("\\Device\\LanmanRedirector\\"), lpDeviceName, 25) == 0)
			{
				//Mapped network drive 
				DWORD dwParam;
				char cDriveLetter;
				TCHAR lpSharedName[0x1000];

				if (_stscanf(lpDeviceName, _T("\\Device\\LanmanRedirector\\;%c:%d\\%s"), &cDriveLetter, &dwParam, lpSharedName) != 3)
					return FALSE;

				_tcscpy(lpDeviceName, _T("\\Device\\LanmanRedirector\\"));
				_tcscat(lpDeviceName, lpSharedName);
			}

		_tcscat(lpDeviceName, lpFsFileName + 2);

		deviceFileName = lpDeviceName;

		rc = TRUE;
	}

	return rc;
}

//Get NT version
DWORD SystemInfoUtils::GetNTMajorVersion()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
	// which is supported on Windows 2000.
	//
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

	if (bOsVersionInfoEx == 0)
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi))
			return FALSE;
	}

	return osvi.dwMajorVersion;
}

// Create a string with last error message
std::string SystemInfoUtils::GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

int SystemInfoUtils::print_log(const char* format, ...)
{
	static char s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringA(s_printf_buf);
	OutputDebugStringA("\n");
	return 0;
}