#pragma once

#include <ntifs.h>

#define IOCTL_GETBASE	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_RPM		CTL_CODE( FILE_DEVICE_UNKNOWN, 0x1338, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_EXEC		CTL_CODE( FILE_DEVICE_UNKNOWN, 0x1339, METHOD_BUFFERED, FILE_ANY_ACCESS )


typedef struct _HEADER {
	ULONG		pID;
	ULONG		imageBase;
} HEADER, *PHEADER;

typedef struct _RPM {
	ULONG		topPtr; //BUFFER
	ULONG		lowPtr; //RETVALUE

	ULONG		pID;
	ULONG		size;
	ULONG		dAddress;
	BOOLEAN		write;
} RPM, *PRPM;

typedef struct _FUNC {
	ULONG		pID;
	ULONG		pAssemblyBuffer;
	ULONG		AssemblyLength;
	ULONG		dFlags;
} FUNC, *PFUNC;