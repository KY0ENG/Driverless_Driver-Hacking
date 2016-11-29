#pragma once

#include <ntifs.h>
#include "NtStructs.h"

#define TOP_ETHREAD 0x2f8
#define ALERTABLE(x) ( ( x & 0x10 ) > 0 )

NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
	IN PEPROCESS P1,
	IN PVOID A1,
	IN PEPROCESS P2,
	OUT PVOID A2,
	IN SIZE_T BSize,
	IN KPROCESSOR_MODE Mode,
	OUT PSIZE_T NumberOfBytesCopied
);

NTKERNELAPI
NTSTATUS
IoCreateDriver(
	IN PUNICODE_STRING DriverName, OPTIONAL
	IN PDRIVER_INITIALIZE InitializationFunction
);

NTKERNELAPI
PVOID
PsGetProcessSectionBaseAddress( IN PEPROCESS peProcess );

NTKERNELAPI
NTSTATUS
#pragma warning(suppress: 28252)
ZwAllocateVirtualMemory(
	_In_    HANDLE ProcessHandle,
	_Inout_ PVOID *BaseAddress,
	_In_    ULONG_PTR ZeroBits,
	_Inout_ PSIZE_T RegionSize,
	_In_    ULONG AllocationType,
	_In_    ULONG Protect
	);

NTKERNELAPI 
VOID 
KeInitializeApc(
	PKAPC Apc,
	PKTHREAD Thread,
	KAPC_ENVIRONMENT Environment,
	PKKERNEL_ROUTINE KernelRoutine,
	PKRUNDOWN_ROUTINE RundownRoutine,
	PKNORMAL_ROUTINE NormalRoutine,
	KPROCESSOR_MODE ProcessorMode,
	PVOID NormalContext
	);

BOOLEAN
NTAPI
KeInsertQueueApc( 
	IN PKAPC Apc,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2,
	IN KPRIORITY PriorityBoost 
	);