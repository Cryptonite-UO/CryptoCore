/**
* @file os_windows.h
* @brief Windows-specific declarations.
*/

#ifndef _INC_OS_WINDOWS_H
#define _INC_OS_WINDOWS_H

//#define SLASH_PATH	"\\"
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501	// By default we target Windows XP
#endif

#undef FD_SETSIZE
#define FD_SETSIZE 1024		// for max of n users ! default = 64	(FD: file descriptor)

// disable useless windows.h features
#define WIN32_LEAN_AND_MEAN	// include just windows.h without the other winapi headers, we'll add them manually when needed
#undef NOMINMAX
#define NOMINMAX			// we don't want to have windows min and max macros, we have our minimum and maximum
#define NOATOM
#define NOCRYPT
#define NOMCX
#define NOMETAFILE
#define NOKANJI
#define NOKERNEL
#define NOOPENFILE
#define NORASTEROPS
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH

#include <windows.h>


/*  cross-platform functions macros  */
#define strcmpi		_strcmpi	// Non ANSI equiv functions ?
#define strnicmp	_strnicmp
#define ATOI		atoi
#define ITOA		_itoa
#define LTOA		_ltoa
#define STRREV		_strrev

/*	file handling definitions  */
#define STDFUNC_FILENO(_x_)		_get_osfhandle(_fileno(_x_))
#define STDFUNC_UNLINK			_unlink

/*	threading definitions  */
#define THREAD_ENTRY_RET		void
#define STDFUNC_GETPID			_getpid

// since the only way to make windows not to buffer file is to remove buffer, we
// use this instead flushing
#define	FILE_SETNOCACHE(_x_)	setvbuf(_x_, NULL, _IONBF, 0)
#define FILE_FLUSH(_x_)

#ifndef STRICT
	#define STRICT			// strict conversion of handles and pointers.
#endif

#ifndef _MSC_VER	// No Microsoft compiler
	#define _cdecl	__cdecl
	/*
	There is a problem with the UNREFERENCED_PARAMETER macro from mingw and sphereserver.
	operator= is on many clases private and the UNREFERENCED_PARAMETER macro from mingw is (P)=(P),
	so we have a compilation error here.
	*/
	#undef  UNREFERENCED_PARAMETER
	#define UNREFERENCED_PARAMETER(P)	(void)(P)
	// Not defined for mingw.
	#define LSTATUS int
	typedef void (__cdecl *_invalid_parameter_handler)(const wchar_t *,const wchar_t *,const wchar_t *,unsigned int,uintptr_t);
	// Stuctured exception handling windows api not implemented on mingw.
	#define __except(P)		catch(int)
#endif  // _MSC_VER


const OSVERSIONINFO * Sphere_GetOSInfo();
extern bool NTWindow_Init(HINSTANCE hInstance, LPTSTR lpCmdLinel, int nCmdShow);
extern void NTWindow_Exit();
extern void NTWindow_DeleteIcon();
extern bool NTWindow_OnTick(int iWaitmSec);
extern bool NTWindow_PostMsg(LPCTSTR pszMsg);
extern bool NTWindow_PostMsgColor(COLORREF color);
extern void NTWindow_SetWindowTitle(LPCTSTR pText = NULL);


#endif	// _INC_OS_WINDOWS_H
