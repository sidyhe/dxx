#include <wdm.h>
#include <errno.h>
#include <stdio.h>
#define _KCRT_CORE_
#include "kcrt.h"

char _CRT_CWD[260] = { 0 };

BOOLEAN _get_systemroot(PCHAR lpBuffer, ULONG nCount)
{
	BOOLEAN ok;
	NTSTATUS st;
	HANDLE KeyHandle;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING KeyPath;

	ok = FALSE;
	RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
	InitializeObjectAttributes(&oa, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	st = ZwOpenKeyEx(&KeyHandle, KEY_READ, &oa, 0);

	if (NT_SUCCESS(st))
	{
		ULONG nRes = 0;
		UNICODE_STRING ValueName;

		RtlInitUnicodeString(&ValueName, L"SystemRoot");
		ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, NULL, 0, &nRes);
		if (nRes > 0)
		{
			KEY_VALUE_PARTIAL_INFORMATION* kvpi;
			kvpi = (KEY_VALUE_PARTIAL_INFORMATION*)ExAllocatePoolWithTag(PagedPool, nRes, 'sfc_');

			if (kvpi)
			{
				st = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, kvpi, nRes, &nRes);

				if (NT_SUCCESS(st) && (kvpi->Type == REG_SZ))
				{
					ANSI_STRING ValueDataA;
					UNICODE_STRING ValueDataW;

					RtlInitUnicodeString(&ValueDataW, (PWSTR)kvpi->Data);
					st = RtlUnicodeStringToAnsiString(&ValueDataA, &ValueDataW, TRUE);

					if (NT_SUCCESS(st))
					{
						if (ValueDataA.Length < nCount)
						{
							ok = TRUE;
							memcpy(lpBuffer, ValueDataA.Buffer, ValueDataA.Length + 1);
						}

						RtlFreeAnsiString(&ValueDataA);
					}
				}

				ExFreePool(kvpi);
			}
		}

		ZwClose(KeyHandle);
	}

	return ok;
}

void __cdecl _cinitfs(void)
{
	_get_systemroot(_CRT_CWD, _countof(_CRT_CWD));
}

#if 0
typedef struct _FILE_CONTROL_BLOCK
{
	HANDLE hFile;
}FILE_CONTROL_BLOCK;

FILE* __cdecl fopen(char const* FileName, char const* Mode)
{
	FileName;
	Mode;

	return NULL;
}

int __cdecl fclose(FILE* f)
{
	f;

	return 0;
}

int __cdecl feof(FILE* f)
{
	f->_Placeholder;

	return EOF;
}

int __cdecl fgetc(FILE* f)
{
	f;

	return EOF;
}

size_t __cdecl fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* f)
{
	Buffer;
	ElementSize;
	ElementCount;
	f;

	return 0;
}

int __cdecl ferror(FILE* f)
{
	f;

	return EPERM;
}
#endif // 0
