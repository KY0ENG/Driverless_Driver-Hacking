#pragma once

#include <ntifs.h>

#include "IOCTL.h"
#include "Memory.h"


NTSYSAPI
PMMIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader( PVOID Base );

#pragma warning( disable : 4201 4311 )

const WCHAR g_drvName[]					= L"\\Driver\\DAB7";
const WCHAR g_deviceNameBuffer[]		= L"\\Device\\DAB7";
const WCHAR g_deviceSymLinkBuffer[]		= L"\\DosDevices\\DAB7";

PIMAGE_MAP_DATA g_pImageData;

struct _DRIVER_OBJECT* g_pDriverObject	= NULL;

_Dispatch_type_( IRP_MJ_CREATE );
DRIVER_DISPATCH  Function_IRP_MJ_OTHERS;
_Dispatch_type_(IRP_MJ_CLOSE);
DRIVER_DISPATCH  Function_IRP_MJ_UNLOAD;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL);
DRIVER_DISPATCH  Function_IRP_DEVICE_CONTROL;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text( INIT, DriverEntry )