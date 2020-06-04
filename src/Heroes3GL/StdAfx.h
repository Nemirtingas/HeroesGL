/*
	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN
//#define WINVER 0x0400

#include "windows.h"
#include "mmreg.h"
#include "math.h"
#include "shellscalingapi.h"
#include "ddraw.h"
#include "ExtraTypes.h"

#define WC_DRAW "drawclass"
#define WM_CHECK_MENU "WM_CHECK_MENU"

typedef DWORD(__stdcall *AIL_WAVEOUTOPEN)(LPVOID driver, DWORD a1, DWORD a2, LPPCMWAVEFORMAT waveFormat);
typedef LPVOID(__stdcall *AIL_OPEN_STREAM)(LPVOID driver, CHAR* filePath, DWORD unknown);
typedef DWORD(__stdcall *AIL_STREAM_POSITION)(LPVOID stream);
typedef VOID(__stdcall *AIL_SET_STREAM_POSITION)(LPVOID stream, DWORD position);

typedef HRESULT(__stdcall *DIRECTDRAWCREATE)(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter);

extern DWORD pDirectDrawCreate;

typedef HANDLE(__stdcall *CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall *RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall *ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall *DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef DWORD(__stdcall* SETTHREADLANGUAGE)(DWORD lang);

extern SETTHREADLANGUAGE SetThreadLanguage;

typedef HRESULT(__stdcall* SETPROCESSDPIAWARENESS)(PROCESS_DPI_AWARENESS);

extern SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _iobuf {
	void* _Placeholder;
} FILE;
#endif

//#define BINKNOFILLIOBUF 0x00200000L // Don't Fill the IO buffer (in BinkOpen and BinkCopyTo)
//#define BINKSIMULATE 0x00400000L // Simulate the speed (call BinkSim first)
//#define BINKNOTHREADEDIO 0x08000000L // Don't use a background thread for IO 

extern "C"
{
	_CRTIMP int __cdecl sprintf(char*, const char*, ...);
	_CRTIMP FILE* __cdecl fopen(const char*, const char*);
	_CRTIMP int __cdecl fclose(FILE*);

	/*_CRTIMP HANDLE __stdcall _BinkOpen(HANDLE, DWORD);
	_CRTIMP VOID __stdcall _BinkClose(HANDLE);
	_CRTIMP BOOL __stdcall _BinkPause(HANDLE, DWORD);
	_CRTIMP BOOL __stdcall _BinkSetSimulate(DWORD);*/
}

#define MemoryAlloc(size) malloc(size)
#define MemoryFree(block) free(block)
#define MemorySet(dst, val, size) memset(dst, val, size)
#define MemoryZero(dst, size) memset(dst, 0, size)
#define MemoryCopy(dst, src, size) memcpy(dst, src, size)
#define MathCeil(x) ceil(x)
#define MathFloor(x) floor(x)
#define StrPrint(buf, fmt, ...) sprintf(buf, fmt, __VA_ARGS__)
#define StrCompare(str1, str2) strcmp(str1, str2)
#define StrCompareInsensitive(str1, str2) _stricmp(str1, str2)
#define StrCopy(dst, src) strcpy(dst, src)
#define StrCat(dst, src) strcat(dst, src)
#define StrChar(str, ch) strchr(str, ch)
#define StrLastChar(str, ch) strrchr(str, ch)
#define StrStr(str, substr) strstr(str, substr)
#define StrDuplicate(str) _strdup(str)
#define StrToAnsi(dst, src, size) wcstombs(dst, src, size)
#define StrLength(str) strlen(str)
#define FileOpen(filename, mode) fopen(filename, mode)
#define FileClose(stream) fclose(stream)
#define Random() rand()
#define SeedRandom(seed) srand(seed)
#define Exit(code) exit(code)

DOUBLE __fastcall MathRound(DOUBLE);
VOID* __fastcall AlignedAlloc(size_t size);
VOID __fastcall AlignedFree(VOID* block);

extern HMODULE hDllModule;
extern HANDLE hActCtx;

extern DisplayMode displayMode;

VOID LoadKernel32();
VOID LoadDDraw();
VOID LoadShcore();