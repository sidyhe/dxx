#include <ntddk.h>

char *_sys_errlist[] =
{
	/*  0              */  "No error",
	/*  1 EPERM        */  "Operation not permitted",
	/*  2 ENOENT       */  "No such file or directory",
	/*  3 ESRCH        */  "No such process",
	/*  4 EINTR        */  "Interrupted function call",
	/*  5 EIO          */  "Input/output error",
	/*  6 ENXIO        */  "No such device or address",
	/*  7 E2BIG        */  "Arg list too long",
	/*  8 ENOEXEC      */  "Exec format error",
	/*  9 EBADF        */  "Bad file descriptor",
	/* 10 ECHILD       */  "No child processes",
	/* 11 EAGAIN       */  "Resource temporarily unavailable",
	/* 12 ENOMEM       */  "Not enough space",
	/* 13 EACCES       */  "Permission denied",
	/* 14 EFAULT       */  "Bad address",
	/* 15 ENOTBLK      */  "Unknown error",                     /* not POSIX */
	/* 16 EBUSY        */  "Resource device",
	/* 17 EEXIST       */  "File exists",
	/* 18 EXDEV        */  "Improper link",
	/* 19 ENODEV       */  "No such device",
	/* 20 ENOTDIR      */  "Not a directory",
	/* 21 EISDIR       */  "Is a directory",
	/* 22 EINVAL       */  "Invalid argument",
	/* 23 ENFILE       */  "Too many open files in system",
	/* 24 EMFILE       */  "Too many open files",
	/* 25 ENOTTY       */  "Inappropriate I/O control operation",
	/* 26 ETXTBSY      */  "Unknown error",                     /* not POSIX */
	/* 27 EFBIG        */  "File too large",
	/* 28 ENOSPC       */  "No space left on device",
	/* 29 ESPIPE       */  "Invalid seek",
	/* 30 EROFS        */  "Read-only file system",
	/* 31 EMLINK       */  "Too many links",
	/* 32 EPIPE        */  "Broken pipe",
	/* 33 EDOM         */  "Domain error",
	/* 34 ERANGE       */  "Result too large",
	/* 35 EUCLEAN      */  "Unknown error",                     /* not POSIX */
	/* 36 EDEADLK      */  "Resource deadlock avoided",
	/* 37 UNKNOWN      */  "Unknown error",
	/* 38 ENAMETOOLONG */  "Filename too long",
	/* 39 ENOLCK       */  "No locks available",
	/* 40 ENOSYS       */  "Function not implemented",
	/* 41 ENOTEMPTY    */  "Directory not empty",
	/* 42 EILSEQ       */  "Illegal byte sequence",
	/* 43              */  "Unknown error"

}; 

int _sys_nerr = sizeof(_sys_errlist) / sizeof(_sys_errlist[0]) - 1;

#define _sys_err_msg(m) _sys_errlist[(((m)<0)||((m)>=_sys_nerr)?_sys_nerr:(m))]

__inline
const char *_get_sys_err_msg(int m)
{
	return _sys_err_msg(m);
}

#define _SYS_MSGMAX 38
#define _ERRMSGLEN_ (94+_SYS_MSGMAX+2)

char * __cdecl strerror(int errnum) {
	static char errmsg[_ERRMSGLEN_ + 1];

	strcpy_s(errmsg, _ERRMSGLEN_, _get_sys_err_msg(errnum));

	return (errmsg);
}

int _kerrno = 0;

int* __cdecl _errno(void) {
	return &_kerrno;
}

