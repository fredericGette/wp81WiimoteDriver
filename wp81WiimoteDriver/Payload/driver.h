#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
//#include <wdf.h>
#include <ntstrsafe.h>
#include <bthdef.h>
#include <bthioctl.h>
#include <initguid.h> 
#include <bthguid.h>
#include <bthddi.h>

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

enum DEVSTATE {
	STOPPED,								// device stopped
	WORKING,								// started and working
	PENDINGSTOP,							// stop pending
	PENDINGREMOVE,							// remove pending
	SURPRISEREMOVED,						// removed by surprise
	REMOVED									// removed
};

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT LowerDeviceObject;
	PDEVICE_OBJECT Pdo;
	UNICODE_STRING ifname;
	IO_REMOVE_LOCK RemoveLock;
//	DEVSTATE devstate;
//	DEVSTATE prevstate;
	DEVICE_POWER_STATE devpower;
	SYSTEM_POWER_STATE syspower;
	DEVICE_CAPABILITIES devcaps;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION; 