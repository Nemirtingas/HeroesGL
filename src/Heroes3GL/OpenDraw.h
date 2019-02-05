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

#pragma once
#include "IDraw.h"
#include "ExtraTypes.h"
#include "OpenDrawSurface.h"

class OpenDraw : public IDraw
{
public:
	OpenDrawSurface* attachedSurface;

	HWND hDraw;
	HDC hDc;

	DisplayMode mode;
	BOOL isNextIsMode;
	BOOL isFinish;

	HANDLE hDrawThread;
	HANDLE hDrawEvent;

	Viewport viewport;
	WindowState windowState;

	BOOL isStateChanged;
	BOOL isTakeSnapshot;

	OpenDraw(IDraw**);

	BOOL CheckView();
	VOID __fastcall ScaleMouse(LPPOINT);

	VOID RenderStart();
	VOID RenderStop();

	VOID RenderOld();
	VOID RenderMid();
	VOID RenderNew();

	// Inherited via  IDraw
	ULONG __stdcall Release();
	HRESULT __stdcall CreateClipper(DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*);
	HRESULT __stdcall CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE*, IUnknown*);
	HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD);
};