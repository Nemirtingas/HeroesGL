/*
	MIT License

	Copyright (c) 2018 Oleksiy Ryabchun

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
#include "FpsCounter.h"
#include "Config.h"
#include "Window.h"
#include "OpenWindow.h"

VOID __fastcall UseShaderProgram(ShaderProgram* program)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLuint vShader = GL::CompileShaderSource(program->vertexName, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(program->fragmentName, GL_FRAGMENT_SHADER);
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
	}
	else
		GLUseProgram(program->id);
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	do
	{
		if (ddraw->mode && ddraw->attachedSurface && ddraw->attachedSurface->width)
		{
			ddraw->hDc = ::GetDC(ddraw->hDraw);
			{
				PIXELFORMATDESCRIPTOR pfd;
				GL::PreparePixelFormatDescription(&pfd);
				if (!glPixelFormat && !GL::PreparePixelFormat(&pfd))
				{
					glPixelFormat = ChoosePixelFormat(ddraw->hDc, &pfd);
					if (!glPixelFormat)
						Main::ShowError("ChoosePixelFormat failed", __FILE__, __LINE__);
					else if (pfd.dwFlags & PFD_NEED_PALETTE)
						Main::ShowError("Needs palette", __FILE__, __LINE__);
				}

				if (!SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
					Main::ShowError("SetPixelFormat failed", __FILE__, __LINE__);

				MemoryZero(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
				pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
				pfd.nVersion = 1;
				if (DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
					Main::ShowError("DescribePixelFormat failed", __FILE__, __LINE__);

				if ((pfd.iPixelType != PFD_TYPE_RGBA) ||
					(pfd.cRedBits < 5) || (pfd.cGreenBits < 5) || (pfd.cBlueBits < 5))
					Main::ShowError("Bad pixel type", __FILE__, __LINE__);

				HGLRC hRc = WGLCreateContext(ddraw->hDc);
				if (hRc)
				{
					WGLMakeCurrent(ddraw->hDc, hRc);
					{
						GL::CreateContextAttribs(ddraw->hDc, &hRc);
						if (glVersion >= GL_VER_3_0)
						{
							DWORD maxSize = ddraw->mode->width > ddraw->mode->height ? ddraw->mode->width : ddraw->mode->height;

							DWORD maxTexSize = 1;
							while (maxTexSize < maxSize)
								maxTexSize <<= 1;

							DWORD glMaxTexSize;
							GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
							if (maxTexSize > glMaxTexSize)
								glVersion = GL_VER_1_1;
						}

						Window::CheckMenu(ddraw->hWnd);
						if (glVersion >= GL_VER_3_0)
							ddraw->RenderNew();
						else
							ddraw->RenderOld();
					}
					WGLMakeCurrent(ddraw->hDc, NULL);
					WGLDeleteContext(hRc);
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
	if (configImageFilter == FilterCubic || configImageFilter == FilterXRBZ || configImageFilter == FilterScaleHQ ||
		configImageFilter == FilterXSal || configImageFilter == FilterEagle || configImageFilter == FilterScaleNx)
		configImageFilter = FilterLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size = this->mode->width > this->mode->height ? this->mode->width : this->mode->height;
	DWORD maxAllow = 1;
	while (maxAllow < size)
		maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = configImageFilter == FilterNearest ? GL_NEAREST : GL_LINEAR;

	DWORD framePerWidth = this->mode->width / maxTexSize + (this->mode->width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->mode->height / maxTexSize + (this->mode->height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;
	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < this->mode->height; y += maxTexSize)
		{
			DWORD height = this->mode->height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < this->mode->width; x += maxTexSize, ++frame)
			{
				DWORD width = this->mode->width - x;
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
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);

				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				if (glVersion > GL_VER_1_1)
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				else
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)this->mode->width, (GLdouble)this->mode->height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		this->viewport.refresh = TRUE;

		VOID* frameBuffer = MemoryAlloc(maxTexSize * maxTexSize * (glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			DWORD fpsQueue[FPS_COUNT];
			DWORD tickQueue[FPS_COUNT];

			DWORD fpsIdx = -1;
			DWORD fpsTotal = 0;
			DWORD fpsCount = 0;
			INT fpsSum = 0;
			MemoryZero(fpsQueue, sizeof(fpsQueue));
			MemoryZero(tickQueue, sizeof(tickQueue));

			BOOL isVSync = FALSE;
			if (WGLSwapInterval)
				WGLSwapInterval(0);

			DWORD clear = TRUE;
			do
			{
				OpenDrawSurface* surface = this->attachedSurface;
				if (surface)
				{
					if (WGLSwapInterval)
					{
						if (!isVSync)
						{
							if (configImageVSync)
							{
								isVSync = TRUE;
								WGLSwapInterval(1);
							}
						}
						else
						{
							if (!configImageVSync)
							{
								isVSync = FALSE;
								WGLSwapInterval(0);
							}
						}
					}

					if (fpsState)
					{
						DWORD tick = GetTickCount();

						if (isFpsChanged)
						{
							isFpsChanged = FALSE;

							fpsIdx = -1;
							fpsTotal = 0;
							fpsCount = 0;
							fpsSum = 0;
							MemoryZero(fpsQueue, sizeof(fpsQueue));
							MemoryZero(tickQueue, sizeof(tickQueue));
						}

						++fpsTotal;
						if (fpsCount < FPS_COUNT)
							++fpsCount;

						++fpsIdx;
						if (fpsIdx == FPS_COUNT)
							fpsIdx = 0;

						DWORD diff = tick - tickQueue[fpsTotal != fpsCount ? fpsIdx : 0];
						tickQueue[fpsIdx] = tick;

						DWORD fps = diff ? (DWORD)MathRound(1000.0f / diff * fpsCount) : 9999;

						DWORD* queue = &fpsQueue[fpsIdx];
						fpsSum -= *queue - fps;
						*queue = fps;
					}
					else if (isFpsChanged)
					{
						isFpsChanged = FALSE;
						clear = TRUE;
					}

					if (this->CheckView())
					{
						clear = TRUE;
						GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
					}

					glFilter = 0;
					if (this->isStateChanged)
					{
						this->isStateChanged = FALSE;
						clear = TRUE;
						glFilter = configImageFilter == FilterNearest ? GL_NEAREST : GL_LINEAR;
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
								updateClip->rect.right = this->mode->width;
								updateClip->rect.bottom = this->mode->height;
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

									if (texWidth == this->mode->width)
									{
										if (glVersion > GL_VER_1_1)
											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->indexBuffer + update.top * texWidth);
										else
										{
											WORD* source = surface->indexBuffer + update.top * texWidth;
											DWORD* dest = (DWORD*)frameBuffer;
											DWORD copyWidth = texWidth;
											DWORD copyHeight = texHeight;
											do
											{
												WORD* src = source;
												source += this->mode->width;

												DWORD count = copyWidth;
												do
												{
													WORD px = *src++;
													*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
												} while (--count);
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
										}
									}
									else
									{
										if (texWidth & 1)
										{
											++texWidth;
											if (update.left)
												--update.left;
											else
												++update.right;
										}

										if (glVersion > GL_VER_1_1)
										{
											WORD* source = surface->indexBuffer + update.top * this->mode->width + update.left;
											WORD* dest = (WORD*)frameBuffer;
											DWORD copyHeight = texHeight;
											do
											{
												MemoryCopy(dest, source, texWidth << 1);
												source += this->mode->width;
												dest += texWidth;
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
										}
										else
										{
											WORD* source = surface->indexBuffer + update.top * this->mode->width + update.left;
											DWORD* dest = (DWORD*)frameBuffer;
											DWORD copyWidth = texWidth;
											DWORD copyHeight = texHeight;
											do
											{
												WORD* src = source;
												source += this->mode->width;

												DWORD count = copyWidth;
												do
												{
													WORD px = *src++;
													*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
												} while (--count);
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
										}
									}
								}

								if (++updateClip == surface->endClip)
									updateClip = surface->clipsList;
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
										if (clipWidth & 1)
										{
											++clipWidth;
											if (clip.left != rect->x)
												--clip.left;
											else
												++clip.right;
										}

										if (glVersion > GL_VER_1_1)
										{
											WORD* source = surface->indexBuffer + clip.top * this->mode->width + clip.left;
											WORD* dest = (WORD*)frameBuffer;

											DWORD copyHeight = clipHeight;
											do
											{
												MemoryCopy(dest, source, clipWidth << 1);
												source += this->mode->width;
												dest += clipWidth;
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - rect->x, clip.top - rect->y, clipWidth, clipHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
										}
										else
										{
											WORD* source = surface->indexBuffer + clip.top * this->mode->width + clip.left;
											DWORD* dest = (DWORD*)frameBuffer;
											DWORD copyWidth = clipWidth;
											DWORD copyHeight = clipHeight;
											do
											{
												WORD* src = source;
												source += this->mode->width;

												DWORD count = copyWidth;
												do
												{
													WORD px = *src++;
													*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
												} while (--count);
											} while (--copyHeight);

											GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - rect->x, clip.top - rect->y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
										}
									}
								}

								if (++update == surface->endClip)
									update = surface->clipsList;
							}
						}

						if (fpsState && frame == frames)
						{
							DWORD fps = (DWORD)MathRound((FLOAT)fpsSum / fpsCount);
							DWORD digCount = 0;
							DWORD current = fps;
							do
							{
								++digCount;
								current = current / 10;
							} while (current);

							if (glVersion > GL_VER_1_1)
							{
								WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
								DWORD dcount = digCount;
								do
								{
									bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * (fps % 10);

									for (DWORD y = 0; y < FPS_HEIGHT; ++y)
									{
										WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
											FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

										WORD* pix = (WORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
											(FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD width = FPS_STEP;
										do
											*pix++ = *idx++;
										while (--width);

										width = FPS_WIDTH;
										do
										{
											*pix++ = *lpDig++ ? fpsColor : *idx;
											++idx;
										} while (--width);
									}

									fps = fps / 10;
								} while (--dcount);

								dcount = 4;
								while (dcount != digCount)
								{
									for (DWORD y = 0; y < FPS_HEIGHT; ++y)
									{
										WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
											FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

										WORD* pix = (WORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
											(FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD width = FPS_STEP + FPS_WIDTH;
										do
											*pix++ = *idx++;
										while (--width);
									}

									--dcount;
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_STEP + FPS_WIDTH) * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
							}
							else
							{
								DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
								DWORD dcount = digCount;
								do
								{
									bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * (fps % 10);

									for (DWORD y = 0; y < FPS_HEIGHT; ++y)
									{
										WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
											FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD* pix = (DWORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
											(FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD width = FPS_STEP;
										do
										{
											WORD px = *idx++;
											*pix++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
										} while (--width);

										width = FPS_WIDTH;
										do
										{
											if (*lpDig++)
												*pix = fpsColor;
											else
											{
												WORD px = *idx;
												*pix = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											}

											++pix;
											++idx;
										} while (--width);
									}

									fps = fps / 10;
								} while (--dcount);

								dcount = 4;
								while (dcount != digCount)
								{
									for (DWORD y = 0; y < FPS_HEIGHT; ++y)
									{
										WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
											FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD* pix = (DWORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
											(FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD width = FPS_STEP + FPS_WIDTH;
										do
										{
											WORD px = *idx++;
											*pix++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
										} while (--width);
									}

									--dcount;
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_STEP + FPS_WIDTH) * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
							}
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

					if (this->isTakeSnapshot)
					{
						this->isTakeSnapshot = FALSE;

						if (OpenClipboard(NULL))
						{
							EmptyClipboard();

							DWORD texWidth = this->mode->width;
							DWORD texHeight = this->mode->height;
							DWORD dataSize = texWidth * texHeight * sizeof(WORD);
							HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + dataSize);
							{
								VOID* data = GlobalLock(hMemory);
								{
									BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
									MemoryZero(bmi, sizeof(BITMAPINFOHEADER));
									bmi->bV5Size = sizeof(BITMAPV5HEADER);
									bmi->bV5Width = texWidth;
									bmi->bV5Height = -*(LONG*)&texHeight;
									bmi->bV5Planes = 1;
									bmi->bV5BitCount = 16;
									bmi->bV5Compression = BI_BITFIELDS;
									bmi->bV5XPelsPerMeter = 1;
									bmi->bV5YPelsPerMeter = 1;
									bmi->bV5RedMask = 0xF800;
									bmi->bV5GreenMask = 0x07E0;
									bmi->bV5BlueMask = 0x001F;

									MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER), surface->indexBuffer, dataSize);
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
			} while (!this->isFinish);
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

VOID OpenDraw::RenderNew()
{
	DWORD maxSize = this->mode->width > this->mode->height ? this->mode->width : this->mode->height;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize) maxTexSize <<= 1;
	FLOAT texWidth = this->mode->width == maxTexSize ? 1.0f : (FLOAT)this->mode->width / maxTexSize;
	FLOAT texHeight = this->mode->height == maxTexSize ? 1.0f : (FLOAT)this->mode->height / maxTexSize;

	FLOAT buffer[8][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->mode->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->mode->width, (FLOAT)this->mode->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->mode->height, 0.0f, texHeight },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ (FLOAT)this->mode->width, 0.0f, 1.0f, 1.0f },
		{ (FLOAT)this->mode->width, (FLOAT)this->mode->height, 1.0f, 0.0f },
		{ 0.0f, (FLOAT)this->mode->height, 0.0f, 0.0f }
	};

	FLOAT mvpMatrix[4][4] = {
		{ 2.0f / this->mode->width, 0.0f, 0.0f, 0.0f },
		{ 0.0f, -2.0f / this->mode->height, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	ShaderProgramsList shaders = {
		0, IDR_STENCIL_VERTEX, IDR_STENCIL_FRAGMENT, (GLfloat*)mvpMatrix,
		0, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix,
		0, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix,
		0, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_2X, (GLfloat*)mvpMatrix,
		0, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_3X, (GLfloat*)mvpMatrix,
		0, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_4X, (GLfloat*)mvpMatrix,
		0, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_5X, (GLfloat*)mvpMatrix,
		0, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_6X, (GLfloat*)mvpMatrix,
		0, IDR_SCALEHQ_VERTEX_2X, IDR_SCALEHQ_FRAGMENT_2X, (GLfloat*)mvpMatrix,
		0, IDR_SCALEHQ_VERTEX_4X, IDR_SCALEHQ_FRAGMENT_4X, (GLfloat*)mvpMatrix,
		0, IDR_XSAL_VERTEX, IDR_XSAL_FRAGMENT, (GLfloat*)mvpMatrix,
		0, IDR_EAGLE_VERTEX, IDR_EAGLE_FRAGMENT, (GLfloat*)mvpMatrix,
		0, IDR_SCALENX_VERTEX_2X, IDR_SCALENX_FRAGMENT_2X, (GLfloat*)mvpMatrix,
		0, IDR_SCALENX_VERTEX_3X, IDR_SCALENX_FRAGMENT_3X, (GLfloat*)mvpMatrix
	};

	ShaderProgram* filterProgram = &shaders.linear;
	ShaderProgram* filterProgram2 = &shaders.linear;
	{
		POINTFLOAT* stencil = NULL;
		GLuint stArrayName, stBufferName, arrayName;

		GLGenVertexArrays(1, &arrayName);
		{
			GLBindVertexArray(arrayName);
			{
				GLuint bufferName;
				GLGenBuffers(1, &bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
					{
						GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

						UseShaderProgram(filterProgram);
						GLint attrCoordsLoc = GLGetAttribLocation(filterProgram->id, "vCoord");
						GLEnableVertexAttribArray(attrCoordsLoc);
						GLVertexAttribPointer(attrCoordsLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

						GLint attrTexCoordsLoc = GLGetAttribLocation(filterProgram->id, "vTexCoord");
						GLEnableVertexAttribArray(attrTexCoordsLoc);
						GLVertexAttribPointer(attrTexCoordsLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

						GLuint textureId;
						GLGenTextures(1, &textureId);
						{
							DWORD filter = configImageFilter == FilterLinear ? GL_LINEAR : GL_NEAREST;
							GLActiveTexture(GL_TEXTURE0);
							GLBindTexture(GL_TEXTURE_2D, textureId);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0, tboId = 0;
								{
									GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

									VOID* frameBuffer = MemoryAlloc(this->mode->width * this->mode->height * sizeof(WORD));
									{
										this->isStateChanged = TRUE;

										DWORD fpsQueue[FPS_COUNT];
										DWORD tickQueue[FPS_COUNT];

										DWORD fpsIdx = -1;
										DWORD fpsTotal = 0;
										DWORD fpsCount = 0;
										INT fpsSum = 0;
										MemoryZero(fpsQueue, sizeof(fpsQueue));
										MemoryZero(tickQueue, sizeof(tickQueue));

										BOOL isVSync = FALSE;
										if (WGLSwapInterval)
											WGLSwapInterval(0);

										DWORD clear = TRUE;
										do
										{
											OpenDrawSurface* surface = this->attachedSurface;
											if (surface)
											{
												if (WGLSwapInterval)
												{
													if (!isVSync)
													{
														if (configImageVSync)
														{
															isVSync = TRUE;
															WGLSwapInterval(1);
														}
													}
													else
													{
														if (!configImageVSync)
														{
															isVSync = FALSE;
															WGLSwapInterval(0);
														}
													}
												}

												if (this->isStateChanged)
												{
													this->viewport.refresh = TRUE;
													isFpsChanged = TRUE;
												}

												if (fpsState)
												{
													DWORD tick = GetTickCount();

													if (isFpsChanged)
													{
														fpsIdx = -1;
														fpsTotal = 0;
														fpsCount = 0;
														fpsSum = 0;
														MemoryZero(fpsQueue, sizeof(fpsQueue));
														MemoryZero(tickQueue, sizeof(tickQueue));
													}

													++fpsTotal;
													if (fpsCount < FPS_COUNT)
														++fpsCount;

													++fpsIdx;
													if (fpsIdx == FPS_COUNT)
														fpsIdx = 0;

													DWORD diff = tick - tickQueue[fpsTotal != fpsCount ? fpsIdx : 0];
													tickQueue[fpsIdx] = tick;

													DWORD fps = diff ? (DWORD)MathRound(1000.0f / diff * fpsCount) : 9999;

													DWORD* queue = &fpsQueue[fpsIdx];
													fpsSum -= *queue - fps;
													*queue = fps;
												}

												BOOL isTakeSnapshot = this->isTakeSnapshot;
												if (isTakeSnapshot)
													this->isTakeSnapshot = FALSE;

												UpdateRect* updateClip = surface->poinetrClip;
												UpdateRect* finClip = surface->currentClip;
												surface->poinetrClip = finClip;

												ImageFilter frameFilter = configImageFilter;
												if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ ||
													frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
												{
													GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

													if (this->isStateChanged)
													{
														this->isStateChanged = FALSE;

														DWORD scaleValue;
														switch (frameFilter)
														{
														case FilterScaleNx:
															scaleValue = LOWORD(configImageScaleNx);
															switch (scaleValue)
															{
															case 4:
																filterProgram = &shaders.scaleNx_3x;
																break;
															default:
																filterProgram = &shaders.scaleNx_2x;
																break;
															}
															filterProgram2 = HIWORD(configImageScaleNx) ? &shaders.cubic : &shaders.linear;
															break;

														case FilterScaleHQ:
															scaleValue = LOWORD(configImageScaleHQ);
															switch (scaleValue)
															{
															case 4:
																filterProgram = &shaders.scaleHQ_4x;
																break;
															default:
																filterProgram = &shaders.scaleHQ_2x;
																break;
															}
															filterProgram2 = HIWORD(configImageScaleHQ) ? &shaders.cubic : &shaders.linear;
															break;

														case FilterXRBZ:
															scaleValue = LOWORD(configImageXBRZ);
															switch (scaleValue)
															{
															case 6:
																filterProgram = &shaders.xBRz_6x;
																break;
															case 5:
																filterProgram = &shaders.xBRz_5x;
																break;
															case 4:
																filterProgram = &shaders.xBRz_4x;
																break;
															case 3:
																filterProgram = &shaders.xBRz_3x;
																break;
															default:
																filterProgram = &shaders.xBRz_2x;
																break;
															}
															filterProgram2 = HIWORD(configImageXBRZ) ? &shaders.cubic : &shaders.linear;
															break;

														case FilterXSal:
															scaleValue = 2;
															filterProgram = &shaders.xSal_2x;
															filterProgram2 = HIWORD(configImageXSal) ? &shaders.cubic : &shaders.linear;
															break;

														default:
															scaleValue = 2;
															filterProgram = &shaders.eagle_2x;
															filterProgram2 = HIWORD(configImageEagle) ? &shaders.cubic : &shaders.linear;
															break;

														}

														UseShaderProgram(filterProgram);

														DWORD newSize = (this->mode->width * scaleValue) | ((this->mode->height * scaleValue) << 16);
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
																stencil = (POINTFLOAT*)MemoryAlloc((STENCIL_COUNT + 1) * sizeof(POINTFLOAT) * 6); // +1 for FPS counter
																GLGenVertexArrays(1, &stArrayName);
																GLGenBuffers(1, &stBufferName);
																GLBindVertexArray(stArrayName);
																GLBindBuffer(GL_ARRAY_BUFFER, stBufferName);
																{
																	GLBufferData(GL_ARRAY_BUFFER, STENCIL_COUNT * 4 * 2 * sizeof(FLOAT), NULL, GL_STREAM_DRAW);
																	GLEnableVertexAttribArray(attrCoordsLoc);
																	GLVertexAttribPointer(attrCoordsLoc, 2, GL_FLOAT, GL_FALSE, 8, (GLvoid*)0);
																}
																GLBindVertexArray(arrayName);
																GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
															}
														}
													}

													if (isFpsChanged)
													{
														isFpsChanged = FALSE;
														clear = TRUE;
													}

													GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));
													if (this->CheckView())
														clear = TRUE;

													// Clear and stencil
													if (clear == TRUE)
													{
														++clear;

														updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
														updateClip->rect.left = 0;
														updateClip->rect.top = 0;
														updateClip->rect.right = this->mode->width;
														updateClip->rect.bottom = this->mode->height;
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

														GLEnable(GL_STENCIL_TEST);
														GLClear(GL_STENCIL_BUFFER_BIT);

														UseShaderProgram(&shaders.stencil);
														{
															GLColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
															GLStencilFunc(GL_ALWAYS, 0x01, 0x01);
															GLStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
															{
																GLBindVertexArray(stArrayName);
																GLBindBuffer(GL_ARRAY_BUFFER, stBufferName);
																{
																	POINTFLOAT* point = stencil;
																	UpdateRect* clip = updateClip;
																	while (clip != finClip)
																	{
																		if (clip->isActive)
																		{
																			point->x = (FLOAT)clip->rect.left;  point->y = (FLOAT)clip->rect.top;
																			++point;
																			point->x = (FLOAT)clip->rect.right;  point->y = (FLOAT)clip->rect.top;
																			++point;
																			point->x = (FLOAT)clip->rect.right;  point->y = (FLOAT)clip->rect.bottom;
																			++point;

																			point->x = (FLOAT)clip->rect.left;  point->y = (FLOAT)clip->rect.top;
																			++point;
																			point->x = (FLOAT)clip->rect.right;  point->y = (FLOAT)clip->rect.bottom;
																			++point;
																			point->x = (FLOAT)clip->rect.left;  point->y = (FLOAT)clip->rect.bottom;
																			++point;
																		}

																		if (++clip == surface->endClip)
																			clip = surface->clipsList;
																	}

																	if (fpsState)
																	{
																		point->x = (FLOAT)(FPS_X);  point->y = (FLOAT)(FPS_Y);
																		++point;
																		point->x = (FLOAT)(FPS_X + (FPS_STEP + FPS_WIDTH) * 4);  point->y = (FLOAT)(FPS_Y);
																		++point;
																		point->x = (FLOAT)(FPS_X + (FPS_STEP + FPS_WIDTH) * 4);  point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																		++point;

																		point->x = (FLOAT)(FPS_X);  point->y = (FLOAT)(FPS_Y);
																		++point;
																		point->x = (FLOAT)(FPS_X + (FPS_STEP + FPS_WIDTH) * 4);  point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																		++point;
																		point->x = (FLOAT)(FPS_X);  point->y = (FLOAT)(FPS_Y + FPS_HEIGHT);
																		++point;
																	}

																	DWORD count = point - stencil;
																	if (count)
																	{
																		GLBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(POINTFLOAT), stencil);
																		GLDrawArrays(GL_TRIANGLES, 0, count);
																	}
																}
																GLBindVertexArray(arrayName);
																GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
															}
															GLColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
															GLStencilFunc(GL_EQUAL, 0x01, 0x01);
															GLStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
														}
														UseShaderProgram(filterProgram);
													}

													GLBindTexture(GL_TEXTURE_2D, textureId);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
												}
												else
												{
													if (isFpsChanged)
													{
														isFpsChanged = FALSE;
														clear = TRUE;
													}

													if (this->CheckView())
													{
														clear = TRUE;
														GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
													}

													if (this->isStateChanged)
													{
														this->isStateChanged = FALSE;

														filterProgram = frameFilter == FilterCubic ? &shaders.cubic : &shaders.linear;
														UseShaderProgram(filterProgram);

														if (viewSize)
														{
															GLDeleteTextures(1, &tboId);
															GLDeleteRenderbuffers(1, &rboId);
															viewSize = 0;
														}

														filter = frameFilter == FilterLinear ? GL_LINEAR : GL_NEAREST;
														GLBindTexture(GL_TEXTURE_2D, textureId);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
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
																updateClip->rect.right = this->mode->width;
																updateClip->rect.bottom = this->mode->height;
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

															if (texWidth == this->mode->width)
																GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->indexBuffer + update.top * texWidth);
															else
															{
																if (texWidth & 1)
																{
																	++texWidth;
																	if (update.left)
																		--update.left;
																	else
																		++update.right;
																}

																WORD* source = surface->indexBuffer + update.top * this->mode->width + update.left;
																WORD* dest = (WORD*)frameBuffer;
																DWORD copyHeight = texHeight;
																do
																{
																	MemoryCopy(dest, source, texWidth << 1);
																	source += this->mode->width;
																	dest += texWidth;
																} while (--copyHeight);

																GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
															}
														}

														if (++updateClip == surface->endClip)
															updateClip = surface->clipsList;
													}

													// Update FPS
													if (fpsState && !isTakeSnapshot)
													{
														DWORD fps = (DWORD)MathRound((FLOAT)fpsSum / fpsCount);
														DWORD digCount = 0;
														DWORD current = fps;
														do
														{
															++digCount;
															current = current / 10;
														} while (current);

														WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
														DWORD dcount = digCount;
														do
														{
															bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * (fps % 10);

															for (DWORD y = 0; y < FPS_HEIGHT; ++y)
															{
																WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
																	FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

																WORD* pix = (WORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
																	(FPS_STEP + FPS_WIDTH) * (dcount - 1);

																DWORD width = FPS_STEP;
																do
																	*pix++ = *idx++;
																while (--width);

																width = FPS_WIDTH;
																do
																{
																	*pix++ = *lpDig++ ? fpsColor : *idx;
																	++idx;
																} while (--width);
															}

															fps = fps / 10;
														} while (--dcount);

														dcount = 4;
														while (dcount != digCount)
														{
															for (DWORD y = 0; y < FPS_HEIGHT; ++y)
															{
																WORD* idx = surface->indexBuffer + (FPS_Y + y) * this->mode->width +
																	FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

																WORD* pix = (WORD*)frameBuffer + y * (FPS_STEP + FPS_WIDTH) * 4 +
																	(FPS_STEP + FPS_WIDTH) * (dcount - 1);

																DWORD width = FPS_STEP + FPS_WIDTH;
																do
																	*pix++ = *idx++;
																while (--width);
															}

															--dcount;
														}

														GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_STEP + FPS_WIDTH) * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
													}

													// Draw into FBO texture
													GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
												}

												// Draw from FBO
												if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ ||
													frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
												{
													GLDisable(GL_STENCIL_TEST);
													//GLFinish();
													GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

													UseShaderProgram(filterProgram2);
													{
														GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

														GLClear(GL_COLOR_BUFFER_BIT);
														GLBindTexture(GL_TEXTURE_2D, tboId);

														filter = filterProgram2 == &shaders.linear ? GL_LINEAR : GL_NEAREST;
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
													UseShaderProgram(filterProgram);
												}
												else
												{
													if (isTakeSnapshot)
													{
														if (OpenClipboard(NULL))
														{
															EmptyClipboard();

															DWORD texWidth = this->mode->width;
															DWORD texHeight = this->mode->height;
															DWORD dataSize = texWidth * texHeight * sizeof(WORD);
															HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + dataSize);
															{
																VOID* data = GlobalLock(hMemory);
																{
																	BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
																	MemoryZero(bmi, sizeof(BITMAPINFOHEADER));
																	bmi->bV5Size = sizeof(BITMAPV5HEADER);
																	bmi->bV5Width = texWidth;
																	bmi->bV5Height = -*(LONG*)&texHeight;
																	bmi->bV5Planes = 1;
																	bmi->bV5BitCount = 16;
																	bmi->bV5Compression = BI_BITFIELDS;
																	bmi->bV5XPelsPerMeter = 1;
																	bmi->bV5YPelsPerMeter = 1;
																	bmi->bV5RedMask = 0xF800;
																	bmi->bV5GreenMask = 0x07E0;
																	bmi->bV5BlueMask = 0x001F;

																	MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER), surface->indexBuffer, dataSize);
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
										} while (!this->isFinish);
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
	DWORD count = sizeof(ShaderProgramsList) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

	} while (--count);
}

VOID OpenDraw::ResetDisplayMode(DWORD width, DWORD height)
{
	OpenDrawSurface* surface = this->attachedSurface;
	if (!this->mode || this->mode->width != width || this->mode->height != height ||
		surface && (surface->width != width || surface->height != height))
	{
		DisplayMode* mode = modesList;
		DWORD count = sizeof(modesList) / sizeof(DisplayMode);
		do
		{
			if (mode->width == width && mode->height == height)
			{
				this->RenderStop();
				{
					this->mode = mode;
					OpenDrawSurface* surface = this->attachedSurface;
					if (surface)
					{
						surface->ReleaseBuffer();
						surface->CreateBuffer(mode->width, mode->height);
					}
				}
				this->RenderStart();
				return;
			}

			++mode;
		} while (--count);
	}
}

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;
	GL::Load();

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (this->windowState != WinStateWindowed)
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
			WC_STATIC,
			NULL,
			WS_VISIBLE | WS_POPUP,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}
	else
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT,
			WC_STATIC,
			NULL,
			WS_VISIBLE | WS_CHILD,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}

	OpenWindow::OldPanelProc = (WNDPROC)SetWindowLongPtr(this->hDraw, GWLP_WNDPROC, (LONG_PTR)OpenWindow::PanelProc);

	SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right - rect.left;
	this->viewport.height = rect.bottom - rect.top;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, NORMAL_PRIORITY_CLASS, &threadId);
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

	BOOL wasFull = GetWindowLong(this->hDraw, GWL_STYLE) & WS_POPUP;
	if (DestroyWindow(this->hDraw))
		this->hDraw = NULL;

	if (wasFull)
		GL::ResetContext();

	ClipCursor(NULL);
}

BOOL OpenDraw::CheckView()
{
	if (this->viewport.refresh)
	{
		this->viewport.refresh = FALSE;

		this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
		this->viewport.rectangle.width = this->viewport.width;
		this->viewport.rectangle.height = this->viewport.height;

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / this->mode->width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / this->mode->height;

		if (configImageAspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * this->mode->width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * this->mode->height;
				this->viewport.rectangle.height = (INT)MathRound(fh);
				this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
				this->viewport.clipFactor.y = this->viewport.viewFactor.x;
			}
		}

		if (configImageAspect && this->windowState != WinStateWindowed)
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
	p->x = (LONG)MathRound((FLOAT)(p->x + 1 - this->viewport.rectangle.x) * (FLOAT)this->mode->width / (FLOAT)this->viewport.rectangle.width) - 1;
	p->y = (LONG)MathRound((FLOAT)(p->y + 1 - this->viewport.rectangle.y) * (FLOAT)this->mode->height / (FLOAT)this->viewport.rectangle.height) - 1;
}

OpenDraw::OpenDraw(IDraw7** last)
{
	this->last = *last;
	*last = this;

	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;

	this->attachedSurface = NULL;

	this->hWnd = NULL;
	this->hDraw = NULL;
	this->hDc = NULL;

	this->mode = NULL;
	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;

	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

ULONG __stdcall OpenDraw::Release()
{
	this->RenderStop();

	CloseHandle(this->hDrawEvent);
	ClipCursor(NULL);

	delete this;
	return 0;
}

HRESULT __stdcall OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	if (this->hWnd != hWnd)
	{
		this->hDc = NULL;
		this->hWnd = hWnd;

		if (!OpenWindow::blackBrush)
			OpenWindow::blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

		if (!Window::OldWindowProc)
			Window::OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)OpenWindow::WindowProc);
	}

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

HRESULT __stdcall OpenDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback)
{
	DDSURFACEDESC2 ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC2));

	DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		ddSurfaceDesc.dwWidth = mode->width;
		ddSurfaceDesc.dwHeight = mode->height;
		ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = mode->bpp;
		ddSurfaceDesc.ddpfPixelFormat.dwRBitMask = 0xF800;
		ddSurfaceDesc.ddpfPixelFormat.dwGBitMask = 0x07E0;
		ddSurfaceDesc.ddpfPixelFormat.dwBBitMask = 0x001F;

		if (!lpEnumModesCallback(&ddSurfaceDesc, NULL))
			break;

		++mode;
	} while (--count);

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	this->mode = NULL;

	DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		if (mode->width == dwWidth && mode->height == dwHeight && mode->bpp == dwBPP)
		{
			this->mode = mode;
			break;
		}

		++mode;
	} while (--count);

	if (!this->mode)
		return DDERR_INVALIDMODE;

	MoveWindow(this->hWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);

	SetForegroundWindow(this->hWnd);

	this->RenderStop();
	this->RenderStart();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7* lplpDDSurface, IUnknown* pUnkOuter)
{
	BOOL isPrimary = lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE;

	this->surfaceEntries = new OpenDrawSurface(this, !isPrimary);
	*lplpDDSurface = (LPDIRECTDRAWSURFACE7)this->surfaceEntries;

	if (isPrimary)
		this->attachedSurface = (OpenDrawSurface*)this->surfaceEntries;

	DWORD width, height;
	if (lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
	{
		width = lpDDSurfaceDesc->dwWidth;
		height = lpDDSurfaceDesc->dwHeight;

		((OpenDrawSurface*)this->surfaceEntries)->CreateBuffer(width, height);
	}
	else if (this->windowState != WinStateWindowed)
	{
		width = this->mode->width;
		height = this->mode->height;

		((OpenDrawSurface*)this->surfaceEntries)->CreateBuffer(width, height);
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	this->clipperEntries = new OpenDrawClipper(this);
	*lplpDDClipper = (LPDIRECTDRAWCLIPPER)this->clipperEntries;

	return DD_OK;
}