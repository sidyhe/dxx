#include <ntddk.h>
#include "kcrt.h"

#define _CRTALLOC(x) __declspec(allocate(x))

// C initializers
#pragma const_seg(".CRT$XIA")
extern _CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL };
#pragma const_seg(".CRT$XIZ")
extern _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL };
// C++ initializers
#pragma const_seg(".CRT$XCA")
extern _CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };
#pragma const_seg(".CRT$XCZ")
extern _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };
#pragma const_seg()
#pragma comment(linker, "/merge:.CRT=.rdata")

#define KCRT_POOL_DEFAULT_TAG	'trck' // kcrt

typedef struct _ON_EXIT_ENTRY {
	LIST_ENTRY Entry;
	_PVFV func;
}ON_EXIT_ENTRY, *PON_EXIT_ENTRY;

LIST_ENTRY __onexithead;

void __cdecl free(void* ptr) {
	if (ptr) {
		ExFreePool(ptr);
	}
}

void* __cdecl malloc(size_t size) {
	return kmalloc(size, PagedPool);
}

void* __cdecl kmalloc(SIZE_T size, POOL_TYPE PoolType) {
	void* ptr;

	ptr = ExAllocatePoolWithTag(PoolType, size, KCRT_POOL_DEFAULT_TAG);
	if (ptr) {
		RtlZeroMemory(ptr, size);
	}
	return ptr;
}

void _initterm(_PVFV* pfbegin, _PVFV* pfend)
{
	/*
	* walk the table of function pointers from the bottom up, until
	* the end is encountered.  Do not skip the first entry.  The initial
	* value of pfbegin points to the first valid entry.  Do not try to
	* execute what pfend points to.  Only entries before pfend are valid.
	*/
	while (pfbegin < pfend)
	{
		/*
		* if current table entry is non-NULL, call thru it.
		*/
		if (*pfbegin != NULL)
			(**pfbegin)();
		++pfbegin;
	}
}

int __cdecl _cinit(void) {
	InitializeListHead(&__onexithead);

	/*
	* do C++ initializations
	*/
	_initterm(__xc_a, __xc_z);

	return 0;
}

_PVFV _onexit(_PVFV lpfn) {
	PON_EXIT_ENTRY _Entry = (PON_EXIT_ENTRY)malloc(sizeof(ON_EXIT_ENTRY));

	if (!_Entry)
		return NULL;

	_Entry->func = lpfn;
	InsertHeadList(&__onexithead, &_Entry->Entry);
	return lpfn;
}

int __cdecl atexit(_PVFV func) {
	return (_onexit(func) == NULL) ? -1 : 0;
}

void doexit(int code, int quick, int retcaller) {
	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(retcaller);

	if (!quick) {
		while (!IsListEmpty(&__onexithead)) {
			PLIST_ENTRY _Entry = RemoveHeadList(&__onexithead);
			PON_EXIT_ENTRY Entry = (PON_EXIT_ENTRY)_Entry;

			Entry->func();
			free(Entry);
		}
	}
}

void __cdecl _cexit(void) {
	doexit(0, 0, 1);    /* full term, return to caller */
}

int __cdecl _purecall(void) {
	KdBreakPoint();
	ExRaiseStatus(STATUS_NOT_IMPLEMENTED);
}

//////////////////////////////////////////////////////////////////////////
// ntoskrnl force
NTSYSAPI BOOLEAN NTAPI RtlTimeToSecondsSince1970(_In_ PLARGE_INTEGER Time, _Out_ PULONG ElapsedSeconds);
int __cdecl _vsnprintf_s(char* const _Buffer, size_t const _BufferCount, size_t const _MaxCount, char const* const _Format, va_list _ArgList);

//////////////////////////////////////////////////////////////////////////
// file
struct _iobuf;
typedef struct _iobuf FILE;
#define EOF    (-1)

FILE* __cdecl fopen(char const* _FileName, char const* _Mode) {
	_FileName;
	_Mode;

	return NULL;
}

FILE* __cdecl freopen(char const* _FileName, char const* _Mode, FILE* _Stream) {
	_FileName;
	_Mode;
	_Stream;

	return NULL;
}

int __cdecl fclose(FILE* _Stream) {
	_Stream;

	return 0;
}

int __cdecl feof(FILE* _Stream) {
	_Stream;

	return 1;
}

size_t __cdecl fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream) {
	_Buffer;
	_ElementSize;
	_ElementCount;
	_Stream;

	return 0;
}

size_t __cdecl fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream) {
	_Buffer;
	_ElementSize;
	_ElementCount;
	_Stream;

	return 0;
}

int __cdecl getc(FILE* _Stream) {
	_Stream;

	return EOF;
}

int __cdecl ferror(FILE* _Stream) {
	_Stream;

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// time

typedef long clock_t;
clock_t __cdecl clock(void) {
	LARGE_INTEGER li;

	KeQueryTickCount(&li);
	return li.LowPart;
}

typedef __int64 __time64_t;
__time64_t __cdecl _time64(__time64_t* _Time) {
	ULONG uTime;
	LARGE_INTEGER li;
	__time64_t uTime64;

	KeQuerySystemTime(&li);
	RtlTimeToSecondsSince1970(&li, &uTime);
	uTime64 = uTime;
	if (_Time) {
		*_Time = uTime64;
	}

	return uTime64;
}

//////////////////////////////////////////////////////////////////////////
// sprintf

#include <stdarg.h>

int __cdecl ksprintf(char* const s, size_t const sz, char const* const f, ...) {
	int n = 0;
	va_list _ArgList;

	va_start(_ArgList, f);
	n = _vsnprintf_s(s, sz, _TRUNCATE, f, _ArgList);
	va_end(_ArgList);
	return n;
}
