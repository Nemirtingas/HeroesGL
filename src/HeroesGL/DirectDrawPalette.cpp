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
#include "DirectDrawPalette.h"
#include "IDraw.h"

DirectDrawPalette::DirectDrawPalette(IDraw* lpDD, IDirectDrawPalette* lpDDPalette)
{
	this->ddraw = lpDD;
	this->last = lpDD->paletteEntries;
	lpDD->paletteEntries = this;
	this->lpDDPalette = lpDDPalette;
}

ULONG __stdcall DirectDrawPalette::Release()
{
	ULONG res = this->lpDDPalette->Release();
	delete this;
	return res;
}

HRESULT __stdcall DirectDrawPalette::SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
	HRESULT res = this->lpDDPalette->SetEntries(dwFlags, dwStartingEntry, dwCount, lpEntries);
	return res != DDERR_GENERIC ? res : DD_OK;
}
