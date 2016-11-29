
#include "Memory.h"

ULONG MmProtectToValue[ 32 ] =
{
	PAGE_NOACCESS,
	PAGE_READONLY,
	PAGE_EXECUTE,
	PAGE_EXECUTE_READ,
	PAGE_READWRITE,
	PAGE_WRITECOPY,
	PAGE_EXECUTE_READWRITE,
	PAGE_EXECUTE_WRITECOPY,
	PAGE_NOACCESS,
	PAGE_NOCACHE | PAGE_READONLY,
	PAGE_NOCACHE | PAGE_EXECUTE,
	PAGE_NOCACHE | PAGE_EXECUTE_READ,
	PAGE_NOCACHE | PAGE_READWRITE,
	PAGE_NOCACHE | PAGE_WRITECOPY,
	PAGE_NOCACHE | PAGE_EXECUTE_READWRITE,
	PAGE_NOCACHE | PAGE_EXECUTE_WRITECOPY,
	PAGE_NOACCESS,
	PAGE_GUARD | PAGE_READONLY,
	PAGE_GUARD | PAGE_EXECUTE,
	PAGE_GUARD | PAGE_EXECUTE_READ,
	PAGE_GUARD | PAGE_READWRITE,
	PAGE_GUARD | PAGE_WRITECOPY,
	PAGE_GUARD | PAGE_EXECUTE_READWRITE,
	PAGE_GUARD | PAGE_EXECUTE_WRITECOPY,
	PAGE_NOACCESS,
	PAGE_WRITECOMBINE | PAGE_READONLY,
	PAGE_WRITECOMBINE | PAGE_EXECUTE,
	PAGE_WRITECOMBINE | PAGE_EXECUTE_READ,
	PAGE_WRITECOMBINE | PAGE_READWRITE,
	PAGE_WRITECOMBINE | PAGE_WRITECOPY,
	PAGE_WRITECOMBINE | PAGE_EXECUTE_READWRITE,
	PAGE_WRITECOMBINE | PAGE_EXECUTE_WRITECOPY
};

VOID KernelRoutine( KAPC *Apc,
	PKNORMAL_ROUTINE *NormalRoutine,
	PVOID *NormalContext,
	PVOID *SystemArgument1,
	PVOID *SystemArgument2 )
{
	UNREFERENCED_PARAMETER( NormalRoutine );
	UNREFERENCED_PARAMETER( NormalContext );
	UNREFERENCED_PARAMETER( SystemArgument1 );
	UNREFERENCED_PARAMETER( SystemArgument2 );

	PsWrapApcWow64Thread( NormalContext, (PVOID*)NormalRoutine );
	ExFreePoolWithTag( Apc, 'ExAP' );

	//Function_IRP_MJ_UNLOAD( g_pDriverObject->DeviceObject, g_pDriverObject->DeviceObject->CurrentIrp );
}

NTSTATUS copyMemory( PRPM rwm )
{
	PEPROCESS					targetProc = NULL;
	NTSTATUS status;

	status = PsLookupProcessByProcessId( (HANDLE)rwm->pID, &targetProc );
	if( NT_SUCCESS( status ) )
	{
		SIZE_T bytes;
		if( rwm->write )
		{
			if( rwm->dAddress && rwm->topPtr )
				status = MmCopyVirtualMemory( PsGetCurrentProcess(), (PVOID)rwm->topPtr, targetProc, (PVOID)rwm->dAddress, rwm->size, KernelMode, &bytes );
		} else {
			if( rwm->dAddress && rwm->size > 0 )
			{
				status = MmCopyVirtualMemory( targetProc, (PVOID)rwm->dAddress, PsGetCurrentProcess(), (PVOID)rwm->lowPtr, rwm->size, KernelMode, &bytes );
			}

		}
	}

	if( targetProc )
		ObDereferenceObject( targetProc );

	return status;
}

NTSTATUS getBase( PHEADER header )
{
	NTSTATUS status;
	PEPROCESS					targetProc = NULL;
	PVOID						value;

	status = PsLookupProcessByProcessId( (HANDLE)header->pID, &targetProc );
	if( NT_SUCCESS( status ) )
	{
		KAPC_STATE	apc;
		KeStackAttachProcess( targetProc, &apc );

		value = PsGetProcessSectionBaseAddress( targetProc );
		header->imageBase = (ULONG)(value);

		KeUnstackDetachProcess( &apc );
	}

	return status;
}

PKTHREAD getAlertableThread( _PEPROCESS pProcess )
{
	_PETHREAD pThread = NULL;
	PLIST_ENTRY pHead = NULL, pCurrent = NULL;

	if( !pProcess )
		return NULL;

	pHead = &( pProcess->Pcb.ThreadListHead );
	pThread = (_PETHREAD)( ( (PUCHAR)( pHead->Flink ) ) - TOP_ETHREAD );
	pHead = &( pThread->Tcb.ThreadListEntry );

	pCurrent = pHead;
	do {
		ULONG threadState = ( (PAKTHREAD)pThread )->state;

		if( ALERTABLE( threadState ) )
			return (PETHREAD)pThread;

		pCurrent = pCurrent->Flink;
		pThread = (_PETHREAD)( ( (PUCHAR)( pCurrent ) ) - TOP_ETHREAD );

	} while( pCurrent != pHead );

	return NULL;
}

NTSTATUS injectAPC( PFUNC pFunc )
{
	PEPROCESS	pProcess = NULL; PETHREAD pThread = NULL;
	PVOID baseAddress = NULL; SIZE_T regionSize = 16384;
	KAPC_STATE	apc_state; PKAPC pApc;

	NTSTATUS status;

	if( pFunc == NULL || pFunc->pID == 0 || pFunc->pAssemblyBuffer == 0 )
		return STATUS_INVALID_PARAMETER;

	status = PsLookupProcessByProcessId( (HANDLE)pFunc->pID, &pProcess );
	if( NT_SUCCESS( status ) )
	{
		pThread = getAlertableThread( (_PEPROCESS)pProcess );
		if( pThread == NULL )
		{
			return STATUS_UNSUCCESSFUL;
		}

		/* Alternative */
		
		KeStackAttachProcess( (PRKPROCESS)pProcess, &apc_state );

		#pragma warning(suppress: 30030)
		status = ZwAllocateVirtualMemory( ZwCurrentProcess(), &baseAddress, (ULONG_PTR)NULL, &regionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
		if( !NT_SUCCESS( status ) )
		{
			KeUnstackDetachProcess( &apc_state );
			return STATUS_UNSUCCESSFUL;
		}
		
		KeUnstackDetachProcess( &apc_state );
		

		
		ALLOCATE_FREE_MEMORY		request			= { 0 };
		ALLOCATE_FREE_MEMORY_RESULT mapResult		= { 0 };

		request.allocate		= TRUE;
		request.physical		= TRUE;
		request.protection		= PAGE_EXECUTE_READWRITE;
		request.size			= pFunc->AssemblyLength;

		status = AllocateFreePhysical( pProcess, &request, &mapResult );
		if( !NT_SUCCESS( status ) )
			return status;

		DbgPrint( "Im here!" );
		return status;
		

		//SIZE_T bytes;
		//status = MmCopyVirtualMemory( PsGetCurrentProcess(), (PVOID)pFunc->pAssemblyBuffer, pProcess, (PVOID)mapResult.address, pFunc->AssemblyLength, KernelMode, &bytes );
		

		SIZE_T bytes;
		status = MmCopyVirtualMemory( PsGetCurrentProcess(), (PVOID)pFunc->pAssemblyBuffer, pProcess, (PVOID)baseAddress, pFunc->AssemblyLength, KernelMode, &bytes );

		#pragma warning(suppress: 28197) // Leaking memory
		pApc = (PKAPC)ExAllocatePoolWithTag( NonPagedPool, sizeof( KAPC ), 'ExAP' );
		if( !pApc )
		{
			//ExFreePoolWithTag( (PVOID)mapResult.address, 'ExAP' );

			
			KeStackAttachProcess( pProcess, &apc_state );
			ZwFreeVirtualMemory( ZwCurrentProcess(), &baseAddress, &regionSize, MEM_RELEASE );
			KeUnstackDetachProcess( &apc_state );
			

			return STATUS_UNSUCCESSFUL;
		}

		RtlZeroMemory( pApc, sizeof( KAPC ) );
		KeInitializeApc( pApc, (PKTHREAD)pThread, OriginalApcEnvironment, (PKKERNEL_ROUTINE)KernelRoutine, NULL, (PKNORMAL_ROUTINE)(ULONG_PTR)baseAddress, UserMode, NULL );
		( (PAKTHREAD)pThread )->ApcState.UserApcPending = TRUE;

		if( !KeInsertQueueApc( pApc, 0, 0, 0 ) )
		{
			//ExFreePool( pApc );
			//ExFreePoolWithTag( (PVOID)mapResult.address, 'ExAP' );

			KeStackAttachProcess( pProcess, &apc_state );
			ZwFreeVirtualMemory( ZwCurrentProcess(), &baseAddress, &regionSize, MEM_RELEASE );
			KeUnstackDetachProcess( &apc_state );

			return STATUS_UNSUCCESSFUL;
		}

		
		LARGE_INTEGER x;
		x.QuadPart = -50000000I64; // wait 10 seconds

		KeDelayExecutionThread( KernelMode, FALSE, &x );
		
		KeStackAttachProcess( pProcess, &apc_state );
		ZwFreeVirtualMemory( ZwCurrentProcess(), &baseAddress, &regionSize, MEM_RELEASE );
		KeUnstackDetachProcess( &apc_state );

		return STATUS_SUCCESS;
	}

	return STATUS_UNSUCCESSFUL;
}

NTSTATUS AllocateFreePhysical( IN PEPROCESS pProcess, IN PALLOCATE_FREE_MEMORY pAllocFree, OUT PALLOCATE_FREE_MEMORY_RESULT pResult )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pRegionBase = NULL;
	PMDL pMDL = NULL;
	LARGE_INTEGER x;
	x.QuadPart = -30000000I64; // wait 10 seconds
	DbgPrint( "Start of func!" );

	KeDelayExecutionThread( KernelMode, FALSE, &x );
	if( !NT_SUCCESS( RtlInsertInvertedFunctionTable( g_pImageData ) ) )
	{
		DbgPrint( "Failed to insert!" );
		return STATUS_UNSUCCESSFUL;
	} else {
		DbgPrint( "SUCCESSFULLY INSERTED!" );
		//return STATUS_SUCCESS;
	}

	if( pProcess == NULL || pResult == NULL )
		return STATUS_INVALID_PARAMETER;

	if( pAllocFree->size > 0xFFFFFFFF )
		return STATUS_INVALID_PARAMETER;

	pAllocFree->base = (ULONGLONG)PAGE_ALIGN( pAllocFree->base );
	pAllocFree->size = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pAllocFree->base, pAllocFree->size ) << PAGE_SHIFT;

	if( pAllocFree->allocate != FALSE )
	{
		KeDelayExecutionThread( KernelMode, FALSE, &x );
		DbgPrint( "1st FindVAD" );
		PMMVAD_SHORT pVad = NULL;
		if( pAllocFree->base != 0 && FindVAD( pProcess, pAllocFree->base, &pVad ) != STATUS_NOT_FOUND )
			return STATUS_ALREADY_COMMITTED;

		KeDelayExecutionThread( KernelMode, FALSE, &x );
		DbgPrint( "1st ExAllocatePoolWithTag" );

		pRegionBase = ExAllocatePoolWithTag( NonPagedPool, pAllocFree->size, 'ExAP' );
		if( !pRegionBase )
			return STATUS_NO_MEMORY;

		RtlZeroMemory( pRegionBase, pAllocFree->size );

		pMDL = IoAllocateMdl( pRegionBase, (ULONG)pAllocFree->size, FALSE, FALSE, NULL );
		if( pMDL == NULL )
		{
			ExFreePoolWithTag( pRegionBase, 'ExAP' );
			return STATUS_NO_MEMORY;
		}

		KeDelayExecutionThread( KernelMode, FALSE, &x );
		DbgPrint( "MmBuildMdlForNonPagedPool" );
		MmBuildMdlForNonPagedPool( pMDL );

		__try {
			pResult->address = (ULONGLONG)MmMapLockedPagesSpecifyCache(
				pMDL, UserMode, MmCached, (PVOID)pAllocFree->base, FALSE, NormalPagePriority
			);
		} __except( EXCEPTION_EXECUTE_HANDLER ) { }

		KeDelayExecutionThread( KernelMode, FALSE, &x );
		DbgPrint( "MmBuildMdlForNonPagedPool secound" );

		if( pResult->address == 0 && pAllocFree->base != 0 )
		{
			__try {
			pResult->address = (ULONGLONG)MmMapLockedPagesSpecifyCache(
				pMDL, UserMode, MmCached, NULL, FALSE, NormalPagePriority
			);
			} __except( EXCEPTION_EXECUTE_HANDLER ) { }
		}

		if( pResult->address )
		{
			PMEM_PHYS_PROCESS_ENTRY pEntry	= NULL;
			PMEM_PHYS_ENTRY pMemEntry		= NULL;

			DbgPrint( "ProtectVAD" );
			KeDelayExecutionThread( KernelMode, FALSE, &x );
			
			pResult->size = pAllocFree->size;

			__try {
				ProtectVAD( PsGetCurrentProcess(), pResult->address, ConvertProtection( pAllocFree->protection, FALSE ) );
			} __except( EXCEPTION_EXECUTE_HANDLER ) { }

			return STATUS_UNSUPPORTED_PREAUTH;

			// Make pages executable
			if( pAllocFree->protection & ( PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE ) )
			{
				DbgPrint( "1st GetPTEForVA" );
				KeDelayExecutionThread( KernelMode, FALSE, &x );
				
				for( ULONG_PTR pAdress = pResult->address; pAdress < pResult->address + pResult->size; pAdress += PAGE_SIZE )
					GetPTEForVA( (PVOID)pAdress )->u.Hard.NoExecute = 0;
			}

			DbgPrint( "1st LookupPhysProcessEntry" );
			KeDelayExecutionThread( KernelMode, FALSE, &x );
			
			// Add to list
			pEntry = LookupPhysProcessEntry( (HANDLE)pAllocFree->pid );
			if( pEntry == NULL )
			{
				pEntry = ExAllocatePoolWithTag( PagedPool, sizeof( MEM_PHYS_PROCESS_ENTRY ), 'ExAP' );
				pEntry->pid = (HANDLE)pAllocFree->pid;

				InitializeListHead( &pEntry->pVadList );
				InsertTailList( &g_PhysProcesses, &pEntry->link );
			}

			pMemEntry = ExAllocatePoolWithTag( PagedPool, sizeof( MEM_PHYS_ENTRY ), 'ExAP' );

			pMemEntry->pMapped = (PVOID)pResult->address;
			pMemEntry->pMDL = pMDL;
			pMemEntry->ptr = pRegionBase;
			pMemEntry->size = pAllocFree->size;

			InsertTailList( &pEntry->pVadList, &pMemEntry->link );
		} else
		{
			// Failed, cleanup
			IoFreeMdl( pMDL );
			ExFreePoolWithTag( pRegionBase, 'ExAP' );

			status = STATUS_NONE_MAPPED;
		}

	}

	return status;
}

/// <summary>
/// Convert protection flags
/// </summary>
/// <param name="prot">Protection flags.</param>
/// <param name="fromPTE">If TRUE - convert to PTE protection, if FALSE - convert to Win32 protection</param>
/// <returns>Resulting protection flags</returns>
ULONG ConvertProtection( IN ULONG prot, IN BOOLEAN fromPTE )
{
	LARGE_INTEGER x;
	x.QuadPart = -30000000I64; // wait 10 seconds

	DbgPrint( "ConvertProtection!" );
	KeDelayExecutionThread( KernelMode, FALSE, &x );
	if( fromPTE != FALSE )
	{
		// Sanity check
		if( prot < ARRAYSIZE( MmProtectToValue ) )
			return MmProtectToValue[ prot ];
	} else
	{
		for( int i = 0; i < ARRAYSIZE( MmProtectToValue ); i++ )
			if( MmProtectToValue[ i ] == prot )
				return i;
	}

	return 0;
}


/// <summary>
/// Find memory allocation process entry
/// </summary>
/// <param name="pid">Target PID</param>
/// <returns>Found entry, NULL if not found</returns>
PMEM_PHYS_PROCESS_ENTRY LookupPhysProcessEntry( IN HANDLE pid )
{
	for( PLIST_ENTRY pListEntry = g_PhysProcesses.Flink; pListEntry != &g_PhysProcesses; pListEntry = pListEntry->Flink )
	{
		PMEM_PHYS_PROCESS_ENTRY pEntry = CONTAINING_RECORD( pListEntry, MEM_PHYS_PROCESS_ENTRY, link );
		if( pEntry->pid == pid )
			return pEntry;
	}

	return NULL;
}



/// <summary>
/// Get ntoskrnl base address
/// </summary>
/// <param name="pSize">Size of module</param>
/// <returns>Found address, NULL if not found</returns>
PVOID GetKernelBase( OUT PULONG pSize )
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG bytes = 0;
	PRTL_PROCESS_MODULES pMods = NULL;
	PVOID checkPtr = NULL;
	UNICODE_STRING routineName;

	PVOID g_KernelBase = NULL;
	ULONG g_KernelSize = 0;


	RtlInitUnicodeString( &routineName, L"NtOpenFile" );

	checkPtr = MmGetSystemRoutineAddress( &routineName );
	if( checkPtr == NULL )
		return NULL;

	// Protect from UserMode AV
	status = ZwQuerySystemInformation( SystemModuleInformation, 0, bytes, &bytes );
	if( bytes == 0 )
	{
		return NULL;
	}

	pMods = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag( NonPagedPool, bytes, 'ExAP' );
	RtlZeroMemory( pMods, bytes );

	status = ZwQuerySystemInformation( SystemModuleInformation, pMods, bytes, &bytes );

	if( NT_SUCCESS( status ) )
	{
		PRTL_PROCESS_MODULE_INFORMATION pMod = pMods->Modules;

		for( ULONG i = 0; i < pMods->NumberOfModules; i++ )
		{
			// System routine is inside module
			if( checkPtr >= pMod[ i ].ImageBase &&
				checkPtr < (PVOID)( (PUCHAR)pMod[ i ].ImageBase + pMod[ i ].ImageSize ) )
			{
				g_KernelBase = pMod[ i ].ImageBase;
				g_KernelSize = pMod[ i ].ImageSize;
				if( pSize )
					*pSize = g_KernelSize;
				break;
			}
		}
	}

	if( pMods )
		ExFreePoolWithTag( pMods, 'ExAP' );

	return g_KernelBase;
}

NTSTATUS SearchPattern( IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound )
{
	ASSERT( ppFound != NULL && pattern != NULL && base != NULL );
	if( ppFound == NULL || pattern == NULL || base == NULL )
		return STATUS_INVALID_PARAMETER;

	for( ULONG_PTR i = 0; i < size - len; i++ )
	{
		BOOLEAN found = TRUE;
		for( ULONG_PTR j = 0; j < len; j++ )
		{
			if( pattern[ j ] != wildcard && pattern[ j ] != ( (PCUCHAR)base )[ i + j ] )
			{
				found = FALSE;
				break;
			}
		}

		if( found != FALSE )
		{
			*ppFound = (PUCHAR)base + i;
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

NTSTATUS RtlInsertInvertedFunctionTable( PIMAGE_MAP_DATA pImage )
{
	PVOID pattern_addr			= NULL, kernel_base = NULL;
	ULONG kernelSize			= 0;
	NTSTATUS status				= STATUS_UNSUCCESSFUL;

	UCHAR pattern[]				= "\x48\x89\x5C\x24\xCC\x55\x56\x57\x48\x83\xEC\x30\x8B\xF2";

	kernel_base					= GetKernelBase( &kernelSize );

	if( NT_SUCCESS( SearchPattern( pattern, 0xCC, sizeof( pattern ) - 1, kernel_base, (ULONG_PTR)kernelSize, &pattern_addr ) ) )
	{
		typedef NTSTATUS( NTAPI* RtlInsertInvertedFunctionTable )( PVOID, SIZE_T );
		RtlInsertInvertedFunctionTable RtlInsertInvertedFunctionTableT = (RtlInsertInvertedFunctionTable)pattern_addr;

		status = RtlInsertInvertedFunctionTableT(
			(PVOID)pImage->ImageBase,
			pImage->SizeOfImage );

		return status;
	}

	return STATUS_UNSUCCESSFUL;
	
}