// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EBE3CE44_4C81_4783_A62F_89523B9B0337__INCLUDED_)
#define AFX_STDAFX_H__EBE3CE44_4C81_4783_A62F_89523B9B0337__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif                        

#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE            // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500    // Change this to the appropriate value to target IE 5.0 or later.
#endif

#ifndef WINNT            // Allow use of features specific to IE 4.0 or later.
#define WINNT 0x0500    // Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)

#include <windows.h>
#include <sysinfoapi.h>
#include <atlstr.h>
#include <map>
#include <list>
#include <process.h>
#include <string>
#include <tchar.h>
#include <stdio.h>

// TODO: reference additional headers your program requires here


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EBE3CE44_4C81_4783_A62F_89523B9B0337__INCLUDED_)
