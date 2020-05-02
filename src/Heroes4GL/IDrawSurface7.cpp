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
#include "IDrawSurface7.h"
#include "IDraw7.h"

IDrawSurface7::~IDrawSurface7()
{
	if (this->ddraw->surfaceEntries == this)
		this->ddraw->surfaceEntries = this->ddraw->surfaceEntries->last;
	else
	{
		IDrawSurface7* entry = this->ddraw->surfaceEntries;
		while (entry)
		{
			if (entry->last == this)
			{
				entry->last = this->last;
				break;
			}

			entry = entry->last;
		}
	}
}

HRESULT __stdcall IDrawSurface7::QueryInterface(REFIID, LPVOID*) { return DD_OK; }
ULONG __stdcall IDrawSurface7::AddRef() { return 0; }
ULONG __stdcall IDrawSurface7::Release() { return 0; }
HRESULT __stdcall IDrawSurface7::AddAttachedSurface(LPDIRECTDRAWSURFACE7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::AddOverlayDirtyRect(LPRECT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::BltBatch(LPDDBLTBATCH, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::BltFast(DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Flip(LPDIRECTDRAWSURFACE7, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetAttachedSurface(LPDDSCAPS2, LPDIRECTDRAWSURFACE7*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetBltStatus(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetCaps(LPDDSCAPS2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetClipper(LPDIRECTDRAWCLIPPER*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetDC(HDC*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetFlipStatus(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetOverlayPosition(LPLONG, LPLONG) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPalette(LPDIRECTDRAWPALETTE*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPixelFormat(LPDDPIXELFORMAT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::IsLost() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Lock(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::ReleaseDC(HDC) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Restore() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetClipper(LPDIRECTDRAWCLIPPER) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetOverlayPosition(LONG, LONG) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPalette(LPDIRECTDRAWPALETTE) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Unlock(LPRECT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlayDisplay(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetDDInterface(LPVOID*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::PageLock(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::PageUnlock(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetSurfaceDesc(LPDDSURFACEDESC2, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPrivateData(REFGUID, LPVOID, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPrivateData(REFGUID, LPVOID, LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::FreePrivateData(REFGUID) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetUniquenessValue(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::ChangeUniquenessValue() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPriority(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPriority(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetLOD(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetLOD(LPDWORD) { return DD_OK; }