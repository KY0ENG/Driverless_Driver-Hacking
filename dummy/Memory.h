#pragma once

#include <ntifs.h>
#include <windef.h>

#include "IOCTL.h"
#include "WinAPI.h"
#include "VadMemory.h"
#include "MemoryStructs.h"

extern PIMAGE_MAP_DATA g_pImageData;
extern PVOID pImageBase;

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);



VOID KernelRoutine( KAPC *Apc,
	PKNORMAL_ROUTINE *NormalRoutine,
	PVOID *NormalContext,
	PVOID *SystemArgument1,
	PVOID *SystemArgument2 );

NTSTATUS copyMemory( PRPM rwm );
NTSTATUS getBase( PHEADER header );
PKTHREAD getAlertableThread( _PEPROCESS pProcess );
NTSTATUS injectAPC( PFUNC pFunc );

// Allocation
NTSTATUS AllocateFreePhysical( IN PEPROCESS pProcess, IN PALLOCATE_FREE_MEMORY pAllocFree, OUT PALLOCATE_FREE_MEMORY_RESULT pResult );
ULONG ConvertProtection( IN ULONG prot, IN BOOLEAN fromPTE );
PMEM_PHYS_PROCESS_ENTRY LookupPhysProcessEntry( IN HANDLE pid );

// Base
PVOID GetKernelBase( OUT PULONG pSize );
NTSTATUS SearchPattern( IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound );
NTSTATUS RtlInsertInvertedFunctionTable( PIMAGE_MAP_DATA pImage );

// Globals
LIST_ENTRY g_PhysProcesses;
