#include "log.h"

// Logger
HANDLE openLogFile()
{
	HANDLE hLogFile;
	
	UNICODE_STRING str;
	WCHAR filepath[100]= L"\\??\\\\C:\\Data\\USERS\\Public\\Documents\\wp81wiimote.log";
	RtlInitUnicodeString(&str, filepath);
	OBJECT_ATTRIBUTES obj;	
	InitializeObjectAttributes(&obj, &str, OBJ_CASE_INSENSITIVE, NULL, NULL);

	IO_STATUS_BLOCK isb;
	NTSTATUS status = ZwCreateFile(&hLogFile, FILE_GENERIC_WRITE, &obj, &isb, 0, FILE_ATTRIBUTE_NORMAL,FILE_SHARE_WRITE, FILE_OPEN_IF,	FILE_RANDOM_ACCESS|FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	
	return hLogFile;
}

void debug(char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[1000];
	RtlStringCchVPrintfA(buffer, sizeof(buffer), format, args);
	
	HANDLE hLogFile = openLogFile();
	
	LARGE_INTEGER ByteOffset;

	ByteOffset.HighPart = -1;
	ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
	
	size_t size;
	RtlStringCbLengthA(buffer, sizeof(buffer), &size);
	IO_STATUS_BLOCK isb;
	ZwWriteFile(hLogFile, NULL, NULL, NULL, &isb, buffer, size, &ByteOffset, NULL);
	
	ZwClose(hLogFile);
	
	va_end(args);
}
