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
#include "Hooks.h"

DirectDraw::DirectDraw(IDraw** last, IDirectDraw* lpDD)
{
	this->last = *last;
	*last = this;

	this->surfaceEntries = NULL;
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

HRESULT __stdcall DirectDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	return this->lpDD->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
}

HRESULT __stdcall DirectDraw::RestoreDisplayMode()
{
	return this->lpDD->RestoreDisplayMode();
}

HRESULT __stdcall DirectDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	HRESULT res = this->lpDD->SetCooperativeLevel(hWnd, dwFlags);
	Hooks::CheckRefreshRate();
	return res;
}

HRESULT __stdcall DirectDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	HRESULT res = this->lpDD->SetDisplayMode(dwWidth, dwHeight, dwBPP);
	Hooks::CheckRefreshRate();
	return res;
}