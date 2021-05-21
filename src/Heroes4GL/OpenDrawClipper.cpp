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
#include "OpenDrawClipper.h"
#include "OpenDraw.h"

OpenDrawClipper::OpenDrawClipper(IDraw7* lpDD)
{
	this->refCount = 1;
	this->ddraw = lpDD;
	this->last = lpDD->clipperEntries;
	lpDD->clipperEntries = this;

	this->rgnData = {};
	this->rgnData.rdh.dwSize = sizeof(RGNDATAHEADER);
	this->rgnData.rdh.iType = RDH_RECTANGLES;
	this->rgnData.rdh.nCount = 1;
	this->rgnData.rdh.nRgnSize = sizeof(RECT);
}

ULONG __stdcall OpenDrawClipper::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall OpenDrawClipper::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDrawClipper::GetClipList(LPRECT lpRect, LPRGNDATA lpClipList, LPDWORD lpdwSize)
{
	*lpdwSize = sizeof(RGNRECTDATA);

	if (lpClipList)
	{
		this->rgnData.rdh.rcBound = *lpRect;
		this->rgnData.rect = *lpRect;

		*(RGNRECTDATA*)lpClipList = this->rgnData;

		((OpenDraw*)this->ddraw)->ResetDisplayMode(lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
	}

	return DD_OK;
}