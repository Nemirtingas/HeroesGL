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
#include "PixelBuffer.h"
#include "intrin.h"

namespace ASM
{
	DWORD __declspec(naked) __fastcall ForwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__asm {
			push ebp
			mov ebp, esp
			push esi
			push edi

			mov esi, ptr1
			mov edi, ptr2

			sal edx, 2
			add esi, edx
			add edi, edx

			repe cmpsd
			jz lbl_ret
			inc ecx

			lbl_ret:
			mov eax, ecx

			pop edi
			pop esi
			mov esp, ebp
			pop ebp

			retn 8
		}
	}

	DWORD __declspec(naked) __fastcall BackwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__asm {
			push ebp
			mov ebp, esp
			push esi
			push edi

			mov esi, ptr1
			mov edi, ptr2

			sal edx, 2
			add esi, edx
			add edi, edx

			std
			repe cmpsd
			cld
			jz lbl_ret
			inc ecx

			lbl_ret:
			mov eax, ecx

			pop edi
			pop esi
			mov esp, ebp
			pop ebp

			retn 8
		}
	}

	BOOL __declspec(naked) __fastcall BlockForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		__asm {
			push ebp
			mov ebp, esp
			push ebx
			push esi
			push edi

			mov eax, ecx
			mov esp, edx

			mov ebx, pitch
			sub ebx, eax
			sal ebx, 2

			mov esi, ptr1
			mov edi, ptr2

			mov ecx, slice
			sal ecx, 2

			add esi, ecx
			add edi, ecx

			lbl_cycle:
				mov ecx, eax
				repe cmpsd
				jne lbl_break

				add esi, ebx
				add edi, ebx
			dec edx
			jnz lbl_cycle

			xor eax, eax
			jmp lbl_ret

			lbl_break:
			mov ebx, p
		
			inc ecx
			sub eax, ecx
			mov [ebx], eax

			sub esp, edx
			mov [ebx+4], esp

			xor eax, eax
			inc eax

			lbl_ret:
			mov esp, ebp
			sub esp, 12
			pop edi
			pop esi
			pop ebx
			pop ebp

			retn 20
		}
	}

	BOOL __declspec(naked) __fastcall BlockBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		__asm {
			push ebp
			mov ebp, esp
			push ebx
			push esi
			push edi

			mov eax, ecx

			mov ebx, pitch
			sub ebx, eax
			sal ebx, 2

			mov esi, ptr1
			mov edi, ptr2

			mov ecx, slice
			sal ecx, 2

			add esi, ecx
			add edi, ecx

			std

			lbl_cycle:
				mov ecx, eax
				repe cmpsd
				jnz lbl_break

				sub esi, ebx
				sub edi, ebx
			dec edx
			jnz lbl_cycle

			xor eax, eax
			jmp lbl_ret

			lbl_break:
			mov ebx, p
		
			mov [ebx], ecx

			dec edx
			mov [ebx+4], edx

			xor eax, eax
			inc eax

			lbl_ret:
			cld
			pop edi
			pop esi
			pop ebx
			mov esp, ebp
			pop ebp

			retn 20
		}
	}

	DWORD __declspec(naked) __fastcall SideForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__asm {
			push ebp
			mov ebp, esp
			push ebx
			push esi
			push edi

			mov eax, ecx
			mov esp, ecx

			mov ebx, pitch
			sub ebx, eax
			sal ebx, 2

			mov esi, ptr1
			mov edi, ptr2

			mov ecx, slice
			sal ecx, 2

			add esi, ecx
			add edi, ecx

			lbl_cycle:
				mov ecx, eax
				repe cmpsd
				jz lbl_inc
			
				sub eax, ecx
				dec eax
				jz lbl_ret

				sal ecx, 2
				add ebx, ecx
				add esi, ebx
				add edi, ebx
				add ebx, 4
				jmp lbl_cont

				lbl_inc:
				add esi, ebx
				add edi, ebx
			
				lbl_cont:
			dec edx
			jnz lbl_cycle

			lbl_ret:
			sub esp, eax
			mov eax, esp

			mov esp, ebp
			sub esp, 12
			pop edi
			pop esi
			pop ebx
			pop ebp

			retn 16
		}
	}

	DWORD __declspec(naked) __fastcall SideBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__asm {
			push ebp
			mov ebp, esp
			push ebx
			push esi
			push edi

			mov eax, ecx
			mov esp, ecx

			mov ebx, pitch
			sub ebx, eax
			sal ebx, 2

			mov esi, ptr1
			mov edi, ptr2

			mov ecx, slice
			sal ecx, 2

			add esi, ecx
			add edi, ecx

			std

			lbl_cycle:
				mov ecx, eax
				repe cmpsd
				jz lbl_inc
			
				sub eax, ecx
				dec eax
				jz lbl_ret

				sal ecx, 2
				add ebx, ecx
				sub esi, ebx
				sub edi, ebx
				add ebx, 4
				jmp lbl_cont

				lbl_inc:
				sub esi, ebx
				sub edi, ebx
			
				lbl_cont:
			dec edx
			jnz lbl_cycle

			lbl_ret:
			sub esp, eax
			mov eax, esp

			cld

			mov esp, ebp
			sub esp, 12
			pop edi
			pop esi
			pop ebx
			pop ebp

			retn 16
		}
	}
}

namespace CPP
{
	DWORD __fastcall ForwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		ptr1 += slice;
		ptr2 += slice;

		for (DWORD i = 0; i < count; ++i)
			if (ptr1[i] != ptr2[i])
				return count - i;
	}

	DWORD __fastcall BackwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		ptr1 += slice;
		ptr2 += slice;

		for (DWORD i = 0; i < count; ++i)
			if (ptr1[-i] != ptr2[-i])
				return count - i;

		return 0;
	}

	BOOL __fastcall BlockForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		ptr1 += slice;
		ptr2 += slice;

		for (LONG y = 0; y < height; ++y)
		{
			for (LONG x = 0; x < width; ++x)
			{
				if (ptr1[x] != ptr2[x])
				{
					p->x = x;
					p->y = y;
					return TRUE;
				}
			}

			ptr1 += pitch;
			ptr2 += pitch;
		}

		return FALSE;
	}

	BOOL __fastcall BlockBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		ptr1 += slice;
		ptr2 += slice;

		for (LONG y = 0; y < height; ++y)
		{
			for (LONG x = 0; x < width; ++x)
			{
				if (ptr1[-x] != ptr2[-x])
				{
					p->x = width - x - 1;
					p->y = height - y - 1;
					return TRUE;
				}
			}

			ptr1 -= pitch;
			ptr2 -= pitch;
		}

		return FALSE;
	}

	DWORD __fastcall SideForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		DWORD count = width;
		ptr1 += slice;
		ptr2 += slice;
		for (LONG x = 0; x < width; ++x, ++ptr1, ++ptr2)
		{
			DWORD* cmp1 = ptr1;
			DWORD* cmp2 = ptr2;
			for (LONG y = 0; y < height; ++y, cmp1 += pitch, cmp2 += pitch)
				if (*cmp1 != *cmp2)
					return count - x;
		}

		return 0;
	}

	DWORD __fastcall SideBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		DWORD count = width;
		ptr1 += slice;
		ptr2 += slice;
		for (LONG x = 0; x < width; ++x, --ptr1, --ptr2)
		{
			DWORD* cmp1 = ptr1;
			DWORD* cmp2 = ptr2;
			for (LONG y = 0; y < height; ++y, cmp1 -= pitch, cmp2 -= pitch)
				if (*cmp1 != *cmp2)
					return count - x;
		}

		return 0;
	}
}

namespace SSE
{
	DWORD __fastcall ForwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__m128i* a = (__m128i*)(ptr1 + slice);
		__m128i* b = (__m128i*)(ptr2 + slice);
		count >>= 2;
		do
		{
			__m128i diff = _mm_xor_si128(_mm_load_si128(a), _mm_load_si128(b));
			if (!_mm_testz_si128(diff, diff))
				return count << 2;

			++a;
			++b;
		} while (--count);

		return 0;
	}

	DWORD __fastcall BackwardCompare(DWORD count, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		__m128i* a = (__m128i*)(ptr1 + slice - 3);
		__m128i* b = (__m128i*)(ptr2 + slice - 3);
		count >>= 2;

		do
		{
			__m128i diff = _mm_xor_si128(_mm_load_si128(a), _mm_load_si128(b));
			if (!_mm_testz_si128(diff, diff))
				return count << 2;

			--a;
			--b;
		} while (--count);

		return 0;
	}

	BOOL __fastcall BlockForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		pitch -= width;
		pitch >>= 2;
		width >>= 2;

		__m128i* a = (__m128i*)(ptr1 + slice);
		__m128i* b = (__m128i*)(ptr2 + slice);
		for (LONG y = 0; y < height; ++y, a += pitch, b += pitch)
		{
			DWORD count = width;
			do
			{
				__m128i diff = _mm_xor_si128(_mm_load_si128(a), _mm_load_si128(b));
				if (!_mm_testz_si128(diff, diff))
				{
					DWORD* ptr = (DWORD*)&diff;
					LONG c = 3;
					do
					{
						if (*ptr)
							break;
						++ptr;
					} while (--c);

					p->x = ((width - count) << 2) + 3 - c;
					p->y = y;
					return TRUE;
				}

				++a;
				++b;
			} while (--count);
		}

		return FALSE;
	}

	BOOL __fastcall BlockBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2, POINT* p)
	{
		pitch -= width;
		pitch >>= 2;
		width >>= 2;

		__m128i* a = (__m128i*)(ptr1 + slice - 3);
		__m128i* b = (__m128i*)(ptr2 + slice - 3);
		for (LONG y = 0; y < height; ++y, a -= pitch, b -= pitch)
		{
			DWORD count = width;
			do
			{
				__m128i diff = _mm_xor_si128(_mm_load_si128(a), _mm_load_si128(b));
				if (!_mm_testz_si128(diff, diff))
				{
					DWORD* ptr = (DWORD*)&diff + 3;
					LONG c = 3;
					do
					{
						if (*ptr)
							break;
						--ptr;
					} while (--c);

					p->x = (count << 2) - (4 - c);
					p->y = height - y - 1;
					return TRUE;
				}

				--a;
				--b;
			} while (--count);
		}

		return FALSE;
	}

	DWORD __fastcall SideForwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		DWORD count = width;
		DWORD swd = width >> 2;
		DWORD spt = pitch >> 2;

		__m128i* a = (__m128i*)(ptr1 + slice);
		__m128i* b = (__m128i*)(ptr2 + slice);

		LONG i = 0, j;
		for (i = 0; i < swd; ++i, ++a, ++b)
		{
			__m128i* cmp1 = a;
			__m128i* cmp2 = b;
			for (j = 0; j < height; ++j, cmp1 += spt, cmp2 += spt)
			{
				__m128i diff = _mm_xor_si128(_mm_load_si128(cmp1), _mm_load_si128(cmp2));
				if (!_mm_testz_si128(diff, diff))
				{
					DWORD* ptr = (DWORD*)&diff;
					LONG c = 3;
					do
					{
						if (*ptr)
							break;
						++ptr;
					} while (--c);

					++j;
					a = cmp1 + spt;
					b = cmp2 + spt;
					width = (i << 2) + 3 - c;
					goto lbl_dword;
				}
			}
		}

		j = 0;

	lbl_dword:;
		ptr1 = (DWORD*)a;
		ptr2 = (DWORD*)b;
		for (LONG x = i << 2; x < width; ++x, ++ptr1, ++ptr2)
		{
			DWORD* cmp1 = ptr1;
			DWORD* cmp2 = ptr2;
			for (LONG y = j; y < height; ++y, cmp1 += pitch, cmp2 += pitch)
				if (*cmp1 != *cmp2)
					return count - x;
		}

		return count - width;
	}

	DWORD __fastcall SideBackwardCompare(LONG width, LONG height, DWORD pitch, DWORD slice, DWORD* ptr1, DWORD* ptr2)
	{
		DWORD count = width;
		DWORD swd = width >> 2;
		DWORD spt = pitch >> 2;

		__m128i* a = (__m128i*)(ptr1 + slice - 3);
		__m128i* b = (__m128i*)(ptr2 + slice - 3);

		LONG i = 0, j;
		for (i = 0; i < swd; ++i, --a, --b)
		{
			__m128i* cmp1 = a;
			__m128i* cmp2 = b;
			for (j = 0; j < height; ++j, cmp1 -= spt, cmp2 -= spt)
			{
				__m128i diff = _mm_xor_si128(_mm_load_si128(cmp1), _mm_load_si128(cmp2));
				if (!_mm_testz_si128(diff, diff))
				{
					DWORD* ptr = (DWORD*)&diff + 3;
					LONG c = 3;
					do
					{
						if (*ptr)
							break;
						--ptr;
					} while (--c);

					++j;
					a = cmp1 - spt;
					b = cmp2 - spt;
					width = (i << 2) + 3 - c;
					goto lbl_dword;
				}
			}
		}

		j = 0;

	lbl_dword:;
		ptr1 = (DWORD*)a + 3;
		ptr2 = (DWORD*)b + 3;
		for (LONG x = i; x < width; ++x, --ptr1, --ptr2)
		{
			DWORD* cmp1 = ptr1;
			DWORD* cmp2 = ptr2;
			for (LONG y = j; y < height; ++y, cmp1 -= pitch, cmp2 -= pitch)
				if (*cmp1 != *cmp2)
					return count - x;
		}

		return count - width;
	}
}

PixelBuffer::PixelBuffer(DWORD width, DWORD height, BOOL isTrue, GLenum format, UpdateMode mode)
{
	this->width = width;
	this->height = height;
	this->pitch = width;
	this->isTrue = isTrue;
	this->format = format;
	this->block = BLOCK_SIZE;
	this->reset = TRUE;

	if (!this->isTrue)
	{
		this->pitch >>= 1;
		this->block >>= 1;
		this->type = GL_UNSIGNED_SHORT_5_6_5;
	}
	else
		this->type = GL_UNSIGNED_BYTE;

	this->size = this->pitch * this->height * sizeof(DWORD);
	this->primaryBuffer = (DWORD*)AlignedAlloc(this->size);
	this->secondaryBuffer = (DWORD*)AlignedAlloc(this->size);
	MemoryZero(this->primaryBuffer, this->size);
	MemoryZero(this->secondaryBuffer, this->size);

	switch (mode)
	{
	case UpdateSSE:
		this->ForwardCompare = SSE::ForwardCompare;
		this->BackwardCompare = SSE::BackwardCompare;
		this->BlockForwardCompare = SSE::BlockForwardCompare;
		this->BlockBackwardCompare = SSE::BlockBackwardCompare;
		this->SideForwardCompare = SSE::SideForwardCompare;
		this->SideBackwardCompare = SSE::SideBackwardCompare;
		break;
	case UpdateCPP:
		this->ForwardCompare = CPP::ForwardCompare;
		this->BackwardCompare = CPP::BackwardCompare;
		this->BlockForwardCompare = CPP::BlockForwardCompare;
		this->BlockBackwardCompare = CPP::BlockBackwardCompare;
		this->SideForwardCompare = CPP::SideForwardCompare;
		this->SideBackwardCompare = CPP::SideBackwardCompare;
		break;
	case UpdateASM:
		this->ForwardCompare = ASM::ForwardCompare;
		this->BackwardCompare = ASM::BackwardCompare;
		this->BlockForwardCompare = ASM::BlockForwardCompare;
		this->BlockBackwardCompare = ASM::BlockBackwardCompare;
		this->SideForwardCompare = ASM::SideForwardCompare;
		this->SideBackwardCompare = ASM::SideBackwardCompare;
		break;
	default:
		this->ForwardCompare = NULL;
		this->BackwardCompare = NULL;
		this->BlockForwardCompare = NULL;
		this->BlockBackwardCompare = NULL;
		this->SideForwardCompare = NULL;
		this->SideBackwardCompare = NULL;
		break;
	}
}

PixelBuffer::~PixelBuffer()
{
	AlignedFree(this->primaryBuffer);
	AlignedFree(this->secondaryBuffer);
}

VOID PixelBuffer::Reset()
{
	this->reset = TRUE;
}

VOID PixelBuffer::Update(Rect* rect)
{
	GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->width);
	if (!this->ForwardCompare || this->reset)
	{
		if (rect)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rect->width, rect->height, this->format, this->type, this->primaryBuffer + rect->y * this->pitch + (this->isTrue ? rect->x : rect->x >> 1));
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, this->format, this->type, this->primaryBuffer);
	}
	else if (rect)
	{
		RECT rc = { rect->x, rect->y, rect->x + rect->width, rect->y + rect->height };
		if (!this->isTrue)
		{
			rc.left >>= 1;
			rc.right >>= 1;
		}
		this->UpdateBlock(&rc, (POINT*)rect);
	}
	else
	{
		DWORD left, right;
		DWORD length = this->pitch * this->height;
		if ((left = this->ForwardCompare(length, 0, this->primaryBuffer, this->secondaryBuffer))
			&& (right = this->BackwardCompare(length, length - 1, this->primaryBuffer, this->secondaryBuffer)))
		{
			DWORD top = (length - left) / this->pitch;
			DWORD bottom = (right - 1) / this->pitch + 1;

			POINT offset = { 0, 0 };
			for (DWORD y = top; y < bottom; y += this->block)
			{
				DWORD bt = y + this->block;
				if (bt > bottom)
					bt = bottom;

				for (DWORD x = 0; x < this->width; x += this->block)
				{
					DWORD rt = x + this->block;
					if (rt > this->width)
						rt = this->width;

					RECT rc = { *(LONG*)&x, *(LONG*)&y, *(LONG*)&rt, *(LONG*)&bt };
					this->UpdateBlock(&rc, &offset);
				}
			}
		}
	}
	GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

VOID PixelBuffer::UpdateBlock(RECT* rect, POINT* offset)
{
	RECT rc;
	LONG width = rect->right-- - rect->left;
	LONG height = rect->bottom-- - rect->top;
	if (this->BlockForwardCompare(width, height, this->pitch, rect->top * this->pitch + rect->left, this->primaryBuffer, this->secondaryBuffer, (POINT*)&rc.left)
		&& this->BlockBackwardCompare(width, height, this->pitch, rect->bottom * this->pitch + rect->right, this->primaryBuffer, this->secondaryBuffer, (POINT*)&rc.right))
	{
		if (rc.left > rc.right)
		{
			LONG p = rc.left;
			rc.left = rc.right;
			rc.right = p;
		}

		rc.left += rect->left;
		rc.right += rect->left;
		rc.top += rect->top;
		rc.bottom += rect->top;

		height = rc.bottom - rc.top;
		if (height)
		{
			width = rc.left - rect->left;
			if (width)
				rc.left -= this->SideForwardCompare(width, height, this->pitch, (rc.top + 1) * this->pitch + rect->left, this->primaryBuffer, this->secondaryBuffer);

			width = rect->right - rc.right;
			if (width)
				rc.right += this->SideBackwardCompare(width, height, this->pitch, (rc.bottom - 1) * this->pitch + rect->right, this->primaryBuffer, this->secondaryBuffer);
		}

		Rect rect = { rc.left, rc.top, rc.right - rc.left + 1, height + 1 };
		DWORD* ptr = this->primaryBuffer + rect.y * this->pitch + rect.x;

		if (!this->isTrue)
		{
			rect.x <<= 1;
			rect.width <<= 1;
		}

		GLTexSubImage2D(GL_TEXTURE_2D, 0, rect.x - offset->x, rect.y - offset->y, rect.width, rect.height, this->format, this->type, ptr);
	}
}

VOID PixelBuffer::Copy(VOID* buffer)
{
	MemoryCopy(this->primaryBuffer, buffer, this->size);
}

VOID* PixelBuffer::GetBuffer()
{
	return this->primaryBuffer;
}

VOID PixelBuffer::SwapBuffers()
{
	DWORD* buff = this->primaryBuffer;
	this->primaryBuffer = this->secondaryBuffer;
	this->secondaryBuffer = buff;

	this->reset = FALSE;
}