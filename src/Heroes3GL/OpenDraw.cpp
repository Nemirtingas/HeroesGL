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
#include "OpenDraw.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Main.h"
#include "Config.h"
#include "Window.h"
#include "Hooks.h"
#include "ShaderGroup.h"
#include "PixelBuffer.h"
#include "FpsCounter.h"

DWORD __fastcall GetPow2(DWORD value)
{
	DWORD res = 1;
	while (res < value)
		res <<= 1;
	return res;
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
	if (ddraw->hDc)
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

		::ReleaseDC(ddraw->hDraw, ddraw->hDc);
	}

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

		BOOL isVSync = FALSE;
		if (WGLSwapInterval)
			WGLSwapInterval(0);

		DWORD clear = 0;

		BOOL isDirectUpdate = this->mode.bpp == 32 && !config.gl.caps.bgra || this->mode.bpp == 16 && config.gl.version.value <= GL_VER_1_1;
		FpsCounter* fpsCounter = new FpsCounter(isDirectUpdate ? FpsRgba : (this->mode.bpp == 32 ? FpsBgra : FpsRgb), this->texWidth);
		PixelBuffer* pixelBuffer = new PixelBuffer(this->texWidth, this->mode.height, isDirectUpdate || this->mode.bpp == 32, isDirectUpdate ? GL_RGBA : (this->mode.bpp == 32 ? GL_BGRA_EXT : GL_RGB), config.updateMode);
		{
			do
			{
				OpenDrawSurface* surface = this->attachedSurface;
				if (!surface)
					continue;

				BOOL isFps = this->isFpsChanged;
				this->isFpsChanged = FALSE;
				if (config.fps.state)
				{
					if (isFps)
						fpsCounter->Reset();
					fpsCounter->Calculate();
				}

				BOOL vs = config.image.vSync && this->windowState != WinStateWindowed;
				if (isVSync != vs)
				{
					isVSync = vs;
					if (WGLSwapInterval)
						WGLSwapInterval(isVSync);
				}

				DWORD glFilter = 0;
				FilterState state = this->filterState;
				this->filterState.flags = FALSE;
				if (state.flags)
					glFilter = state.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

				BOOL isSnapshot = this->isTakeSnapshot;
				this->isTakeSnapshot = FALSE;
				if (state.flags || isFps || isSnapshot)
					clear = 0;

				FLOAT currScale = surface->scale;
				if (this->CheckView())
				{
					GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
					clear = 0;
				}

				if (clear++ <= 1)
					GLClear(GL_COLOR_BUFFER_BIT);

				if (isDirectUpdate)
				{
					BYTE* source = surface->indexBuffer;
					DWORD* dst = (DWORD*)pixelBuffer->GetBuffer();
					DWORD copyWidth = this->mode.width;
					DWORD copyHeight = this->mode.height;
					if (this->mode.bpp == 32)
					{
						do
						{
							DWORD* src = (DWORD*)source;
							source += this->pitch;

							DWORD count = copyWidth;
							do
								*dst++ = _byteswap_ulong(_rotl(*src++, 8));
							while (--count);
						} while (--copyHeight);
					}
					else
					{
						do
						{
							WORD* src = (WORD*)source;
							source += this->pitch;

							DWORD count = copyWidth;
							do
							{
								WORD px = *src++;
								*dst++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
							} while (--count);
						} while (--copyHeight);
					}
				}
				else
					pixelBuffer->Copy(surface->indexBuffer);

				fpsCounter->Draw(config.fps.state, pixelBuffer->GetBuffer());

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

						pixelBuffer->Update();
					}
					else
					{
						GLBindTexture(GL_TEXTURE_2D, frame->id);

						if (glFilter)
						{
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
						}

						pixelBuffer->Update(&frame->rect);
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

				if (isSnapshot)
					surface->TakeSnapshot();

				pixelBuffer->SwapBuffers();
				SwapBuffers(this->hDc);
				if (clear > 1 && config.fps.state != FpsBenchmark)
					WaitForSingleObject(this->hDrawEvent, INFINITE);
				GLFinish();
			} while (!this->isFinish);
		}
		delete pixelBuffer;
		delete fpsCounter;

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

	struct {
		ShaderGroup* linear;
		ShaderGroup* hermite;
		ShaderGroup* cubic;
		ShaderGroup* lanczos;
	} shaders = {
		new ShaderGroup(GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_HERMITE_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_CUBIC_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LANCZOS_FRAGMENT, SHADER_LEVELS)
	};

	ShaderGroup* program = NULL;
	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				FLOAT buffer[4][8] = {
					{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
					{ (FLOAT)this->mode.width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, 0.0f, 0.0f },
					{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, 0.0f, 1.0f, texWidth, texHeight, 0.0f, 0.0f },
					{ 0.0f, (FLOAT)this->mode.height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, 0.0f }
				};

				{
					FLOAT mvp[4][4] = {
						{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
						{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
						{ 0.0f, 0.0f, 2.0f, 0.0f },
						{ -1.0f, 1.0f, -1.0f, 1.0f }
					};

					for (DWORD i = 0; i < 4; ++i)
					{
						FLOAT* vector = &buffer[i][0];
						for (DWORD j = 0; j < 4; ++j)
						{
							FLOAT sum = 0.0f;
							for (DWORD v = 0; v < 4; ++v)
								sum += mvp[v][j] * vector[v];

							vector[j] = sum;
						}
					}

					GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
				}

				{
					GLEnableVertexAttribArray(0);
					GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

					GLEnableVertexAttribArray(1);
					GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
				}

				GLuint textureId;
				GLGenTextures(1, &textureId);
				{
					GLActiveTexture(GL_TEXTURE0);

					GLBindTexture(GL_TEXTURE_2D, textureId);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

					if (this->mode.bpp == 32)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

					BOOL isVSync = FALSE;
					if (WGLSwapInterval)
						WGLSwapInterval(0);

					FLOAT oldScale = 1.0f;
					DWORD clear = 0;

					FpsCounter* fpsCounter = new FpsCounter(this->mode.bpp == 32 ? FpsBgra : FpsRgb, this->texWidth);
					PixelBuffer* pixelBuffer = new PixelBuffer(this->texWidth, this->mode.height, this->mode.bpp == 32, this->mode.bpp == 32 ? GL_BGRA_EXT : GL_RGB, config.updateMode);
					{
						do
						{
							OpenDrawSurface* surface = this->attachedSurface;
							if (!surface)
								continue;

							BOOL isFps = this->isFpsChanged;
							this->isFpsChanged = FALSE;
							if (config.fps.state)
							{
								if (isFps)
									fpsCounter->Reset();
								fpsCounter->Calculate();
							}

							BOOL vs = config.image.vSync && this->windowState != WinStateWindowed;
							if (isVSync != vs)
							{
								isVSync = vs;
								if (WGLSwapInterval)
									WGLSwapInterval(isVSync);
							}

							FilterState state = this->filterState;
							this->filterState.flags = FALSE;

							if (program && program->Check())
								state.flags = TRUE;

							BOOL isSnapshot = this->isTakeSnapshot;
							this->isTakeSnapshot = FALSE;
							if (state.flags || isFps || isSnapshot)
								clear = 0;

							FLOAT currScale = surface->scale;

							if (this->CheckView())
							{
								GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);
								clear = 0;
							}

							if (clear++ <= 1)
								GLClear(GL_COLOR_BUFFER_BIT);

							if (state.flags)
							{
								switch (state.interpolation)
								{
								case InterpolateHermite:
									program = shaders.hermite;
									break;
								case InterpolateCubic:
									program = shaders.cubic;
									break;
								case InterpolateLanczos:
									program = shaders.lanczos;
									break;
								default:
									program = shaders.linear;
									break;
								}

								program->Use(texSize);

								DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
								GLBindTexture(GL_TEXTURE_2D, textureId);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
							}

							// NEXT UNCHANGED
							{
								pixelBuffer->Copy(surface->indexBuffer);
								fpsCounter->Draw(config.fps.state, pixelBuffer->GetBuffer());
								pixelBuffer->Update();
								pixelBuffer->SwapBuffers();

								if (oldScale != currScale)
								{
									oldScale = currScale;

									buffer[1][4] = texWidth * currScale;

									buffer[2][4] = texWidth * currScale;
									buffer[2][5] = texHeight * currScale;

									buffer[3][5] = texHeight * currScale;

									GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(buffer), buffer);
								}

								GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
							}

							if (isSnapshot)
								surface->TakeSnapshot();

							SwapBuffers(this->hDc);
							if (clear > 1 && config.fps.state != FpsBenchmark)
								WaitForSingleObject(this->hDrawEvent, INFINITE);
							GLFinish();
						} while (!this->isFinish);
					}
					delete pixelBuffer;
					delete fpsCounter;
				}
				GLDeleteTextures(1, &textureId);
			}
			GLBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
		GLDeleteBuffers(1, &bufferName);
	}
	GLUseProgram(NULL);

	ShaderGroup** shader = (ShaderGroup**)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderGroup*);
	do
		delete *shader++;
	while (--count);
}

VOID OpenDraw::RenderNew()
{
	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	DWORD maxTexSize = GetPow2(this->mode.width > this->mode.height ? this->mode.width : this->mode.height);
	FLOAT texWidth = this->mode.width == maxTexSize ? 1.0f : (FLOAT)this->mode.width / maxTexSize;
	FLOAT texHeight = this->mode.height == maxTexSize ? 1.0f : (FLOAT)this->mode.height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	struct {
		ShaderGroup* linear;
		ShaderGroup* hermite;
		ShaderGroup* cubic;
		ShaderGroup* lanczos;
		ShaderGroup* xBRz_2x;
		ShaderGroup* xBRz_3x;
		ShaderGroup* xBRz_4x;
		ShaderGroup* xBRz_5x;
		ShaderGroup* xBRz_6x;
		ShaderGroup* scaleHQ_2x;
		ShaderGroup* scaleHQ_4x;
		ShaderGroup* xSal_2x;
		ShaderGroup* eagle_2x;
		ShaderGroup* scaleNx_2x;
		ShaderGroup* scaleNx_3x;
	} shaders = {
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_HERMITE_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_CUBIC_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LANCZOS_FRAGMENT, SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XBRZ_FRAGMENT_2X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XBRZ_FRAGMENT_3X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XBRZ_FRAGMENT_4X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XBRZ_FRAGMENT_5X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XBRZ_FRAGMENT_6X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_SCALEHQ_FRAGMENT_2X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_SCALEHQ_FRAGMENT_4X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_XSAL_FRAGMENT, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_EAGLE_FRAGMENT, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_SCALENX_FRAGMENT_2X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_SCALENX_FRAGMENT_3X, NULL)
	};

	ShaderGroup* program = NULL;
	ShaderGroup* upscaleProgram = NULL;
	{
		GLuint arrayName;
		GLGenVertexArrays(1, &arrayName);
		{
			GLBindVertexArray(arrayName);
			{
				GLuint bufferName;
				GLGenBuffers(1, &bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
					{
						FLOAT buffer[8][8] = {
							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
							{ (FLOAT)this->mode.width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, 0.0f, 0.0f },
							{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, 0.0f, 1.0f, texWidth, texHeight, 0.0f, 0.0f },
							{ 0.0f, (FLOAT)this->mode.height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, 0.0f },

							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
							{ (FLOAT)this->mode.width, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
							{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f },
							{ 0.0f, (FLOAT)this->mode.height, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }
						};

						{
							FLOAT mvp[4][4] = {
								{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
								{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
								{ 0.0f, 0.0f, 2.0f, 0.0f },
								{ -1.0f, 1.0f, -1.0f, 1.0f }
							};

							for (DWORD i = 0; i < 8; ++i)
							{
								FLOAT* vector = &buffer[i][0];
								for (DWORD j = 0; j < 4; ++j)
								{
									FLOAT sum = 0.0f;
									for (DWORD v = 0; v < 4; ++v)
										sum += mvp[v][j] * vector[v];

									vector[j] = sum;
								}
							}

							GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
						}

						{
							GLEnableVertexAttribArray(0);
							GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

							GLEnableVertexAttribArray(1);
							GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
						}

						struct {
							GLuint primary;
							GLuint secondary;
							GLuint buffer;
						} texId;

						GLGenTextures(1, &texId.primary);
						{
							GLActiveTexture(GL_TEXTURE0);
							GLBindTexture(GL_TEXTURE_2D, texId.primary);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

							if (this->mode.bpp == 32)
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
							else
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

							GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

							BOOL isVSync = FALSE;
							if (WGLSwapInterval)
								WGLSwapInterval(0);

							FLOAT oldScale = 1.0f;
							DWORD clear = 0;

							FpsCounter* fpsCounter = new FpsCounter(this->mode.bpp == 32 ? FpsBgra : FpsRgb, this->texWidth);
							PixelBuffer* firstBuffer = new PixelBuffer(this->texWidth, this->mode.height, this->mode.bpp == 32, this->mode.bpp == 32 ? GL_BGRA_EXT : GL_RGB, config.updateMode);
							{
								GLuint fboId = 0;
								DWORD viewSize;
								BOOL activeIndex;
								VOID* emptyBuffer;
								PixelBuffer* secondBuffer;

								do
								{
									OpenDrawSurface* surface = this->attachedSurface;
									if (!surface)
										continue;

									BOOL isFps = this->isFpsChanged;
									this->isFpsChanged = FALSE;
									if (config.fps.state)
									{
										if (isFps)
											fpsCounter->Reset();
										fpsCounter->Calculate();
									}

									BOOL vs = config.image.vSync && this->windowState != WinStateWindowed;
									if (isVSync != vs)
									{
										isVSync = vs;
										if (WGLSwapInterval)
											WGLSwapInterval(isVSync);
									}

									FilterState state = this->filterState;
									this->filterState.flags = FALSE;

									if (program && program->Check())
										state.flags = TRUE;

									BOOL isSnapshot = this->isTakeSnapshot;
									this->isTakeSnapshot = FALSE;
									if (state.flags || isFps || isSnapshot)
										clear = 0;

									FLOAT currScale = surface->scale;
									PixelBuffer* pixelBuffer;

									if (state.upscaling)
									{
										if (state.flags)
										{
											switch (state.upscaling)
											{
											case UpscaleScaleNx:
												switch (state.value)
												{
												case 3:
													upscaleProgram = shaders.scaleNx_3x;
													break;
												default:
													upscaleProgram = shaders.scaleNx_2x;
													break;
												}

												break;

											case UpscaleScaleHQ:
												switch (state.value)
												{
												case 4:
													upscaleProgram = shaders.scaleHQ_4x;
													break;
												default:
													upscaleProgram = shaders.scaleHQ_2x;
													break;
												}

												break;

											case UpscaleXRBZ:
												switch (state.value)
												{
												case 6:
													upscaleProgram = shaders.xBRz_6x;
													break;
												case 5:
													upscaleProgram = shaders.xBRz_5x;
													break;
												case 4:
													upscaleProgram = shaders.xBRz_4x;
													break;
												case 3:
													upscaleProgram = shaders.xBRz_3x;
													break;
												default:
													upscaleProgram = shaders.xBRz_2x;
													break;
												}

												break;

											case UpscaleXSal:
												upscaleProgram = shaders.xSal_2x;
												break;

											default:
												upscaleProgram = shaders.eagle_2x;
												break;
											}

											if (!fboId)
											{
												viewSize = MAKELONG(this->mode.width * state.value, this->mode.height * state.value);
												activeIndex = TRUE;
												firstBuffer->Reset();
												secondBuffer = new PixelBuffer(this->texWidth, this->mode.height, this->mode.bpp == 32, this->mode.bpp == 32 ? GL_BGRA_EXT : GL_RGB, config.updateMode);

												DWORD size = this->pitch * this->mode.height;
												emptyBuffer = AlignedAlloc(size);
												MemoryZero(emptyBuffer, size);

												GLGenTextures(2, &texId.secondary);

												GLBindTexture(GL_TEXTURE_2D, texId.secondary);
												{
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

													GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->mode.width);
													{
														if (this->mode.bpp == 32)
														{
															GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGBA, GL_UNSIGNED_BYTE, emptyBuffer);
														}
														else
														{
															GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, emptyBuffer);
														}
													}
													GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
												}

												GLBindTexture(GL_TEXTURE_2D, texId.buffer);
												{
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
													GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
												}

												GLGenFramebuffers(1, &fboId);
												GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
												GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId.buffer, 0);
											}
											else
											{
												GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

												DWORD newSize = MAKELONG(this->mode.width * state.value, this->mode.height * state.value);
												if (viewSize != newSize)
												{
													viewSize = newSize;
													activeIndex = TRUE;
													firstBuffer->Reset();
													secondBuffer->Reset();

													GLBindTexture(GL_TEXTURE_2D, texId.secondary);
													GLPixelStorei(GL_UNPACK_ROW_LENGTH, this->mode.width);
													{
														if (this->mode.bpp == 32)
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGBA, GL_UNSIGNED_BYTE, emptyBuffer);
														else
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, emptyBuffer);
													}
													GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

													GLBindTexture(GL_TEXTURE_2D, texId.buffer);
													GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
												}
											}
										}
										else
											GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

										if (this->CheckView())
											clear = 0;

										GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

										GLActiveTexture(GL_TEXTURE1);
										GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&texId.primary)[activeIndex]);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

										activeIndex = !activeIndex;

										GLActiveTexture(GL_TEXTURE0);
										GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&texId.primary)[activeIndex]);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

										upscaleProgram->Use(texSize);

										pixelBuffer = !activeIndex ? firstBuffer : secondBuffer;
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
											if (fboId)
											{
												GLBindTexture(GL_TEXTURE_2D, texId.primary);
												GLDeleteTextures(2, &texId.secondary);
												GLDeleteFramebuffers(1, &fboId);
												AlignedFree(emptyBuffer);

												firstBuffer->Reset();
												delete secondBuffer;

												fboId = 0;
											}

											switch (state.interpolation)
											{
											case InterpolateHermite:
												program = shaders.hermite;
												break;
											case InterpolateCubic:
												program = shaders.cubic;
												break;
											case InterpolateLanczos:
												program = shaders.lanczos;
												break;
											default:
												program = shaders.linear;
												break;
											}

											program->Use(texSize);

											DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
										}

										pixelBuffer = firstBuffer;
									}

									// NEXT UNCHANGED
									{
										pixelBuffer->Copy(surface->indexBuffer);
										fpsCounter->Draw(config.fps.state, pixelBuffer->GetBuffer());
										pixelBuffer->Update();
										pixelBuffer->SwapBuffers();

										if (oldScale != currScale)
										{
											oldScale = currScale;

											buffer[1][4] = texWidth * currScale;

											buffer[2][4] = texWidth * currScale;
											buffer[2][5] = texHeight * currScale;

											buffer[3][5] = texHeight * currScale;

											GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(buffer) >> 1, buffer);
										}

										GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
									}

									// Draw from FBO
									if (state.upscaling)
									{
										GLFinish();
										GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

										GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

										if (clear++ <= 1)
											GLClear(GL_COLOR_BUFFER_BIT);

										switch (state.interpolation)
										{
										case InterpolateHermite:
											program = shaders.hermite;
											break;
										case InterpolateCubic:
											program = shaders.cubic;
											break;
										case InterpolateLanczos:
											program = shaders.lanczos;
											break;
										default:
											program = shaders.linear;
											break;
										}

										program->Use(viewSize);

										GLBindTexture(GL_TEXTURE_2D, texId.buffer);

										DWORD filter = state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

										GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);

										if (isSnapshot && OpenClipboard(NULL))
										{
											EmptyClipboard();

											DWORD size = LOWORD(viewSize) * HIWORD(viewSize) * 3;
											DWORD slice = sizeof(BITMAPINFOHEADER);
											HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, slice + size);
											if (hMemory)
											{
												VOID* data = GlobalLock(hMemory);
												if (data)
												{
													BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*)data;
													bmi->biSize = sizeof(BITMAPINFOHEADER);
													bmi->biWidth = LOWORD(viewSize);
													bmi->biHeight = HIWORD(viewSize);
													bmi->biPlanes = 1;
													bmi->biBitCount = 24;
													bmi->biCompression = BI_RGB;
													bmi->biSizeImage = size;
													bmi->biXPelsPerMeter = 1;
													bmi->biYPelsPerMeter = 1;
													bmi->biClrUsed = 0;
													bmi->biClrImportant = 0;

													GLGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, (BYTE*)data + slice);

													GlobalUnlock(hMemory);
													SetClipboardData(CF_DIB, hMemory);
												}

												GlobalFree(hMemory);
											}

											CloseClipboard();
										}
									}
									else if (isSnapshot)
										surface->TakeSnapshot();

									SwapBuffers(this->hDc);
									if (clear > 1 && config.fps.state != FpsBenchmark)
										WaitForSingleObject(this->hDrawEvent, INFINITE);
									GLFinish();
								} while (!this->isFinish);

								if (fboId)
								{
									GLDeleteTextures(2, &texId.secondary);
									GLDeleteFramebuffers(1, &fboId);
									AlignedFree(emptyBuffer);
									delete secondBuffer;
								}
							}
							delete firstBuffer;
							delete fpsCounter;
						}
						GLDeleteTextures(1, &texId.primary);
					}
					GLBindBuffer(GL_ARRAY_BUFFER, NULL);
				}
				GLDeleteBuffers(1, &bufferName);
			}
			GLBindVertexArray(NULL);
		}
		GLDeleteVertexArrays(1, &arrayName);
	}
	GLUseProgram(NULL);

	ShaderGroup** shader = (ShaderGroup**)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderGroup*);
	do
		delete *shader++;
	while (--count);
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
		{
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		}
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
	this->isFpsChanged = TRUE;

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
		GL::ResetPixelFormat(this->hWnd);
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
	this->pitch = this->mode.width * this->mode.bpp >> 3;
	if (this->pitch & 3)
		this->pitch = (this->pitch & 0xFFFFFFFC) + 4;
	this->texWidth = this->pitch / (this->mode.bpp >> 3);

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

	this->RenderStop();
	this->windowState = (dwFlags & DDSCL_FULLSCREEN) ? WinStateFullScreen : WinStateWindowed;
	Hooks::CheckRefreshRate();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	this->mode = { dwWidth, dwHeight, dwBPP };
	this->pitch = this->mode.width * this->mode.bpp >> 3;
	if (this->pitch & 3)
		this->pitch = (this->pitch & 0xFFFFFFFC) + 4;
	this->texWidth = this->pitch / (this->mode.bpp >> 3);

	RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	AdjustWindowRect(&rect, GetWindowLong(this->hWnd, GWL_STYLE), FALSE);
	MoveWindow(this->hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

	SetForegroundWindow(this->hWnd);
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

			if (this->attachedSurface)
			{
				width = GetSystemMetrics(SM_CXSCREEN);
				height = GetSystemMetrics(SM_CYSCREEN);

				this->attachedSurface->CreateBuffer(max(this->mode.width, width), max(this->mode.height, height));
			}

			RenderStart();
			this->isNextIsMode = FALSE;
		}
	}
	else
	{
		if (this->windowState != WinStateWindowed)
		{
			((OpenDrawSurface*)this->surfaceEntries)->CreateBuffer(this->mode.width, this->mode.height);
			RenderStart();
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