#pragma once

#ifndef _KCRT_CORE_
static_assert(0, "include not allow");
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int __cdecl _cinit(void);
void __cdecl _cexit(void);
void __cdecl _cinitfs(void);

typedef void(__cdecl *_PVFV)(void);
typedef int (__cdecl *_PIFV)(void);

int __cdecl atexit(_PVFV func);

void  __cdecl free(void* ptr);
void* __cdecl malloc(size_t size); // PagedPool
void* __cdecl kmalloc(SIZE_T size, POOL_TYPE PoolType);

#ifdef __cplusplus
}
#endif // __cplusplus
