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
#include "IDraw.h"
#include "Main.h"

IDraw::~IDraw()
{
	IDrawSurface* surfaceEntry = this->surfaceEntries;
	while (surfaceEntry)
	{
		IDrawSurface* curr = surfaceEntry->last;
		delete surfaceEntry;
		surfaceEntry = curr;
	}

	IDrawClipper* clipperEntry = this->clipperEntries;
	while (clipperEntry)
	{
		IDrawClipper* curr = clipperEntry->last;
		delete clipperEntry;
		clipperEntry = curr;
	}

	if (drawList == this)
		drawList = drawList->last;
	else
	{
		IDraw* ddraw = drawList;
		while (ddraw)
		{
			if (ddraw->last == this)
			{
				ddraw->last = ddraw->last->last;
				break;
			}

			ddraw = ddraw->last;
		}
	}
}

HRESULT __stdcall IDraw::QueryInterface(REFIID, LPVOID*) { return DD_OK; }
ULONG __stdcall IDraw::AddRef() { return 0; }
ULONG __stdcall IDraw::Release() { return 0; }
HRESULT __stdcall IDraw::Compact() { return DD_OK; }
HRESULT __stdcall IDraw::CreateClipper(DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw::CreatePalette(DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE*, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw::CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE*, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw::DuplicateSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE*) { return DD_OK; }
HRESULT __stdcall IDraw::EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK) { return DD_OK; }
HRESULT __stdcall IDraw::EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT __stdcall IDraw::FlipToGDISurface() { return DD_OK; }
HRESULT __stdcall IDraw::GetCaps(LPDDCAPS, LPDDCAPS) { return DD_OK; }
HRESULT __stdcall IDraw::GetDisplayMode(LPDDSURFACEDESC) { return DD_OK; }
HRESULT __stdcall IDraw::GetFourCCCodes(LPDWORD, LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw::GetGDISurface(LPDIRECTDRAWSURFACE*) { return DD_OK; }
HRESULT __stdcall IDraw::GetMonitorFrequency(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw::GetScanLine(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw::GetVerticalBlankStatus(LPBOOL) { return DD_OK; }
HRESULT __stdcall IDraw::Initialize(GUID*) { return DD_OK; }
HRESULT __stdcall IDraw::RestoreDisplayMode() { return DD_OK; }
HRESULT __stdcall IDraw::SetCooperativeLevel(HWND, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw::SetDisplayMode(DWORD, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw::WaitForVerticalBlank(DWORD, HANDLE) { return DD_OK; }