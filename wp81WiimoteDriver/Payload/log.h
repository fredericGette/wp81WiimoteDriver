#pragma once

#include <ntifs.h>
#include <ntstrsafe.h>

HANDLE openLogFile();
void debug(char *format, ...);