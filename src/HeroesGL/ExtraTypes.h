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

enum ImageFilter
{
	FilterNearest = 0,
	FilterLinear = 1,
	FilterCubic = 2,
	FilterXRBZ = 3,
	FilterScaleHQ = 4,
	FilterXSal = 5,
	FilterEagle = 6,
	FilterScaleNx = 7
};

struct FilterType
{
	WORD value;
	WORD type;
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
	DWORD method2_nop_size;
	DWORD method2_jmp;
	BYTE method2_jmp_short;
	DWORD mode_nop;
	BYTE mode_nop_size;
	DWORD setFullScreenStatus;
	DWORD ddSetFullScreenStatus;
	DWORD checkChangeCursor;

	AppSettings* appSettings;

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
	BOOL isNoGL;
	DWORD language;
	HICON icon;
	HFONT font;

	BOOL singleThread;
	BOOL singleWindow;
	BOOL coldCPU;
	BOOL pointerFix;
	BOOL pointerHidden;
	DWORD pointerIndex;

	struct {
		BOOL aspect;
		BOOL vSync;
		ImageFilter filter;
		FilterType scaleNx;
		FilterType xSal;
		FilterType eagle;
		FilterType scaleHQ;
		FilterType xBRz;
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

struct MappedFile
{
	HMODULE hModule;
	HANDLE hFile;
	HANDLE hMap;
	VOID* address;
};

struct Pointer {
	BOOL isActive;
	POINT pos;
	SIZE size;
	POINT offset;
};