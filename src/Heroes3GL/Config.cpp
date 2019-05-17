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
#include "Config.h"

ConfigItems config;

namespace Config
{
	VOID __fastcall Load(HMODULE hModule, const AddressSpace* hookSpace)
	{
		GetModuleFileName(hModule, config.file, MAX_PATH - 1);
		CHAR* p = StrLastChar(config.file, '\\');
		*p = NULL;
		StrCopy(p, "\\config.ini");

		FILE* file = FileOpen(config.file, "rb");
		if (file)
		{
			config.isExist = TRUE;
			FileClose(file);
		}

		config.language = hookSpace->resLanguage;
		config.icon = LoadIcon(hModule, MAKEINTRESOURCE(RESOURCE_ICON));
		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));

		config.singleThread = TRUE;
		DWORD processMask, systemMask;
		if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask))
		{
			BOOL isSingle = FALSE;
			DWORD count = sizeof(DWORD) << 3;
			do
			{
				if (processMask & 1)
				{
					if (isSingle)
					{
						config.singleThread = FALSE;
						break;
					}
					else
						isSingle = TRUE;
				}

				processMask >>= 1;
			} while (--count);
		}

		HMODULE hLibrary = LoadLibrary("NTDLL.dll");
		if (hLibrary)
		{
			if (GetProcAddress(hLibrary, "wine_get_version"))
				config.singleWindow = TRUE;
			FreeLibrary(hLibrary);
		}

		if (!config.singleWindow)
			config.singleWindow = Config::Get(CONFIG_WRAPPER, "SingleWindow", FALSE);

		if (!config.isNoGL)
		{
			if (!config.isExist)
				Config::Set(CONFIG_WRAPPER, "UseOpenGL", !config.isNoGL);
			else
				config.isNoGL = !(BOOL)Config::Get(CONFIG_WRAPPER, "UseOpenGL", TRUE);
		}
		else
		{
			if (!config.isExist)
				Config::Set(CONFIG_WRAPPER, "UseOpenGL", TRUE);
		}

		if (!config.isExist)
		{
			config.coldCPU = FALSE;
			Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);

			config.image.aspect = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

			config.image.vSync = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

			config.image.filter = FilterLinear;
			Config::Set(CONFIG_WRAPPER, "ImageFilter", *(INT*)&config.image.filter);

			config.image.scaleNx.value = 2;
			config.image.scaleNx.type = 0;
			Config::Set(CONFIG_WRAPPER, "ImageScaleNx", *(INT*)&config.image.scaleNx);

			config.image.xSal.value = 2;
			config.image.xSal.type = 0;
			Config::Set(CONFIG_WRAPPER, "ImageXSal", *(INT*)&config.image.xSal);

			config.image.eagle.value = 2;
			config.image.eagle.type = 0;
			Config::Set(CONFIG_WRAPPER, "ImageEagle", *(INT*)&config.image.eagle);

			config.image.scaleHQ.value = 2;
			config.image.scaleHQ.type = 0;
			Config::Set(CONFIG_WRAPPER, "ImageScaleHQ", *(INT*)&config.image.scaleHQ);

			config.image.xBRz.value = 2;
			config.image.xBRz.type = 0;
			Config::Set(CONFIG_WRAPPER, "ImageXBRZ", *(INT*)&config.image.xBRz);

			config.keys.fpsCounter = 2;
			Config::Set(CONFIG_KEYS, "FpsCounter", config.keys.fpsCounter);

			config.keys.imageFilter = 3;
			Config::Set(CONFIG_KEYS, "ImageFilter", config.keys.imageFilter);

			config.keys.windowedMode = 4;
			Config::Set(CONFIG_KEYS, "WindowedMode", config.keys.windowedMode);

			config.keys.aspectRatio = 5;
			Config::Set(CONFIG_KEYS, "AspectRatio", config.keys.aspectRatio);

			config.keys.vSync = 0;
			Config::Set(CONFIG_KEYS, "VSync", "");
		}
		else
			config.coldCPU = (BOOL)Config::Get(CONFIG_WRAPPER, "ColdCPU", FALSE);

		if (!config.isNoGL)
		{
			if (config.isExist)
			{
				config.image.aspect = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageAspect", TRUE);
				config.image.vSync = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageVSync", TRUE);

				INT value = Config::Get(CONFIG_WRAPPER, "ImageFilter", FilterLinear);
				config.image.filter = *(ImageFilter*)&value;
				if (config.image.filter < FilterNearest || config.image.filter > FilterScaleNx)
					config.image.filter = FilterLinear;

				value = Config::Get(CONFIG_WRAPPER, "ImageScaleNx", 2);
				config.image.scaleNx = *(FilterType*)&value;
				if (config.image.scaleNx.value != 2 && config.image.scaleNx.value != 3)
					config.image.scaleNx.value = 2;
				if (config.image.scaleNx.type & 0xFE)
					config.image.scaleNx.type = 0;

				value = Config::Get(CONFIG_WRAPPER, "ImageXSal", 2);
				config.image.xSal = *(FilterType*)&value;
				if (config.image.xSal.value != 2)
					config.image.xSal.value = 2;
				if (config.image.xSal.type & 0xFE)
					config.image.xSal.type = 0;
				
				value = Config::Get(CONFIG_WRAPPER, "ImageEagle", 2);
				config.image.eagle = *(FilterType*)&value;
				if (config.image.eagle.value != 2)
					config.image.eagle.value = 2;
				if (config.image.eagle.type & 0xFE)
					config.image.eagle.type = 0;
				
				value = Config::Get(CONFIG_WRAPPER, "ImageScaleHQ", 2);
				config.image.scaleHQ = *(FilterType*)&value;
				if (config.image.scaleHQ.value != 2 && config.image.scaleHQ.value != 4)
					config.image.scaleHQ.value = 2;
				if (config.image.scaleHQ.type & 0xFE)
					config.image.scaleHQ.type = 0;
				
				value = Config::Get(CONFIG_WRAPPER, "ImageXBRZ", 2);
				config.image.xBRz = *(FilterType*)&value;
				if (config.image.xBRz.value < 2 || config.image.xBRz.value > 6)
					config.image.xBRz.value = 6;
				if (config.image.xBRz.type & 0xFE)
					config.image.xBRz.type = 0;


				CHAR buffer[20];
				if (Config::Get(CONFIG_KEYS, "FpsCounter", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "FpsCounter", 0);
					config.keys.fpsCounter = LOBYTE(value);
					if (config.keys.fpsCounter > 24)
						config.keys.fpsCounter = 0;
				}

				if (Config::Get(CONFIG_KEYS, "ImageFilter", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "ImageFilter", 0);
					config.keys.imageFilter = LOBYTE(value);
					if (config.keys.imageFilter > 24)
						config.keys.imageFilter = 0;
				}

				if (Config::Get(CONFIG_KEYS, "WindowedMode", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "WindowedMode", 0);
					config.keys.windowedMode = LOBYTE(value);
					if (config.keys.windowedMode > 24)
						config.keys.windowedMode = 0;
				}

				if (Config::Get(CONFIG_KEYS, "AspectRatio", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "AspectRatio", 0);
					config.keys.aspectRatio = LOBYTE(value);
					if (config.keys.aspectRatio > 24)
						config.keys.aspectRatio = 0;
				}

				if (Config::Get(CONFIG_KEYS, "VSync", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "VSync", 0);
					config.keys.vSync = LOBYTE(value);
					if (config.keys.vSync > 24)
						config.keys.vSync = 0;
				}
			}
		}
		else
		{
			config.image.aspect = FALSE;
			config.image.vSync = FALSE;
			config.image.filter = FilterLinear;

			config.image.scaleNx.value = 2;
			config.image.scaleNx.type = 0;

			config.image.xSal.value = 2;
			config.image.xSal.type = 0;

			config.image.eagle.value = 2;
			config.image.eagle.type = 0;

			config.image.scaleHQ.value = 2;
			config.image.scaleHQ.type = 0;

			config.image.xBRz.value = 2;
			config.image.xBRz.type = 0;

			config.keys.fpsCounter = 2;
			config.keys.imageFilter = 3;
			config.keys.windowedMode = 4;
			config.keys.aspectRatio = 5;
			config.keys.vSync = 0;
		}
	}

	INT __fastcall Get(const CHAR* app, const CHAR* key, INT default)
	{
		return GetPrivateProfileInt(app, key, (INT)default, config.file);
	}

	DWORD __fastcall Get(const CHAR* app, const CHAR* key, CHAR* default, CHAR* returnString, DWORD nSize)
	{
		return GetPrivateProfileString(app, key, default, returnString, nSize, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, INT value)
	{
		CHAR res[20];
		StrPrint(res, "%d", value);
		return WritePrivateProfileString(app, key, res, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, CHAR* value)
	{
		return WritePrivateProfileString(app, key, value, config.file);
	}
}