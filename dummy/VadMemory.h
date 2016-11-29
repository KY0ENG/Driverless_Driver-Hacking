#pragma once

#include <ntifs.h>
#include "VadStructs.h"

#ifndef PDE_BASE
#define PDE_BASE    0xFFFFF6FB40000000UI64
#endif
#ifndef PTE_BASE
#define PTE_BASE    0xFFFFF68000000000UI64
#endif

#ifndef PTE_SHIFT
#define PTE_SHIFT 3
#endif
#ifndef PTI_SHIFT
#define PTI_SHIFT 12
#endif
#ifndef PDI_SHIFT
#define PDI_SHIFT 21
#endif
#ifndef PPI_SHIFT
#define PPI_SHIFT 30
#endif
#ifndef PXI_SHIFT
#define PXI_SHIFT 39
#endif

#define GET_VAD_ROOT(Table) Table->BalancedRoot

#define VIRTUAL_ADDRESS_BITS 48
#define VIRTUAL_ADDRESS_MASK ((((ULONG_PTR)1) << VIRTUAL_ADDRESS_BITS) - 1)

#define MiGetPdeAddress(va) \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + PDE_BASE))

#define MiGetPteAddress(va) \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + PTE_BASE))

// Memory Allocations
NTSTATUS ProtectVAD( IN PEPROCESS pProcess, IN ULONG_PTR address, IN ULONG prot );

NTSTATUS FindVAD( IN PEPROCESS pProcess, IN ULONG_PTR address, OUT PMMVAD_SHORT* pResult );

NTSTATUS InitDynamicData( IN OUT PDYNAMIC_DATA pData );

TABLE_SEARCH_RESULT
MiFindNodeOrParent(
	IN PMM_AVL_TABLE Table,
	IN ULONG_PTR StartingVpn,
	OUT PMMADDRESS_NODE *NodeOrParent
);

NTSTATUS LocatePageTables( IN OUT PDYNAMIC_DATA pData );

PMMPTE GetPTEForVA( IN PVOID pAddress );

// Globals
DYNAMIC_DATA dynData;