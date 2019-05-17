/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

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

DWORD
	pWinGBitBlt,
	pWinGCreateBitmap,
	pWinGCreateDC,
	pWinGCreateHalftoneBrush,
	pWinGCreateHalftonePalette,
	pWinGGetDIBColorTable,
	pWinGGetDIBPointer,
	pWinGRecommendDIBFormat,
	pWinGSetDIBColorTable,
	pWinGStretchBlt;


VOID _declspec(naked) __stdcall exWinGBitBlt() { _asm { JMP pWinGBitBlt } }
VOID _declspec(naked) __stdcall exWinGCreateBitmap() { _asm { JMP pWinGCreateBitmap } }
VOID _declspec(naked) __stdcall exWinGCreateDC() { _asm { JMP pWinGCreateDC } }
VOID _declspec(naked) __stdcall exWinGCreateHalftoneBrush() { _asm { JMP pWinGCreateHalftoneBrush } }
VOID _declspec(naked) __stdcall exWinGCreateHalftonePalette() { _asm { JMP pWinGCreateHalftonePalette } }
VOID _declspec(naked) __stdcall exWinGGetDIBColorTable() { _asm { JMP pWinGGetDIBColorTable } }
VOID _declspec(naked) __stdcall exWinGGetDIBPointer() { _asm { JMP pWinGGetDIBPointer } }
VOID _declspec(naked) __stdcall exWinGRecommendDIBFormat() { _asm { JMP pWinGRecommendDIBFormat } }
VOID _declspec(naked) __stdcall exWinGSetDIBColorTable() { _asm { JMP pWinGSetDIBColorTable } }
VOID _declspec(naked) __stdcall exWinGStretchBlt() { _asm { JMP pWinGStretchBlt } }

DOUBLE __fastcall MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5f > number ? floorVal : MathCeil(number);
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
	}
}

VOID LoadWinG32()
{
	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\WING32.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
		{
			pWinGBitBlt = (DWORD)GetProcAddress(hLib, "WinGBitBlt");
			pWinGCreateBitmap = (DWORD)GetProcAddress(hLib, "WinGCreateBitmap");
			pWinGCreateDC = (DWORD)GetProcAddress(hLib, "WinGCreateDC");
			pWinGCreateHalftoneBrush = (DWORD)GetProcAddress(hLib, "WinGCreateHalftoneBrush");
			pWinGCreateHalftonePalette = (DWORD)GetProcAddress(hLib, "WinGCreateHalftonePalette");
			pWinGGetDIBColorTable = (DWORD)GetProcAddress(hLib, "WinGGetDIBColorTable");
			pWinGGetDIBPointer = (DWORD)GetProcAddress(hLib, "WinGGetDIBPointer");
			pWinGRecommendDIBFormat = (DWORD)GetProcAddress(hLib, "WinGRecommendDIBFormat");
			pWinGSetDIBColorTable = (DWORD)GetProcAddress(hLib, "WinGSetDIBColorTable");
			pWinGStretchBlt = (DWORD)GetProcAddress(hLib, "WinGStretchBlt");
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