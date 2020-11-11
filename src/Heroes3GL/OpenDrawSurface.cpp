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
#include "intrin.h"
#include "GLib.h"
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "Config.h"

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
	this->pitch = 0;

	this->scale = 1.0f;

	this->colorKey = 0;
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

	this->mode = { width, height, ((OpenDraw*)this->ddraw)->mode.bpp };
	this->pitch = this->mode.width * this->mode.bpp >> 3;
	if (this->pitch & 15)
		this->pitch = (this->pitch & 0xFFFFFFF0) + 16;

	DWORD size = this->mode.height * this->pitch;
	this->indexBuffer = (BYTE*)AlignedAlloc(size);
	MemoryZero(this->indexBuffer, size);

	if (((OpenDraw*)this->ddraw)->attachedSurface == this)
		((OpenDraw*)this->ddraw)->RenderStart();
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->indexBuffer)
	{
		if (((OpenDraw*)this->ddraw)->attachedSurface == this)
			((OpenDraw*)this->ddraw)->RenderStop();

		AlignedFree(this->indexBuffer);
		this->indexBuffer = NULL;
	}
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
	lpDDSurfaceDesc->lPitch = this->pitch;
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

		if (config.isSSE2)
		{
			__m128i color;
			if (this->mode.bpp == 32)
			{
				count >>= 2;
				color = _mm_set1_epi32(lpDDBltFx->dwFillColor);
			}
			else
			{
				count >>= 3;
				color = _mm_set1_epi16(LOWORD(lpDDBltFx->dwFillColor));
			}

			__m128i* dest = (__m128i*)this->indexBuffer;
			do
				_mm_store_si128(dest++, color);
			while (--count);
		}
		else
		{
			DWORD color = lpDDBltFx->dwFillColor;
			if (this->mode.bpp != 32)
			{
				color = LOWORD(color) | (color << 16);
				count >>= 1;
			}

			DWORD* dest = (DWORD*)this->indexBuffer;
			do
				*dest++ = color;
			while (--count);
		}
	}
	else
	{
		FLOAT currScale = (FLOAT)(lpSrcRect->right - lpSrcRect->left) / (lpDestRect->right - lpDestRect->left);

		OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

		DWORD sPitch;
		if (surface->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(surface->attachedClipper->hWnd, &offset);
			OffsetRect(lpSrcRect, -offset.x, -offset.y);

			sPitch = ((OpenDraw*)this->ddraw)->pitch;
		}
		else
			sPitch = surface->pitch;

		DWORD dPitch;
		if (this->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(this->attachedClipper->hWnd, &offset);
			OffsetRect(lpDestRect, -offset.x, -offset.y);

			dPitch = ((OpenDraw*)this->ddraw)->pitch;
		}
		else
			dPitch = this->pitch;

		INT width = lpSrcRect->right - lpSrcRect->left;
		INT height = lpSrcRect->bottom - lpSrcRect->top;

		if (this->mode.bpp == 32)
		{
			sPitch /= sizeof(DWORD);
			dPitch /= sizeof(DWORD);

			DWORD* src = (DWORD*)surface->indexBuffer + lpSrcRect->top * sPitch + lpSrcRect->left;
			DWORD* dst = (DWORD*)this->indexBuffer + lpDestRect->top * dPitch + lpDestRect->left;

			if (surface->colorKey)
			{
				DWORD key = surface->colorKey;

				if (config.isSSE2)
				{
					DWORD count = width >> 2;
					if (count)
					{
						DWORD sStep = (sPitch >> 2) - count;
						DWORD dStep = (dPitch >> 2) - count;

						__m128i* s = (__m128i*)src;
						__m128i* d = (__m128i*)dst;
						__m128i k = _mm_set1_epi32(key);
						DWORD h = height;
						if ((DWORD)s & 0xF)
						{
							if ((DWORD)d & 0xF)
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_loadu_si128(s);
										__m128i mask = _mm_cmpeq_epi32(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_storeu_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_loadu_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
							else
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_loadu_si128(s);
										__m128i mask = _mm_cmpeq_epi32(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_store_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_load_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
						}
						else
						{
							if ((DWORD)d & 0xF)
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_load_si128(s);
										__m128i mask = _mm_cmpeq_epi32(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_storeu_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_loadu_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
							else
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_load_si128(s);
										__m128i mask = _mm_cmpeq_epi32(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_store_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_load_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
						}

						count <<= 2;
						width -= count;
						if (!width)
							goto lbl_end;

						src += count;
						dst += count;
					}
				}

				sPitch -= width;
				dPitch -= width;

				do
				{
					DWORD count = width;
					do
					{
						if (*src != key)
							*dst = *src;

						++src;
						++dst;
					} while (--count);

					src += sPitch;
					dst += dPitch;
				} while (--height);
			}
			else
			{
				width *= sizeof(DWORD);
				do
				{
					MemoryCopy(src, dst, width);
					src += sPitch;
					dst += dPitch;
				} while (--height);
			}
		}
		else
		{
			sPitch /= sizeof(WORD);
			dPitch /= sizeof(WORD);

			WORD* src = (WORD*)surface->indexBuffer + lpSrcRect->top * sPitch + lpSrcRect->left;
			WORD* dst = (WORD*)this->indexBuffer + lpDestRect->top * dPitch + lpDestRect->left;

			if (LOWORD(surface->colorKey))
			{
				WORD key = LOWORD(surface->colorKey);
				if (config.isSSE2)
				{
					DWORD count = width >> 3;
					if (count)
					{
						DWORD sStep = (sPitch >> 3) - count;
						DWORD dStep = (dPitch >> 3) - count;

						__m128i* s = (__m128i*)src;
						__m128i* d = (__m128i*)dst;
						__m128i k = _mm_set1_epi16(key);
						DWORD h = height;
						if ((DWORD)s & 0xF)
						{
							if ((DWORD)d & 0xF)
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_loadu_si128(s);
										__m128i mask = _mm_cmpeq_epi16(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_storeu_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_loadu_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
							else
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_loadu_si128(s);
										__m128i mask = _mm_cmpeq_epi16(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_store_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_load_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
						}
						else
						{
							if ((DWORD)d & 0xF)
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_load_si128(s);
										__m128i mask = _mm_cmpeq_epi16(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_storeu_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_loadu_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
							else
								do
								{
									DWORD w = count;
									do
									{
										__m128i a = _mm_load_si128(s);
										__m128i mask = _mm_cmpeq_epi16(a, k);
										if (_mm_movemask_epi8(mask) != 0xFFFF)
											_mm_store_si128(d, _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, _mm_load_si128(d))));

										++s;
										++d;
									} while (--w);

									s += sStep;
									d += dStep;
								} while (--h);
						}

						count <<= 3;
						width -= count;
						if (!width)
							goto lbl_end;

						src += count;
						dst += count;
					}
				}

				sPitch -= width;
				dPitch -= width;

				do
				{
					DWORD w = width;
					do
					{
						if (*src != key)
							*dst = *src;

						++src;
						++dst;
					} while (--w);

					src += sPitch;
					dst += dPitch;
				} while (--height);
			}
			else
			{
				width *= sizeof(WORD);
				do
				{
					MemoryCopy(dst, src, width);
					src += sPitch;
					dst += dPitch;
				} while (--height);
			}
		}

	lbl_end:
		if (this->scale != currScale)
			this->scale = currScale;

		if (((OpenDraw*)this->ddraw)->attachedSurface == this)
		{
			SetEvent(((OpenDraw*)this->ddraw)->hDrawEvent);
			Sleep(0);
		}
	}

	return DD_OK;
}
