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
#include "OpenDraw.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Main.h"
#include "Hooks.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Window.h"

DWORD __fastcall GetPow2(DWORD value)
{
	DWORD res = 1;
	while (res < value)
		res <<= 1;
	return res;
}

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLBindAttribLocation(program->id, 0, "vCoord");
		GLBindAttribLocation(program->id, 1, "vTexCoord");

		GLuint vShader = GL::CompileShaderSource(program->vertexName, program->version, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(program->fragmentName, program->version, GL_FRAGMENT_SHADER);
		{
			GLAttachShader(program->id, vShader);
			GLAttachShader(program->id, fShader);
			{
				GLLinkProgram(program->id);
			}
			GLDetachShader(program->id, fShader);
			GLDetachShader(program->id, vShader);
		}
		GLDeleteShader(fShader);
		GLDeleteShader(vShader);

		GLUseProgram(program->id);
		GLUniformMatrix4fv(GLGetUniformLocation(program->id, "mvp"), 1, GL_FALSE, program->mvp);
		GLUniform1i(GLGetUniformLocation(program->id, "tex01"), 0);

		program->texSize.location = GLGetUniformLocation(program->id, "texSize");
		if (program->texSize.location >= 0)
		{
			program->texSize.value = texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
	else
	{
		GLUseProgram(program->id);

		if (program->texSize.location >= 0 && program->texSize.value != texSize)
		{
			program->texSize.value = texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	do
	{
		if (ddraw->width)
		{
			ddraw->hDc = ::GetDC(ddraw->hDraw);
			{
				if (!::GetPixelFormat(ddraw->hDc))
				{
					PIXELFORMATDESCRIPTOR pfd;
					GL::PreparePixelFormatDescription(&pfd);
					INT glPixelFormat = GL::PreparePixelFormat(&pfd);
					if (!glPixelFormat)
					{
						glPixelFormat = ::ChoosePixelFormat(ddraw->hDc, &pfd);
						if (!glPixelFormat)
							Main::ShowError(IDS_ERROR_CHOOSE_PF, __FILE__, __LINE__);
						else if (pfd.dwFlags & PFD_NEED_PALETTE)
							Main::ShowError(IDS_ERROR_NEED_PALETTE, __FILE__, __LINE__);
					}

					GL::ResetPixelFormatDescription(&pfd);
					if (::DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
						Main::ShowError(IDS_ERROR_DESCRIBE_PF, __FILE__, __LINE__);

					if (!::SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
						Main::ShowError(IDS_ERROR_SET_PF, __FILE__, __LINE__);

					if ((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.cRedBits < 5) || (pfd.cGreenBits < 6) || (pfd.cBlueBits < 5))
						Main::ShowError(IDS_ERROR_BAD_PF, __FILE__, __LINE__);
				}

				HGLRC hRc = wglCreateContext(ddraw->hDc);
				if (hRc)
				{
					if (wglMakeCurrent(ddraw->hDc, hRc))
					{
						GL::CreateContextAttribs(ddraw->hDc, &hRc);
						if (glVersion >= GL_VER_2_0)
						{
							DWORD glMaxTexSize;
							GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
							if (glMaxTexSize < GetPow2(ddraw->width > ddraw->height ? ddraw->width : ddraw->height))
								glVersion = GL_VER_1_1;
						}

						if (glVersion >= GL_VER_3_0)
							ddraw->RenderNew();
						else if (glVersion >= GL_VER_2_0)
							ddraw->RenderMid();
						else
							ddraw->RenderOld();

						wglMakeCurrent(ddraw->hDc, NULL);
					}

					wglDeleteContext(hRc);
				}
			}
			::ReleaseDC(ddraw->hDraw, ddraw->hDc);
			ddraw->hDc = NULL;
			break;
		}

		Sleep(0);
	} while (!ddraw->isFinish);

	return NULL;
}

VOID OpenDraw::RenderOld()
{
	if (this->filterState.interpolation > InterpolateLinear)
		this->filterState.interpolation = InterpolateLinear;

	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD maxAllow = GetPow2(this->width > this->height ? this->width : this->height);
	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;

	DWORD framePerWidth = this->width / maxTexSize + (this->width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->height / maxTexSize + (this->height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;
	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < this->height; y += maxTexSize)
		{
			DWORD height = this->height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < this->width; x += maxTexSize, ++frame)
			{
				DWORD width = this->width - x;
				if (width > maxTexSize)
					width = maxTexSize;

				frame->rect.x = x;
				frame->rect.y = y;
				frame->rect.width = width;
				frame->rect.height = height;

				frame->vSize.width = x + width;
				frame->vSize.height = y + height;

				frame->tSize.width = width == maxTexSize ? 1.0f : (FLOAT)width / maxTexSize;
				frame->tSize.height = height == maxTexSize ? 1.0f : (FLOAT)height / maxTexSize;

				GLGenTextures(1, &frame->id);
				GLBindTexture(GL_TEXTURE_2D, frame->id);

				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)this->width, (GLdouble)this->height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		VOID* frameBuffer = MemoryAlloc(maxTexSize * maxTexSize * sizeof(DWORD));
		{
			FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
			{
				BOOL isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(isVSync);

				Pointer pointersList[2];
				Pointer* pointer = pointersList;
				MemoryZero(pointersList, sizeof(pointersList));

				ICONINFO* iconInfo = NULL;
				BITMAP* maskInfo = NULL;
				BITMAP* colorInfo = NULL;

				DWORD clear = TRUE;
				do
				{
					OpenDrawSurface* surface = this->attachedSurface;
					if (surface)
					{
						OpenDrawPalette* palette = surface->attachedPalette;
						if (palette)
						{
							if (isVSync != config.image.vSync)
							{
								isVSync = config.image.vSync;
								if (WGLSwapInterval)
									WGLSwapInterval(isVSync);
							}

							if (fpsState)
							{
								if (isFpsChanged)
									fpsCounter->Reset();

								fpsCounter->Calculate();
							}
							
							if (isFpsChanged)
							{
								isFpsChanged = FALSE;
								clear = TRUE;
							}

							if (this->CheckView())
							{
								GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
								clear = TRUE;
							}

							DWORD glFilter = 0;
							FilterState state = this->filterState;
							this->filterState.flags = FALSE;
							if (state.flags)
								glFilter = state.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

							if (palette->isChanged)
							{
								palette->isChanged = FALSE;
								clear = TRUE;
							}

							pointer = &pointersList[pointer == pointersList];
							if (config.pointerIndex && !config.pointerHidden)
							{
								DWORD ptrIndex = config.pointerIndex - 1;

								iconInfo = &((ICONINFO*)Hooks::hookSpace->icons_info)[ptrIndex];
								maskInfo = &((BITMAP*)Hooks::hookSpace->masks_info)[ptrIndex];
								colorInfo = Hooks::hookSpace->colors_info && maskInfo->bmHeight == maskInfo->bmWidth
									? &((BITMAP*)Hooks::hookSpace->colors_info)[ptrIndex]
									: NULL;

								GetCursorPos(&pointer->pos);
								ScreenToClient(this->hWnd, &pointer->pos);

								pointer->pos.x = (LONG)((FLOAT)((pointer->pos.x - this->viewport.rectangle.x) * this->width) / this->viewport.rectangle.width) - iconInfo->xHotspot;
								pointer->pos.y = (LONG)((FLOAT)((pointer->pos.y - this->viewport.rectangle.y) * this->height) / this->viewport.rectangle.height) - iconInfo->yHotspot;

								pointer->size.cx = 32;
								pointer->size.cy = 32;

								if (pointer->pos.x < 0)
								{
									pointer->offset.x = -pointer->pos.x;
									pointer->size.cx += pointer->pos.x;
									pointer->pos.x = 0;
								}
								else
									pointer->offset.x = 0;

								if (pointer->pos.y < 0)
								{
									pointer->offset.y = -pointer->pos.y;
									pointer->size.cy += pointer->pos.y;
									pointer->pos.y = 0;
								}
								else
									pointer->offset.y = 0;

								if (pointer->size.cx > 0 && pointer->size.cy > 0)
								{
									if (pointer->pos.x + pointer->size.cx >= *(INT*)&this->width)
										pointer->size.cx = this->width - pointer->pos.x;

									if (pointer->pos.y + pointer->size.cy >= *(INT*)&this->height)
										pointer->size.cy = this->height - pointer->pos.y;

									if (pointer->size.cx > 0 && pointer->size.cy > 0)
										pointer->isActive = TRUE;
								}
							}

							UpdateRect* updateClip = surface->poinetrClip;
							UpdateRect* finClip = surface->currentClip;
							surface->poinetrClip = finClip;

							if (clear)
							{
								if (clear < 3)
								{
									if (clear & 1)
									{
										updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
										updateClip->rect.left = 0;
										updateClip->rect.top = 0;
										updateClip->rect.right = this->width;
										updateClip->rect.bottom = this->height;
										updateClip->isActive = TRUE;
									}

									++clear;
								}
								else
									clear = FALSE;

								GLClear(GL_COLOR_BUFFER_BIT);
							}

							DWORD count = frameCount;
							frame = frames;
							while (count--)
							{
								if (frameCount == 1)
								{
									if (glFilter)
									{
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
									}

									while (updateClip != finClip)
									{
										if (updateClip->isActive)
										{
											RECT update = updateClip->rect;
											DWORD texWidth = update.right - update.left;
											DWORD texHeight = update.bottom - update.top;

											if (texWidth == this->width)
												GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixelBuffer + update.top * texWidth);
											else
											{
												DWORD* source = surface->pixelBuffer + update.top * this->width + update.left;
												DWORD* dest = (DWORD*)frameBuffer;
												DWORD copyHeight = texHeight;
												do
												{
													MemoryCopy(dest, source, texWidth << 2);
													source += this->width;
													dest += texWidth;
												} while (--copyHeight);

												GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
											}
										}

										if (++updateClip == surface->endClip)
											updateClip = surface->clipsList;
									}

									// Clear prev pointer
									Pointer* prevPointer = &pointersList[pointer == pointersList];
									if (prevPointer->isActive)
									{
										DWORD* source = surface->pixelBuffer + prevPointer->pos.y * this->width + prevPointer->pos.x;
										DWORD* dest = (DWORD*)frameBuffer;

										LONG copyHeight = prevPointer->size.cy;
										do
										{
											MemoryCopy(dest, source, prevPointer->size.cx << 2);
											source += this->width;
											dest += prevPointer->size.cx;
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, prevPointer->pos.x, prevPointer->pos.y, prevPointer->size.cx, prevPointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}

									// Update pointer
									if (pointer->isActive)
									{
										DWORD* source = surface->pixelBuffer + pointer->pos.y * this->width + pointer->pos.x;
										DWORD* dst = (DWORD*)frameBuffer;

										DWORD initMask = 8 - (pointer->offset.x % 8);
										DWORD initOffset = pointer->offset.x & (8 - 1);

										INT shadowIdx = pointer->offset.y - SHADOW_OFFSET;
										LONG copyHeight = pointer->size.cy;

										if (colorInfo)
										{
											BYTE* sourceColor = (BYTE*)colorInfo->bmBits + pointer->offset.y * colorInfo->bmWidthBytes + pointer->offset.x;
											BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes;
											BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;

											do
											{
												DWORD* src = source;
												BYTE* srcColor = sourceColor;
												BYTE* srcMask = sourceMask;
												BYTE* shadMask = shadowMask;

												BYTE xorMask = *srcMask++;
												BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

												xorMask <<= initOffset;
												shdMask <<= initOffset;

												DWORD countMask = initMask;
												LONG copyWidth = pointer->size.cx;
												do
												{
													if (xorMask & 0x80)
													{
														DWORD color = *src;

														if (!(shdMask & 0x80))
														{
															BYTE* cp = (BYTE*)&color;
															DWORD cc = 3;
															do
																*cp++ >>= 1;
															while (--cc);
														}

														*dst = color;
													}
													else
														*dst = _byteswap_ulong(_rotl(Hooks::palEntries[*srcColor], 8));

													if (--countMask)
													{
														xorMask <<= 1;
														shdMask <<= 1;
													}
													else
													{
														countMask = 8;
														xorMask = *srcMask++;
														shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
													}

													++src;
													++srcColor;
													++dst;
												} while (--copyWidth);

												source += this->width;
												sourceColor += colorInfo->bmWidthBytes;
												sourceMask += maskInfo->bmWidthBytes;
												shadowMask += maskInfo->bmWidthBytes;

												++shadowIdx;
											} while (--copyHeight);
										}
										else
										{
											BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes;
											BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;
											BYTE* colorMask = (BYTE*)sourceMask + 32 * maskInfo->bmWidthBytes;

											do
											{
												DWORD* src = source;
												BYTE* clrMask = colorMask;
												BYTE* srcMask = sourceMask;
												BYTE* shadMask = shadowMask;

												BYTE andMask = *clrMask++;
												BYTE xorMask = *srcMask++;
												BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

												andMask <<= initOffset;
												xorMask <<= initOffset;
												shdMask <<= initOffset;

												DWORD countMask = initMask;
												LONG copyWidth = pointer->size.cx;
												do
												{
													if (xorMask & 0x80)
													{
														DWORD color = *src;

														if (!(shdMask & 0x80))
														{
															BYTE* cp = (BYTE*)&color;
															DWORD cc = 3;
															do
																*cp++ >>= 1;
															while (--cc);
														}

														*dst = color;
													}
													else
														*dst = (andMask & 0x80) ? 0xFFFFFFFF : 0xFF000000;

													if (--countMask)
													{
														andMask <<= 1;
														xorMask <<= 1;
														shdMask <<= 1;
													}
													else
													{
														countMask = 8;
														andMask = *clrMask++;
														xorMask = *srcMask++;
														shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
													}

													++src;
													++dst;
												} while (--copyWidth);

												source += this->width;

												colorMask += maskInfo->bmWidthBytes;
												sourceMask += maskInfo->bmWidthBytes;
												shadowMask += maskInfo->bmWidthBytes;

												++shadowIdx;
											} while (--copyHeight);
										}

										GLTexSubImage2D(GL_TEXTURE_2D, 0, pointer->pos.x, pointer->pos.y, pointer->size.cx, pointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
								else
								{
									GLBindTexture(GL_TEXTURE_2D, frame->id);

									if (glFilter)
									{
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
									}

									Rect* rect = &frame->rect;
									INT rect_right = rect->x + rect->width;
									INT rect_bottom = rect->y + rect->height;

									UpdateRect* update = updateClip;
									while (update != finClip)
									{
										if (update->isActive)
										{
											RECT clip = {
												rect->x > update->rect.left ? rect->x : update->rect.left,
												rect->y > update->rect.top ? rect->y : update->rect.top,
												rect_right < update->rect.right ? rect_right : update->rect.right,
												rect_bottom < update->rect.bottom ? rect_bottom : update->rect.bottom
											};

											INT clipWidth = clip.right - clip.left;
											INT clipHeight = clip.bottom - clip.top;
											if (clipWidth > 0 && clipHeight > 0)
											{
												DWORD* source = surface->pixelBuffer + clip.top * this->width + clip.left;
												DWORD* dest = (DWORD*)frameBuffer;
												INT copyHeight = clipHeight;
												do
												{
													MemoryCopy(dest, source, clipWidth << 2);
													source += this->width;
													dest += clipWidth;
												} while (--copyHeight);

												GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - rect->x, clip.top - rect->y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
											}
										}

										if (++update == surface->endClip)
											update = surface->clipsList;
									}

									// Clear prev pointer
									Pointer* prevPointer = &pointersList[pointer == pointersList];
									if (prevPointer->isActive)
									{
										INT ptr_right = prevPointer->pos.x + prevPointer->size.cx;
										INT ptr_bottom = prevPointer->pos.y + prevPointer->size.cy;

										RECT clip = {
											rect->x > prevPointer->pos.x ? rect->x : prevPointer->pos.x,
											rect->y > prevPointer->pos.y ? rect->y : prevPointer->pos.y,
											rect_right < ptr_right ? rect_right : ptr_right,
											rect_bottom < ptr_bottom ? rect_bottom : ptr_bottom
										};

										INT clipWidth = clip.right - clip.left;
										INT clipHeight = clip.bottom - clip.top;
										if (clipWidth > 0 && clipHeight > 0)
										{
											DWORD* source = surface->pixelBuffer + clip.top * this->width + clip.left;
											DWORD* dest = (DWORD*)frameBuffer;
											INT copyHeight = clipHeight;
											do
											{
												MemoryCopy(dest, source, clipWidth << 2);
												source += this->width;
												dest += clipWidth;
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - rect->x, clip.top - rect->y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
										}
									}

									// Update pointer
									if (pointer->isActive)
									{
										INT ptr_right = pointer->pos.x + pointer->size.cx;
										INT ptr_bottom = pointer->pos.y + pointer->size.cy;

										RECT clip = {
											rect->x > pointer->pos.x ? rect->x : pointer->pos.x,
											rect->y > pointer->pos.y ? rect->y : pointer->pos.y,
											rect_right < ptr_right ? rect_right : ptr_right,
											rect_bottom < ptr_bottom ? rect_bottom : ptr_bottom
										};

										INT clipWidth = clip.right - clip.left;
										INT clipHeight = clip.bottom - clip.top;
										if (clipWidth > 0 && clipHeight > 0)
										{
											DWORD* source = surface->pixelBuffer + clip.top * this->width + clip.left;
											DWORD* dst = (DWORD*)frameBuffer;

											POINT offset = {
												clip.left - pointer->pos.x,
												clip.top - pointer->pos.y
											};

											DWORD initMask = 8 - (offset.x % 8);
											DWORD initOffset = offset.x & (8 - 1);

											INT shadowIdx = offset.y - SHADOW_OFFSET;
											INT copyHeight = clipHeight;
											if (colorInfo)
											{
												BYTE* sourceColor = (BYTE*)colorInfo->bmBits + offset.y * colorInfo->bmWidthBytes + offset.x;
												BYTE* sourceMask = (BYTE*)maskInfo->bmBits + offset.y * maskInfo->bmWidthBytes + (offset.x / 8);
												BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;

												do
												{
													DWORD* src = source;
													BYTE* srcColor = sourceColor;
													BYTE* srcMask = sourceMask;
													BYTE* shadMask = shadowMask;

													BYTE xorMask = *srcMask++;
													BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

													xorMask <<= initOffset;
													shdMask <<= initOffset;

													DWORD countMask = initMask;
													LONG copyWidth = clipWidth;
													do
													{
														if (xorMask & 0x80)
														{
															DWORD color = *src;

															if (!(shdMask & 0x80))
															{
																BYTE* cp = (BYTE*)&color;
																DWORD cc = 3;
																do
																	*cp++ >>= 1;
																while (--cc);
															}

															*dst = color;
														}
														else
															*dst = _byteswap_ulong(_rotl(Hooks::palEntries[*srcColor], 8));

														if (--countMask)
														{
															xorMask <<= 1;
															shdMask <<= 1;
														}
														else
														{
															countMask = 8;
															xorMask = *srcMask++;
															shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
														}

														++src;
														++srcColor;
														++dst;
													} while (--copyWidth);

													source += this->width;
													sourceColor += colorInfo->bmWidthBytes;
													sourceMask += maskInfo->bmWidthBytes;
													shadowMask += maskInfo->bmWidthBytes;

													++shadowIdx;
												} while (--copyHeight);
											}
											else
											{
												BYTE* sourceMask = (BYTE*)maskInfo->bmBits + offset.y * maskInfo->bmWidthBytes + (offset.x / 8);
												BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;
												BYTE* colorMask = (BYTE*)sourceMask + 32 * maskInfo->bmWidthBytes;

												do
												{
													DWORD* src = source;
													BYTE* clrMask = colorMask;
													BYTE* srcMask = sourceMask;
													BYTE* shadMask = shadowMask;

													BYTE andMask = *clrMask++;
													BYTE xorMask = *srcMask++;
													BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

													andMask <<= initOffset;
													xorMask <<= initOffset;
													shdMask <<= initOffset;

													DWORD countMask = initMask;
													LONG copyWidth = clipWidth;
													do
													{
														if (xorMask & 0x80)
														{
															DWORD color = *src;

															if (!(shdMask & 0x80))
															{
																BYTE* cp = (BYTE*)&color;
																DWORD cc = 3;
																do
																	*cp++ >>= 1;
																while (--cc);
															}

															*dst = color;
														}
														else
															*dst = (andMask & 0x80) ? 0xFFFFFFFF : 0xFF000000;

														if (--countMask)
														{
															andMask <<= 1;
															xorMask <<= 1;
															shdMask <<= 1;
														}
														else
														{
															countMask = 8;
															andMask = *clrMask++;
															xorMask = *srcMask++;
															shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
														}

														++src;
														++dst;
													} while (--copyWidth);

													source += this->width;

													colorMask += maskInfo->bmWidthBytes;
													sourceMask += maskInfo->bmWidthBytes;
													shadowMask += maskInfo->bmWidthBytes;

													++shadowIdx;
												} while (--copyHeight);
											}

											GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - rect->x, clip.top - rect->y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
										}
									}
								}

								if (fpsState && frame == frames)
								{
									DWORD fps = fpsCounter->value;
									DWORD digCount = 0;
									DWORD current = fps;
									do
									{
										++digCount;
										current = current / 10;
									} while (current);

									DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
									DWORD dcount = digCount;
									do
									{
										WORD* lpDig = (WORD*)counters[fps % 10];

										for (DWORD y = 0; y < FPS_HEIGHT; ++y)
										{
											DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
											DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

											WORD check = *lpDig++;
											DWORD width = FPS_WIDTH;
											do
											{
												*pix++ = (check & 1) ? fpsColor : *idx;
												++idx;
												check >>= 1;
											} while (--width);
										}

										fps = fps / 10;
									} while (--dcount);

									dcount = 4;
									while (dcount != digCount)
									{
										for (DWORD y = 0; y < FPS_HEIGHT; ++y)
										{
											DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
											DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

											DWORD width = FPS_WIDTH;
											do
												*pix++ = *idx++;
											while (--width);
										}

										--dcount;
									}

									GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
								}

								GLBegin(GL_TRIANGLE_FAN);
								{
									GLTexCoord2f(0.0f, 0.0f);
									GLVertex2s(frame->rect.x, frame->rect.y);

									GLTexCoord2f(frame->tSize.width, 0.0f);
									GLVertex2s(frame->vSize.width, frame->rect.y);

									GLTexCoord2f(frame->tSize.width, frame->tSize.height);
									GLVertex2s(frame->vSize.width, frame->vSize.height);

									GLTexCoord2f(0.0f, frame->tSize.height);
									GLVertex2s(frame->rect.x, frame->vSize.height);
								}
								GLEnd();
								++frame;
							}

							pointersList[pointer == pointersList].isActive = FALSE;

							if (this->isTakeSnapshot)
							{
								this->isTakeSnapshot = FALSE;

								if (OpenClipboard(NULL))
								{
									EmptyClipboard();

									DWORD texWidth = this->width;
									DWORD texHeight = this->height;
									DWORD dataSize = texWidth * texHeight * sizeof(WORD);
									HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + 1024 + dataSize);
									{
										VOID* data = GlobalLock(hMemory);
										{
											BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
											MemoryZero(bmi, sizeof(BITMAPV5HEADER));
											bmi->bV5Size = sizeof(BITMAPV5HEADER);
											bmi->bV5Width = texWidth;
											bmi->bV5Height = -*(LONG*)&texHeight;
											bmi->bV5Planes = 1;
											bmi->bV5BitCount = 8;
											bmi->bV5Compression = BI_RGB;

											DWORD* src = (DWORD*)surface->attachedPalette->entries;
											DWORD* dst = (DWORD*)((BYTE*)data + sizeof(BITMAPV5HEADER));
											DWORD count = 256;
											do
											{
												*dst++ = ((*src & 0x00FF0000) >> 16) | (*src & 0x0000FF00) | ((*src & 0x000000FF) << 16);
												++src;
											} while (--count);
											MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER) + 1024, surface->indexBuffer, dataSize);
										}
										GlobalUnlock(hMemory);

										SetClipboardData(CF_DIBV5, hMemory);
									}
									GlobalFree(hMemory);

									CloseClipboard();
									clear = TRUE;
								}
							}

							SwapBuffers(this->hDc);
							if (!clear && fpsState != FpsBenchmark)
								WaitForSingleObject(this->hDrawEvent, INFINITE);
							if (isVSync)
								GLFinish();
						}
					}
				} while (!this->isFinish);
			}
			delete fpsCounter;
		}
		MemoryFree(frameBuffer);

		frame = frames;
		DWORD count = frameCount;
		while (count--)
		{
			GLDeleteTextures(1, &frame->id);
			++frame;
		}
	}
	MemoryFree(frames);
}

VOID OpenDraw::RenderMid()
{
	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	DWORD maxTexSize = GetPow2(this->width > this->height ? this->width : this->height);
	FLOAT texWidth = this->width == maxTexSize ? 1.0f : (FLOAT)this->width / maxTexSize;
	FLOAT texHeight = this->height == maxTexSize ? 1.0f : (FLOAT)this->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->width, (FLOAT)this->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->height, 0.0f, texHeight }
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct
	{
		ShaderProgram linear;
		ShaderProgram hermite;
		ShaderProgram cubic;
	} shaders = {
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix }
	};

	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

				GLEnableVertexAttribArray(0);
				GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

				GLEnableVertexAttribArray(1);
				GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

				GLuint textureId;
				GLGenTextures(1, &textureId);
				{
					GLActiveTexture(GL_TEXTURE0);
					GLBindTexture(GL_TEXTURE_2D, textureId);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

					VOID* frameBuffer = MemoryAlloc(this->width * this->height * sizeof(DWORD));
					{
						FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
						{
							BOOL isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);

							Pointer pointersList[2];
							Pointer* pointer = pointersList;
							MemoryZero(pointersList, sizeof(pointersList));

							ICONINFO* iconInfo = NULL;
							BITMAP* maskInfo = NULL;
							BITMAP* colorInfo = NULL;

							DWORD clear = TRUE;
							do
							{
								OpenDrawSurface* surface = this->attachedSurface;
								if (surface)
								{
									OpenDrawPalette* palette = surface->attachedPalette;
									if (palette)
									{
										if (isVSync != config.image.vSync)
										{
											isVSync = config.image.vSync;
											if (WGLSwapInterval)
												WGLSwapInterval(isVSync);
										}

										FilterState state = this->filterState;
										this->filterState.flags = FALSE;

										if (state.flags)
										{
											this->viewport.refresh = TRUE;
											isFpsChanged = TRUE;
										}

										if (fpsState)
										{
											if (isFpsChanged)
												fpsCounter->Reset();

											fpsCounter->Calculate();
										}
										
										if(isFpsChanged)
										{
											isFpsChanged = FALSE;
											clear = TRUE;
										}

										BOOL isTakeSnapshot = this->isTakeSnapshot;
										if (isTakeSnapshot)
											this->isTakeSnapshot = FALSE;

										UpdateRect* updateClip = surface->poinetrClip;
										UpdateRect* finClip = surface->currentClip;
										surface->poinetrClip = finClip;

										if (this->CheckView())
										{
											GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
											clear = TRUE;
										}

										if (state.flags)
										{
											ShaderProgram* program;
											switch (state.interpolation)
											{
											case InterpolateHermite:
												program = &shaders.hermite;
												break;
											case InterpolateCubic:
												program = &shaders.cubic;
												break;
											default:
												program = &shaders.linear;
												break;
											}
											UseShaderProgram(program, texSize);

											DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
											GLBindTexture(GL_TEXTURE_2D, textureId);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
										}

										if (palette->isChanged)
										{
											palette->isChanged = FALSE;
											clear = TRUE;
										}

										if (clear)
										{
											if (clear < 3)
											{
												if (clear & 1)
												{
													updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
													updateClip->rect.left = 0;
													updateClip->rect.top = 0;
													updateClip->rect.right = this->width;
													updateClip->rect.bottom = this->height;
													updateClip->isActive = TRUE;
												}

												++clear;
											}
											else
												clear = FALSE;

											GLClear(GL_COLOR_BUFFER_BIT);
										}

										// NEXT UNCHANGED
										{
											// Update texture
											while (updateClip != finClip)
											{
												if (updateClip->isActive)
												{
													RECT update = updateClip->rect;
													DWORD texWidth = update.right - update.left;
													DWORD texHeight = update.bottom - update.top;

													if (texWidth == this->width)
														GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixelBuffer + update.top * texWidth);
													else
													{
														DWORD* source = surface->pixelBuffer + update.top * this->width + update.left;
														DWORD* dest = (DWORD*)frameBuffer;
														DWORD copyHeight = texHeight;
														do
														{
															MemoryCopy(dest, source, texWidth << 2);
															source += this->width;
															dest += texWidth;
														} while (--copyHeight);

														GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
													}
												}

												if (++updateClip == surface->endClip)
													updateClip = surface->clipsList;
											}

											// Clear prev pointer
											if (pointer->isActive)
											{
												pointer->isActive = FALSE;

												DWORD* source = surface->pixelBuffer + pointer->pos.y * this->width + pointer->pos.x;
												DWORD* dest = (DWORD*)frameBuffer;

												LONG copyHeight = pointer->size.cy;
												do
												{
													MemoryCopy(dest, source, pointer->size.cx << 2);
													source += this->width;
													dest += pointer->size.cx;
												} while (--copyHeight);

												GLTexSubImage2D(GL_TEXTURE_2D, 0, pointer->pos.x, pointer->pos.y, pointer->size.cx, pointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
											}

											// Update pointer
											pointer = &pointersList[pointer == pointersList];
											if (config.pointerIndex && !config.pointerHidden)
											{
												DWORD ptrIndex = config.pointerIndex - 1;

												iconInfo = &((ICONINFO*)Hooks::hookSpace->icons_info)[ptrIndex];
												maskInfo = &((BITMAP*)Hooks::hookSpace->masks_info)[ptrIndex];
												colorInfo = Hooks::hookSpace->colors_info && maskInfo->bmHeight == maskInfo->bmWidth
													? &((BITMAP*)Hooks::hookSpace->colors_info)[ptrIndex]
													: NULL;

												GetCursorPos(&pointer->pos);
												ScreenToClient(this->hWnd, &pointer->pos);

												pointer->pos.x = (LONG)((FLOAT)((pointer->pos.x - this->viewport.rectangle.x) * this->width) / this->viewport.rectangle.width) - iconInfo->xHotspot;
												pointer->pos.y = (LONG)((FLOAT)((pointer->pos.y - this->viewport.rectangle.y) * this->height) / this->viewport.rectangle.height) - iconInfo->yHotspot;

												pointer->size.cx = 32;
												pointer->size.cy = 32;

												if (pointer->pos.x < 0)
												{
													pointer->offset.x = -pointer->pos.x;
													pointer->size.cx += pointer->pos.x;
													pointer->pos.x = 0;
												}
												else
													pointer->offset.x = 0;

												if (pointer->pos.y < 0)
												{
													pointer->offset.y = -pointer->pos.y;
													pointer->size.cy += pointer->pos.y;
													pointer->pos.y = 0;
												}
												else
													pointer->offset.y = 0;

												if (pointer->size.cx > 0 && pointer->size.cy > 0)
												{
													if (pointer->pos.x + pointer->size.cx >= *(INT*)&this->width)
														pointer->size.cx = this->width - pointer->pos.x;

													if (pointer->pos.y + pointer->size.cy >= *(INT*)&this->height)
														pointer->size.cy = this->height - pointer->pos.y;

													if (pointer->size.cx > 0 && pointer->size.cy > 0)
													{
														pointer->isActive = TRUE;

														DWORD* source = surface->pixelBuffer + pointer->pos.y * this->width + pointer->pos.x;
														DWORD* dst = (DWORD*)frameBuffer;

														DWORD initMask = 8 - (pointer->offset.x % 8);
														DWORD initOffset = pointer->offset.x & (8 - 1);

														INT shadowIdx = pointer->offset.y - SHADOW_OFFSET;
														LONG copyHeight = pointer->size.cy;

														if (colorInfo)
														{
															BYTE* sourceColor = (BYTE*)colorInfo->bmBits + pointer->offset.y * colorInfo->bmWidthBytes + pointer->offset.x;
															BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes + (pointer->offset.x / 8);
															BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;

															do
															{
																DWORD* src = source;
																BYTE* srcColor = sourceColor;
																BYTE* srcMask = sourceMask;
																BYTE* shadMask = shadowMask;

																BYTE xorMask = *srcMask++;
																BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

																xorMask <<= initOffset;
																shdMask <<= initOffset;

																DWORD countMask = initMask;
																LONG copyWidth = pointer->size.cx;
																do
																{
																	if (xorMask & 0x80)
																	{
																		DWORD color = *src;

																		if (!(shdMask & 0x80))
																		{
																			BYTE* cp = (BYTE*)&color;
																			DWORD cc = 3;
																			do
																				*cp++ >>= 1;
																			while (--cc);
																		}

																		*dst = color;
																	}
																	else
																		*dst = _byteswap_ulong(_rotl(Hooks::palEntries[*srcColor], 8));

																	if (--countMask)
																	{
																		xorMask <<= 1;
																		shdMask <<= 1;
																	}
																	else
																	{
																		countMask = 8;
																		xorMask = *srcMask++;
																		shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
																	}

																	++src;
																	++srcColor;
																	++dst;
																} while (--copyWidth);

																source += this->width;
																sourceColor += colorInfo->bmWidthBytes;
																sourceMask += maskInfo->bmWidthBytes;
																shadowMask += maskInfo->bmWidthBytes;

																++shadowIdx;
															} while (--copyHeight);
														}
														else
														{
															BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes + (pointer->offset.x / 8);
															BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;
															BYTE* colorMask = (BYTE*)sourceMask + 32 * maskInfo->bmWidthBytes;

															do
															{
																DWORD* src = source;
																BYTE* clrMask = colorMask;
																BYTE* srcMask = sourceMask;
																BYTE* shadMask = shadowMask;

																BYTE andMask = *clrMask++;
																BYTE xorMask = *srcMask++;
																BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

																andMask <<= initOffset;
																xorMask <<= initOffset;
																shdMask <<= initOffset;

																DWORD countMask = initMask;
																LONG copyWidth = pointer->size.cx;
																do
																{
																	if (xorMask & 0x80)
																	{
																		DWORD color = *src;

																		if (!(shdMask & 0x80))
																		{
																			BYTE* cp = (BYTE*)&color;
																			DWORD cc = 3;
																			do
																				*cp++ >>= 1;
																			while (--cc);
																		}

																		*dst = color;
																	}
																	else
																		*dst = (andMask & 0x80) ? 0xFFFFFFFF : 0xFF000000;

																	if (--countMask)
																	{
																		andMask <<= 1;
																		xorMask <<= 1;
																		shdMask <<= 1;
																	}
																	else
																	{
																		countMask = 8;
																		andMask = *clrMask++;
																		xorMask = *srcMask++;
																		shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
																	}

																	++src;
																	++dst;
																} while (--copyWidth);

																source += this->width;

																colorMask += maskInfo->bmWidthBytes;
																sourceMask += maskInfo->bmWidthBytes;
																shadowMask += maskInfo->bmWidthBytes;

																++shadowIdx;
															} while (--copyHeight);
														}

														GLTexSubImage2D(GL_TEXTURE_2D, 0, pointer->pos.x, pointer->pos.y, pointer->size.cx, pointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
													}
												}
											}

											// Update FPS
											if (fpsState && !isTakeSnapshot)
											{
												DWORD fps = fpsCounter->value;
												DWORD digCount = 0;
												DWORD current = fps;
												do
												{
													++digCount;
													current = current / 10;
												} while (current);

												DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
												DWORD dcount = digCount;
												do
												{
													WORD* lpDig = (WORD*)counters[fps % 10];

													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
														DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

														WORD check = *lpDig++;
														DWORD width = FPS_WIDTH;
														do
														{
															*pix++ = (check & 1) ? fpsColor : *idx;
															++idx;
															check >>= 1;
														} while (--width);
													}

													fps = fps / 10;
												} while (--dcount);

												dcount = 4;
												while (dcount != digCount)
												{
													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
														DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

														DWORD width = FPS_WIDTH;
														do
															*pix++ = *idx++;
														while (--width);
													}

													--dcount;
												}

												GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
											}

											GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
										}

										if (isTakeSnapshot)
										{
											if (OpenClipboard(NULL))
											{
												EmptyClipboard();

												DWORD texWidth = this->width;
												DWORD texHeight = this->height;
												DWORD dataSize = texWidth * texHeight * sizeof(WORD);
												HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + 1024 + dataSize);
												{
													VOID* data = GlobalLock(hMemory);
													{
														BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
														MemoryZero(bmi, sizeof(BITMAPV5HEADER));
														bmi->bV5Size = sizeof(BITMAPV5HEADER);
														bmi->bV5Width = texWidth;
														bmi->bV5Height = -*(LONG*)&texHeight;
														bmi->bV5Planes = 1;
														bmi->bV5BitCount = 8;
														bmi->bV5Compression = BI_RGB;

														DWORD* src = (DWORD*)surface->attachedPalette->entries;
														DWORD* dst = (DWORD*)((BYTE*)data + sizeof(BITMAPV5HEADER));
														DWORD count = 256;
														do
														{
															*dst++ = ((*src & 0x00FF0000) >> 16) | (*src & 0x0000FF00) | ((*src & 0x000000FF) << 16);
															++src;
														} while (--count);
														MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER) + 1024, surface->indexBuffer, dataSize);
													}
													GlobalUnlock(hMemory);

													SetClipboardData(CF_DIBV5, hMemory);
												}
												GlobalFree(hMemory);

												CloseClipboard();
												clear = TRUE;
											}
										}

										// Swap
										SwapBuffers(this->hDc);
										if (!clear && fpsState != FpsBenchmark)
											WaitForSingleObject(this->hDrawEvent, INFINITE);
										if (isVSync)
											GLFinish();
									}
								}
							} while (!this->isFinish);
						}
						delete fpsCounter;
					}
					MemoryFree(frameBuffer);
				}
				GLDeleteTextures(1, &textureId);
			}
			GLBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
		GLDeleteBuffers(1, &bufferName);
	}
	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

		++shaderProgram;
	} while (--count);
}

VOID OpenDraw::RenderNew()
{
	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	DWORD maxTexSize = GetPow2(this->width > this->height ? this->width : this->height);
	FLOAT texWidth = this->width == maxTexSize ? 1.0f : (FLOAT)this->width / maxTexSize;
	FLOAT texHeight = this->height == maxTexSize ? 1.0f : (FLOAT)this->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[8][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->width, (FLOAT)this->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->height, 0.0f, texHeight },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ (FLOAT)this->width, 0.0f, 1.0f, 1.0f },
		{ (FLOAT)this->width, (FLOAT)this->height, 1.0f, 0.0f },
		{ 0.0f, (FLOAT)this->height, 0.0f, 0.0f }
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct
	{
		ShaderProgram stencil;
		ShaderProgram linear;
		ShaderProgram hermite;
		ShaderProgram cubic;
		ShaderProgram xBRz_2x;
		ShaderProgram xBRz_3x;
		ShaderProgram xBRz_4x;
		ShaderProgram xBRz_5x;
		ShaderProgram xBRz_6x;
		ShaderProgram scaleHQ_2x;
		ShaderProgram scaleHQ_4x;
		ShaderProgram xSal_2x;
		ShaderProgram eagle_2x;
		ShaderProgram scaleNx_2x;
		ShaderProgram scaleNx_3x;
	} shaders = {
		{ 0, GLSL_VER_1_30, IDR_STENCIL_VERTEX, IDR_STENCIL_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_3X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_4X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_5X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_6X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_2X, IDR_SCALEHQ_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_4X, IDR_SCALEHQ_FRAGMENT_4X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XSAL_VERTEX, IDR_XSAL_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_EAGLE_VERTEX, IDR_EAGLE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_2X, IDR_SCALENX_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_3X, IDR_SCALENX_FRAGMENT_3X, (GLfloat*)mvpMatrix }
	};

	{
		POINTFLOAT* stencil = NULL;
		GLuint stArrayName, stBufferName, arrayName, bufferName;

		GLGenVertexArrays(1, &arrayName);
		{
			GLBindVertexArray(arrayName);
			{
				GLGenBuffers(1, &bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
					{
						GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

						GLEnableVertexAttribArray(0);
						GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

						GLEnableVertexAttribArray(1);
						GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

						GLuint textureId;
						GLGenTextures(1, &textureId);
						{
							GLActiveTexture(GL_TEXTURE0);
							GLBindTexture(GL_TEXTURE_2D, textureId);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0, tboId = 0;
								{
									GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

									VOID* frameBuffer = MemoryAlloc(this->width * this->height * sizeof(DWORD));
									{
										FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
										{
											ShaderProgram* upscaleProgram;
											BOOL isVSync = config.image.vSync;
											if (WGLSwapInterval)
												WGLSwapInterval(isVSync);

											Pointer pointersList[2];
											Pointer* pointer = pointersList;
											MemoryZero(pointersList, sizeof(pointersList));

											ICONINFO* iconInfo = NULL;
											BITMAP* maskInfo = NULL;
											BITMAP* colorInfo = NULL;

											DWORD clear = TRUE;
											do
											{
												OpenDrawSurface* surface = this->attachedSurface;
												if (surface)
												{
													OpenDrawPalette* palette = surface->attachedPalette;
													if (palette)
													{
														if (isVSync != config.image.vSync)
														{
															isVSync = config.image.vSync;
															if (WGLSwapInterval)
																WGLSwapInterval(isVSync);
														}

														FilterState state = this->filterState;
														this->filterState.flags = FALSE;

														if (state.flags)
														{
															this->viewport.refresh = TRUE;
															isFpsChanged = TRUE;
														}

														if (fpsState)
														{
															if (isFpsChanged)
																fpsCounter->Reset();

															fpsCounter->Calculate();
														}
														
														if (isFpsChanged)
														{
															isFpsChanged = FALSE;
															clear = TRUE;
														}

														BOOL isTakeSnapshot = this->isTakeSnapshot;
														if (isTakeSnapshot)
															this->isTakeSnapshot = FALSE;

														pointer = &pointersList[pointer == pointersList];
														if (config.pointerIndex && !config.pointerHidden && this->viewport.rectangle.width && this->viewport.rectangle.height)
														{
															DWORD ptrIndex = config.pointerIndex - 1;

															iconInfo = &((ICONINFO*)Hooks::hookSpace->icons_info)[ptrIndex];
															maskInfo = &((BITMAP*)Hooks::hookSpace->masks_info)[ptrIndex];
															colorInfo = Hooks::hookSpace->colors_info && maskInfo->bmHeight == maskInfo->bmWidth
																? &((BITMAP*)Hooks::hookSpace->colors_info)[ptrIndex]
																: NULL;

															GetCursorPos(&pointer->pos);
															ScreenToClient(this->hWnd, &pointer->pos);

															pointer->pos.x = (LONG)((FLOAT)((pointer->pos.x - this->viewport.rectangle.x) * this->width) / this->viewport.rectangle.width) - iconInfo->xHotspot;
															pointer->pos.y = (LONG)((FLOAT)((pointer->pos.y - this->viewport.rectangle.y) * this->height) / this->viewport.rectangle.height) - iconInfo->yHotspot;

															pointer->size.cx = 32;
															pointer->size.cy = 32;

															if (pointer->pos.x < 0)
															{
																pointer->offset.x = -pointer->pos.x;
																pointer->size.cx += pointer->pos.x;
																pointer->pos.x = 0;
															}
															else
																pointer->offset.x = 0;

															if (pointer->pos.y < 0)
															{
																pointer->offset.y = -pointer->pos.y;
																pointer->size.cy += pointer->pos.y;
																pointer->pos.y = 0;
															}
															else
																pointer->offset.y = 0;

															if (pointer->size.cx > 0 && pointer->size.cy > 0)
															{
																if (pointer->pos.x + pointer->size.cx >= *(INT*)&this->width)
																	pointer->size.cx = this->width - pointer->pos.x;

																if (pointer->pos.y + pointer->size.cy >= *(INT*)&this->height)
																	pointer->size.cy = this->height - pointer->pos.y;

																if (pointer->size.cx > 0 && pointer->size.cy > 0)
																	pointer->isActive = TRUE;
															}
														}

														UpdateRect* updateClip = surface->poinetrClip;
														UpdateRect* finClip = surface->currentClip;
														surface->poinetrClip = finClip;

														if (state.upscaling)
														{
															GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

															if (state.flags)
															{
																switch (state.upscaling)
																{
																case UpscaleScaleNx:
																	switch (state.value)
																	{
																	case 3:
																		upscaleProgram = &shaders.scaleNx_3x;
																		break;
																	default:
																		upscaleProgram = &shaders.scaleNx_2x;
																		break;
																	}

																	break;

																case UpscaleScaleHQ:
																	switch (state.value)
																	{
																	case 4:
																		upscaleProgram = &shaders.scaleHQ_4x;
																		break;
																	default:
																		upscaleProgram = &shaders.scaleHQ_2x;
																		break;
																	}

																	break;

																case UpscaleXRBZ:
																	switch (state.value)
																	{
																	case 6:
																		upscaleProgram = &shaders.xBRz_6x;
																		break;
																	case 5:
																		upscaleProgram = &shaders.xBRz_5x;
																		break;
																	case 4:
																		upscaleProgram = &shaders.xBRz_4x;
																		break;
																	case 3:
																		upscaleProgram = &shaders.xBRz_3x;
																		break;
																	default:
																		upscaleProgram = &shaders.xBRz_2x;
																		break;
																	}

																	break;

																case UpscaleXSal:
																	upscaleProgram = &shaders.xSal_2x;

																	break;

																default:
																	upscaleProgram = &shaders.eagle_2x;

																	break;
																}

																UseShaderProgram(upscaleProgram, texSize);

																DWORD newSize = MAKELONG(this->width * state.value, this->height * state.value);
																if (newSize != viewSize)
																{
																	if (!viewSize)
																	{
																		GLGenTextures(1, &tboId);
																		GLGenRenderbuffers(1, &rboId);
																	}

																	viewSize = newSize;

																	// Gen texture
																	GLBindTexture(GL_TEXTURE_2D, tboId);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
																	GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

																	// Get storage
																	GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
																	GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, LOWORD(viewSize), HIWORD(viewSize));
																	GLBindRenderbuffer(GL_RENDERBUFFER, NULL);

																	GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tboId, 0);
																	GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

																	if (!stencil)
																	{
																		DWORD size = (STENCIL_COUNT + 3) * sizeof(POINTFLOAT) * STENCIL_POINTS;
																		stencil = (POINTFLOAT*)MemoryAlloc(size); // +1 for FPS counter +2 for cursor

																		// FPS points
																		{
																			POINTFLOAT* point = stencil;

																			point->x = (FLOAT)(FPS_X);
																			point->y = (FLOAT)(FPS_Y);
																			++point;
																			point->x = (FLOAT)(FPS_X + FPS_WIDTH * 4);
																			point->y = (FLOAT)(FPS_Y);
																			++point;
																			point->x = (FLOAT)(FPS_X + FPS_WIDTH * 4);
																			point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																			++point;

																			point->x = (FLOAT)(FPS_X);
																			point->y = (FLOAT)(FPS_Y);
																			++point;
																			point->x = (FLOAT)(FPS_X + FPS_WIDTH * 4);
																			point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																			++point;
																			point->x = (FLOAT)(FPS_X);
																			point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																		}

																		UseShaderProgram(&shaders.stencil, 0);
																		{
																			GLGenVertexArrays(1, &stArrayName);
																			GLBindVertexArray(stArrayName);
																			GLGenBuffers(1, &stBufferName);
																			GLBindBuffer(GL_ARRAY_BUFFER, stBufferName);
																			{
																				GLBufferData(GL_ARRAY_BUFFER, size, stencil, GL_STREAM_DRAW);

																				GLEnableVertexAttribArray(0);
																				GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
																			}

																			GLBindVertexArray(arrayName);
																			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
																		}
																		UseShaderProgram(upscaleProgram, texSize);
																	}
																}
															}

															GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));
															if (this->CheckView())
																clear = TRUE;

															// Clear and stencil
															if (clear == TRUE)
															{
																++clear;

																palette->isChanged = FALSE;
																updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
																updateClip->rect.left = 0;
																updateClip->rect.top = 0;
																updateClip->rect.right = this->width;
																updateClip->rect.bottom = this->height;
																updateClip->isActive = TRUE;

																GLClear(GL_COLOR_BUFFER_BIT);
															}
															else
															{
																if (clear)
																{
																	if (clear < 3)
																		++clear;
																	else
																		clear = FALSE;
																}

																if (palette->isChanged)
																{
																	palette->isChanged = FALSE;
																	updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
																	updateClip->rect.left = 0;
																	updateClip->rect.top = 0;
																	updateClip->rect.right = this->width;
																	updateClip->rect.bottom = this->height;
																	updateClip->isActive = TRUE;
																}

																GLEnable(GL_STENCIL_TEST);
																GLClear(GL_STENCIL_BUFFER_BIT);

																UseShaderProgram(&shaders.stencil, 0);
																{
																	GLColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
																	GLStencilFunc(GL_ALWAYS, 0x01, 0x01);
																	GLStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
																	{
																		GLBindVertexArray(stArrayName);
																		GLBindBuffer(GL_ARRAY_BUFFER, stBufferName);
																		{
																			POINTFLOAT* start = stencil + STENCIL_POINTS;
																			POINTFLOAT* point = start;
																			UpdateRect* clip = updateClip;
																			while (clip != finClip)
																			{
																				if (clip->isActive)
																				{
																					FLOAT left = (FLOAT)clip->rect.left;
																					FLOAT top = (FLOAT)clip->rect.top;
																					FLOAT right = (FLOAT)clip->rect.right;
																					FLOAT bottom = (FLOAT)clip->rect.bottom;

																					point->x = left;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = bottom;
																					++point;

																					point->x = left;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = bottom;
																					++point;
																					point->x = left;
																					point->y = bottom;
																					++point;
																				}

																				if (++clip == surface->endClip)
																					clip = surface->clipsList;
																			}

																			Pointer* ptr = pointersList;
																			DWORD count = sizeof(pointersList) / sizeof(Pointer);
																			do
																			{
																				if (ptr->isActive)
																				{
																					FLOAT left = (FLOAT)ptr->pos.x;
																					FLOAT top = (FLOAT)ptr->pos.y;
																					FLOAT right = (FLOAT)(ptr->pos.x + ptr->size.cx);
																					FLOAT bottom = (FLOAT)(ptr->pos.y + ptr->size.cy);

																					point->x = left;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = bottom;
																					++point;

																					point->x = left;
																					point->y = top;
																					++point;
																					point->x = right;
																					point->y = bottom;
																					++point;
																					point->x = left;
																					point->y = bottom;
																					++point;
																				}

																				++ptr;
																			} while (--count);

																			count = point - start;
																			if (count)
																				GLBufferSubData(GL_ARRAY_BUFFER, STENCIL_POINTS * sizeof(POINTFLOAT), count * sizeof(POINTFLOAT), start);

																			if (fpsState == FpsDisabled)
																				GLDrawArrays(GL_TRIANGLES, STENCIL_POINTS, count);
																			else
																				GLDrawArrays(GL_TRIANGLES, 0, STENCIL_POINTS + count);
																		}
																		GLBindVertexArray(arrayName);
																		GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
																	}
																	GLColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
																	GLStencilFunc(GL_EQUAL, 0x01, 0x01);
																	GLStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
																}
																UseShaderProgram(upscaleProgram, texSize);
															}

															GLBindTexture(GL_TEXTURE_2D, textureId);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
														}
														else
														{
															if (this->CheckView())
															{
																GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
																clear = TRUE;
															}

															if (state.flags)
															{
																if (viewSize)
																{
																	GLDeleteTextures(1, &tboId);
																	GLDeleteRenderbuffers(1, &rboId);
																	viewSize = 0;
																}

																ShaderProgram* program;
																switch (state.interpolation)
																{
																case InterpolateHermite:
																	program = &shaders.hermite;
																	break;
																case InterpolateCubic:
																	program = &shaders.cubic;
																	break;
																default:
																	program = &shaders.linear;
																	break;
																}

																UseShaderProgram(program, texSize);

																GLBindTexture(GL_TEXTURE_2D, textureId);

																DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
															}

															if (palette->isChanged)
															{
																palette->isChanged = FALSE;
																clear = TRUE;
															}

															if (clear)
															{
																if (clear < 3)
																{
																	if (clear & 1)
																	{
																		updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
																		updateClip->rect.left = 0;
																		updateClip->rect.top = 0;
																		updateClip->rect.right = this->width;
																		updateClip->rect.bottom = this->height;
																		updateClip->isActive = TRUE;
																	}

																	++clear;
																}
																else
																	clear = FALSE;

																GLClear(GL_COLOR_BUFFER_BIT);
															}
														}

														// NEXT UNCHANGED
														{
															// Update texture
															while (updateClip != finClip)
															{
																if (updateClip->isActive)
																{
																	RECT update = updateClip->rect;
																	DWORD texWidth = update.right - update.left;
																	DWORD texHeight = update.bottom - update.top;

																	if (texWidth == this->width)
																		GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixelBuffer + update.top * texWidth);
																	else
																	{
																		DWORD* source = surface->pixelBuffer + update.top * this->width + update.left;
																		DWORD* dest = (DWORD*)frameBuffer;
																		DWORD copyHeight = texHeight;
																		do
																		{
																			MemoryCopy(dest, source, texWidth << 2);
																			source += this->width;
																			dest += texWidth;
																		} while (--copyHeight);

																		GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
																	}
																}

																if (++updateClip == surface->endClip)
																	updateClip = surface->clipsList;
															}

															// Clear prev pointer
															Pointer* prevPointer = &pointersList[pointer == pointersList];
															if (prevPointer->isActive)
															{
																prevPointer->isActive = FALSE;

																DWORD* source = surface->pixelBuffer + prevPointer->pos.y * this->width + prevPointer->pos.x;
																DWORD* dest = (DWORD*)frameBuffer;

																LONG copyHeight = prevPointer->size.cy;
																do
																{
																	MemoryCopy(dest, source, prevPointer->size.cx << 2);
																	source += this->width;
																	dest += prevPointer->size.cx;
																} while (--copyHeight);

																GLTexSubImage2D(GL_TEXTURE_2D, 0, prevPointer->pos.x, prevPointer->pos.y, prevPointer->size.cx, prevPointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
															}

															// Update pointer
															if (pointer->isActive)
															{
																DWORD* source = surface->pixelBuffer + pointer->pos.y * this->width + pointer->pos.x;
																DWORD* dst = (DWORD*)frameBuffer;

																DWORD initMask = 8 - (pointer->offset.x % 8);
																DWORD initOffset = pointer->offset.x & (8 - 1);

																INT shadowIdx = pointer->offset.y - SHADOW_OFFSET;
																LONG copyHeight = pointer->size.cy;

																if (colorInfo)
																{
																	BYTE* sourceColor = (BYTE*)colorInfo->bmBits + pointer->offset.y * colorInfo->bmWidthBytes + pointer->offset.x;
																	BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes;
																	BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;

																	do
																	{
																		DWORD* src = source;
																		BYTE* srcColor = sourceColor;
																		BYTE* srcMask = sourceMask;
																		BYTE* shadMask = shadowMask;

																		BYTE xorMask = *srcMask++;
																		BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

																		xorMask <<= initOffset;
																		shdMask <<= initOffset;

																		DWORD countMask = initMask;
																		LONG copyWidth = pointer->size.cx;
																		do
																		{
																			if (xorMask & 0x80)
																			{
																				DWORD color = *src;

																				if (!(shdMask & 0x80))
																				{
																					BYTE* cp = (BYTE*)&color;
																					DWORD cc = 3;
																					do
																						*cp++ >>= 1;
																					while (--cc);
																				}

																				*dst = color;
																			}
																			else
																				*dst = _byteswap_ulong(_rotl(Hooks::palEntries[*srcColor], 8));

																			if (--countMask)
																			{
																				xorMask <<= 1;
																				shdMask <<= 1;
																			}
																			else
																			{
																				countMask = 8;
																				xorMask = *srcMask++;
																				shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
																			}

																			++src;
																			++srcColor;
																			++dst;
																		} while (--copyWidth);

																		source += this->width;
																		sourceColor += colorInfo->bmWidthBytes;
																		sourceMask += maskInfo->bmWidthBytes;
																		shadowMask += maskInfo->bmWidthBytes;

																		++shadowIdx;
																	} while (--copyHeight);
																}
																else
																{
																	BYTE* sourceMask = (BYTE*)maskInfo->bmBits + pointer->offset.y * maskInfo->bmWidthBytes + (pointer->offset.x / 8);
																	BYTE* shadowMask = (BYTE*)sourceMask - SHADOW_OFFSET * maskInfo->bmWidthBytes;
																	BYTE* colorMask = (BYTE*)sourceMask + 32 * maskInfo->bmWidthBytes;

																	do
																	{
																		DWORD* src = source;
																		BYTE* clrMask = colorMask;
																		BYTE* srcMask = sourceMask;
																		BYTE* shadMask = shadowMask;

																		BYTE andMask = *clrMask++;
																		BYTE xorMask = *srcMask++;
																		BYTE shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;

																		andMask <<= initOffset;
																		xorMask <<= initOffset;
																		shdMask <<= initOffset;

																		DWORD countMask = initMask;
																		LONG copyWidth = pointer->size.cx;
																		do
																		{
																			if (xorMask & 0x80)
																			{
																				DWORD color = *src;

																				if (!(shdMask & 0x80))
																				{
																					BYTE* cp = (BYTE*)&color;
																					DWORD cc = 3;
																					do
																						*cp++ >>= 1;
																					while (--cc);
																				}

																				*dst = color;
																			}
																			else
																				*dst = (andMask & 0x80) ? 0xFFFFFFFF : 0xFF000000;

																			if (--countMask)
																			{
																				andMask <<= 1;
																				xorMask <<= 1;
																				shdMask <<= 1;
																			}
																			else
																			{
																				countMask = 8;
																				andMask = *clrMask++;
																				xorMask = *srcMask++;
																				shdMask = shadowIdx > 0 ? *shadMask++ : 0xFF;
																			}

																			++src;
																			++dst;
																		} while (--copyWidth);

																		source += this->width;

																		colorMask += maskInfo->bmWidthBytes;
																		sourceMask += maskInfo->bmWidthBytes;
																		shadowMask += maskInfo->bmWidthBytes;

																		++shadowIdx;
																	} while (--copyHeight);
																}

																GLTexSubImage2D(GL_TEXTURE_2D, 0, pointer->pos.x, pointer->pos.y, pointer->size.cx, pointer->size.cy, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
															}

															// Update FPS
															if (fpsState && !isTakeSnapshot)
															{
																DWORD fps = fpsCounter->value;
																DWORD digCount = 0;
																DWORD current = fps;
																do
																{
																	++digCount;
																	current = current / 10;
																} while (current);

																DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
																DWORD dcount = digCount;
																do
																{
																	WORD* lpDig = (WORD*)counters[fps % 10];

																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
																		DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

																		WORD check = *lpDig++;
																		DWORD width = FPS_WIDTH;
																		do
																		{
																			*pix++ = (check & 1) ? fpsColor : *idx;
																			++idx;
																			check >>= 1;
																		} while (--width);
																	}

																	fps = fps / 10;
																} while (--dcount);

																dcount = 4;
																while (dcount != digCount)
																{
																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		DWORD* idx = surface->pixelBuffer + (FPS_Y + y) * this->width + FPS_X + FPS_WIDTH * (dcount - 1);
																		DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 + FPS_WIDTH * (dcount - 1);

																		DWORD width = FPS_WIDTH;
																		do
																			*pix++ = *idx++;
																		while (--width);
																	}

																	--dcount;
																}

																GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
															}

															GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
														}

														// Draw from FBO
														if (state.upscaling)
														{
															GLDisable(GL_STENCIL_TEST);
															//GLFinish();
															GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

															ShaderProgram* program;
															switch (state.interpolation)
															{
															case InterpolateHermite:
																program = &shaders.hermite;
																break;
															case InterpolateCubic:
																program = &shaders.cubic;
																break;
															default:
																program = &shaders.linear;
																break;
															}

															UseShaderProgram(program, viewSize);
															{
																GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

																GLClear(GL_COLOR_BUFFER_BIT);
																GLBindTexture(GL_TEXTURE_2D, tboId);

																DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

																GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);

																if (isTakeSnapshot)
																{
																	GLFinish();

																	if (OpenClipboard(NULL))
																	{
																		EmptyClipboard();

																		DWORD dataSize = LOWORD(viewSize) * HIWORD(viewSize) * 3;
																		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
																		{
																			VOID* data = GlobalLock(hMemory);
																			{
																				BITMAPINFOHEADER* bmiHeader = (BITMAPINFOHEADER*)data;
																				MemoryZero(bmiHeader, sizeof(BITMAPINFOHEADER));
																				bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
																				bmiHeader->biWidth = LOWORD(viewSize);
																				bmiHeader->biHeight = HIWORD(viewSize);
																				bmiHeader->biPlanes = 1;
																				bmiHeader->biBitCount = 24;
																				bmiHeader->biCompression = BI_RGB;
																				bmiHeader->biXPelsPerMeter = 1;
																				bmiHeader->biYPelsPerMeter = 1;

																				VOID* pixels = (BITMAPINFOHEADER*)((BYTE*)data + sizeof(BITMAPINFOHEADER));
																				GLGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
																			}
																			GlobalUnlock(hMemory);

																			SetClipboardData(CF_DIB, hMemory);
																		}
																		GlobalFree(hMemory);

																		CloseClipboard();
																		clear = TRUE;
																	}
																}
															}
															UseShaderProgram(upscaleProgram, texSize);
														}
														else
														{
															if (isTakeSnapshot)
															{
																if (OpenClipboard(NULL))
																{
																	EmptyClipboard();

																	DWORD texWidth = this->width;
																	DWORD texHeight = this->height;
																	DWORD dataSize = texWidth * texHeight * sizeof(WORD);
																	HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + 1024 + dataSize);
																	{
																		VOID* data = GlobalLock(hMemory);
																		{
																			BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
																			MemoryZero(bmi, sizeof(BITMAPV5HEADER));
																			bmi->bV5Size = sizeof(BITMAPV5HEADER);
																			bmi->bV5Width = texWidth;
																			bmi->bV5Height = -*(LONG*)&texHeight;
																			bmi->bV5Planes = 1;
																			bmi->bV5BitCount = 8;
																			bmi->bV5Compression = BI_RGB;

																			DWORD* src = (DWORD*)surface->attachedPalette->entries;
																			DWORD* dst = (DWORD*)((BYTE*)data + sizeof(BITMAPV5HEADER));
																			DWORD count = 256;
																			do
																			{
																				*dst++ = ((*src & 0x00FF0000) >> 16) | (*src & 0x0000FF00) | ((*src & 0x000000FF) << 16);
																				++src;
																			} while (--count);
																			MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER) + 1024, surface->indexBuffer, dataSize);
																		}
																		GlobalUnlock(hMemory);

																		SetClipboardData(CF_DIBV5, hMemory);
																	}
																	GlobalFree(hMemory);

																	CloseClipboard();
																	clear = TRUE;
																}
															}
														}

														// Swap
														SwapBuffers(this->hDc);
														if (!clear && fpsState != FpsBenchmark)
															WaitForSingleObject(this->hDrawEvent, INFINITE);
														if (isVSync)
															GLFinish();
													}
												}
											} while (!this->isFinish);
										}
										delete fpsCounter;
									}
									MemoryFree(frameBuffer);
								}

								if (viewSize)
								{
									GLDeleteRenderbuffers(1, &rboId);
									GLDeleteTextures(1, &tboId);
								}
							}
							GLDeleteFramebuffers(1, &fboId);
						}
						GLDeleteTextures(1, &textureId);
					}
					GLBindBuffer(GL_ARRAY_BUFFER, NULL);
				}
				GLDeleteBuffers(1, &bufferName);
			}
			GLBindVertexArray(NULL);
		}
		GLDeleteVertexArrays(1, &arrayName);

		if (stencil)
		{
			MemoryFree(stencil);
			GLDeleteBuffers(1, &stBufferName);
			GLDeleteVertexArrays(1, &stArrayName);
		}
	}
	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

		++shaderProgram;
	} while (--count);
}

VOID OpenDraw::LoadFilterState()
{
	FilterState state;
	state.interpolation = config.image.interpolation;
	state.upscaling = config.image.upscaling;

	switch (state.upscaling)
	{
	case UpscaleScaleNx:
		state.value = config.image.scaleNx;
		break;

	case UpscaleScaleHQ:
		state.value = config.image.scaleHQ;
		break;

	case UpscaleXRBZ:
		state.value = config.image.xBRz;
		break;

	case UpscaleXSal:
		state.value = config.image.xSal;
		break;

	case UpscaleEagle:
		state.value = config.image.eagle;
		break;

	default:
		state.value = 0;
		break;
	}

	state.flags = TRUE;
	this->filterState = state;
}

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (config.singleWindow)
		this->hDraw = this->hWnd;
	else
	{
		if (this->windowState != WinStateWindowed)
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP | WS_MAXIMIZE,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		else
		{
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_CHILD,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		}
		Window::SetCapturePanel(this->hDraw);

		SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
		RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	}

	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->filterState.flags = TRUE;
	this->viewport.width = rect.right;
	this->viewport.height = rect.bottom;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, 256 * 1024, RenderThread, this, NORMAL_PRIORITY_CLASS, &threadId);
}

VOID OpenDraw::RenderStop()
{
	if (this->isFinish)
		return;

	this->isFinish = TRUE;
	SetEvent(this->hDrawEvent);
	WaitForSingleObject(this->hDrawThread, INFINITE);
	CloseHandle(this->hDrawThread);
	this->hDrawThread = NULL;

	if (this->hDraw != this->hWnd)
	{
		DestroyWindow(this->hDraw);
		GL::ResetPixelFormat();
	}

	this->hDraw = NULL;

	ClipCursor(NULL);

	glVersion = NULL;
	Window::CheckMenu(this->hWnd);
}

BOOL OpenDraw::CheckView()
{
	if (this->viewport.refresh)
	{
		this->viewport.refresh = FALSE;

		this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
		this->viewport.rectangle.width = this->viewport.width;
		this->viewport.rectangle.height = this->viewport.height;

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / this->width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / this->height;

		if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * this->width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * this->height;
				this->viewport.rectangle.height = (INT)MathRound(fh);
				this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
				this->viewport.clipFactor.y = this->viewport.viewFactor.x;
			}
		}

		Hooks::ScalePointer(this->viewport.clipFactor.x, this->viewport.clipFactor.y);

		HWND hActive = GetForegroundWindow();
		if (config.image.aspect && this->windowState != WinStateWindowed && (hActive == this->hWnd || hActive == this->hDraw))
		{
			RECT clipRect;
			GetClientRect(this->hWnd, &clipRect);

			clipRect.left = this->viewport.rectangle.x;
			clipRect.right = clipRect.left + this->viewport.rectangle.width;
			clipRect.bottom = clipRect.bottom - this->viewport.rectangle.y;
			clipRect.top = clipRect.bottom - this->viewport.rectangle.height;

			ClientToScreen(this->hWnd, (POINT*)&clipRect.left);
			ClientToScreen(this->hWnd, (POINT*)&clipRect.right);

			ClipCursor(&clipRect);
		}
		else
			ClipCursor(NULL);

		return TRUE;
	}

	return FALSE;
}

VOID OpenDraw::ScaleMouse(LPPOINT p)
{
	if (this->viewport.rectangle.width && this->viewport.rectangle.height)
	{
		p->x = (LONG)((FLOAT)((p->x - this->viewport.rectangle.x) * this->viewport.width) / this->viewport.rectangle.width);
		p->y = (LONG)((FLOAT)((p->y - this->viewport.rectangle.y) * this->viewport.height) / this->viewport.rectangle.height);
	}
}

OpenDraw::OpenDraw(IDraw** last)
{
	this->refCount = 1;
	this->last = *last;
	*last = this;

	this->surfaceEntries = NULL;
	this->paletteEntries = NULL;
	this->clipperEntries = NULL;

	this->attachedSurface = NULL;

	this->hDc = NULL;
	this->hWnd = NULL;
	this->hDraw = NULL;

	this->width = 0;
	this->height = 0;
	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;
	this->LoadFilterState();

	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	this->RenderStop();
	CloseHandle(this->hDrawEvent);
	ClipCursor(NULL);
}

ULONG __stdcall OpenDraw::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall OpenDraw::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	this->hWnd = hWnd;

	if (dwFlags & DDSCL_FULLSCREEN)
		this->windowState = WinStateFullScreen;
	else
	{
		this->windowState = WinStateWindowed;
		this->RenderStop();
		this->RenderStart();
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	this->width = dwWidth;
	this->height = dwHeight;

	RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	AdjustWindowRect(&rect, GetWindowLong(this->hWnd, GWL_STYLE), FALSE);
	MoveWindow(this->hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	SetForegroundWindow(this->hWnd);

	this->RenderStop();
	this->RenderStart();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE* lplpDDPalette, IUnknown* pUnkOuter)
{
	*lplpDDPalette = new OpenDrawPalette(this);
	this->paletteEntries->SetEntries(0, 0, 256, lpDDColorArray);
	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	BOOL isPrimary = lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE;
	*lplpDDSurface = new OpenDrawSurface(this, !isPrimary);

	if (lpDDSurfaceDesc->dwFlags & DDSD_WIDTH)
		this->width = lpDDSurfaceDesc->dwWidth;

	if (lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT)
		this->height = lpDDSurfaceDesc->dwHeight;

	if (isPrimary)
		this->attachedSurface = (OpenDrawSurface*)this->surfaceEntries;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	*lplpDDClipper = new OpenDrawClipper(this);
	return DD_OK;
}