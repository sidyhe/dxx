#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int __cdecl _cinit(void);
void __cdecl _cexit(void);

typedef void(__cdecl *_PVFV)(void);
typedef int (__cdecl *_PIFV)(void);

int __cdecl atexit(_PVFV func);



void __cdecl free(void* ptr);
void* __cdecl malloc(size_t size);

#ifdef __cplusplus
}
#endif // __cplusplus
