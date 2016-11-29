#pragma once
#pragma warning( disable : 4214 )
#pragma warning( disable : 4201 )

#include <ntifs.h>

union _EX_PUSH_LOCK // Size=8
{
	struct
	{
		unsigned __int64 Locked : 1; // Size=8 Offset=0 BitOffset=0 BitCount=1
		unsigned __int64 Waiting : 1; // Size=8 Offset=0 BitOffset=1 BitCount=1
		unsigned __int64 Waking : 1; // Size=8 Offset=0 BitOffset=2 BitCount=1
		unsigned __int64 MultipleShared : 1; // Size=8 Offset=0 BitOffset=3 BitCount=1
		unsigned __int64 Shared : 60; // Size=8 Offset=0 BitOffset=4 BitCount=60
	};
	unsigned __int64 Value; // Size=8 Offset=0
	void * Ptr; // Size=8 Offset=0
};

struct _MMVAD_FLAGS // Size=4
{
	unsigned long VadType : 3; // Size=4 Offset=0 BitOffset=0 BitCount=3
	unsigned long Protection : 5; // Size=4 Offset=0 BitOffset=3 BitCount=5
	unsigned long PreferredNode : 6; // Size=4 Offset=0 BitOffset=8 BitCount=6
	unsigned long NoChange : 1; // Size=4 Offset=0 BitOffset=14 BitCount=1
	unsigned long PrivateMemory : 1; // Size=4 Offset=0 BitOffset=15 BitCount=1
	unsigned long Teb : 1; // Size=4 Offset=0 BitOffset=16 BitCount=1
	unsigned long PrivateFixup : 1; // Size=4 Offset=0 BitOffset=17 BitCount=1
	unsigned long ManySubsections : 1; // Size=4 Offset=0 BitOffset=18 BitCount=1
	unsigned long Spare : 12; // Size=4 Offset=0 BitOffset=19 BitCount=12
	unsigned long DeleteInProgress : 1; // Size=4 Offset=0 BitOffset=31 BitCount=1
};
struct _MMVAD_FLAGS1 // Size=4
{
	unsigned long CommitCharge : 31; // Size=4 Offset=0 BitOffset=0 BitCount=31
	unsigned long MemCommit : 1; // Size=4 Offset=0 BitOffset=31 BitCount=1
};

union ___unnamed1951 // Size=4
{
	unsigned long LongFlags; // Size=4 Offset=0
	struct _MMVAD_FLAGS VadFlags; // Size=4 Offset=0
};

union ___unnamed1952 // Size=4
{
	unsigned long LongFlags1; // Size=4 Offset=0
	struct _MMVAD_FLAGS1 VadFlags1; // Size=4 Offset=0
};

typedef struct _MMVAD_SHORT // Size=64
{
	union
	{
		struct _RTL_BALANCED_NODE VadNode; // Size=24 Offset=0
		struct _MMVAD_SHORT * NextVad; // Size=8 Offset=0
	};
	unsigned long StartingVpn; // Size=4 Offset=24
	unsigned long EndingVpn; // Size=4 Offset=28
	unsigned char StartingVpnHigh; // Size=1 Offset=32
	unsigned char EndingVpnHigh; // Size=1 Offset=33
	unsigned char CommitChargeHigh; // Size=1 Offset=34
	unsigned char SpareNT64VadUChar; // Size=1 Offset=35
	long ReferenceCount; // Size=4 Offset=36
	union _EX_PUSH_LOCK PushLock; // Size=8 Offset=40
	union ___unnamed1951 u; // Size=4 Offset=48
	union ___unnamed1952 u1; // Size=4 Offset=52
	struct _MI_VAD_EVENT_BLOCK * EventList; // Size=8 Offset=56
} MMVAD_SHORT, *PMMVAD_SHORT;

typedef struct _MM_AVL_NODE // Size=24
{
	struct _MM_AVL_NODE * LeftChild; // Size=8 Offset=0
	struct _MM_AVL_NODE * RightChild; // Size=8 Offset=8

	union ___unnamed1666 // Size=8
	{
		struct
		{
			__int64 Balance : 2; // Size=8 Offset=0 BitOffset=0 BitCount=2
		};
		struct _MM_AVL_NODE * Parent; // Size=8 Offset=0
	} u1;
} MM_AVL_NODE, *PMM_AVL_NODE, *PMMADDRESS_NODE;

typedef struct _RTL_AVL_TREE // Size=8
{
	PMM_AVL_NODE BalancedRoot;
	void * NodeHint;
	unsigned __int64 NumberGenericTableElements;
} RTL_AVL_TREE, *PRTL_AVL_TREE, MM_AVL_TABLE, *PMM_AVL_TABLE;

typedef enum _WinVer
{
	WINVER_7 = 0x0610,
	WINVER_7_SP1 = 0x0611,
	WINVER_8 = 0x0620,
	WINVER_81 = 0x0630,
	WINVER_10 = 0x0A00,
	WINVER_10_AU = 0x0A01,
} WinVer;

/// <summary>
/// OS-dependent stuff
/// </summary>
typedef struct _DYNAMIC_DATA
{
	WinVer  ver;            // OS version
	BOOLEAN correctBuild;   // OS kernel build number is correct and supported

	ULONG KExecOpt;         // KPROCESS::ExecuteOptions 
	ULONG Protection;       // EPROCESS::Protection
	ULONG ObjTable;         // EPROCESS::ObjectTable
	ULONG VadRoot;          // EPROCESS::VadRoot
	ULONG NtProtectIndex;   // NtProtectVirtualMemory SSDT index
	ULONG NtCreateThdIndex; // NtCreateThreadEx SSDT index
	ULONG NtTermThdIndex;   // NtTerminateThread SSDT index
	ULONG PrevMode;         // KTHREAD::PreviousMode
	ULONG ExitStatus;       // ETHREAD::ExitStatus
	ULONG MiAllocPage;      // MiAllocateDriverPage offset
	ULONG ExRemoveTable;    // ExRemoveHandleTable offset

	ULONG_PTR DYN_PDE_BASE; // Win10 AU relocated PDE base VA
	ULONG_PTR DYN_PTE_BASE; // Win10 AU relocated PTE base VA
} DYNAMIC_DATA, *PDYNAMIC_DATA;

typedef struct _MMPTE_HARDWARE64
{
	ULONGLONG Valid : 1;
	ULONGLONG Dirty1 : 1;
	ULONGLONG Owner : 1;
	ULONGLONG WriteThrough : 1;
	ULONGLONG CacheDisable : 1;
	ULONGLONG Accessed : 1;
	ULONGLONG Dirty : 1;
	ULONGLONG LargePage : 1;
	ULONGLONG Global : 1;
	ULONGLONG CopyOnWrite : 1;
	ULONGLONG Unused : 1;
	ULONGLONG Write : 1;
	ULONGLONG PageFrameNumber : 36;
	ULONGLONG reserved1 : 4;
	ULONGLONG SoftwareWsIndex : 11;
	ULONGLONG NoExecute : 1;
} MMPTE_HARDWARE64, *PMMPTE_HARDWARE64;

typedef struct _MMPTE
{
	union
	{
		ULONG_PTR Long;
		MMPTE_HARDWARE64 Hard;
	} u;
} MMPTE;
typedef MMPTE *PMMPTE;