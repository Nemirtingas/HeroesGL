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
#include "IDrawSurface.h"
#include "IDraw.h"
#include "OpenDrawPalette.h"
#include "OpenDrawClipper.h"
#include "ExtraTypes.h"

#define STENCIL_COUNT 32
#define STENCIL_POINTS 6

class OpenDrawSurface : public IDrawSurface
{
protected:
	ULONG refCount;

public:
	DWORD index;

	OpenDrawPalette* attachedPalette;
	OpenDrawClipper* attachedClipper;

	BYTE indexBuffer[RES_WIDTH * RES_HEIGHT];
	DWORD* pixelBuffer;

	OpenDrawSurface(IDraw*, DWORD);
	~OpenDrawSurface();

	VOID TakeSnapshot(DWORD, DWORD);

	// Inherited via IDrawSurface
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall Blt(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
	HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
	HRESULT __stdcall SetClipper(LPDIRECTDRAWCLIPPER);
	HRESULT __stdcall SetPalette(LPDIRECTDRAWPALETTE);
};

