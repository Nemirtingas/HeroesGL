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

#include "stdafx.h"

HMODULE hDllModule;
HANDLE hActCtx;

//DIRECTDRAWCREATEEX DDCreateEx;

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

SETTHREADLANGUAGE SetThreadLanguage;

SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

#define LIBEXP(a) DWORD p##a; VOID __declspec(naked,nothrow) __stdcall ex##a() { LoadDDraw(); _asm { jmp p##a } }
#define LIBLOAD(lib, a) p##a = (DWORD)GetProcAddress(lib, #a);

LIBEXP(AcquireDDThreadLock)
LIBEXP(CompleteCreateSysmemSurface)
LIBEXP(D3DParseUnknownCommand)
LIBEXP(DDGetAttachedSurfaceLcl)
LIBEXP(DDInternalLock)
LIBEXP(DDInternalUnlock)
LIBEXP(DSoundHelp)
LIBEXP(DirectDrawCreate)
LIBEXP(DirectDrawCreateClipper)
LIBEXP(DirectDrawCreateEx)
LIBEXP(DirectDrawEnumerateA)
LIBEXP(DirectDrawEnumerateExA)
LIBEXP(DirectDrawEnumerateExW)
LIBEXP(DirectDrawEnumerateW)
LIBEXP(DllCanUnloadNow)
LIBEXP(DllGetClassObject)
LIBEXP(GetDDSurfaceLocal)
LIBEXP(GetOLEThunkData)
LIBEXP(GetSurfaceFromDC)
LIBEXP(RegisterSpecialCase)
LIBEXP(ReleaseDDThreadLock)
LIBEXP(SetAppCompatData)

DOUBLE MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5f > number ? floorVal : MathCeil(number);
}

struct Aligned {
	Aligned* last;
	VOID* data;
	VOID* block;
} * alignedList;

VOID* AlignedAlloc(size_t size)
{
	Aligned* entry = (Aligned*)MemoryAlloc(sizeof(Aligned));
	entry->last = alignedList;
	alignedList = entry;

	entry->data = MemoryAlloc(size + 16);
	entry->block = (VOID*)(((DWORD)entry->data + 16) & 0xFFFFFFF0);

	return entry->block;
}

VOID AlignedFree(VOID* block)
{
	Aligned* entry = alignedList;
	if (entry)
	{
		if (entry->block == block)
		{
			Aligned* last = entry->last;
			MemoryFree(entry->data);
			MemoryFree(entry);
			alignedList = last;
			return;
		}
		else
			while (entry->last)
			{
				if (entry->last->block == block)
				{
					Aligned* last = entry->last->last;
					MemoryFree(entry->last->data);
					MemoryFree(entry->last);
					entry->last = last;
					return;
				}

				entry = entry->last;
			}
	}
}

VOID LoadKernel32()
{
	HMODULE hLib = GetModuleHandle("KERNEL32.dll");
	if (hLib)
	{
		CreateActCtxC = (CREATEACTCTXA)GetProcAddress(hLib, "CreateActCtxA");
		ReleaseActCtxC = (RELEASEACTCTX)GetProcAddress(hLib, "ReleaseActCtx");
		ActivateActCtxC = (ACTIVATEACTCTX)GetProcAddress(hLib, "ActivateActCtx");
		DeactivateActCtxC = (DEACTIVATEACTCTX)GetProcAddress(hLib, "DeactivateActCtx");
		SetThreadLanguage = (SETTHREADLANGUAGE)GetProcAddress(hLib, "SetThreadUILanguage");
	}
}

VOID LoadDDraw()
{
	static BOOL isLoaded;

	if (isLoaded)
		return;

	isLoaded = TRUE;

	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\DDRAW.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
		{
			LIBLOAD(hLib, AcquireDDThreadLock);
			LIBLOAD(hLib, CompleteCreateSysmemSurface);
			LIBLOAD(hLib, D3DParseUnknownCommand);
			LIBLOAD(hLib, DDGetAttachedSurfaceLcl);
			LIBLOAD(hLib, DDInternalLock);
			LIBLOAD(hLib, DDInternalUnlock);
			LIBLOAD(hLib, DSoundHelp);
			LIBLOAD(hLib, DirectDrawCreate);
			LIBLOAD(hLib, DirectDrawCreateClipper);
			LIBLOAD(hLib, DirectDrawCreateEx);
			LIBLOAD(hLib, DirectDrawEnumerateA);
			LIBLOAD(hLib, DirectDrawEnumerateExA);
			LIBLOAD(hLib, DirectDrawEnumerateExW);
			LIBLOAD(hLib, DirectDrawEnumerateW);
			LIBLOAD(hLib, DllCanUnloadNow);
			LIBLOAD(hLib, DllGetClassObject);
			LIBLOAD(hLib, GetDDSurfaceLocal);
			LIBLOAD(hLib, GetOLEThunkData);
			LIBLOAD(hLib, GetSurfaceFromDC);
			LIBLOAD(hLib, RegisterSpecialCase);
			LIBLOAD(hLib, ReleaseDDThreadLock);
			LIBLOAD(hLib, SetAppCompatData);
		}
	}
}

VOID LoadShcore()
{
	HMODULE hLib = LoadLibrary("SHCORE.dll");
	if (hLib)
		SetProcessDpiAwarenessC = (SETPROCESSDPIAWARENESS)GetProcAddress(hLib, "SetProcessDpiAwareness");
}