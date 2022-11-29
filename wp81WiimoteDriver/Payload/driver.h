#pragma once

#include <ntifs.h>
#include <ntddk.h>
//#include <wdm.h>
//#include <wdf.h>
#include <ntstrsafe.h>

#define WIIMOTE_DEVICE 0x8000

#define IOCTL_WIIMOTE_TEST CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)