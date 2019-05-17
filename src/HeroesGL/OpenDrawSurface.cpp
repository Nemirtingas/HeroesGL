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

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (lpSrcRect->right == lpSrcRect->left || lpSrcRect->bottom == lpSrcRect->top)
		return DD_OK;

	if (this->attachedClipper)
		ScreenToClient(this->attachedClipper->hWnd, (POINT*)&lpDestRect->left);

	RECT rect;
	GetClientRect(this->ddraw->hWnd, &rect);

	DWORD left = lpDestRect->left * RES_WIDTH / (rect.right - rect.left);
	DWORD top = lpDestRect->top * RES_HEIGHT / (rect.bottom - rect.top);
	DWORD width = lpSrcRect->right - lpSrcRect->left;
	DWORD height = lpSrcRect->bottom - lpSrcRect->top;

	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

	BYTE* source = surface->indexBuffer + lpSrcRect->top * RES_WIDTH + lpSrcRect->left;
	BYTE* destination = this->indexBuffer + top * RES_WIDTH + left;
	DWORD* pixels = this->pixelBuffer + top * RES_WIDTH + left;

	DWORD copyHeight = height;
	do
	{
		BYTE* src = source;
		source += RES_WIDTH;

		BYTE* dest = destination;
		destination += RES_WIDTH;

		DWORD* pix = pixels;
		pixels += RES_WIDTH;

		DWORD copyWidth = width;
		do
		{
			*pix++ = *(DWORD*)&this->attachedPalette->entries[*src];
			*dest++ = *src++;
		} while (--copyWidth);
	} while (--copyHeight);

	this->currentClip->rect.left = left;
	this->currentClip->rect.top = top;
	this->currentClip->rect.right = left + width;
	this->currentClip->rect.bottom = top + height;
	this->currentClip->isActive = TRUE;

	if (width == ((OpenDraw*)this->ddraw)->width && height == ((OpenDraw*)this->ddraw)->height)
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