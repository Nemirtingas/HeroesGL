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
#include "GLib.h"
#include "OpenDrawSurface.h"
#include "OpenDraw.h"

OpenDrawSurface::OpenDrawSurface(IDraw* lpDD, DWORD index)
{
	this->refCount = 1;
	this->ddraw = lpDD;
	this->last = lpDD->surfaceEntries;
	lpDD->surfaceEntries = this;

	this->attachedClipper = NULL;

	this->index = index;
	this->indexBuffer = NULL;

	this->mode.width = 0;
	this->mode.height = 0;
	this->mode.bpp = 0;

	this->scale = 1.0f;
	this->isSizeChanged = TRUE;

	this->colorKey = 0;

	this->clipsList = NULL;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
		((OpenDraw*)this->ddraw)->attachedSurface = NULL;

	if (this->attachedClipper)
		this->attachedClipper->Release();

	this->ReleaseBuffer();
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->indexBuffer)
		MemoryFree(this->indexBuffer);

	if (this->clipsList)
		MemoryFree(this->clipsList);
}

VOID OpenDrawSurface::TakeSnapshot()
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		DWORD texWidth = DWORD(this->scale * this->mode.width);
		DWORD texHeight = DWORD(this->scale * this->mode.height);

		DWORD pitch = texWidth * (this->mode.bpp == 16 ? 2 : 3);
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		DWORD size = pitch * texHeight;
		DWORD slice = sizeof(BITMAPINFOHEADER) + (this->mode.bpp == 16 ? 12 : 0);
		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, slice + size);
		if (hMemory)
		{
			VOID* data = GlobalLock(hMemory);
			if (data)
			{
				BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
				bmi->bV5Size = sizeof(BITMAPINFOHEADER);
				bmi->bV5Width = texWidth;
				bmi->bV5Height = texHeight;
				bmi->bV5Planes = 1;
				bmi->bV5SizeImage = size;
				bmi->bV5XPelsPerMeter = 1;
				bmi->bV5YPelsPerMeter = 1;
				bmi->bV5ClrUsed = 0;
				bmi->bV5ClrImportant = 0;

				BYTE* dstData = (BYTE*)data + slice + size - pitch;
				if (this->mode.bpp == 16)
				{
					bmi->bV5BitCount = 16;
					bmi->bV5Compression = BI_BITFIELDS;
					bmi->bV5RedMask = 0xF800;
					bmi->bV5GreenMask = 0x07E0;
					bmi->bV5BlueMask = 0x001F;

					WORD* src = (WORD*)this->indexBuffer;
					DWORD height = texHeight;
					do
					{
						WORD* dst = (WORD*)dstData;
						DWORD width = texWidth;
						do
							*dst++ = *src++;
						while (--width);

						dstData -= pitch;
					} while (--height);
				}
				else
				{
					bmi->bV5BitCount = 24;
					bmi->bV5Compression = BI_RGB;

					BYTE* src = (BYTE*)this->indexBuffer;
					DWORD height = texHeight;
					do
					{
						BYTE* dst = dstData;
						DWORD width = texWidth;
						do
						{
							DWORD count = 3;
							do
								*dst++ = *src++;
							while (--count);

							++src;
						} while (--width);

						dstData -= pitch;
					} while (--height);
				}

				GlobalUnlock(hMemory);
				SetClipboardData(CF_DIB, hMemory);
			}

			GlobalFree(hMemory);
		}
		
		CloseClipboard();
	}
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height)
{
	this->ReleaseBuffer();
	this->mode.width = width;
	this->mode.height = height;
	this->mode.bpp = ((OpenDraw*)this->ddraw)->mode.bpp;

	DWORD size = width * height * (this->mode.bpp >> 3);
	this->indexBuffer = MemoryAlloc(size);
	MemoryZero(this->indexBuffer, size);

	this->clipsList = !this->index ? (UpdateRect*)MemoryAlloc(STENCIL_COUNT * sizeof(UpdateRect)) : NULL;
	this->endClip = this->clipsList + (!this->index ? STENCIL_COUNT : 0);
	this->poinetrClip = this->currentClip = this->clipsList;
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
	lpDDPixelFormat->dwRGBBitCount = this->mode.bpp;
	lpDDPixelFormat->dwRBitMask = 0xF800;
	lpDDPixelFormat->dwGBitMask = 0x07E0;
	lpDDPixelFormat->dwBBitMask = 0x001F;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	this->colorKey = lpDDColorKey->dwColorSpaceLowValue;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	lpDDSurfaceDesc->lPitch = this->mode.width * (this->mode.bpp >> 3);
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

HRESULT __stdcall OpenDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (dwFlags & DDBLT_COLORFILL)
	{
		DWORD count = this->mode.width * this->mode.height;

		if (this->mode.bpp == 32)
		{
			DWORD* dest = (DWORD*)this->indexBuffer;
			DWORD color = lpDDBltFx->dwFillColor;
			do
				*dest++ = color;
			while (--count);
		}
		else if (count & 1)
		{
			WORD* dest = (WORD*)this->indexBuffer;
			WORD color = LOWORD(lpDDBltFx->dwFillColor);
			do
				*dest++ = color;
			while (--count);
		}
		else
		{
			count >>= 1;
			DWORD* dest = (DWORD*)this->indexBuffer;
			DWORD color = lpDDBltFx->dwFillColor | (lpDDBltFx->dwFillColor << 16);
			do
				*dest++ = color;
			while (--count);
		}
	}
	else
	{
		FLOAT currScale = (FLOAT)(lpSrcRect->right - lpSrcRect->left) / (lpDestRect->right - lpDestRect->left);

		OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

		DWORD sWidth;
		if (surface->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(surface->attachedClipper->hWnd, &offset);
			OffsetRect(lpSrcRect, -offset.x, -offset.y);

			sWidth = ((OpenDraw*)this->ddraw)->mode.width;
		}
		else
			sWidth = surface->mode.width;

		DWORD dWidth;
		if (this->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(this->attachedClipper->hWnd, &offset);
			OffsetRect(lpDestRect, -offset.x, -offset.y);

			dWidth = ((OpenDraw*)this->ddraw)->mode.width;
		}
		else
			dWidth = this->mode.width;

		if (currScale != 1.0f)
			dWidth = DWORD(currScale * dWidth);

		INT width = lpSrcRect->right - lpSrcRect->left;
		INT height = lpSrcRect->bottom - lpSrcRect->top;

		{
			if (this->mode.bpp == 32)
			{
				DWORD* source = (DWORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
				DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

				if (surface->colorKey)
				{
					do
					{
						DWORD* src = source;
						DWORD* dest = destination;
						source += sWidth;
						destination += dWidth;

						DWORD count = width;
						do
						{
							if (*src != surface->colorKey)
								*dest = *src;
							++src;
							++dest;
						} while (--count);
					} while (--height);
				}
				else
				{
					do
					{
						MemoryCopy(destination, source, width << 2);
						source += sWidth;
						destination += dWidth;
					} while (--height);
				}
			}
			else
			{
				WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
				WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

				if (LOWORD(surface->colorKey))
				{
					do
					{
						WORD* src = source;
						WORD* dest = destination;
						source += sWidth;
						destination += dWidth;

						DWORD count = width;
						do
						{
							if (*src != LOWORD(surface->colorKey))
								*dest = *src;
							++src;
							++dest;
						} while (--count);
					} while (--height);
				}
				else
				{
					do
					{
						MemoryCopy(destination, source, width << 1);
						source += sWidth;
						destination += dWidth;
					} while (--height);
				}
			}
		}

		if (this->scale != currScale)
		{
			this->scale = currScale;
			this->isSizeChanged = TRUE;
		}

		if (!this->index)
		{
			this->currentClip->rect = *lpDestRect;
			this->currentClip->isActive = TRUE;

			if (lpDestRect->right - lpDestRect->left == ((OpenDraw*)this->ddraw)->mode.width &&
				lpDestRect->bottom - lpDestRect->top == ((OpenDraw*)this->ddraw)->mode.height)
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
	}

	return DD_OK;
}
