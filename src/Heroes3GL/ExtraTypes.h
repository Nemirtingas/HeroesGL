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

#pragma once
#include "windows.h"
#include "GLib.h"

struct Rect
{
	INT x;
	INT y;
	INT width;
	INT height;
};

struct VecSize
{
	INT width;
	INT height;
};

struct TexSize
{
	FLOAT width;
	FLOAT height;
};

struct Frame
{
	GLuint id;
	Rect rect;
	POINT point;
	VecSize vSize;
	TexSize tSize;
};

struct Viewport
{
	BOOL refresh;
	INT width;
	INT height;
	Rect rectangle;
	POINTFLOAT viewFactor;
	POINTFLOAT clipFactor;
};

enum WindowState
{
	WinStateNone = 0,
	WinStateFullScreen,
	WinStateWindowed
};

enum RendererType
{
	RendererAuto = 0,
	RendererOpenGL1 = 1,
	RendererOpenGL2 = 2,
	RendererOpenGL3 = 3
};

enum InterpolationFilter : BYTE
{
	InterpolateNearest = 0,
	InterpolateLinear = 1,
	InterpolateHermite = 2,
	InterpolateCubic = 3
};

enum UpscalingFilter : BYTE
{
	UpscaleNone = 0,
	UpscaleXRBZ = 1,
	UpscaleScaleHQ = 2,
	UpscaleXSal = 3,
	UpscaleEagle = 4,
	UpscaleScaleNx = 5
};

struct FilterState {
	InterpolationFilter interpolation;
	UpscalingFilter upscaling;
	BYTE value;
	BYTE flags;
};

struct DisplayMode
{
	DWORD width;
	DWORD height;
	DWORD bpp;
};

enum FpsState
{
	FpsDisabled = 0,
	FpsNormal,
	FpsBenchmark
};

struct UpdateRect
{
	RECT rect;
	BOOL isActive;
};

struct ShaderProgram
{
	GLuint id;
	const CHAR* version;
	DWORD vertexName;
	DWORD fragmentName;
	GLfloat* mvp;
	struct {
		GLint location;
		DWORD value;
	} texSize;
};

struct MoveObject {
	DWORD distance[5];
	DWORD timeout[5];
	LONG offset[3];
};

struct AddressSpace
{
	DWORD check_1;
	DWORD check_2;
	DWORD equal_address;
	DWORD equal_value;
	DWORD renderNop;
	DWORD cursor_time_1;
	DWORD cursor_time_2;
	DWORD move_object;
	DWORD move_address;
	DWORD bpp_address;
	/*DWORD video_address;
	DWORD video_count;*/
	LCID resLanguage;
	DWORD move_oldCenter;
	DWORD move_drawRect;
	DWORD move_lifeCycle;
	const CHAR* windowName;
};

struct TrackInfo
{
	TrackInfo* last;
	DWORD position;
	CHAR* group;
	CHAR* path;
};

struct VideoInfo
{
	CHAR* fileName;
	CHAR* altName;
	BYTE isBink;
	BYTE flags[11];
};

struct VideoFile
{
	CHAR name[40];
	DWORD stride;
};

struct ConfigItems
{
	BOOL isDDraw;
	LCID language;
	CHAR title[MAX_PATH];
	HCURSOR cursor;
	HICON icon;
	HFONT font;
	UINT msgMenu;

	BOOL singleWindow;
	BOOL coldCPU;
	RendererType renderer;

	struct {
		BOOL scroll;
		BOOL move;
	} smooth;

	struct {
		struct {
			DWORD real;
			DWORD value;
		} version;
		struct {
			BOOL clampToEdge;
			BOOL bgra;
		} caps;
	} gl;

	struct {
		BOOL aspect;
		BOOL vSync;
		InterpolationFilter interpolation;
		UpscalingFilter upscaling;
		BYTE scaleNx;
		BYTE xSal;
		BYTE eagle;
		BYTE scaleHQ;
		BYTE xBRz;
	} image;

	struct {
		BYTE fpsCounter;
		BYTE imageFilter;
		BYTE windowedMode;
		BYTE aspectRatio;
		BYTE vSync;
	} keys;

	BOOL isExist;
	CHAR file[MAX_PATH];
};

struct MenuItemData {
	HMENU hParent;
	HMENU hMenu;
	UINT index;
	UINT childId;
};

enum MenuType
{
	MenuAspect,
	MenuVSync,
	MenuInterpolate,
	MenuUpscale,
	MenuCpu,
	MenuSmoothScroll,
	MenuSmoothMove,
	MenuRenderer
};