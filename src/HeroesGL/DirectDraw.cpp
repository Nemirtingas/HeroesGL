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
#include "DirectDraw.h"
#include "DirectDrawSurface.h"
#include "DirectDrawPalette.h"

DirectDraw::DirectDraw(IDraw** last, IDirectDraw* lpDD)
{
	this->last = *last;
	*last = this;

	this->surfaceEntries = NULL;
	this->paletteEntries = NULL;
	this->clipperEntries = NULL;

	this->lpDD = lpDD;
}

ULONG __stdcall DirectDraw::Release()
{
	ULONG res = this->lpDD->Release();
	delete this;
	return res;
}

HRESULT __stdcall DirectDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	return this->lpDD->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}

HRESULT __stdcall DirectDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE* lplpDDPalette, IUnknown* pUnkOuter)
{
	HRESULT res = this->lpDD->CreatePalette(dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);
	if (res == DD_OK)
		*lplpDDPalette = new DirectDrawPalette(this, *lplpDDPalette);
	return res;
}

HRESULT __stdcall DirectDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		lpDDSurfaceDesc->dwFlags = DDSD_CAPS;
	else
	{
		lpDDSurfaceDesc->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		lpDDSurfaceDesc->ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;
	}

	HRESULT res = this->lpDD->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
	if (res == DD_OK)
		*lplpDDSurface = new DirectDrawSurface(this, *lplpDDSurface);
	return res;
}

HRESULT __stdcall DirectDraw::RestoreDisplayMode()
{
	return this->lpDD->RestoreDisplayMode();
}

HRESULT __stdcall DirectDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	return this->lpDD->SetCooperativeLevel(hWnd, dwFlags);
}

HRESULT __stdcall DirectDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	return this->lpDD->SetDisplayMode(dwWidth, dwHeight, dwBPP);
}
