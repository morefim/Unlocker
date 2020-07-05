#ifndef CFORCEDEL_H_INCLUDED
#define CFORCEDEL_H_INCLUDED

class CForceDel
{
public:
	CForceDel();
	~CForceDel();

	int Run(LPCTSTR lpPath, BOOL bUsage, BOOL bSoft);
	void ShowUsage();
	void EnableDebugPriv(void);
	DWORD CloseRemoteHandle(LPCTSTR, DWORD, HANDLE);
	void CloseRemoteFileHandles(LPCTSTR);
	BOOL DeleteTheFile(LPCTSTR);
};

#endif
