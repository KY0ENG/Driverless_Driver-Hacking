#include "main.h"

NTSTATUS Function_IRP_MJ_OTHERS(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS Function_IRP_MJ_UNLOAD( PDEVICE_OBJECT pDeviceObject, PIRP Irp )
{
	UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( pDeviceObject );

	UNICODE_STRING symbLink; RtlInitUnicodeString( &symbLink, g_deviceSymLinkBuffer );

 	IoDeleteSymbolicLink( &symbLink );
 	IoDeleteDevice( pDeviceObject );

	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

NTSTATUS Function_IRP_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER( pDeviceObject );

	ULONG						bytesIO = 0;
	PIO_STACK_LOCATION			pIoStackLocation;
	NTSTATUS					status				= STATUS_SUCCESS;
	PVOID						pBuf				= Irp->AssociatedIrp.SystemBuffer;
	ULONG						outputBufferLength;

	pIoStackLocation	= IoGetCurrentIrpStackLocation(Irp);
	outputBufferLength	= pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	do {

		if ( pIoStackLocation == NULL || pBuf == NULL )
		{
			status = STATUS_INTERNAL_ERROR;
			break;
		}

		switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode) {
			case IOCTL_RPM:
			{
				bytesIO = 0;
				if ( (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (ULONG)sizeof(RPM) )
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				PRPM pData = pBuf;

				status = copyMemory(pData);
				bytesIO = sizeof(RPM);

				break;
			}
			case IOCTL_GETBASE:
			{
				if ( (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (ULONG)sizeof(HEADER) ) 
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				// GET PROCESS HANDLE
				bytesIO = sizeof( HEADER );

				PHEADER pData = pBuf;
				status = getBase( pData );

				break;
			}
			case IOCTL_EXEC:
			{
				if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (ULONG)sizeof( FUNC ))
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				// GET PROCESS HANDLE
				bytesIO = 0;
				status = injectAPC( (PFUNC)pBuf );

				break;
			}
			default:
			{
				status = STATUS_INVALID_MESSAGE;
			}
		}

	} while (FALSE);

	Irp->IoStatus.Information	= bytesIO;
	Irp->IoStatus.Status		= status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}


NTSTATUS DriverInitialize(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING						deviceNameUnicodeString, deviceSymLinkUnicodeString;
	NTSTATUS							status;

	PDEVICE_OBJECT						devobj;

	// INITILISING STRINGS INSIDE THE DRIVER
	RtlInitUnicodeString(&deviceNameUnicodeString, g_deviceNameBuffer);
	RtlInitUnicodeString(&deviceSymLinkUnicodeString, g_deviceSymLinkBuffer);

	status = IoCreateDevice(DriverObject,
		0,
		&deviceNameUnicodeString,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&devobj);

	g_pDriverObject = DriverObject;
	if (!NT_SUCCESS(status)) {
		return status;
	}

	devobj->Flags |= DO_BUFFERED_IO;

	status = IoCreateSymbolicLink(&deviceSymLinkUnicodeString, &deviceNameUnicodeString);

	DriverObject->MajorFunction[IRP_MJ_CREATE]			= &Function_IRP_MJ_OTHERS;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]			= &Function_IRP_MJ_UNLOAD;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= &Function_IRP_DEVICE_CONTROL;

	devobj->Flags &= ~DO_DEVICE_INITIALIZING;

	return status;
}


NTSTATUS DriverEntry(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	g_pImageData					= (PIMAGE_MAP_DATA)DriverObject;
	PMMIMAGE_NT_HEADERS pNTHeader	= NULL;
	

	//pNTHeader = RtlImageNtHeader( (PVOID)g_pImageData->ImageBase );
	//if( pNTHeader )
	//{
	//	RtlZeroMemory( (PVOID)g_pImageData->ImageBase, pNTHeader->OptionalHeader.SizeOfHeaders );
	//}
	
	UNICODE_STRING  drvName;

	RtlInitUnicodeString( &drvName, g_drvName );
	NTSTATUS status = IoCreateDriver( &drvName, &DriverInitialize );

	return status;
}

/*
NTSTATUS injectAPC( PFUNC pFunc )
{
PEPROCESS	pProcess = NULL; PETHREAD pThread = NULL;
HANDLE		hProc = NULL; PVOID baseAddress = NULL; SIZE_T regionSize = 16384;

CLIENT_ID	client_id; OBJECT_ATTRIBUTES objAttr;
KAPC_STATE	apc_state; PKAPC pApc;

NTSTATUS status;

if( pFunc == NULL || pFunc->pID == 0 || pFunc->pAssemblyBuffer == 0 )
return STATUS_INVALID_PARAMETER;

status = PsLookupProcessByProcessId( (HANDLE)pFunc->pID, &pProcess );
if (NT_SUCCESS( status ))
{
pThread = getAlertableThread( (_PEPROCESS)pProcess );
if (pThread == NULL)
{
DbgPrint( "Didn't find any alertable threads!" );
return STATUS_UNSUCCESSFUL;
}

DbgPrint( "Alertable thread found: %p", pThread );

client_id.UniqueProcess	= (HANDLE)pFunc->pID;
client_id.UniqueThread	= PsGetThreadId( pThread );

objAttr.Length			= sizeof( OBJECT_ATTRIBUTES );
objAttr.RootDirectory	= NULL;
objAttr.Attributes		= OBJ_KERNEL_HANDLE;
objAttr.ObjectName		= NULL;
objAttr.SecurityDescriptor			= NULL;
objAttr.SecurityQualityOfService	= NULL;

status = ZwOpenProcess( &hProc, GENERIC_ALL, &objAttr, &client_id );
if (!NT_SUCCESS( status ))
{
DbgPrint( "ZwOpenProcess failed, error: %d", status );
return STATUS_UNSUCCESSFUL;
}

#pragma warning(suppress: 30030)
status = ZwAllocateVirtualMemory( hProc, &baseAddress, (ULONG_PTR)NULL, &regionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
if (!NT_SUCCESS( status ))
{
ZwClose( hProc );
DbgPrint( "ZwAllocateVirtualMemory failed, error: %d", status );
return STATUS_UNSUCCESSFUL;
}
DbgPrint( "Memory allocated successfully, size: %ul", (ULONG)regionSize );

SIZE_T bytes;
status = MmCopyVirtualMemory( PsGetCurrentProcess(), (PVOID)pFunc->pAssemblyBuffer, pProcess, baseAddress, pFunc->AssemblyLength, KernelMode, &bytes );

#pragma warning(suppress: 28197) // Leaking memory
pApc = (PKAPC)ExAllocatePool( NonPagedPool, sizeof( KAPC ) );
if ( !pApc )
{
DbgPrint( "APC Memory allocation failed, error" );
ZwFreeVirtualMemory( hProc, &baseAddress, &regionSize, MEM_RELEASE );
ZwClose( hProc );
return STATUS_UNSUCCESSFUL;
}

DbgPrint( "Address of allocated memory: %p", baseAddress );
RtlZeroMemory( pApc, sizeof( KAPC ) );
KeInitializeApc( pApc, (PKTHREAD)pThread, OriginalApcEnvironment, (PKKERNEL_ROUTINE)KernelRoutine, NULL, (PKNORMAL_ROUTINE)(ULONG_PTR)baseAddress, UserMode, NULL );

( (PAKTHREAD)pThread )->ApcState.UserApcPending = TRUE;

if (!KeInsertQueueApc( pApc, 0, 0, 0 ) )
{
DbgPrint( "Failed to InsertAPC into Que, error" );

ExFreePool( pApc );
ZwFreeVirtualMemory( hProc, &baseAddress, &regionSize, MEM_RELEASE );
ZwClose( hProc );
return STATUS_UNSUCCESSFUL;
}

DbgPrint( "Success, deallocation memory" );
//ZwFreeVirtualMemory( hProc, &baseAddress, &regionSize, MEM_RELEASE );
ZwClose( hProc );
return STATUS_SUCCESS;

} else {
DbgPrint( "Failed to get process by it's ID: %ul", pFunc->pID );
}

return STATUS_UNSUCCESSFUL;
}
*/