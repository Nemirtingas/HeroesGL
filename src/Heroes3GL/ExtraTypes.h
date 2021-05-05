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
	Rect align;
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
	InterpolateCubic = 3,
	InterpolateLanczos = 4
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

union Levels
{
	struct {
		FLOAT rgb;
		FLOAT red;
		FLOAT green;
		FLOAT blue;
	};
	FLOAT chanel[4];
};

struct Range {
	Levels left;
	Levels right;
};

struct Adjustment {
	struct {
		FLOAT hueShift;
		FLOAT saturation;
	} satHue;
	Range input;
	Levels gamma;
	Range output;
};

union LevelColors
{
	struct {
		DWORD red;
		DWORD green;
		DWORD blue;
	};
	DWORD chanel[3];
};

union LevelColorsFloat
{
	struct {
		FLOAT red;
		FLOAT green;
		FLOAT blue;
	};
	FLOAT chanel[3];
};

struct LevelsData {
	HDC hDc;
	HBITMAP hBmp;
	DWORD* data;
	DWORD bmpData[100 * 516];
	LevelColorsFloat colors[256];
	FLOAT delta;
	Adjustment values;
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
	DWORD start_bonus_fix;
	DWORD video_address;
	DWORD video_count;
	LCID resLanguage;
	DWORD atof;
	DWORD move_oldCenter;
	DWORD move_drawRect;
	DWORD move_lifeCycle;
	DWORD windowName;
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
	BYTE bink;
	BYTE flags_0[2];
	BYTE preload;
	BYTE flags_1[8];
};

struct VideoFile
{
	CHAR name[40];
	DWORD stride;
};

enum FpsState
{
	FpsDisabled = 0,
	FpsNormal,
	FpsBenchmark
};

struct FpsItem {
	DWORD tick;
	DWORD span;
};

enum FpsMode
{
	FpsRgb,
	FpsRgba,
	FpsBgra
};

enum UpdateMode
{
	UpdateNone = 0,
	UpdateSSE = 1,
	UpdateCPP = 2,
	UpdateASM = 3
};

struct ConfigItems
{
	BOOL isDDraw;
	CHAR title[MAX_PATH];
	HCURSOR cursor;
	HICON icon;
	HFONT font;
	UINT msgMenu;

	BOOL singleWindow;
	BOOL coldCPU;
	BOOL isSSE2;
	RendererType renderer;
	UpdateMode updateMode;

	struct {
		LCID current;
		LCID futured;
	} language;

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

	FpsState fps;

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

	struct {
		const Adjustment* current;
		Adjustment active;
	} colors;

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
	MenuFps,
	MenuInterpolate,
	MenuUpscale,
	MenuColors,
	MenuCpu,
	MenuSmoothScroll,
	MenuSmoothMove,
	MenuRenderer,
	MenuLanguage
};

class OpenDraw;
struct DialogParams {
	HWND hWnd;
	BOOL isGrayed;
	OpenDraw* ddraw;
	BOOL isFullscreen;
	ULONG_PTR cookie;
};