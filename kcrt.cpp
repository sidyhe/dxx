#include <ntddk.h>

//////////////////////////////////////////////////////////////////////////
// main
NTSTATUS SysMain(PDRIVER_OBJECT, PUNICODE_STRING);

//////////////////////////////////////////////////////////////////////////
// CRT
typedef void (__cdecl *_PVFV)(void);
#define _CRTALLOC(x) __declspec(allocate(x))

// C++ initializers
#pragma data_seg(".CRT$XCA")
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };
#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };
#pragma data_seg()
#pragma comment(linker, "/merge:.CRT=.kcrt")

void __cdecl free(void* ptr) {
	if (ptr) {
		ExFreePool(ptr);
	}
}

void* __cdecl malloc(size_t size) {
	return ExAllocatePoolWithTag(NonPagedPool, size, 'kcrt');
}

struct _ON_EXIT_ENTRY : LIST_ENTRY {
	_PVFV func;
};
typedef _ON_EXIT_ENTRY ON_EXIT_ENTRY, *PON_EXIT_ENTRY;

LIST_ENTRY __onexithead;
PDRIVER_UNLOAD __user_unload;

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

int _cinit() {
	/*
	* do C++ initializations
	*/
	_initterm(__xc_a, __xc_z);

	return 0;
}

_PVFV _onexit(_PVFV lpfn) {
	PON_EXIT_ENTRY Entry = (PON_EXIT_ENTRY)malloc(sizeof(ON_EXIT_ENTRY));

	if (!Entry)
		return NULL;

	Entry->func = lpfn;
	InsertHeadList(&__onexithead, Entry);
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

void _cexit(void) {
	doexit(0, 0, 1);    /* full term, return to caller */
}

extern "C"
VOID _driverunload(PDRIVER_OBJECT DrvObject) {
	if (__user_unload) {
		__user_unload(DrvObject);
	}

	_cexit();
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	NTSTATUS st;

	__user_unload = NULL;
	InitializeListHead(&__onexithead);
	if (_cinit() != 0)
		return STATUS_APP_INIT_FAILURE;

	st = SysMain(DrvObject, RegPath);
	if (NT_SUCCESS(st)) {
		PDRIVER_UNLOAD du = DrvObject->DriverUnload;

		if (du) {
			__user_unload = du;
			DrvObject->DriverUnload = _driverunload;
		}
	} else {
		_cexit();
	}

	return st;
}

//////////////////////////////////////////////////////////////////////////
// new & delete
void* __cdecl operator new(size_t size) {
	return malloc(size);
}

void* __cdecl operator new[](size_t size) {
	return malloc(size);
}

void __cdecl operator delete(void* ptr) {
	free(ptr);
}

void __cdecl operator delete(void* ptr, size_t) {
	free(ptr);
}
