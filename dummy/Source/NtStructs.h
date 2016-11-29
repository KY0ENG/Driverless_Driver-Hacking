#pragma once

#include <windef.h>

#pragma warning( disable : 4201 )

typedef enum _KAPC_ENVIRONMENT
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT, *PKAPC_ENVIRONMENT;

typedef VOID( NTAPI *PKRUNDOWN_ROUTINE )(__in PRKAPC Apc);

typedef VOID( NTAPI *PKNORMAL_ROUTINE )(
	__in PVOID NormalContext,
	__in PVOID SystemArgument1,
	__in PVOID SystemArgument2
	);

typedef VOID
(NTAPI *PKKERNEL_ROUTINE)(
	IN struct _KAPC *Apc,
	IN OUT PKNORMAL_ROUTINE *NormalRoutine OPTIONAL,
	IN OUT PVOID *NormalContext OPTIONAL,
	IN OUT PVOID *SystemArgument1 OPTIONAL,
	IN OUT PVOID *SystemArgument2 OPTIONAL);



typedef struct _KGDTENTRY
{
	WORD LimitLow;
	WORD BaseLow;
	ULONG HighWord;
} KGDTENTRY, *PKGDTENTRY;

typedef struct _KIDTENTRY
{
	WORD Offset;
	WORD Selector;
	WORD Access;
	WORD ExtendedOffset;
} KIDTENTRY, *PKIDTENTRY;

#pragma warning( disable : 4214 )
typedef struct _KEXECUTE_OPTIONS
{
	ULONG ExecuteDisable : 1;
	ULONG ExecuteEnable : 1;
	ULONG DisableThunkEmulation : 1;
	ULONG Permanent : 1;
	ULONG ExecuteDispatchEnable : 1;
	ULONG ImageDispatchEnable : 1;
	ULONG Spare : 2;
} KEXECUTE_OPTIONS, *PKEXECUTE_OPTIONS;

typedef struct _KPROCESS {
	/*0x000*/ struct _DISPATCHER_HEADER Header;      // _DISPATCHER_HEADER
	/*0x018*/ LIST_ENTRY ProfileListHead;            // _LIST_ENTRY
	/*0x028*/ ULONG DirectoryTableBase;              // Uint8B
			  ULONG Unused;
	/*0x030*/ LIST_ENTRY ThreadListHead;             // _LIST_ENTRY
} KPROCESS, *PKPROCESS;


typedef struct _EPROCESS
{
	struct _KPROCESS Pcb;
	EX_PUSH_LOCK ProcessLock;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER ExitTime;
	EX_RUNDOWN_REF RundownProtect;
	PVOID UniqueProcessId;
	LIST_ENTRY ActiveProcessLinks;
	ULONG QuotaUsage[3];
	ULONG QuotaPeak[3];
	ULONG CommitCharge;
	ULONG PeakVirtualSize;
	ULONG VirtualSize;
	LIST_ENTRY SessionProcessLinks;
} _EPROCESS, *_PEPROCESS;

typedef struct _KWAIT_STATUS_REGISTER {
	/*0x000*/ BYTE Flags;            // UChar
	/*0x000*/ BYTE State : 2;            // Pos 0, 2 Bits
	/*0x000*/ BYTE Affinity : 1;         // Pos 2, 1 Bit
	/*0x000*/ BYTE Priority : 1;         // Pos 3, 1 Bit
	/*0x000*/ BYTE Apc : 1;              // Pos 4, 1 Bit
	/*0x000*/ BYTE UserApc : 1;          // Pos 5, 1 Bit
	/*0x000*/ BYTE Alert : 1;            // Pos 6, 1 Bit
	/*0x000*/ BYTE Unused : 1;           // Pos 7, 1 Bit
} KWAIT_STATUS_REGISTER, *PKWAIT_STATUS_REGISTER;


typedef struct _KTHREAD {
	/*0x000*/ DISPATCHER_HEADER Header;				   // _DISPATCHER_HEADER
	/*0x018*/ UINT64 CycleTime;						   // Uint8B
	/*0x020*/ UINT64 QuantumTarget;					   // Uint8B
	/*0x028*/ PVOID InitialStack;					   // Ptr64 Void
	/*0x030*/ PVOID StackLimit;						   // Ptr64 Void
	/*0x038*/ PVOID KernelStack;						   // Ptr64 Void
	/*0x040*/ UINT64 ThreadLock;						   // Uint8B
	/*0x048*/ struct _KWAIT_STATUS_REGISTER WaitRegister;       // _KWAIT_STATUS_REGISTER
	/*0x049*/ BYTE Running;							   // UChar
	/*0x04a*/ BYTE Alerted[2];						   // [2] UChar

	union
	{
		struct
		{
			/*0x04c*/		DWORD KernelStackResident : 1;     // Pos 0, 1 Bit
			/*0x04c*/		DWORD ReadyTransition : 1;         // Pos 1, 1 Bit
			/*0x04c*/		DWORD ProcessReadyQueue : 1;       // Pos 2, 1 Bit
			/*0x04c*/		DWORD WaitNext : 1;                // Pos 3, 1 Bit
			/*0x04c*/		DWORD SystemAffinityActive : 1;    // Pos 4, 1 Bit
			/*0x04c*/		DWORD Alertable : 1;               // Pos 5, 1 Bit
			/*0x04c*/		DWORD GdiFlushActive : 1;          // Pos 6, 1 Bit
			/*0x04c*/		DWORD UserStackWalkActive : 1;     // Pos 7, 1 Bit
			/*0x04c*/		DWORD ApcInterruptRequest : 1;     // Pos 8, 1 Bit
			/*0x04c*/		DWORD ForceDeferSchedule : 1;      // Pos 9, 1 Bit
			/*0x04c*/		DWORD QuantumEndMigrate : 1;       // Pos 10, 1 Bit
			/*0x04c*/		DWORD UmsDirectedSwitchEnable : 1; // Pos 11, 1 Bit
			/*0x04c*/		DWORD TimerActive : 1;			   // Pos 12, 1 Bit
			/*0x04c*/		DWORD SystemThread : 1;			   // Pos 13, 1 Bit
			/*0x04c*/		DWORD Reserved : 1;				   // Pos 14, 18 Bits
		};
		/*0x04c*/	INT32 MiscFlags;                       // Int4B
	};

	union
	{
		/*0x050*/	struct _KAPC_STATE ApcState;           // _KAPC_STATE
		struct
		{
			/*0x050*/		BYTE ApcStateFill[43];             // [43] UChar
			/*0x07b*/		CHAR Priority;                     // Char
			/*0x07c*/		DWORD NextProcessor;               // Uint4B
		};
	};

	/*0x080*/ DWORD DeferredProcessor;                  // Uint4B
	BYTE _PADDING0_[4];
	/*0x088*/ UINT64 ApcQueueLock;                       // Uint8B
	/*0x090*/ INT64 WaitStatus;                         // Int8B
	/*0x098*/ struct _KWAIT_BLOCK *WaitBlockList;       // Ptr64 _KWAIT_BLOCK

	union
	{
		/*0x0a0*/	LIST_ENTRY WaitListEntry;              // _LIST_ENTRY
		/*0x0a0*/	SINGLE_LIST_ENTRY SwapListEntry;       // _SINGLE_LIST_ENTRY
	};

	/*0x0b0*/ struct _KQUEUE *Queue;                    // Ptr64 _KQUEUE
	/*0x0b8*/ PVOID Teb;                                // Ptr64 Void
	/*0x0c0*/ KTIMER Timer;                             // _KTIMER

	union
	{
		struct
		{
			/*0x100*/		DWORD AutoAlignment : 1;               // Pos 0, 1 Bit
			/*0x100*/		DWORD DisableBoost : 1;                // Pos 1, 1 Bit
			/*0x100*/		DWORD EtwStackTraceApc1Inserted : 1;   // Pos 2, 1 Bit
			/*0x100*/		DWORD EtwStackTraceApc2Inserted : 1;   // Pos 3, 1 Bit
			/*0x100*/		DWORD CalloutActive : 1;               // Pos 4, 1 Bit
			/*0x100*/		DWORD ApcQueueable : 1;                // Pos 5, 1 Bit
			/*0x100*/		DWORD EnableStackSwap : 1;             // Pos 6, 1 Bit
			/*0x100*/		DWORD GuiThread : 1;                   // Pos 7, 1 Bit
			/*0x100*/		DWORD UmsPerformingSyscall : 1;        // Pos 8, 1 Bit
			/*0x100*/		DWORD VdmSafe : 1;                     // Pos 9, 1 Bit
			/*0x100*/		DWORD UmsDispatched : 1;               // Pos 10, 1 Bit
			/*0x100*/		DWORD ReservedFlags : 1;               // Pos 11, 21 Bits
		};
		/*0x100*/	DWORD ThreadFlags;                     // Int4B
	};

	/*0x104*/ DWORD Spare0;                             // Uint4B

	union
	{
		/*0x108*/	struct _KWAIT_BLOCK WaitBlock[4];      // [4] _KWAIT_BLOCK

		struct
		{
			/*0x108*/		BYTE WaitBlockFill4[44];           // [44] UChar
			/*0x134*/		DWORD ContextSwitches;		       // Uint4B
			BYTE _PADDING1_[0x90];
		};

		struct
		{
			/*0x108*/		BYTE WaitBlockFill5[92];	       // [92] UChar
			/*0x164*/		BYTE State;					       // UChar
			/*0x165*/		CHAR NpxState;				       // Char
			/*0x166*/		BYTE WaitIrql;				       // UChar
			/*0x167*/		CHAR WaitMode;				       // Char
			BYTE _PADDING2_[0x60];
		};

		struct
		{
			/*0x108*/		BYTE WaitBlockFill6[140];	       // [140] UChar				
			/*0x194*/		DWORD WaitTime;				       // Uint4B
			BYTE _PADDING3_[0x30];
		};

		struct
		{
			/*0x108*/		BYTE WaitBlockFill7[168];		   // [168] UChar
			/*0x1b0*/		PVOID TebMappedLowVa;			   // Ptr64 Void
			/*0x1b8*/		struct _UMS_CONTROL_BLOCK *Ucb;    // Ptr64 _UMS_CONTROL_BLOCK
			BYTE _PADDING4_[0x8];
		};

		struct
		{
			/*0x108*/		BYTE WaitBlockFill8[188];	       // [188] UChar					
			union
			{
				struct
				{
					/*0x1c4*/				INT16 KernelApcDisable;    // Int2B
					/*0x1c6*/				INT16 SpecialApcDisable;   // Int2B
				};
				/*0x1c4*/			DWORD CombinedApcDisable;	   // Uint4B
			};
		};
	};

	/*0x1c8*/ LIST_ENTRY QueueListEntry;                // _LIST_ENTRY
	/*0x1d8*/ struct _KTRAP_FRAME *TrapFrame;           // Ptr64 _KTRAP_FRAME
	/*0x1e0*/ PVOID FirstArgument;                      // Ptr64 Void

	union
	{
		/*0x1e8*/	PVOID CallbackStack;                   // Ptr64 Void
		/*0x1e8*/	UINT64 CallbackDepth;                   // Uint8B
	};

	/*0x1f0*/ BYTE ApcStateIndex;                       // UChar
	/*0x1f1*/ CHAR BasePriority;                        // Char

	union
	{
		/*0x1f2*/	CHAR PriorityDecrement;                // Char
		struct
		{
			/*0x1f2*/		BYTE ForegroundBoost : 4;          // Pos 0, 4 Bits
			/*0x1f2*/		BYTE UnusualBoost : 4;             // Pos 4, 4 Bits
		};
	};

	/*0x1f3*/ BYTE Preempted;                           // UChar
	/*0x1f4*/ BYTE AdjustReason;                        // UChar
	/*0x1f5*/ CHAR AdjustIncrement;                     // Char
	/*0x1f6*/ CHAR PreviousMode;                        // Char
	/*0x1f7*/ CHAR Saturation;                          // Char
	/*0x1f8*/ DWORD SystemCallNumber;                   // Uint4B
	/*0x1fc*/ DWORD FreezeCount;                        // Uint4B

	/*0x200*/ GROUP_AFFINITY UserAffinity;              // _GROUP_AFFINITY
	/*0x210*/ struct _KPROCESS *Process;                // Ptr64 _KPROCESS
	/*0x218*/ PGROUP_AFFINITY Affinity;                 // _GROUP_AFFINITY

	/*0x228*/ DWORD IdealProcessor;                     // Uint4B
	/*0x22c*/ DWORD UserIdealProcessor;                 // Uint4B
	/*0x230*/ struct _KAPC_STATE *ApcStatePointer[2];   // [2] Ptr64 _KAPC_STATE

	union
	{
		/*0x240*/	struct _KAPC_STATE SavedApcState;      // _KAPC_STATE

		struct
		{
			/*0x240*/		BYTE SavedApcStateFill[43];        // [43] UChar
			/*0x26b*/		BYTE WaitReason;				   // UChar
			/*0x26c*/		CHAR SuspendCount;				   // Char
			/*0x26d*/		CHAR Spare1;					   // Char
			/*0x26e*/		BYTE CodePatchInProgress;          // UChar
			BYTE _PADDING5_[0x1];
		};
	};

	/*0x270*/ PVOID Win32Thread;                        // Ptr64 Void
	/*0x278*/ PVOID StackBase;                          // Ptr64 Void

	union
	{
		/*0x280*/	struct _KAPC SuspendApc;               // _KAPC

		struct
		{
			/*0x280*/		BYTE SuspendApcFill0[1];           // [1] UChar
			/*0x281*/		BYTE ResourceIndex;                // UChar
			BYTE _PADDING6_[0x56];
		};

		struct
		{
			/*0x280*/		BYTE SuspendApcFill1[3];           // [3] UChar
			/*0x283*/		BYTE QuantumReset;                 // UChar
			BYTE _PADDING7_[0x54];
		};

		struct
		{
			/*0x280*/		BYTE SuspendApcFill2[4];           // [4] UChar
			/*0x284*/		DWORD KernelTime;                  // Uint4B
			BYTE _PADDING8_[0x50];
		};

		struct
		{
			/*0x280*/		BYTE SuspendApcFill3[64];          // [64] UChar
			/*0x2c0*/		struct _KPRCB *WaitPrcb;           // Ptr64 _KPRCB
			BYTE _PADDING9_[0x10];
		};

		struct
		{
			/*0x280*/		BYTE SuspendApcFill4[72];          // [72] UChar
			/*0x2c8*/		PVOID LegoData;                    // Ptr64 Void
			BYTE _PADDING10_[0x8];
		};

		struct
		{
			/*0x280*/		BYTE SuspendApcFill5[83];          // [83] UChar
			/*0x2d3*/		BYTE LargeStack;                   // UChar
			/*0x2d4*/		DWORD UserTime;                    // Uint4B
		};
	};

	union {
		/*0x2d8*/	struct _KSEMAPHORE SuspendSemaphore;   // _KSEMAPHORE

		struct
		{
			/*0x2d8*/		BYTE SuspendSemaphorefill[28];     // [28] UChar
			/*0x2f4*/		DWORD SListFaultCount;             // Uint4B
		};
	};

	/*0x2f8*/ LIST_ENTRY ThreadListEntry;               // _LIST_ENTRY
	/*0x308*/ LIST_ENTRY MutantListHead;                // _LIST_ENTRY
	/*0x318*/ PVOID SListFaultAddress;                  // Ptr64 Void
	/*0x320*/ INT64 ReadOperationCount;                 // Int8B
	/*0x328*/ INT64 WriteOperationCount;                // Int8B
	/*0x330*/ INT64 OtherOperationCount;                // Int8B
	/*0x338*/ INT64 ReadTransferCount;                  // Int8B
	/*0x340*/ INT64 WriteTransferCount;                 // Int8B
	/*0x348*/ INT64 OtherTransferCount;                 // Int8B
	/*0x350*/ struct _KTHREAD_COUNTERS *ThreadCounters; // Ptr64 _KTHREAD_COUNTERS
	/*0x358*/ struct _XSAVE_FORMAT *StateSaveArea;      // Ptr64 _XSAVE_FORMAT
	/*0x360*/ struct _XSTATE_SAVE *XStateSave;          // Ptr64 _XSTATE_SAVE

} KTHREAD, *PKTHREAD;

typedef struct _AKTHREAD
{
	UCHAR padding[0x74]; // 0x0
	ULONG state; // 0x74
	UCHAR padding2[0x20]; // 0x98
	KAPC_STATE ApcState;
} AKTHREAD, *PAKTHREAD;

typedef struct _ETHREAD
{
	struct _KTHREAD Tcb;
	LARGE_INTEGER CreateTime;
} _ETHREAD, *_PETHREAD;