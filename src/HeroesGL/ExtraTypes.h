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
	struct {
		FLOAT hueShift;
		FLOAT saturation;
	} satHue;
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
	DWORD bmpData[100 * 516];
	LevelColorsFloat colors[256];
	FLOAT delta;
	Adjustment values;
};

struct AppSettings
{
	BOOL showMenu;
	DWORD x;
	DWORD y;
	DWORD width;
	DWORD height;
	BOOL fullScreen;
	BOOL colorMouseCursor;
};

struct AddressSpace
{
	DWORD check;
	BYTE game_version;
	DWORD entry;
	CHAR* icon;
	LCID resLanguage;
	DWORD method2_nop;
	DWORD method2_jmp;
	DWORD setFullScreenStatus;
	DWORD ddSetFullScreenStatus;
	DWORD checkChangeCursor;
	DWORD moveOffset;
	DWORD invalid_jmp;
	DWORD invalid_esp;

	AppSettings* appSettings;
	DWORD dispelMagicSwitch;
	DWORD dispelMagicFix;

	DWORD color_pointer;
	DWORD color_pointer_nop;
	DWORD icons_info;
	DWORD masks_info;
	DWORD colors_info;
	DWORD icons_list;
	DWORD masks_list;
	DWORD colors_list;
	DWORD pointer_fs_nop;
	DWORD delay_til;
	DWORD update_palette;
	DWORD fadein_tick;
	DWORD fadein_update_1;
	DWORD fadein_update_2;
	DWORD fadeout_tick;
	DWORD fadeout_update;
	DWORD nt_check_nop;
	DWORD sample_end_check;
	DWORD windowName;
};

struct TrackInfo
{
	TrackInfo* last;
	DWORD position;
	BOOL isPositional;
	CHAR* group;
	CHAR* path;
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
		HCURSOR default;
		HCURSOR game;
		BOOL fix;
		BOOL hidden;
		DWORD index;
	} cursor;

	struct {
		RECT rect;
		POINT* offset;
	} update;

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