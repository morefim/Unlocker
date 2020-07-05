#pragma once

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemProcessInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemProcessInformation : public INtDll
{
public:
	typedef LARGE_INTEGER   QWORD;

	typedef struct _PROCESS_BASIC_INFORMATION {
		DWORD ExitStatus;
		PVOID PebBaseAddress;
		DWORD AffinityMask;
		DWORD BasePriority;
		DWORD UniqueProcessId;
		DWORD InheritedFromUniqueProcessId;
	} PROCESS_BASIC_INFORMATION;

	typedef struct _VM_COUNTERS
	{
		DWORD PeakVirtualSize;
		DWORD VirtualSize;
		DWORD PageFaultCount;
		DWORD PeakWorkingSetSize;
		DWORD WorkingSetSize;
		DWORD QuotaPeakPagedPoolUsage;
		DWORD QuotaPagedPoolUsage;
		DWORD QuotaPeakNonPagedPoolUsage;
		DWORD QuotaNonPagedPoolUsage;
		DWORD PagefileUsage;
		DWORD PeakPagefileUsage;
	} VM_COUNTERS;

	typedef struct _SYSTEM_THREAD
	{
		DWORD        u1;
		DWORD        u2;
		DWORD        u3;
		DWORD        u4;
		DWORD        ProcessId;
		DWORD        ThreadId;
		DWORD        dPriority;
		DWORD        dBasePriority;
		DWORD        dContextSwitches;
		DWORD        dThreadState;      // 2=running, 5=waiting
		DWORD        WaitReason;
		DWORD        u5;
		DWORD        u6;
		DWORD        u7;
		DWORD        u8;
		DWORD        u9;
	} SYSTEM_THREAD;

	typedef struct _SYSTEM_PROCESS_INFORMATION
	{
		DWORD          dNext;
		DWORD          dThreadCount;
		DWORD          dReserved01;
		DWORD          dReserved02;
		DWORD          dReserved03;
		DWORD          dReserved04;
		DWORD          dReserved05;
		DWORD          dReserved06;
		QWORD          qCreateTime;
		QWORD          qUserTime;
		QWORD          qKernelTime;
		UNICODE_STRING usName;
		DWORD        BasePriority;
		DWORD          dUniqueProcessId;
		DWORD          dInheritedFromUniqueProcessId;
		DWORD          dHandleCount;
		DWORD          dReserved07;
		DWORD          dReserved08;
		VM_COUNTERS    VmCounters;
		DWORD          dCommitCharge;
		SYSTEM_THREAD  Threads[1];
	} SYSTEM_PROCESS_INFORMATION;

	enum { BufferSize = 0x10000 };

public:
	SystemProcessInformation(BOOL bRefresh = FALSE);
	virtual ~SystemProcessInformation();

private:
	BOOL Refresh();

public:
	std::map<DWORD, SYSTEM_PROCESS_INFORMATION*> m_ProcessInfos;
	SYSTEM_PROCESS_INFORMATION* m_pCurrentProcessInfo;

protected:
	UCHAR* m_pBuffer;
};

