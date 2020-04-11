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
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "GLib.h"
#include "Config.h"

OpenDrawSurface::OpenDrawSurface(IDraw* lpDD, DWORD index)
{
	this->refCount = 1;
	this->ddraw = lpDD;
	this->last = lpDD->surfaceEntries;
	lpDD->surfaceEntries = this;

	this->index = index;

	MemoryZero(this->indexBuffer, RES_WIDTH * RES_HEIGHT);
	if (!index)
	{
		this->pixelBuffer = (DWORD*)MemoryAlloc(RES_WIDTH * RES_HEIGHT * sizeof(DWORD));
		this->clipsList = (UpdateRect*)MemoryAlloc(STENCIL_COUNT * sizeof(UpdateRect));
		this->endClip = this->clipsList + STENCIL_COUNT;

		MemoryZero(this->pixelBuffer, RES_WIDTH * RES_HEIGHT * sizeof(DWORD));
	}
	else
	{
		this->pixelBuffer = NULL;
		this->clipsList = NULL;
		this->endClip = this->clipsList;
	}

	this->poinetrClip = this->currentClip = this->clipsList;

	this->attachedPalette = NULL;
	this->attachedClipper = NULL;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
		((OpenDraw*)this->ddraw)->attachedSurface = NULL;

	if (this->attachedClipper)
		this->attachedClipper->Release();

	if (this->attachedPalette)
		this->attachedPalette->Release();

	if (this->pixelBuffer)
		MemoryFree(this->pixelBuffer);

	if (this->clipsList)
		MemoryFree(this->clipsList);
}

ULONG __stdcall OpenDrawSurface::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall OpenDrawSurface::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (!config.update.rect.right || !config.update.rect.bottom)
		return DD_OK;

	RECT rcSrc = config.update.rect;
	DWORD offset = config.update.offset->x;
	if (offset)
	{
		rcSrc.left = offset + 16;
		rcSrc.right = rcSrc.left + 448;
	}

	offset = config.update.offset->y;
	if (offset)
	{
		rcSrc.top = offset + 16;
		rcSrc.bottom = rcSrc.top + 448;
	}

	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;
	BYTE* src = surface->indexBuffer + rcSrc.top * RES_WIDTH + rcSrc.left;

	offset = config.update.rect.top * RES_WIDTH + config.update.rect.left;
	BYTE* dst = this->indexBuffer + offset;
	DWORD* pix = this->pixelBuffer + offset;

	LONG width = rcSrc.right - rcSrc.left;
	LONG height = rcSrc.bottom - rcSrc.top;
	DWORD pitch = RES_WIDTH - width;

	LONG ch = height;
	do
	{
		LONG cw = width;
		do
		{
			BYTE index = *src++;
			*pix++ = this->attachedPalette->entries[index];
			*dst++ = index;
		} while (--cw);

		src += pitch;
		pix += pitch;
		dst += pitch;
	} while (--ch);

	this->currentClip->rect = { config.update.rect.left, config.update.rect.top, config.update.rect.left + width, config.update.rect.top + height };
	this->currentClip->isActive = TRUE;

	if (width == *(LONG*)&((OpenDraw*)this->ddraw)->width && height == *(LONG*)&((OpenDraw*)this->ddraw)->height)
		this->poinetrClip = this->currentClip;
	else
	{
		UpdateRect* oldClip = surface->poinetrClip;
		UpdateRect* currClip = surface->currentClip;

		while (oldClip != currClip)
		{
			if (oldClip->isActive)
			{
				if (oldClip->rect.left >= currClip->rect.left && oldClip->rect.top >= currClip->rect.top && oldClip->rect.right <= currClip->rect.right && oldClip->rect.bottom <= currClip->rect.bottom)
					oldClip->isActive = FALSE;
				else if (currClip->rect.left >= oldClip->rect.left && currClip->rect.top >= oldClip->rect.top && currClip->rect.right <= oldClip->rect.right && currClip->rect.bottom <= oldClip->rect.bottom)
				{
					currClip->isActive = FALSE;
					break;
				}
			}

			if (++oldClip == surface->endClip)
				oldClip = surface->clipsList;
		}
	}

	this->currentClip = this->currentClip + 1 != this->endClip ? this->currentClip + 1 : this->clipsList;

	SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
	Sleep(0);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	OpenDrawClipper* old = this->attachedClipper;
	this->attachedClipper = (OpenDrawClipper*)lpDDClipper;

	if (this->attachedClipper)
	{
		if (old != this->attachedClipper)
		{
			if (old)
				old->Release();

			this->attachedClipper->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	OpenDrawPalette* old = this->attachedPalette;
	this->attachedPalette = (OpenDrawPalette*)lpDDPalette;

	if (this->attachedPalette)
	{
		if (old != this->attachedPalette)
		{
			if (old)
				old->Release();

			this->attachedPalette->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}