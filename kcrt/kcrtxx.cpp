#include <wdm.h>
#define _KCRT_CORE_
#include "kcrt.h"

//////////////////////////////////////////////////////////////////////////
// main
NTSTATUS SysMain(PDRIVER_OBJECT, PUNICODE_STRING);

//////////////////////////////////////////////////////////////////////////
// Startup
PDRIVER_UNLOAD __user_unload;

extern "C"
VOID _crt_unload(PDRIVER_OBJECT DrvObject) {
	if (__user_unload) {
		__user_unload(DrvObject);
	}

	_cexit();
}

extern "C"
NTSTATUS _crt_load(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	NTSTATUS st;

	if (_cinit() != 0)
		return STATUS_APP_INIT_FAILURE;

	__user_unload = NULL;
	st = SysMain(DrvObject, RegPath);
	if (NT_SUCCESS(st)) {
		PDRIVER_UNLOAD du = DrvObject->DriverUnload;

		if (du) {
			__user_unload = du;
			DrvObject->DriverUnload = _crt_unload;
		}
	}
	else {
		_cexit();
	}

	return st;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	return _crt_load(DrvObject, RegPath);
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

void __cdecl operator delete[](void* ptr) {
	free(ptr);
}

// EASTL
void* operator new[](size_t size, const char*, int, unsigned, const char*, int) {
	return malloc(size);
}
void* operator new[](size_t size, size_t, size_t, const char*, int, unsigned, const char*, int) {
	return malloc(size);
}

