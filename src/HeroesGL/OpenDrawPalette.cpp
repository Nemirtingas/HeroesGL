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
#include "OpenDrawPalette.h"
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "Glib.h"

OpenDrawPalette::OpenDrawPalette(IDraw* lpDD)
{
	this->refCount = 1;
	this->ddraw = lpDD;
	this->last = lpDD->paletteEntries;
	lpDD->paletteEntries = this;
	this->isChanged = FALSE;
}

ULONG __stdcall OpenDrawPalette::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall OpenDrawPalette::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDrawPalette::SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
	DWORD* src = (DWORD*)lpEntries;
	DWORD* dst = this->entries + dwStartingEntry;

	do
	{
		if (*dst != *src)
		{
			MemoryCopy(dst, src, dwCount *sizeof(PALETTEENTRY));

			IDrawSurface* surfaceEntry = this->ddraw->surfaceEntries;
			while (surfaceEntry)
			{
				if (((OpenDrawSurface*)surfaceEntry)->attachedPalette == this)
				{
					BYTE* idx = ((OpenDrawSurface*)surfaceEntry)->indexBuffer;
					DWORD* pix = ((OpenDrawSurface*)surfaceEntry)->pixelBuffer;
					DWORD count = RES_WIDTH * RES_HEIGHT;
					do
						*pix++ = this->entries[*idx++];
					while (--count);
				}

				surfaceEntry = surfaceEntry->last;
			}

			this->isChanged = TRUE;
			SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
			Sleep(0);

			break;
		}

		++dst;
		++src;
	} while (--dwCount);

	return DD_OK;
}