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
#include "Config.h"
#include "Window.h"
#include "Resource.h"

Mod* mods;

namespace Mods
{
	VOID Load()
	{
		CHAR file[MAX_PATH];
		GetModuleFileName(NULL, file, sizeof(file));
		CHAR* dir = StrLastChar(file, '\\') + 1;

		WIN32_FIND_DATA fData;

		{
			CHAR* p = dir;
			StrCopy(p, "mods\\");
			p += StrLength(p);
			StrCopy(p, "*.mod");

			HANDLE hFile = FindFirstFile(file, &fData);
			if (hFile && hFile != INVALID_HANDLE_VALUE)
			{
				Mod* last;
				do
				{
					StrCopy(p, fData.cFileName);
					HMODULE hModule = LoadLibrary(file);
					if (hModule)
					{
						Mod* mod = (Mod*)MemoryAlloc(sizeof(Mod));
						if (mod)
						{
							mod->GetName = (GETNAME)GetProcAddress(hModule, "GetName");
							mod->GetMenu = (GETMENU)GetProcAddress(hModule, "GetMenu");
							mod->SetHWND = (SETHWND)GetProcAddress(hModule, "SetHWND");

							if (mod->GetName && mod->GetMenu && mod->SetHWND)
							{
								mod->hWnd = NULL;
								StrCopy(mod->name, mod->GetName());

								mod->last = NULL;
								if (!mods)
									mods = mod;
								else
									last->last = mod;
								last = mod;

								continue;
							}

							MemoryFree(mod);
						}

						FreeLibrary(hModule);
					}
				} while (FindNextFile(hFile, &fData));

				FindClose(hFile);
			}
		}

		{
			CHAR* p = dir;
			StrCopy(p, "*.asi");

			HANDLE hFile = FindFirstFile(file, &fData);
			if (hFile && hFile != INVALID_HANDLE_VALUE)
			{
				do
				{
					StrCopy(p, fData.cFileName);
					LoadLibrary(file);
				} while (FindNextFile(hFile, &fData));

				FindClose(hFile);
			}
		}
	}

	VOID SetHWND(HWND hWnd)
	{
		for (Mod* mod = mods; mod; mod = mod->last)
		{
			mod->hWnd = hWnd;
			mod->SetHWND(hWnd);
		}
	}

	VOID SetMenu(HMENU hMenu)
	{
		DWORD offset = MENU_OFFSET;

		MenuItemData mData;
		mData.childId = IDM_MODS;
		if (Window::GetMenuByChildID(hMenu, &mData) && DeleteMenu(hMenu, IDM_MODS, MF_BYCOMMAND))
		{
			for (Mod* mod = mods; mod; mod = mod->last)
			{
				mod->added = FALSE;
				if (mod->name)
				{
					HMENU hModMenu = mod->GetMenu(offset);
					if (hModMenu)
					{
						DWORD idx = 0;
						for (Mod* check = mods; check && check != mod; check = check->last)
							if (check->added && !StrCompare(check->name, mod->name))
								++idx;

						CHAR name[256];
						if (idx)
							StrPrint(name, "%s\t#%d", mod->name, idx + 1);
						else
							StrPrint(name, "%s\t", mod->name);

						if (AppendMenu(mData.hMenu, MF_POPUP, (UINT_PTR)hModMenu, name))
						{
							mod->added = TRUE;
							offset += MENU_STEP;
							if (offset == MENU_RESERVED) // game menu space
								offset += MENU_STEP;
						}
					}
				}
			}

			if (offset == MENU_OFFSET)
				DeleteMenu(hMenu, mData.index, MF_BYPOSITION);
		}
	}
}