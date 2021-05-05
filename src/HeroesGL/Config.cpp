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
#include "Config.h"
#include "intrin.h"

ConfigItems config;

Adjustment activeColors;
const Adjustment inactiveColors = {
	0.5f,
	0.0f,

	0.0f,
	0.15200001f,
	0.0700000003f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f,

	0.5f,
	0.604000032f,
	0.539000034f,
	0.478000015f,

	0.0430000015f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f
};

const Adjustment defaultColors = {
	0.5f,
	0.5f,

	0.0f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f,

	0.5f,
	0.5f,
	0.5f,
	0.5f,

	0.0f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f
};

namespace Config
{
	BYTE LoadKey(const CHAR* name)
	{
		CHAR buffer[20];
		if (Config::Get(CONFIG_KEYS, name, "", buffer, sizeof(buffer)))
		{
			INT value = Config::Get(CONFIG_KEYS, name, 0);
			BYTE res = LOBYTE(value);
			if (res <= 24)
				return res;
		}

		return 0;
	}

	VOID LoadRange(const CHAR* name, Range* range, DWORD chanel)
	{
		INT value = Config::Get(CONFIG_COLORS, name, 0x03E80000);
		if (LOWORD(value) < HIWORD(value))
		{
			range->left.chanel[chanel] = 0.001f * min(1000, max(0, LOWORD(value)));
			range->right.chanel[chanel] = 0.001f * min(1000, max(0, HIWORD(value)));
		}
		else
		{
			range->left.chanel[chanel] = 0.0f;
			range->right.chanel[chanel] = 1.0f;
		}
	}

	FLOAT LoadGamma(const CHAR* name)
	{
		return 0.001f * min(1000, max(0, Config::Get(CONFIG_COLORS, name, 500)));
	}

	VOID Load(HMODULE hModule, const AddressSpace* hookSpace)
	{
		GetModuleFileName(hModule, config.file, MAX_PATH);
		StrCopy(StrLastChar(config.file, '\\') + 1, "config.ini");

		FILE* file = FileOpen(config.file, "rb");
		if (file)
		{
			config.isExist = TRUE;
			FileClose(file);
		}

		config.cursor.default = LoadCursor(NULL, IDC_ARROW);
		config.icon = LoadIcon(hModule, hookSpace->icon);
		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));

		config.msgMenu = RegisterWindowMessage(WM_CHECK_MENU);

		HMODULE hLibrary = LoadLibrary("NTDLL.dll");
		if (hLibrary)
		{
			if (GetProcAddress(hLibrary, "wine_get_version"))
				config.singleWindow = TRUE;
			FreeLibrary(hLibrary);
		}

		if (!config.isDDraw)
		{
			if (!config.isExist)
				Config::Set(CONFIG_WRAPPER, "UseOpenGL", !config.isDDraw);
			else
				config.isDDraw = !(BOOL)Config::Get(CONFIG_WRAPPER, "UseOpenGL", TRUE);
		}
		else
		{
			if (!config.isExist)
				Config::Set(CONFIG_WRAPPER, "UseOpenGL", TRUE);
		}

		if (!config.isExist)
		{
			config.language.current = config.language.futured = hookSpace->resLanguage;
			Config::Set(CONFIG_WRAPPER, "Language", *(INT*)&config.language.current);
			if (SetThreadLanguage)
				SetThreadLanguage(config.language.current);

			LoadString(hDllModule, hookSpace->windowName, config.title, sizeof(config.title));
			Config::Set(CONFIG_WRAPPER, "Title", "");

			config.renderer = RendererAuto;
			Config::Set(CONFIG_WRAPPER, "Renderer", *(INT*)&config.renderer);

			config.updateMode = UpdateSSE;
			Config::Set(CONFIG_WRAPPER, "UpdateMode", *(INT*)&config.updateMode);

			config.coldCPU = TRUE;
			Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);

			config.cursor.fix = TRUE;
			Config::Set(CONFIG_WRAPPER, "PointerFix", config.cursor.fix);

			config.image.aspect = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

			config.image.vSync = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

			Config::Set(CONFIG_WRAPPER, "FpsCounter", *(INT*)&config.fps);

			config.image.interpolation = InterpolateHermite;
			Config::Set(CONFIG_WRAPPER, "Interpolation", *(INT*)&config.image.interpolation);

			config.image.upscaling = UpscaleNone;
			Config::Set(CONFIG_WRAPPER, "Upscaling", *(INT*)&config.image.upscaling);

			config.image.scaleNx = 2;
			Config::Set(CONFIG_WRAPPER, "ScaleNx", config.image.scaleNx);

			config.image.xSal = 2;
			Config::Set(CONFIG_WRAPPER, "XSal", config.image.xSal);

			config.image.eagle = 2;
			Config::Set(CONFIG_WRAPPER, "Eagle", config.image.eagle);

			config.image.scaleHQ = 2;
			Config::Set(CONFIG_WRAPPER, "ScaleHQ", config.image.scaleHQ);

			config.image.xBRz = 2;
			Config::Set(CONFIG_WRAPPER, "XBRZ", config.image.xBRz);

			config.keys.imageFilter = 3;
			Config::Set(CONFIG_KEYS, "ImageFilter", config.keys.imageFilter);

			config.colors.active.satHue.hueShift = 0.5f;
			config.colors.active.satHue.saturation = 0.5f;
			Config::Set(CONFIG_COLORS, "HueSat", 0x01F401F4);

			config.colors.active.input.left.rgb = 0.0f;
			config.colors.active.input.right.rgb = 1.0f;
			Config::Set(CONFIG_COLORS, "RgbInput", 0x03E80000);

			config.colors.active.input.left.red = 0.0f;
			config.colors.active.input.right.red = 1.0f;
			Config::Set(CONFIG_COLORS, "RedInput", 0x03E80000);

			config.colors.active.input.left.green = 0.0f;
			config.colors.active.input.right.green = 1.0f;
			Config::Set(CONFIG_COLORS, "GreenInput", 0x03E80000);

			config.colors.active.input.left.blue = 0.0f;
			config.colors.active.input.right.blue = 1.0f;
			Config::Set(CONFIG_COLORS, "BlueInput", 0x03E80000);

			config.colors.active.gamma.rgb = 0.5f;
			Config::Set(CONFIG_COLORS, "RgbGamma", 500);

			config.colors.active.gamma.red = 0.5f;
			Config::Set(CONFIG_COLORS, "RedGamma", 500);

			config.colors.active.gamma.green = 0.5f;
			Config::Set(CONFIG_COLORS, "GreenGamma", 500);

			config.colors.active.gamma.blue = 0.5f;
			Config::Set(CONFIG_COLORS, "BlueGamma", 500);

			config.colors.active.output.left.rgb = 0.0f;
			config.colors.active.output.right.rgb = 1.0f;
			Config::Set(CONFIG_COLORS, "RgbOutput", 0x03E80000);

			config.colors.active.output.left.red = 0.0f;
			config.colors.active.output.right.red = 1.0f;
			Config::Set(CONFIG_COLORS, "RedOutput", 0x03E80000);

			config.colors.active.output.left.green = 0.0f;
			config.colors.active.output.right.green = 1.0f;
			Config::Set(CONFIG_COLORS, "GreenOutput", 0x03E80000);

			config.colors.active.output.left.blue = 0.0f;
			config.colors.active.output.right.blue = 1.0f;
			Config::Set(CONFIG_COLORS, "BlueOutput", 0x03E80000);

			Config::Set(CONFIG_KEYS, "FpsCounter", "");

			config.keys.windowedMode = 4;
			Config::Set(CONFIG_KEYS, "WindowedMode", config.keys.windowedMode);

			Config::Set(CONFIG_KEYS, "AspectRatio", "");
			Config::Set(CONFIG_KEYS, "VSync", "");
		}
		else
		{
			config.language.current = config.language.futured = (LCID)Config::Get(CONFIG_WRAPPER, "Language", hookSpace->resLanguage);
			if (SetThreadLanguage)
				SetThreadLanguage(config.language.current);

			Config::Get(CONFIG_WRAPPER, "Title", "", config.title, sizeof(config.title));
			if (!StrLength(config.title))
				LoadString(hDllModule, hookSpace->windowName, config.title, sizeof(config.title));
			
			config.coldCPU = (BOOL)Config::Get(CONFIG_WRAPPER, "ColdCPU", TRUE);
			config.cursor.fix = (BOOL)Config::Get(CONFIG_WRAPPER, "PointerFix", TRUE);
		}

		if (!config.isDDraw)
		{
			if (config.isExist)
			{
				INT value = Config::Get(CONFIG_WRAPPER, "Renderer", RendererAuto);
				config.renderer = *(RendererType*)&value;
				if (config.renderer < RendererAuto || config.renderer > RendererOpenGL3)
					config.renderer = RendererAuto;

				value = Config::Get(CONFIG_WRAPPER, "UpdateMode", UpdateSSE);
				config.updateMode = *(UpdateMode*)&value;
				if (config.updateMode < UpdateNone || config.updateMode > UpdateASM)
					config.updateMode = UpdateSSE;

				config.image.aspect = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageAspect", TRUE);
				config.image.vSync = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageVSync", TRUE);

				value = Config::Get(CONFIG_WRAPPER, "FpsCounter", FpsDisabled);
				config.fps = *(FpsState*)&value;
				if (config.fps < FpsDisabled || config.fps > FpsBenchmark)
					config.fps = FpsDisabled;

				value = Config::Get(CONFIG_WRAPPER, "Interpolation", InterpolateHermite);
				config.image.interpolation = *(InterpolationFilter*)&value;
				if (config.image.interpolation < InterpolateNearest || config.image.interpolation > InterpolateLanczos)
					config.image.interpolation = InterpolateHermite;

				value = Config::Get(CONFIG_WRAPPER, "Upscaling", UpscaleNone);
				config.image.upscaling = *(UpscalingFilter*)&value;
				if (config.image.upscaling < UpscaleNone || config.image.upscaling > UpscaleScaleNx)
					config.image.upscaling = UpscaleNone;

				config.image.scaleNx = Config::Get(CONFIG_WRAPPER, "ScaleNx", 2);
				if (config.image.scaleNx != 2 && config.image.scaleNx != 3)
					config.image.scaleNx = 2;

				config.image.xSal = Config::Get(CONFIG_WRAPPER, "XSal", 2);
				if (config.image.xSal != 2)
					config.image.xSal = 2;

				config.image.eagle = Config::Get(CONFIG_WRAPPER, "Eagle", 2);
				if (config.image.eagle != 2)
					config.image.eagle = 2;

				config.image.scaleHQ = Config::Get(CONFIG_WRAPPER, "ScaleHQ", 2);
				if (config.image.scaleHQ != 2 && config.image.scaleHQ != 4)
					config.image.scaleHQ = 2;

				config.image.xBRz = Config::Get(CONFIG_WRAPPER, "XBRZ", 2);
				if (config.image.xBRz < 2 || config.image.xBRz > 6)
					config.image.xBRz = 6;

				value = Config::Get(CONFIG_COLORS, "HueSat", 0x01F401F4);
				config.colors.active.satHue.hueShift = 0.001f * min(1000, max(0, LOWORD(value)));
				config.colors.active.satHue.saturation = 0.001f * min(1000, max(0, HIWORD(value)));

				LoadRange("RgbInput", &config.colors.active.input, 0);
				LoadRange("RedInput", &config.colors.active.input, 1);
				LoadRange("GreenInput", &config.colors.active.input, 2);
				LoadRange("BlueInput", &config.colors.active.input, 3);

				LoadRange("RgbOutput", &config.colors.active.output, 0);
				LoadRange("RedOutput", &config.colors.active.output, 1);
				LoadRange("GreenOutput", &config.colors.active.output, 2);
				LoadRange("BlueOutput", &config.colors.active.output, 3);

				config.colors.active.gamma.rgb = LoadGamma("RgbGamma");
				config.colors.active.gamma.red = LoadGamma("RedGamma");
				config.colors.active.gamma.green = LoadGamma("GreenGamma");
				config.colors.active.gamma.blue = LoadGamma("BlueGamma");

				config.keys.fpsCounter = LoadKey("FpsCounter");
				config.keys.imageFilter = LoadKey("ImageFilter");
				config.keys.windowedMode = LoadKey("WindowedMode");
				config.keys.aspectRatio = LoadKey("AspectRatio");
				config.keys.vSync = LoadKey("VSync");
			}
		}
		else
		{
			config.renderer = RendererAuto;

			config.image.aspect = FALSE;
			config.image.vSync = FALSE;

			config.image.interpolation = InterpolateNearest;
			config.image.upscaling = UpscaleNone;
			config.image.scaleNx = 2;
			config.image.xSal = 2;
			config.image.eagle = 2;
			config.image.scaleHQ = 2;
			config.image.xBRz = 2;

			config.keys.fpsCounter = 0;
			config.keys.imageFilter = 3;
			config.keys.windowedMode = 4;
			config.keys.aspectRatio = 5;
			config.keys.vSync = 0;
		}

		config.colors.current = &config.colors.active;

		INT cpuinfo[4];
		__cpuid(cpuinfo, 1);
		config.isSSE2 = cpuinfo[3] & (1 << 26) || FALSE;
		if (!config.isSSE2 && config.updateMode == UpdateSSE)
			config.updateMode = UpdateCPP;
	}

	INT Get(const CHAR* app, const CHAR* key, INT default)
	{
		return GetPrivateProfileInt(app, key, (INT)default, config.file);
	}

	DWORD Get(const CHAR* app, const CHAR* key, const CHAR* default, CHAR* returnString, DWORD nSize)
	{
		return GetPrivateProfileString(app, key, default, returnString, nSize, config.file);
	}

	BOOL Set(const CHAR* app, const CHAR* key, INT value)
	{
		CHAR res[20];
		StrFromInt(value, res, 10);
		return WritePrivateProfileString(app, key, res, config.file);
	}

	BOOL Set(const CHAR* app, const CHAR* key, CHAR* value)
	{
		return WritePrivateProfileString(app, key, value, config.file);
	}
}