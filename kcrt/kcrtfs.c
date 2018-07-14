#include <ntifs.h>
#include <errno.h>
#include <stdio.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

char _CRT_CWD[260] = { 0 };

const char* _get_cwd()
{
	return _CRT_CWD;
}

void _set_cwd(const char* path)
{
	strncpy(_CRT_CWD, path, _countof(_CRT_CWD));
}

BOOLEAN _init_cwd()
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
						_set_cwd(ValueDataA.Buffer);
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
	_init_cwd();
}

extern
size_t path_sanitize(char* s, size_t sz, const char* base, const char* expr);

typedef struct _FILE_CONTROL_BLOCK
{
	HANDLE hFile;
	NTSTATUS err;
}FILE_CONTROL_BLOCK;

FILE_CONTROL_BLOCK* fget_core(FILE* f)
{
	return (FILE_CONTROL_BLOCK*)f->_Placeholder;
}

void fset_core(FILE* f, FILE_CONTROL_BLOCK* fcb)
{
	f->_Placeholder = fcb;
}

FILE* __cdecl fopen(char const* _FileName, char const* _Mode)
{
	NTSTATUS st;
	HANDLE hFile;
	char* path = NULL;
	IO_STATUS_BLOCK isb;
	OBJECT_ATTRIBUTES oa;
	ANSI_STRING FileNameA;
	UNICODE_STRING FileNameW;

	ULONG AccessMask = 0;
	ULONG ShareAccess = 0;
	ULONG CreateOptions = 0;
	ULONG CreateDisposition = 0;
	{
		const char* mode = _Mode;

		if (!strcmp(mode, "r") || !strcmp(mode, "rb"))
		{
			AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES;
			CreateDisposition = FILE_OPEN;
		}
		else if (!strcmp(mode, "w") || !strcmp(mode, "wb"))
		{
			AccessMask = FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
			CreateDisposition = FILE_SUPERSEDE;
		}
		else if (!strcmp(mode, "rw") || !strcmp(mode, "rwb"))
		{
			AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
			CreateDisposition = FILE_OPEN_IF;
		}
		else if (!strcmp(mode, "a+") || !strcmp(mode, "a+b") || !strcmp(mode, "ab+"))
		{
			AccessMask = FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_APPEND_DATA;
			CreateDisposition = FILE_OPEN_IF;
		}
		else
		{
			return NULL;
		}

		AccessMask |= SYNCHRONIZE;
		ShareAccess = FILE_SHARE_READ;
		CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
	}

	path = (char*)ExAllocatePoolWithTag(PagedPool, 512, 0);
	if (!path)
		return NULL;

	const char* cwd = "";
	if (_FileName[0] == '.')
		cwd = _get_cwd();
	if (!path_sanitize(path, 512, cwd, _FileName))
	{
		ExFreePool(path);
		return NULL;
	}

	RtlInitAnsiString(&FileNameA, path);
	if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&FileNameW, &FileNameA, TRUE)))
	{
		ExFreePool(path);
		return NULL;
	}

	InitializeObjectAttributes(&oa, &FileNameW, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
	st = ZwCreateFile(&hFile, AccessMask, &oa, &isb, NULL, FILE_ATTRIBUTE_NORMAL, ShareAccess, CreateDisposition, CreateOptions, NULL, 0);
	ExFreePool(path); path = NULL;
	RtlFreeUnicodeString(&FileNameW);

	if (!NT_SUCCESS(st))
		return NULL;

	FILE* f = (FILE*)ExAllocatePoolWithTag(PagedPool, sizeof(FILE), 0);
	FILE_CONTROL_BLOCK* fcb = (FILE_CONTROL_BLOCK*)ExAllocatePoolWithTag(PagedPool, sizeof(FILE_CONTROL_BLOCK), 0);
	fcb->err = 0;
	fcb->hFile = hFile;
	fset_core(f, fcb);

	return f;
}

int __cdecl fclose(FILE* f)
{
	FILE_CONTROL_BLOCK* fcb = fget_core(f);

	ZwClose(fcb->hFile);
	ExFreePool(fcb);
	ExFreePool(f);

	return 0;
}

int __cdecl feof(FILE* f)
{
	IO_STATUS_BLOCK isb;
	FILE_ALL_INFORMATION fai;
	FILE_CONTROL_BLOCK* fcb = fget_core(f);

	fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fai, sizeof(fai), FileAllInformation);
	if (NT_SUCCESS(fcb->err))
	{
		return (fai.PositionInformation.CurrentByteOffset.QuadPart + 1 >= fai.StandardInformation.EndOfFile.QuadPart);
	}
	else
	{
		return 1;
	}
}

int __cdecl fgetc(FILE* f)
{
	char c = EOF;
	IO_STATUS_BLOCK isb;
	FILE_CONTROL_BLOCK* fcb = fget_core(f);

	fcb->err = ZwReadFile(fcb->hFile, NULL, NULL, NULL, &isb, &c, 1, NULL, NULL);
	return c;
}

size_t __cdecl fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* f)
{
	IO_STATUS_BLOCK isb;
	FILE_CONTROL_BLOCK* fcb = fget_core(f);
	size_t rd_size = ElementSize * ElementCount;

	memset(&isb, 0, sizeof(isb));
	fcb->err = ZwReadFile(fcb->hFile, NULL, NULL, NULL, &isb, Buffer, (ULONG)rd_size, NULL, NULL);
	return isb.Information;
}

size_t __cdecl fwrite(void const* Buffer, size_t ElementSize, size_t ElementCount, FILE* f)
{
	IO_STATUS_BLOCK isb;
	FILE_CONTROL_BLOCK* fcb = fget_core(f);
	size_t wt_size = ElementSize * ElementCount;

	memset(&isb, 0, sizeof(isb));
	fcb->err = ZwWriteFile(fcb->hFile, NULL, NULL, NULL, &isb, (PVOID)Buffer, (ULONG)wt_size, NULL, NULL);
	return isb.Information;
}

int __cdecl ferror(FILE* f)
{
	return fget_core(f)->err;
}

void __cdecl clearerr(FILE* f)
{
	fget_core(f)->err = 0;
}

long __cdecl ftell(FILE* f)
{
	IO_STATUS_BLOCK isb;
	FILE_POSITION_INFORMATION fpi;
	FILE_CONTROL_BLOCK* fcb = fget_core(f);

	fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
	if (NT_SUCCESS(fcb->err))
	{
		return (long)fpi.CurrentByteOffset.QuadPart;
	}
	else
	{
		return -1;
	}
}

int __cdecl fseek(FILE* _Stream, long  _Offset, int _Origin)
{
	IO_STATUS_BLOCK isb;
	FILE_POSITION_INFORMATION fpi;
	FILE_CONTROL_BLOCK* fcb = fget_core(_Stream);

	switch (_Origin)
	{
	case SEEK_CUR:
		{
			long cur_pos = ftell(_Stream);
			if (cur_pos >= 0)
			{
				fpi.CurrentByteOffset.QuadPart = cur_pos + _Offset;
				fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
				if (NT_SUCCESS(fcb->err))
				{
					return 0;
				}
			}
		}
		break;
	case SEEK_END:
		{
			FILE_ALL_INFORMATION fai;

			fcb->err = ZwQueryInformationFile(fcb->hFile, &isb, &fai, sizeof(fai), FileAllInformation);
			if (NT_SUCCESS(fcb->err))
			{
				fpi.CurrentByteOffset.QuadPart = fai.StandardInformation.EndOfFile.QuadPart + _Offset;

				fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
				if (NT_SUCCESS(fcb->err))
				{
					return 0;
				}
			}
		}
		break;
	case SEEK_SET:
		{
			fpi.CurrentByteOffset.QuadPart = _Offset;
			fcb->err = ZwSetInformationFile(fcb->hFile, &isb, &fpi, sizeof(fpi), FilePositionInformation);
			if (NT_SUCCESS(fcb->err))
			{
				return 0;
			}
		}
		break;
	default:
		break;
	}

	return EIO;
}

int __cdecl ungetc(int c, FILE* f)
{
	if (c != EOF)
	{
		if (!fseek(f, -1, SEEK_CUR))
		{
			return c;
		}
	}

	return EOF;
}

