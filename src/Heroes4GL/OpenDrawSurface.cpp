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
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "GLib.h"
#include "Config.h"

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
}

OpenDrawSurface::~OpenDrawSurface()
{
	this->ReleaseBuffer();

	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
		((OpenDraw*)this->ddraw)->attachedSurface = NULL;

	if (this->attachedClipper)
		this->attachedClipper->Release();
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height)
{
	this->ReleaseBuffer();

	if (width && height)
	{
		this->width = width;
		this->height = height;

		if (!config.isGDIHooked)
		{
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

				RenderBuffer* temp = &((OpenDraw*)this->ddraw)->temp;
				if (temp->data && temp->width == this->width && temp->height == this->height)
					MemoryCopy(this->indexBuffer, temp->data, this->width * this->height * sizeof(WORD));

				if (((OpenDraw*)this->ddraw)->attachedSurface == this)
					((OpenDraw*)this->ddraw)->RenderStart();
			}
		}
		else
		{
			DWORD size = this->width * this->height * sizeof(WORD);
			this->indexBuffer = (WORD*)AlignedAlloc(size);

			RenderBuffer* temp = &((OpenDraw*)this->ddraw)->temp;
			if (temp->data && temp->width == this->width && temp->height == this->height)
				MemoryCopy(this->indexBuffer, temp->data, this->width * this->height * sizeof(WORD));

			if (((OpenDraw*)this->ddraw)->attachedSurface == this)
				((OpenDraw*)this->ddraw)->RenderStart();
		}
	}
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->indexBuffer)
	{
		if (((OpenDraw*)this->ddraw)->attachedSurface == this)
			((OpenDraw*)this->ddraw)->RenderStop();

		RenderBuffer* temp = &((OpenDraw*)this->ddraw)->temp;
		if (temp->data && (temp->width != this->width || temp->height != this->height))
		{
			AlignedFree(temp->data);
			temp->data = NULL;
		}

		DWORD size = this->width * this->height * sizeof(WORD);
		if (!((OpenDraw*)this->ddraw)->temp.data)
		{
			temp->width = this->width;
			temp->height = this->height;
			temp->data = (WORD*)AlignedAlloc(size);
		}

		MemoryCopy(temp->data, this->indexBuffer, size);

		if (!config.isGDIHooked)
		{
			if (this->hDc)
			{
				DeleteDC(this->hDc);
				this->hDc = NULL;
			}

			if (this->hBmp)
			{
				DeleteObject(this->hBmp);
				this->hBmp = NULL;
			}
		}
		else
			AlignedFree(this->indexBuffer);

		this->indexBuffer = NULL;
	}
}

VOID OpenDrawSurface::TakeSnapshot()
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		DWORD pitch = this->width * sizeof(WORD);
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		DWORD size = pitch * this->height;
		DWORD slice = sizeof(BITMAPINFOHEADER) + 12;
		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, slice + size);
		if (hMemory)
		{
			VOID* data = GlobalLock(hMemory);
			if (data)
			{
				BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
				bmi->bV5Size = sizeof(BITMAPINFOHEADER);
				bmi->bV5Width = this->width;
				bmi->bV5Height = this->height;
				bmi->bV5Planes = 1;
				bmi->bV5BitCount = 16;
				bmi->bV5Compression = BI_BITFIELDS;
				bmi->bV5SizeImage = size;
				bmi->bV5XPelsPerMeter = 1;
				bmi->bV5YPelsPerMeter = 1;
				bmi->bV5ClrUsed = 0;
				bmi->bV5ClrImportant = 0;
				bmi->bV5RedMask = 0xF800;
				bmi->bV5GreenMask = 0x07E0;
				bmi->bV5BlueMask = 0x001F;

				BYTE* dstData = (BYTE*)data + slice + size - pitch;

				WORD* src = this->indexBuffer;
				DWORD height = this->height;
				do
				{
					WORD* dst = (WORD*)dstData;
					DWORD width = this->width;
					do
						*dst++ = *src++;
					while (--width);

					dstData -= pitch;
				} while (--height);

				GlobalUnlock(hMemory);
				SetClipboardData(CF_DIB, hMemory);
			}

			GlobalFree(hMemory);
		}

		CloseClipboard();
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
	*dcMem = config.isGDIHooked ? (HDC)this : this->hDc;
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

	WORD* source = surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
	WORD* destination = this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

	DWORD copyHeight = height;
	do
	{
		MemoryCopy(destination, source, width << 1);
		source += sWidth;
		destination += dWidth;
	} while (--copyHeight);

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

	WORD* source = surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
	WORD* destination = this->indexBuffer + dwY * dWidth + dwX;

	DWORD copyHeight = height;
	do
	{
		MemoryCopy(destination, source, width << 1);
		source += sWidth;
		destination += dWidth;
	} while (--copyHeight);

	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
	{
		SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
		Sleep(0);
	}

	return DD_OK;
}