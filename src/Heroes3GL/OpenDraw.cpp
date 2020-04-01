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
#include "Config.h"
#include "Window.h"
#include "Hooks.h"

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
		if (ddraw->mode.width)
		{
			ddraw->hDc = ::GetDC(ddraw->hDraw);
			{
				if (!::GetPixelFormat(ddraw->hDc))
				{
					PIXELFORMATDESCRIPTOR pfd;
					INT glPixelFormat = GL::PreparePixelFormat(&pfd);
					if (!glPixelFormat)
					{
						glPixelFormat = ::ChoosePixelFormat(ddraw->hDc, &pfd);
						if (!glPixelFormat)
							Main::ShowError(IDS_ERROR_CHOOSE_PF, "OpenDraw.cpp", __LINE__);
						else if (pfd.dwFlags & PFD_NEED_PALETTE)
							Main::ShowError(IDS_ERROR_NEED_PALETTE, "OpenDraw.cpp", __LINE__);
					}

					GL::ResetPixelFormatDescription(&pfd);
					if (::DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
						Main::ShowError(IDS_ERROR_DESCRIBE_PF, "OpenDraw.cpp", __LINE__);

					if (!::SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
						Main::ShowError(IDS_ERROR_SET_PF, "OpenDraw.cpp", __LINE__);

					if ((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.cRedBits < 5) || (pfd.cGreenBits < 6) || (pfd.cBlueBits < 5))
						Main::ShowError(IDS_ERROR_BAD_PF, "OpenDraw.cpp", __LINE__);
				}

				HGLRC hRc = wglCreateContext(ddraw->hDc);
				if (hRc)
				{
					if (wglMakeCurrent(ddraw->hDc, hRc))
					{
						GL::CreateContextAttribs(ddraw->hDc, &hRc);
						if (config.gl.version.value >= GL_VER_2_0)
						{
							DWORD glMaxTexSize;
							GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
							if (glMaxTexSize < GetPow2(ddraw->mode.width > ddraw->mode.height ? ddraw->mode.width : ddraw->mode.height))
								config.gl.version.value = GL_VER_1_1;
						}

						config.gl.version.real = config.gl.version.value;
						switch (config.renderer)
						{
						case RendererOpenGL1:
							if (config.gl.version.value > GL_VER_1_1)
								config.gl.version.value = GL_VER_1_2;
							break;

						case RendererOpenGL2:
							if (config.gl.version.value >= GL_VER_2_0)
								config.gl.version.value = GL_VER_2_0;
							else
								config.renderer = RendererAuto;
							break;

						case RendererOpenGL3:
							if (config.gl.version.value >= GL_VER_3_0)
								config.gl.version.value = GL_VER_3_0;
							else
								config.renderer = RendererAuto;
							break;

						default:
							break;
						}

						if (config.gl.version.value >= GL_VER_3_0)
							ddraw->RenderNew();
						else if (config.gl.version.value >= GL_VER_2_0)
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

	DWORD maxAllow = GetPow2(this->mode.width > this->mode.height ? this->mode.width : this->mode.height);
	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;

	DWORD framePerWidth = this->mode.width / maxTexSize + (this->mode.width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->mode.height / maxTexSize + (this->mode.height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;
	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < this->mode.height; y += maxTexSize)
		{
			DWORD height = this->mode.height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < this->mode.width; x += maxTexSize, ++frame)
			{
				DWORD width = this->mode.width - x;
				if (width > maxTexSize)
					width = maxTexSize;

				frame->point.x = x;
				frame->point.y = y;

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

				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				if (this->mode.bpp == 16)
				{
					if (config.gl.version.value > GL_VER_1_1)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				}
				else
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, config.gl.caps.bgra ? GL_BGRA_EXT : GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)this->mode.width, (GLdouble)this->mode.height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		VOID* frameBuffer = NULL;
		BOOL isPixelStore = this->mode.bpp == 32 && config.gl.caps.bgra || this->mode.bpp == 16 && config.gl.version.value > GL_VER_1_1;
		if (!isPixelStore)
			frameBuffer = MemoryAlloc(maxTexSize * maxTexSize * (this->mode.bpp == 16 && config.gl.version.value > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			BOOL isVSync = config.image.vSync;
			if (WGLSwapInterval)
				WGLSwapInterval(isVSync);

			BOOL first = TRUE;
			DWORD clear = 0;
			do
			{
				OpenDrawSurface* surface = this->attachedSurface;
				if (this->attachedSurface)
				{
					if (isVSync != config.image.vSync)
					{
						isVSync = config.image.vSync;
						if (WGLSwapInterval)
							WGLSwapInterval(isVSync);
					}

					UpdateRect* updateClip = surface->poinetrClip;
					UpdateRect* finClip = surface->currentClip;
					surface->poinetrClip = finClip;

					if (this->CheckView())
					{
						GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
						clear = 0;
					}

					if (clear++ <= 1)
						GLClear(GL_COLOR_BUFFER_BIT);

					DWORD glFilter = 0;
					FilterState state = this->filterState;
					this->filterState.flags = FALSE;
					if (state.flags)
						glFilter = state.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

					FLOAT currScale = surface->scale;
					if (surface->isSizeChanged || first)
					{
						surface->isSizeChanged = FALSE;
						first = FALSE;

						updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
						updateClip->rect.left = 0;
						updateClip->rect.top = 0;
						updateClip->rect.right = this->mode.width;
						updateClip->rect.bottom = this->mode.height;
						updateClip->isActive = TRUE;
					}

					BOOL isDouble = currScale != 1.0f;
					if (isPixelStore)
						GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->mode.width);
					{
						DWORD frameWidth = isDouble ? DWORD(currScale * this->mode.width) : this->mode.width;
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
										if (isDouble)
										{
											update.left = DWORD(currScale * update.left);
											update.top = DWORD(currScale * update.top);
											update.right = DWORD(currScale * update.right);
											update.bottom = DWORD(currScale * update.bottom);

											texWidth = DWORD(currScale * texWidth);
											texHeight = DWORD(currScale * texHeight);
										}

										if (texWidth == frameWidth)
										{
											if (this->mode.bpp == 32)
											{
												if (config.gl.caps.bgra)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * texWidth);
												else
												{
													DWORD* source = (DWORD*)surface->indexBuffer + update.top * texWidth;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = texWidth;
													DWORD copyHeight = texHeight;
													do
													{
														DWORD* src = source;
														source += frameWidth;

														DWORD count = copyWidth;
														do
															*dest++ = _byteswap_ulong(_rotl(*src++, 8));
														while (--count);
													} while (--copyHeight);

													GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
												}
											}
											else
											{
												if (config.gl.version.value > GL_VER_1_1)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * texWidth);
												else
												{
													WORD* source = (WORD*)surface->indexBuffer + update.top * texWidth;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = texWidth;
													DWORD copyHeight = texHeight;
													do
													{
														WORD* src = source;
														source += frameWidth;

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
										}
										else
										{
											if (this->mode.bpp == 32)
											{
												if (config.gl.caps.bgra)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * frameWidth + update.left);
												else
												{
													DWORD* source = (DWORD*)surface->indexBuffer + update.top * frameWidth + update.left;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = texWidth;
													DWORD copyHeight = texHeight;
													do
													{
														DWORD* src = source;
														source += frameWidth;

														DWORD count = copyWidth;
														do
															*dest++ = _byteswap_ulong(_rotl(*src++, 8));
														while (--count);
													} while (--copyHeight);

													GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
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

												if (config.gl.version.value > GL_VER_1_1)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * frameWidth + update.left);
												else
												{
													WORD* source = (WORD*)surface->indexBuffer + update.top * frameWidth + update.left;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = texWidth;
													DWORD copyHeight = texHeight;
													do
													{
														WORD* src = source;
														source += frameWidth;

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

								INT rect_right = frame->rect.x + frame->rect.width;
								INT rect_bottom = frame->rect.y + frame->rect.height;

								UpdateRect* update = updateClip;
								while (update != finClip)
								{
									if (update->isActive)
									{
										RECT clip = {
											frame->rect.x > update->rect.left ? frame->rect.x : update->rect.left,
											frame->rect.y > update->rect.top ? frame->rect.y : update->rect.top,
											rect_right < update->rect.right ? rect_right : update->rect.right,
											rect_bottom < update->rect.bottom ? rect_bottom : update->rect.bottom
										};

										INT clipWidth = clip.right - clip.left;
										INT clipHeight = clip.bottom - clip.top;
										if (clipWidth > 0 && clipHeight > 0)
										{
											if (this->mode.bpp == 32)
											{
												if (config.gl.caps.bgra)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - frame->rect.x, clip.top - frame->rect.y, clipWidth, clipHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + clip.top * frameWidth + clip.left);
												else
												{
													DWORD* source = (DWORD*)surface->indexBuffer + clip.top * frameWidth + clip.left;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = clipWidth;
													DWORD copyHeight = clipHeight;
													do
													{
														DWORD* src = source;
														source += frameWidth;

														DWORD count = copyWidth;
														do
															*dest++ = _byteswap_ulong(_rotl(*src++, 8));
														while (--count);
													} while (--copyHeight);

													GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - frame->rect.x, clip.top - frame->rect.y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
												}
											}
											else
											{
												if (clipWidth & 1)
												{
													++clipWidth;
													if (clip.left != frame->rect.x)
														--clip.left;
													else
														++clip.right;
												}

												if (config.gl.version.value > GL_VER_1_1)
													GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - frame->rect.x, clip.top - frame->rect.y, clipWidth, clipHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + clip.top * frameWidth + clip.left);
												else
												{
													WORD* source = (WORD*)surface->indexBuffer + clip.top * frameWidth + clip.left;
													DWORD* dest = (DWORD*)frameBuffer;
													DWORD copyWidth = clipWidth;
													DWORD copyHeight = clipHeight;
													do
													{
														WORD* src = source;
														source += frameWidth;

														DWORD count = copyWidth;
														do
														{
															WORD px = *src++;
															*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
														} while (--count);
													} while (--copyHeight);

													GLTexSubImage2D(GL_TEXTURE_2D, 0, clip.left - frame->rect.x, clip.top - frame->rect.y, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
												}
											}
										}
									}

									if (++update == surface->endClip)
										update = surface->clipsList;
								}
							}

							GLBegin(GL_TRIANGLE_FAN);
							{
								FLOAT texX = frame->tSize.width * currScale;
								FLOAT texY = frame->tSize.height * currScale;

								GLTexCoord2f(0.0f, 0.0f);
								GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

								GLTexCoord2f(texX, 0.0f);
								GLVertex2s(frame->vSize.width, (SHORT)frame->point.y);

								GLTexCoord2f(texX, texY);
								GLVertex2s(frame->vSize.width, frame->vSize.height);

								GLTexCoord2f(0.0f, texY);
								GLVertex2s((SHORT)frame->point.x, frame->vSize.height);
							}
							GLEnd();
							++frame;
						}
					}
					if (isPixelStore)
						GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

					if (this->isTakeSnapshot)
					{
						this->isTakeSnapshot = FALSE;

						if (OpenClipboard(NULL))
						{
							EmptyClipboard();

							DWORD texWidth = this->mode.width;
							DWORD texHeight = this->mode.height;
							if (isDouble)
							{
								texWidth = DWORD(currScale * texWidth);
								texHeight = DWORD(currScale * texHeight);
							}

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
						}
					}

					SwapBuffers(this->hDc);
					GLFinish();

					if (clear >= 2)
						WaitForSingleObject(this->hDrawEvent, INFINITE);
				}
			} while (!this->isFinish);
		}
		if (!isPixelStore)
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

	DWORD maxTexSize = GetPow2(this->mode.width > this->mode.height ? this->mode.width : this->mode.height);
	FLOAT texWidth = this->mode.width == maxTexSize ? 1.0f : (FLOAT)this->mode.width / maxTexSize;
	FLOAT texHeight = this->mode.height == maxTexSize ? 1.0f : (FLOAT)this->mode.height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->mode.width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, texHeight }
	};

	FLOAT vertices[4][4];
	MemoryCopy(vertices, buffer, 16 * sizeof(FLOAT));

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
		ShaderProgram linear;
		ShaderProgram hermite;
		ShaderProgram cubic;
	} shaders = {
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix }
	};

	ShaderProgram* filterProgram = &shaders.linear;
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
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

					if (this->mode.bpp == 32)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

					BOOL isVSync = config.image.vSync;
					if (WGLSwapInterval)
						WGLSwapInterval(isVSync);

					FLOAT oldScale = 1.0f;
					BOOL first = TRUE;
					DWORD clear = 0;
					do
					{
						OpenDrawSurface* surface = this->attachedSurface;
						if (surface)
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
								this->viewport.refresh = TRUE;

							BOOL isTakeSnapshot = this->isTakeSnapshot;
							if (isTakeSnapshot)
								this->isTakeSnapshot = FALSE;

							UpdateRect* updateClip = surface->poinetrClip;
							UpdateRect* finClip = surface->currentClip;
							surface->poinetrClip = finClip;

							if (this->CheckView())
							{
								GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
								clear = 0;
							}

							if (clear++ <= 1)
								GLClear(GL_COLOR_BUFFER_BIT);

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

							FLOAT currScale = surface->scale;
							BOOL isDouble = currScale != 1.0f;
							if (surface->isSizeChanged || first)
							{
								surface->isSizeChanged = FALSE;
								first = FALSE;

								updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
								updateClip->rect.left = 0;
								updateClip->rect.top = 0;
								updateClip->rect.right = this->mode.width;
								updateClip->rect.bottom = this->mode.height;
								updateClip->isActive = TRUE;
							}

							// NEXT UNCHANGED
							{
								// Update texture
								GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->mode.width);
								DWORD frameWidth = isDouble ? DWORD(currScale * this->mode.width) : this->mode.width;
								while (updateClip != finClip)
								{
									if (updateClip->isActive)
									{
										RECT update = updateClip->rect;
										DWORD texWidth = update.right - update.left;
										DWORD texHeight = update.bottom - update.top;
										if (isDouble)
										{
											update.left = DWORD(currScale * update.left);
											update.top = DWORD(currScale * update.top);
											update.right = DWORD(currScale * update.right);
											update.bottom = DWORD(currScale * update.bottom);

											texWidth = DWORD(currScale * texWidth);
											texHeight = DWORD(currScale * texHeight);
										}

										if (texWidth == frameWidth)
										{
											if (this->mode.bpp == 32)
												GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * texWidth);
											else
												GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * texWidth);
										}
										else
										{
											if (this->mode.bpp == 32)
												GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * frameWidth + update.left);
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

												GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * frameWidth + update.left);
											}
										}
									}

									if (++updateClip == surface->endClip)
										updateClip = surface->clipsList;
								}
								GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

								if (oldScale != currScale)
								{
									oldScale = currScale;

									if (currScale == 1.0f)
										MemoryCopy(vertices, buffer, 16 * sizeof(FLOAT));
									else
									{
										vertices[1][2] = buffer[1][2] * currScale;

										vertices[2][2] = buffer[2][2] * currScale;
										vertices[2][3] = buffer[2][3] * currScale;

										vertices[3][3] = buffer[3][3] * currScale;
									}

									GLBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(FLOAT), vertices);
								}

								GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
							}

							if (isTakeSnapshot)
							{
								if (OpenClipboard(NULL))
								{
									EmptyClipboard();

									DWORD texWidth = this->mode.width;
									DWORD texHeight = this->mode.height;
									if (isDouble)
									{
										texWidth = DWORD(currScale * texWidth);
										texHeight = DWORD(currScale * texHeight);
									}

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
								}
							}

							SwapBuffers(this->hDc);
							GLFinish();

							if (clear >= 2)
								WaitForSingleObject(this->hDrawEvent, INFINITE);
						}
					} while (!this->isFinish);
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

	DWORD maxTexSize = GetPow2(this->mode.width > this->mode.height ? this->mode.width : this->mode.height);
	FLOAT texWidth = this->mode.width == maxTexSize ? 1.0f : (FLOAT)this->mode.width / maxTexSize;
	FLOAT texHeight = this->mode.height == maxTexSize ? 1.0f : (FLOAT)this->mode.height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[8][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->mode.width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, texHeight },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ (FLOAT)this->mode.width, 0.0f, 1.0f, 1.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, 1.0f, 0.0f },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, 0.0f }
	};

	FLOAT vertices[4][4];
	MemoryCopy(vertices, buffer, 16 * sizeof(FLOAT));

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
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

						GLEnableVertexAttribArray(0);
						GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

						GLEnableVertexAttribArray(1);
						GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

						GLuint textureId;
						GLGenTextures(1, &textureId);
						{
							GLActiveTexture(GL_TEXTURE0);

							GLBindTexture(GL_TEXTURE_2D, textureId);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

							if (this->mode.bpp == 32)
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
							else
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0, tboId = 0;
								{
									GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

									ShaderProgram* upscaleProgram;
									BOOL isVSync = config.image.vSync;
									if (WGLSwapInterval)
										WGLSwapInterval(isVSync);

									FLOAT oldScale = 1.0f;
									BOOL first = TRUE;
									DWORD clear = 0;
									do
									{
										OpenDrawSurface* surface = this->attachedSurface;
										if (surface)
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
												this->viewport.refresh = TRUE;

											BOOL isTakeSnapshot = this->isTakeSnapshot;
											if (isTakeSnapshot)
												this->isTakeSnapshot = FALSE;

											UpdateRect* updateClip = surface->poinetrClip;
											UpdateRect* finClip = surface->currentClip;
											surface->poinetrClip = finClip;

											FLOAT currScale = surface->scale;
											BOOL isDouble = currScale != 1.0f;

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

													DWORD newSize = MAKELONG(this->mode.width * state.value, this->mode.height * state.value);
													if (newSize != viewSize)
													{
														first = TRUE;

														if (!viewSize)
														{
															GLGenTextures(1, &tboId);
															GLGenRenderbuffers(1, &rboId);
														}

														viewSize = newSize;

														// Gen texture
														GLBindTexture(GL_TEXTURE_2D, tboId);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
														GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

														// Get storage
														GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
														GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, LOWORD(viewSize), HIWORD(viewSize));
														GLBindRenderbuffer(GL_RENDERBUFFER, NULL);

														GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tboId, 0);
														GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

														if (!stencil)
														{
															DWORD size = STENCIL_COUNT * sizeof(POINTFLOAT) * STENCIL_POINTS;
															stencil = (POINTFLOAT*)MemoryAlloc(size);

															UseShaderProgram(&shaders.stencil, 0);
															{
																GLGenVertexArrays(1, &stArrayName);
																GLBindVertexArray(stArrayName);
																GLGenBuffers(1, &stBufferName);
																GLBindBuffer(GL_ARRAY_BUFFER, stBufferName);
																{
																	GLBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STREAM_DRAW);

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

												// Clear and stencil
												if (surface->isSizeChanged || first)
												{
													surface->isSizeChanged = FALSE;
													first = FALSE;

													updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
													updateClip->rect.left = 0;
													updateClip->rect.top = 0;
													updateClip->rect.right = this->mode.width;
													updateClip->rect.bottom = this->mode.height;
													updateClip->isActive = TRUE;
												}
												else
												{
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
																POINTFLOAT* point = stencil;
																UpdateRect* clip = updateClip;
																while (clip != finClip)
																{
																	if (clip->isActive)
																	{
																		point->x = (FLOAT)clip->rect.left;
																		point->y = (FLOAT)clip->rect.top;
																		++point;
																		point->x = (FLOAT)clip->rect.right;
																		point->y = (FLOAT)clip->rect.top;
																		++point;
																		point->x = (FLOAT)clip->rect.right;
																		point->y = (FLOAT)clip->rect.bottom;
																		++point;

																		point->x = (FLOAT)clip->rect.left;
																		point->y = (FLOAT)clip->rect.top;
																		++point;
																		point->x = (FLOAT)clip->rect.right;
																		point->y = (FLOAT)clip->rect.bottom;
																		++point;
																		point->x = (FLOAT)clip->rect.left;
																		point->y = (FLOAT)clip->rect.bottom;
																		++point;
																	}

																	if (++clip == surface->endClip)
																		clip = surface->clipsList;
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
													clear = 0;
												}

												if (clear++ <= 1)
													GLClear(GL_COLOR_BUFFER_BIT);

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

												if (surface->isSizeChanged || first)
												{
													surface->isSizeChanged = FALSE;
													first = FALSE;

													updateClip = (finClip == surface->clipsList ? surface->endClip : finClip) - 1;
													updateClip->rect.left = 0;
													updateClip->rect.top = 0;
													updateClip->rect.right = this->mode.width;
													updateClip->rect.bottom = this->mode.height;
													updateClip->isActive = TRUE;
												}
											}

											// NEXT UNCHANGED
											{
												// Update texture
												GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->mode.width);
												DWORD frameWidth = isDouble ? DWORD(currScale * this->mode.width) : this->mode.width;
												while (updateClip != finClip)
												{
													if (updateClip->isActive)
													{
														RECT update = updateClip->rect;
														DWORD texWidth = update.right - update.left;
														DWORD texHeight = update.bottom - update.top;
														if (isDouble)
														{
															update.left = DWORD(currScale * update.left);
															update.top = DWORD(currScale * update.top);
															update.right = DWORD(currScale * update.right);
															update.bottom = DWORD(currScale * update.bottom);

															texWidth = DWORD(currScale * texWidth);
															texHeight = DWORD(currScale * texHeight);
														}

														if (texWidth == frameWidth)
														{
															if (this->mode.bpp == 32)
																GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * texWidth);
															else
																GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * texWidth);
														}
														else
														{
															if (this->mode.bpp == 32)
																GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (DWORD*)surface->indexBuffer + update.top * frameWidth + update.left);
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

																GLTexSubImage2D(GL_TEXTURE_2D, 0, update.left, update.top, texWidth, texHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)surface->indexBuffer + update.top * frameWidth + update.left);
															}
														}
													}

													if (++updateClip == surface->endClip)
														updateClip = surface->clipsList;
												}
												GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

												if (oldScale != currScale)
												{
													oldScale = currScale;

													if (currScale == 1.0f)
														MemoryCopy(vertices, buffer, 16 * sizeof(FLOAT));
													else
													{
														vertices[1][2] = buffer[1][2] * currScale;

														vertices[2][2] = buffer[2][2] * currScale;
														vertices[2][3] = buffer[2][3] * currScale;

														vertices[3][3] = buffer[3][3] * currScale;
													}

													GLBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(FLOAT), vertices);
												}

												// Draw into FBO texture
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
													if (this->CheckView())
														clear = 0;

													GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

													if (clear++ <= 1)
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

														DWORD texWidth = this->mode.width;
														DWORD texHeight = this->mode.height;
														if (isDouble)
														{
															texWidth = DWORD(currScale * texWidth);
															texHeight = DWORD(currScale * texHeight);
														}

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
													}
												}
											}

											SwapBuffers(this->hDc);
											GLFinish();

											if (clear >= 2)
												WaitForSingleObject(this->hDrawEvent, INFINITE);
										}
									} while (!this->isFinish);
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

	this->LoadFilterState();
	this->viewport.width = rect.right;
	this->viewport.height = rect.bottom;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
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

	if (this->hDraw != this->hWnd)
	{
		DestroyWindow(this->hDraw);
		GL::ResetPixelFormat();
	}

	this->hDraw = NULL;

	ClipCursor(NULL);

	config.gl.version.value = NULL;
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

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / this->mode.width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / this->mode.height;

		if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * this->mode.width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * this->mode.height;
				this->viewport.rectangle.height = (INT)MathRound(fh);
				this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
				this->viewport.clipFactor.y = this->viewport.viewFactor.x;
			}
		}

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
		if (p->x < this->viewport.rectangle.x)
			p->x = 0;
		else if (p->x >= this->viewport.rectangle.x + this->viewport.rectangle.width)
			p->x = this->mode.width - 1;
		else
			p->x = (INT)((FLOAT)(p->x - this->viewport.rectangle.x) / this->viewport.clipFactor.x);

		if (p->y < this->viewport.rectangle.y)
			p->y = 0;
		else if (p->y >= this->viewport.rectangle.y + this->viewport.rectangle.height)
			p->y = this->mode.height - 1;
		else
			p->y = (INT)((FLOAT)(p->y - this->viewport.rectangle.y) / this->viewport.clipFactor.y);
	}
}

OpenDraw::OpenDraw(IDraw** last)
{
	this->refCount = 1;
	this->last = *last;
	*last = this;

	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;

	this->attachedSurface = NULL;

	this->hDc = NULL;
	this->hWnd = NULL;
	this->hDraw = NULL;

	this->mode = displayMode;

	this->isNextIsMode = FALSE;
	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;

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

	Hooks::CheckRefreshRate();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	this->mode.width = dwWidth;
	this->mode.height = dwHeight;
	this->mode.bpp = dwBPP;

	RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	AdjustWindowRect(&rect, GetWindowLong(this->hWnd, GWL_STYLE), FALSE);
	MoveWindow(this->hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

	SetForegroundWindow(this->hWnd);

	this->RenderStop();
	this->RenderStart();

	Hooks::CheckRefreshRate();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	BOOL isPrimary = lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE;
	*lplpDDSurface = new OpenDrawSurface(this, !isPrimary);

	if (isPrimary)
		this->attachedSurface = (OpenDrawSurface*)this->surfaceEntries;

	if (lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
	{
		DWORD width = lpDDSurfaceDesc->dwWidth;
		DWORD height = lpDDSurfaceDesc->dwHeight;
		((OpenDrawSurface*)this->surfaceEntries)->CreateBuffer(width, height);

		if (this->isNextIsMode)
		{
			this->mode.width = width;
			this->mode.height = height;
			this->isNextIsMode = FALSE;

			if (this->attachedSurface)
			{
				width = GetSystemMetrics(SM_CXSCREEN);
				height = GetSystemMetrics(SM_CYSCREEN);

				this->attachedSurface->CreateBuffer(
					this->mode.width > width ? this->mode.width : width,
					this->mode.height > height ? this->mode.height : height);
			}
		}
	}
	else
	{
		if (this->windowState != WinStateWindowed)
		{
			((OpenDrawSurface*)this->surfaceEntries)->CreateBuffer(this->mode.width, this->mode.height);
			this->isNextIsMode = FALSE;
		}
		else
			this->isNextIsMode = TRUE;
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	*lplpDDClipper = new OpenDrawClipper(this);
	return DD_OK;
}