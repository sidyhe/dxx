#include <wdm.h>
#define _KCRT_CORE_
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

typedef struct _MALLOC_HEADER
{
	ULONG32 Tags;
	ULONG32 _Resv0;
	ULONG_PTR Size;
}MALLOC_HEADER, *PMALLOC_HEADER;
C_ASSERT(sizeof(MALLOC_HEADER) % sizeof(void*) == 0);

PMALLOC_HEADER GET_MALLOC_HEADER(PVOID ptr) {
	return (MALLOC_HEADER*)((PUCHAR)ptr - sizeof(MALLOC_HEADER));
}

PVOID GET_MALLOC_ADDRESS(PMALLOC_HEADER header) {
	return (PVOID)((PUCHAR)header + sizeof(MALLOC_HEADER));
}

ULONG_PTR GET_MALLOC_SIZE(PVOID ptr) {
	PMALLOC_HEADER header = GET_MALLOC_HEADER(ptr);

	if (header->Tags != KCRT_POOL_DEFAULT_TAG)
		KeBugCheckEx(BAD_POOL_HEADER, 0, 0, 0, 0);

	return header->Size;
}

void __cdecl free(void* ptr) {
	if (ptr) {
		MALLOC_HEADER* mhdr = GET_MALLOC_HEADER(ptr);

		if (mhdr->Tags != KCRT_POOL_DEFAULT_TAG)
			KeBugCheckEx(BAD_POOL_HEADER, 0, 0, 0, 0);

		ExFreePool(mhdr);
	}
}

void* __cdecl malloc(size_t size) {
	PMALLOC_HEADER mhdr = NULL;
	const size_t new_size = size + sizeof(MALLOC_HEADER);

	mhdr = (PMALLOC_HEADER)ExAllocatePoolWithTag(NonPagedPool, new_size, KCRT_POOL_DEFAULT_TAG);
	if (mhdr) {
		RtlZeroMemory(mhdr, new_size);

		mhdr->Tags = KCRT_POOL_DEFAULT_TAG;
		mhdr->Size = size;
		return GET_MALLOC_ADDRESS(mhdr);
	}

	return NULL;
}

void* __cdecl realloc(void* ptr, size_t new_size) {
	if (!ptr) {
		return malloc(new_size);
	}
	else if (new_size == 0) {
		free(ptr);
		return NULL;
	}
	else {
		size_t old_size = GET_MALLOC_SIZE(ptr);

		if (new_size <= old_size) {
			return ptr;
		}
		else {
			void* new_ptr = malloc(new_size);

			if (new_ptr) {
				memcpy(new_ptr, ptr, old_size);
				free(ptr);
				return new_ptr;
			}
		}
	}

	return NULL;
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
	_cinitfs();
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

//////////////////////////////////////////////////////////////////////////
// dummy
char* __cdecl getenv(char const* name) {
	name;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// assert
void __cdecl _wassert(wchar_t const* _Message, wchar_t const* _File, unsigned _Line)
{
	_Message;
	_File;
	_Line;

	KdBreakPoint();
	ExRaiseStatus(STATUS_BAD_DATA);
}

//////////////////////////////////////////////////////////////////////////
// ntoskrnl
NTSYSAPI BOOLEAN NTAPI RtlTimeToSecondsSince1970(_In_ PLARGE_INTEGER Time, _Out_ PULONG ElapsedSeconds);
int __cdecl _vsnprintf_s(char* const _Buffer, size_t const _BufferCount, size_t const _MaxCount, char const* const _Format, va_list _ArgList);

//////////////////////////////////////////////////////////////////////////
// time
typedef long clock_t;
typedef __int64 __time64_t;

clock_t __cdecl clock(void) {
	LARGE_INTEGER li;

	KeQueryTickCount(&li);
	return li.LowPart;
}

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

time_t __cdecl time(time_t* _Time) {
	return _time64(_Time);
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
