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

struct Adjustment {
	FLOAT hueShift;
	FLOAT saturation;
	struct {
		Levels left;
		Levels right;
	} input;
	Levels gamma;
	struct {
		Levels left;
		Levels right;
	} output;
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
	LevelColorsFloat* colors;
	FLOAT delta;
	Adjustment values;
};

struct AddressSpace
{
	DWORD check_1;
	DWORD check_2;
	DWORD equal_address;
	DWORD equal_value;
	DWORD fullscr_nop[2];
	DWORD clientrect_nop[2];
	DWORD updateWindow_nop[4];
	DWORD updateWindowReg_nop;
	LCID resLanguage;
	WORD resDialog;
	DWORD windowName;
};

struct RGNRECTDATA
{
	RGNDATAHEADER rdh;
	RECT rect;
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
	DWORD dialog;
	HCURSOR cursor;
	HICON icon;
	HFONT font;
	UINT msgMenu;

	BOOL singleWindow;
	BOOL coldCPU;
	RendererType renderer;
	UpdateMode updateMode;

	struct {
		BOOL allowed;
		BOOL enabled;
		DWORD systemMask;
	} singleCore;

	struct {
		LCID current;
		LCID futured;
	} language;

	struct {
		struct {
			DWORD real;
			DWORD value;
		} version;
		struct {
			BOOL clampToEdge;
		} caps;
	} gl;

	struct {
		FpsState state;
	} fps;

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
	MenuInterpolate,
	MenuUpscale,
	MenuColors,
	MenuCpu,
	MenuCore,
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