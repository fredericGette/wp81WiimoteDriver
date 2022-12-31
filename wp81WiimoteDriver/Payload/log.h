#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <wdf.h>
#include <ntstrsafe.h>
#include <bthdef.h>
#include <bthioctl.h>
#include <initguid.h> 
#include <bthguid.h>
#include <bthddi.h>



HANDLE openLogFile();
void debug(char *format, ...);