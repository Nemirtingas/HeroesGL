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
#include "Mods.h"

Mod* mods;

namespace Mods
{
	VOID Load()
	{
		CHAR file[MAX_PATH];
		GetModuleFileName(NULL, file, sizeof(file));
		CHAR* p = StrLastChar(file, '\\');
		if (!p)
			return;
		StrCopy(++p, "mods\\");
		p += StrLength(p);
		StrCopy(p, "*.mod");

		WIN32_FIND_DATA fData;
		HANDLE hFile = FindFirstFile(file, &fData);
		if (hFile && hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				StrCopy(p, fData.cFileName);
				HMODULE hMod = LoadLibrary(file);
				if (hMod)
				{
					ISSUPPORTED IsSupported = (ISSUPPORTED)GetProcAddress(hMod, "IsSupported");
					if (IsSupported && IsSupported())
					{
						Mod* mod = (Mod*)MemoryAlloc(sizeof(Mod));
						mod->last = mods;
						mods = mod;

						mod->hModule = hMod;
						mod->IsSupported = IsSupported;
						mod->GetName = (GETNAME)GetProcAddress(hMod, "GetName");
						mod->GetMenu = (GETMENU)GetProcAddress(hMod, "GetMenu");
						mod->SetHWND = (SETHWND)GetProcAddress(hMod, "SetHWND");
						mod->Process = (PROCESS)GetProcAddress(hMod, "Process");

						mod->Process();
					}
					else
						FreeLibrary(hMod);
				}
			} while (FindNextFile(hFile, &fData));

			FindClose(hFile);
		}
	}

	VOID SetHWND(HWND hWnd)
	{
		Mod* mod = mods;
		while (mod)
		{
			mod->SetHWND(hWnd);
			mod = mod->last;
		}
	}
}