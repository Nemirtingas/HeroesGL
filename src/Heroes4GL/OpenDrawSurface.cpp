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

OpenDrawSurface::OpenDrawSurface(IDraw7* lpDD, DWORD index)
{
	this->refCount = 1;
	this->ddraw = lpDD;
	this->last = lpDD->surfaceEntries;
	lpDD->surfaceEntries = this;

	this->attachedClipper = NULL;

	this->index = index;
	this->indexBuffer = NULL;

	this->hBmp = NULL;
	this->hDc = NULL;

	this->width = 0;
	this->height = 0;

	this->clipsList = NULL;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
	{
		((OpenDraw*)this->ddraw)->attachedSurface = NULL;
		((OpenDraw*)this->ddraw)->RenderStop();
	}

	if (this->attachedClipper)
		this->attachedClipper->Release();

	this->ReleaseBuffer();
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->clipsList)
		MemoryFree(this->clipsList);

	if (this->hBmp)
		DeleteObject(this->hBmp);

	if (this->hDc)
		DeleteDC(this->hDc);
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height)
{
	if (this->hBmp)
	{
		DeleteObject(this->hBmp);
		this->hBmp = NULL;
	}

	if (width && height)
	{
		this->width = width;
		this->height = height;

		this->clipsList = !this->index ? (UpdateRect*)MemoryAlloc(STENCIL_COUNT * sizeof(UpdateRect)) : NULL;
		this->endClip = this->clipsList + (!this->index ? STENCIL_COUNT : 0);
		this->poinetrClip = this->currentClip = this->clipsList;

		BITMAPINFO* bmi = (BITMAPINFO*)MemoryAlloc(sizeof(BITMAPINFO) + 8);
		MemoryZero(bmi, sizeof(BITMAPINFO) + 8);
		{
			bmi->bmiHeader.biSize = sizeof(BITMAPINFO) + 8; // 108
			bmi->bmiHeader.biWidth = width;
			bmi->bmiHeader.biHeight = -*(LONG*)&height;
			bmi->bmiHeader.biPlanes = 1;
			bmi->bmiHeader.biBitCount = 16;
			bmi->bmiHeader.biCompression = BI_BITFIELDS;
			DWORD* mask = (DWORD*)&bmi->bmiColors;
			mask[0] = 0xF800;
			mask[1] = 0x07E0;
			mask[2] = 0x001F;

			HDC hDc = ::GetDC(NULL);
			this->hBmp = CreateDIBSection(hDc, bmi, 0, (VOID**)&this->indexBuffer, 0, 0);
			::ReleaseDC(NULL, hDc);
		}
		MemoryFree(bmi);

		if (this->hBmp)
		{
			this->hDc = CreateCompatibleDC(NULL);
			SelectObject(this->hDc, this->hBmp);
		}
	}
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

HRESULT __stdcall OpenDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwRGBBitCount = 16;
	lpDDPixelFormat->dwRBitMask = 0xF800;
	lpDDPixelFormat->dwGBitMask = 0x07E0;
	lpDDPixelFormat->dwBBitMask = 0x001F;

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

HRESULT __stdcall OpenDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = this->width;
	lpDDSurfaceDesc->dwHeight = this->height;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetDC(HDC* dcMem)
{
	*dcMem = this->hDc;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	lpDDSurfaceDesc->dwWidth = this->width;
	lpDDSurfaceDesc->dwHeight = this->height;
	lpDDSurfaceDesc->lPitch = this->width * 2;
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

	DWORD sWidth = surface->width;
	DWORD dWidth;
	if (this->attachedClipper)
	{
		RECT clip = this->attachedClipper->rgnData.rdh.rcBound;

		lpDestRect->left -= clip.left;
		lpDestRect->top -= clip.top;

		dWidth = ((OpenDraw*)this->ddraw)->mode->width;
	}
	else
		dWidth = this->width;

	INT width = lpSrcRect->right - lpSrcRect->left;
	INT height = lpSrcRect->bottom - lpSrcRect->top;

	WORD* source = surface->indexBuffer + lpSrcRect->top* sWidth + lpSrcRect->left;
	WORD* destination = this->indexBuffer + lpDestRect->top* dWidth + lpDestRect->left;

	DWORD copyHeight = height;
	do
	{
		MemoryCopy(destination, source, width << 1);
		source += sWidth;
		destination += dWidth;
	} while (--copyHeight);

	if (!this->index)
	{
		this->currentClip->rect.left = lpDestRect->left;
		this->currentClip->rect.top = lpDestRect->top;
		this->currentClip->rect.right = lpDestRect->left + width;
		this->currentClip->rect.bottom = lpDestRect->top + height;
		this->currentClip->isActive = TRUE;

		if (!((OpenDraw*)this->ddraw)->mode ||
			(width == ((OpenDraw*)this->ddraw)->mode->width &&
				height == ((OpenDraw*)this->ddraw)->mode->height))
			this->poinetrClip = this->currentClip;
		else
		{
			UpdateRect* oldClip = surface->poinetrClip;
			UpdateRect* currClip = surface->currentClip;

			while (oldClip != currClip)
			{
				if (oldClip->isActive)
				{
					if (oldClip->rect.left >= currClip->rect.left &&
						oldClip->rect.top >= currClip->rect.top &&
						oldClip->rect.right <= currClip->rect.right &&
						oldClip->rect.bottom <= currClip->rect.bottom)
						oldClip->isActive = FALSE;
					else if (currClip->rect.left >= oldClip->rect.left &&
						currClip->rect.top >= oldClip->rect.top &&
						currClip->rect.right <= oldClip->rect.right &&
						currClip->rect.bottom <= oldClip->rect.bottom)
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
	}

	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
	{
		SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
		Sleep(0);
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags)
{
	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

	DWORD sWidth = surface->width;
	DWORD dWidth = this->width;

	INT width = lpSrcRect->right - lpSrcRect->left;
	INT height = lpSrcRect->bottom - lpSrcRect->top;

	WORD* source = surface->indexBuffer + lpSrcRect->top* sWidth + lpSrcRect->left;
	WORD* destination = this->indexBuffer + dwY * dWidth + dwX;

	DWORD copyHeight = height;
	do
	{
		MemoryCopy(destination, source, width << 1);
		source += sWidth;
		destination += dWidth;
	} while (--copyHeight);

	if (!this->index)
	{
		this->currentClip->rect.left = dwX;
		this->currentClip->rect.top = dwY;
		this->currentClip->rect.right = dwX + width;
		this->currentClip->rect.bottom = dwY + height;
		this->currentClip->isActive = TRUE;

		if (!((OpenDraw*)this->ddraw)->mode ||
			(width == ((OpenDraw*)this->ddraw)->mode->width &&
				height == ((OpenDraw*)this->ddraw)->mode->height))
			this->poinetrClip = this->currentClip;
		else
		{
			UpdateRect* oldClip = surface->poinetrClip;
			UpdateRect* currClip = surface->currentClip;

			while (oldClip != currClip)
			{
				if (oldClip->isActive)
				{
					if (oldClip->rect.left >= currClip->rect.left &&
						oldClip->rect.top >= currClip->rect.top &&
						oldClip->rect.right <= currClip->rect.right &&
						oldClip->rect.bottom <= currClip->rect.bottom)
						oldClip->isActive = FALSE;
					else if (currClip->rect.left >= oldClip->rect.left &&
						currClip->rect.top >= oldClip->rect.top &&
						currClip->rect.right <= oldClip->rect.right &&
						currClip->rect.bottom <= oldClip->rect.bottom)
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
	}

	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
	{
		SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
		Sleep(0);
	}

	return DD_OK;
}