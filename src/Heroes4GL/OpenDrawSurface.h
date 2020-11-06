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
#include "IDrawSurface7.h"
#include "IDraw7.h"
#include "OpenDrawClipper.h"
#include "ExtraTypes.h"

class OpenDrawSurface : public IDrawSurface7
{
protected:
	ULONG refCount;

public:
	DWORD index;
	DWORD width;
	DWORD height;

	OpenDrawClipper* attachedClipper;

	WORD* indexBuffer;
	HBITMAP hBmp;
	HDC hDc;

	OpenDrawSurface(IDraw7*, DWORD);
	~OpenDrawSurface();

	VOID CreateBuffer(DWORD, DWORD);
	VOID ReleaseBuffer();
	VOID TakeSnapshot();

	// Inherited via IDrawSurface7
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	HRESULT __stdcall BltFast(DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	HRESULT __stdcall GetDC(HDC *);
	HRESULT __stdcall GetPixelFormat(LPDDPIXELFORMAT);
	HRESULT __stdcall GetSurfaceDesc(LPDDSURFACEDESC2);
	HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	HRESULT __stdcall SetClipper(LPDIRECTDRAWCLIPPER);
};