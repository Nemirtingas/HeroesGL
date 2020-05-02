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
	CHAR* icon;
	BYTE resLanguage;
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
	const CHAR* windowName;
};

struct TrackInfo
{
	TrackInfo* last;
	DWORD position;
	BOOL isPositional;
	CHAR* group;
	CHAR* path;
};

struct ConfigItems
{
	BOOL isDDraw;
	DWORD language;
	HICON icon;
	HFONT font;
	UINT msgMenu;

	BOOL singleWindow;
	BOOL coldCPU;
	RendererType renderer;

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
		BYTE imageFilter;
		BYTE windowedMode;
		BYTE aspectRatio;
		BYTE vSync;
	} keys;

	BOOL isExist;
	CHAR file[MAX_PATH];
};

struct Pointer {
	BOOL isActive;
	POINT pos;
	SIZE size;
	POINT offset;
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
	MenuRenderer
};