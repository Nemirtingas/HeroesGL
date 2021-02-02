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

DIRECTDRAWCREATE DDCreate;

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

SETTHREADLANGUAGE SetThreadLanguage;

SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

#define LIBEXP(a) DWORD p##a; VOID __declspec(naked,nothrow) __stdcall ex##a() { LoadWinG32(); _asm { jmp p##a } }
#define LIBLOAD(lib, a) p##a = (DWORD)GetProcAddress(lib, #a);

LIBEXP(WinGBitBlt)
LIBEXP(WinGCreateBitmap)
LIBEXP(WinGCreateDC)
LIBEXP(WinGCreateHalftoneBrush)
LIBEXP(WinGCreateHalftonePalette)
LIBEXP(WinGGetDIBColorTable)
LIBEXP(WinGGetDIBPointer)
LIBEXP(WinGRecommendDIBFormat)
LIBEXP(WinGSetDIBColorTable)
LIBEXP(WinGStretchBlt)

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

VOID LoadWinG32()
{
	static BOOL isLoaded;

	if (isLoaded)
		return;

	isLoaded = TRUE;

	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\WING32.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
		{
			LIBLOAD(hLib, WinGBitBlt)
			LIBLOAD(hLib, WinGCreateBitmap)
			LIBLOAD(hLib, WinGCreateDC)
			LIBLOAD(hLib, WinGCreateHalftoneBrush)
			LIBLOAD(hLib, WinGCreateHalftonePalette)
			LIBLOAD(hLib, WinGGetDIBColorTable)
			LIBLOAD(hLib, WinGGetDIBPointer)
			LIBLOAD(hLib, WinGRecommendDIBFormat)
			LIBLOAD(hLib, WinGSetDIBColorTable)
			LIBLOAD(hLib, WinGStretchBlt)
		}
	}
}

VOID LoadDDraw()
{
	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\DDRAW.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
			DDCreate = (DIRECTDRAWCREATE)GetProcAddress(hLib, "DirectDrawCreate");
	}
}

VOID LoadShcore()
{
	HMODULE hLib = LoadLibrary("SHCORE.dll");
	if (hLib)
		SetProcessDpiAwarenessC = (SETPROCESSDPIAWARENESS)GetProcAddress(hLib, "SetProcessDpiAwareness");
}