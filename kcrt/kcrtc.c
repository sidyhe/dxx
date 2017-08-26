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
	return ExAllocatePoolWithTag(NonPagedPool, size, 'kcrt');
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

