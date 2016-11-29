#include "VadMemory.h"

NTSTATUS ProtectVAD( IN PEPROCESS pProcess, IN ULONG_PTR address, IN ULONG prot )
{
	NTSTATUS status = STATUS_SUCCESS;
	PMMVAD_SHORT pVad = NULL;

	LARGE_INTEGER x;
	x.QuadPart = -30000000I64; // wait 10 seconds

	DbgPrint( "ProtectVAD!" );
	KeDelayExecutionThread( KernelMode, FALSE, &x );
	status = FindVAD( pProcess, address, &pVad );
	if( !NT_SUCCESS( status ) )
		return status;

	pVad->u.VadFlags.Protection = prot;

	return status;
}

NTSTATUS FindVAD( IN PEPROCESS pProcess, IN ULONG_PTR address, OUT PMMVAD_SHORT* pResult )
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR vpnStart = address >> PAGE_SHIFT;

	LARGE_INTEGER x;
	x.QuadPart = -30000000I64; // wait 10 seconds

	DbgPrint( "FindVAD before assert!" );
	KeDelayExecutionThread( KernelMode, FALSE, &x );
	ASSERT( pProcess != NULL && pResult != NULL );
	if( pProcess == NULL || pResult == NULL )
		return STATUS_INVALID_PARAMETER;

	if( dynData.VadRoot == 0 )
	{
		status = STATUS_INVALID_ADDRESS;
	}


	DbgPrint( "FindVAD pTable!" );
	KeDelayExecutionThread( KernelMode, FALSE, &x );
	PMM_AVL_TABLE pTable	= (PMM_AVL_TABLE)( (PUCHAR)pProcess + dynData.VadRoot );
	PMM_AVL_NODE pNode		= GET_VAD_ROOT( pTable );

	DbgPrint( "FindVAD MiFindNodeOrParent!" );
	KeDelayExecutionThread( KernelMode, FALSE, &x );
	// Search VAD
	if( MiFindNodeOrParent( pTable, vpnStart, &pNode ) == TableFoundNode )
	{
		*pResult = (PMMVAD_SHORT)pNode;
	} else
	{
		status = STATUS_NOT_FOUND;
	}

	return status;
}

/// <summary>
/// Initialize dynamic data.
/// </summary>
/// <param name="pData">Data to initialize</param>
/// <returns>Status code</returns>
NTSTATUS InitDynamicData( IN OUT PDYNAMIC_DATA pData )
{
	NTSTATUS status = STATUS_SUCCESS;
	RTL_OSVERSIONINFOEXW verInfo = { 0 };

	if( pData == NULL )
		return STATUS_INVALID_ADDRESS;

	RtlZeroMemory( pData, sizeof( DYNAMIC_DATA ) );
	pData->DYN_PDE_BASE = PDE_BASE;
	pData->DYN_PTE_BASE = PTE_BASE;

	verInfo.dwOSVersionInfoSize = sizeof( verInfo );
	status = RtlGetVersion( (PRTL_OSVERSIONINFOW)&verInfo );

	if( status == STATUS_SUCCESS )
	{
		ULONG ver_short = ( verInfo.dwMajorVersion << 8 ) | ( verInfo.dwMinorVersion << 4 ) | verInfo.wServicePackMajor;
		pData->ver = (WinVer)ver_short;

		// Validate current driver version
		pData->correctBuild = TRUE;

		if( verInfo.dwBuildNumber == 10586 )
		{
			pData->KExecOpt = 0x1BF;
			pData->Protection = 0x6B2;
			pData->ObjTable = 0x418;
			pData->VadRoot = 0x610;
			pData->NtCreateThdIndex = 0xB4;
			pData->NtTermThdIndex = 0x53;
			pData->PrevMode = 0x232;
			pData->ExitStatus = 0x6E0;
			pData->MiAllocPage = 0;
		} else if( verInfo.dwBuildNumber == 14393 )
		{
			pData->ver = WINVER_10_AU;
			pData->KExecOpt = 0x1BF;
			pData->Protection = 0x6C2;
			pData->ObjTable = 0x418;
			pData->VadRoot = 0x620;
			pData->NtCreateThdIndex = 0xB6;
			pData->NtTermThdIndex = 0x53;
			pData->PrevMode = 0x232;
			pData->ExitStatus = 0x6F0;
			pData->MiAllocPage = 0;

			status = LocatePageTables( pData );
		} else
		{
			return STATUS_NOT_SUPPORTED;
		}

		if( pData->ExRemoveTable != 0 )
			pData->correctBuild = TRUE;

		return ( pData->VadRoot != 0 ? status : STATUS_INVALID_KERNEL_INFO_VERSION );
	}

	return status;
}

TABLE_SEARCH_RESULT
MiFindNodeOrParent(
	IN PMM_AVL_TABLE Table,
	IN ULONG_PTR StartingVpn,
	OUT PMMADDRESS_NODE *NodeOrParent
)

/*++
Routine Description:
This routine is used by all of the routines of the generic
table package to locate the a node in the tree.  It will
find and return (via the NodeOrParent parameter) the node
with the given key, or if that node is not in the tree it
will return (via the NodeOrParent parameter) a pointer to
the parent.
Arguments:
Table - The generic table to search for the key.
StartingVpn - The starting virtual page number.
NodeOrParent - Will be set to point to the node containing the
the key or what should be the parent of the node
if it were in the tree.  Note that this will *NOT*
be set if the search result is TableEmptyTree.
Return Value:
TABLE_SEARCH_RESULT - TableEmptyTree: The tree was empty.  NodeOrParent
is *not* altered.
TableFoundNode: A node with the key is in the tree.
NodeOrParent points to that node.
TableInsertAsLeft: Node with key was not found.
NodeOrParent points to what would
be parent.  The node would be the
left child.
TableInsertAsRight: Node with key was not found.
NodeOrParent points to what would
be parent.  The node would be
the right child.
Environment:
Kernel mode.  The PFN lock is held for some of the tables.
--*/

{
	PMMADDRESS_NODE Child;
	PMMADDRESS_NODE NodeToExamine;
	PMMVAD_SHORT    VpnCompare;
	ULONG_PTR       startVpn;
	ULONG_PTR       endVpn;

	if( Table->NumberGenericTableElements == 0 ) {
		return TableEmptyTree;
	}

	NodeToExamine = (PMMADDRESS_NODE)GET_VAD_ROOT( Table );

	for( ;;) {

		VpnCompare = (PMMVAD_SHORT)NodeToExamine;
		startVpn = VpnCompare->StartingVpn;
		endVpn = VpnCompare->EndingVpn;


		startVpn |= (ULONG_PTR)VpnCompare->StartingVpnHigh << 32;
		endVpn |= (ULONG_PTR)VpnCompare->EndingVpnHigh << 32;


		//
		// Compare the buffer with the key in the tree element.
		//

		if( StartingVpn < startVpn ) {

			Child = NodeToExamine->LeftChild;

			if( Child != NULL ) {
				NodeToExamine = Child;
			} else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsLeft;
			}
		} else if( StartingVpn <= endVpn ) {

			//
			// This is the node.
			//

			*NodeOrParent = NodeToExamine;
			return TableFoundNode;
		} else {

			Child = NodeToExamine->RightChild;

			if( Child != NULL ) {
				NodeToExamine = Child;
			} else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsRight;
			}
		}

	};
}

/// <summary>
/// Get relocated PTE and PDE bases
/// </summary>
/// <param name="pData">Dynamic data</param>
/// <returns>Status code</returns>
NTSTATUS LocatePageTables( IN OUT PDYNAMIC_DATA pData )
{
	UNICODE_STRING uName = RTL_CONSTANT_STRING( L"MmGetPhysicalAddress" );
	PUCHAR pMmGetPhysicalAddress = MmGetSystemRoutineAddress( &uName );
	if( pMmGetPhysicalAddress )
	{
		PUCHAR pMiGetPhysicalAddress = *(PLONG)( pMmGetPhysicalAddress + 0xE + 1 ) + pMmGetPhysicalAddress + 0xE + 5;
		pData->DYN_PDE_BASE = *(PULONG_PTR)( pMiGetPhysicalAddress + 0x49 + 2 );
		pData->DYN_PTE_BASE = *(PULONG_PTR)( pMiGetPhysicalAddress + 0x56 + 2 );
		return STATUS_SUCCESS;
	}

	return STATUS_NOT_FOUND;
}

/// <summary>
/// Get page hardware PTE.
/// Address must be valid, otherwise bug check is imminent
/// </summary>
/// <param name="pAddress">Target address</param>
/// <returns>Found PTE</returns>
PMMPTE GetPTEForVA( IN PVOID pAddress )
{
	if( dynData.ver == WINVER_10_AU )
	{
		// Check if large page
		PMMPTE pPDE = (PMMPTE)( ( ( ( (ULONG_PTR)pAddress >> PDI_SHIFT ) << PTE_SHIFT ) & 0x3FFFFFF8ull ) + dynData.DYN_PDE_BASE );
		if( pPDE->u.Hard.LargePage )
			return pPDE;

		return (PMMPTE)( ( ( ( (ULONG_PTR)pAddress >> PTI_SHIFT ) << PTE_SHIFT ) & 0x7FFFFFFFF8ull ) + dynData.DYN_PTE_BASE );
	} else
	{
		// Check if large page
		PMMPTE pPDE = MiGetPdeAddress( pAddress );
		if( pPDE->u.Hard.LargePage )
			return pPDE;

		return MiGetPteAddress( pAddress );
	}
}

