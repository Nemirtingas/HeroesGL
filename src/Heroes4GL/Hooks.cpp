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
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"
#include "Hooker.h"

#define STYLE_FULL_OLD (WS_VISIBLE | WS_POPUP)
#define STYLE_FULL_NEW (WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS)

#define STYLE_WIN_OLD (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define STYLE_WIN_NEW (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX)

const AddressSpace addressArray[] = {
	// === RUS ======================================================================================================================================
#pragma region RUS
	0x00844A4D, 0x00844B35, 0x00401D18, 0x90909090, 0x0084497C, 0x006D59BB, 0x00843736, 0x00843F0E,
	0x00000000, 0x00842B01, 0x00842B36, 0x00842BB2,
	LNG_RUSSIAN, IDD_HELP_ABOUT_RUSSIAN_1_0, IDS_HOMM_4, // Heroes IV - 1.0

	0x008C6411, 0x008C64F9, 0x00000000, 0x00000000, 0x008C6340, 0x00736E79, 0x008C4EEE, 0x008C58B2,
	0x008C4259, 0x008C4290, 0x008C429A, 0x008C4323,
	LNG_RUSSIAN, IDD_HELP_ABOUT_RUSSIAN_2_2_GS, IDS_HOMM_4_GS, // Heroes IV - 2.2GS

	0x008D38B1, 0x008D3999, 0x00000000, 0x00000000, 0x008D37E0, 0x0073D780, 0x008D238E, 0x008D2D52,
	0x008D16F9, 0x008D1730, 0x008D173A, 0x008D17C3,
	LNG_RUSSIAN, IDD_HELP_ABOUT_RUSSIAN_3_0_WOW, IDS_HOMM_4_WOW, // Heroes IV - 3.0WoW
#pragma endregion 

	// === ENG ======================================================================================================================================
#pragma region ENG
	0x00844A4D, 0x00844B35, 0x00000000, 0x00000000, 0x0084497C, 0x006D59BB, 0x00843736, 0x00843F0E,
	0x00000000, 0x00842B01, 0x00842B36, 0x00842BB2,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_1_0, IDS_HOMM_4, // Heroes IV - 1.0

	0x0084C0FD, 0x0084C1E5, 0x00000000, 0x00000000, 0x0084C02C, 0x006DAC3B, 0x0084AD96, 0x0084B5BE,
	0x00000000, 0x0084A161, 0x0084A196, 0x0084A212,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_1_2, IDS_HOMM_4, // Heroes IV - 1.2

	0x008533C1, 0x008534A9, 0x00000000, 0x00000000, 0x008532F0, 0x006DF4F5, 0x0085201E, 0x00852862,
	0x00851389, 0x008513C0, 0x008513CA, 0x00851453,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_1_3, IDS_HOMM_4, // Heroes IV - 1.3

	0x008C5F51, 0x008C6039, 0x00000000, 0x00000000, 0x008C5E80, 0x007362D9, 0x008C4A2E, 0x008C53F2,
	0x008C3D99, 0x008C3DD0, 0x008C3DDA, 0x008C3E63,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_2_0, IDS_HOMM_4, // Heroes IV - 2.0

	0x008C63F1, 0x008C64D9, 0x00000000, 0x00000000, 0x008C6320, 0x007369F9, 0x008C4ECE, 0x008C5892,
	0x008C4239, 0x008C4270, 0x008C427A, 0x008C4303,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_2_2, IDS_HOMM_4, // Heroes IV - 2.2

	0x008D3881, 0x008D3969, 0x00000000, 0x00000000, 0x008D37B0, 0x0073D9B0, 0x008D235E, 0x008D2D22,
	0x008D16C9, 0x008D1700, 0x008D170A, 0x008D1793,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_3_0, IDS_HOMM_4, // Heroes IV - 3.0

	// ---------------------------------------------------------------------------------------------------------------------

	0x008C5F51, 0x008C6039, 0x00401448, 0x00961278, 0x008C5E80, 0x007362D9, 0x008C4A2E, 0x008C53F2,
	0x008C3D99, 0x008C3DD0, 0x008C3DDA, 0x008C3E63,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_2_0_GS, IDS_HOMM_4_GS, // Heroes IV - 2.0GS

	0x008C63F1, 0x008C64D9, 0x00401448, 0x00961278, 0x008C6320, 0x007369F9, 0x008C4ECE, 0x008C5892,
	0x008C4239, 0x008C4270, 0x008C427A, 0x008C4303,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_2_2_GS, IDS_HOMM_4_GS, // Heroes IV - 2.2GS

	0x008D3881, 0x008D3969, 0x00401448, 0x00970278, 0x008D37B0, 0x0073D9B0, 0x008D235E, 0x008D2D22,
	0x008D16C9, 0x008D1700, 0x008D170A, 0x008D1793,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_3_0_GS, IDS_HOMM_4_GS, // Heroes IV - 3.0GS

	// ---------------------------------------------------------------------------------------------------------------------

	0x008D3881, 0x008D3969, 0x00401448, 0x00970200, 0x008D37B0, 0x0073D9B0, 0x008D235E, 0x008D2D22,
	0x008D16C9, 0x008D1700, 0x008D170A, 0x008D1793,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_3_0_WOW, IDS_HOMM_4_WOW, // Heroes IV - 3.0WoW
#pragma endregion 

	// === POL ======================================================================================================================================
#pragma region POL
	0x008D8F91, 0x008D9079, 0x00000000, 0x00000000, 0x008D8EC0, 0x0074085C, 0x008D7A5E, 0x008D8422,
	0x008D6DAE, 0x008D6DE6, 0x008D6DF0, 0x008D6E7A,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_3_0_WOW, IDS_HOMM_4_WOW, // Heroes IV - 3.0WoW

	0x008D1651, 0x008D1739, 0x00000000, 0x00000000, 0x008D1580, 0x0073B0A0, 0x008D012E, 0x008D0AF2,
	0x008CF499, 0x008CF4D0, 0x008CF4DA, 0x008CF563,
	LNG_ENGLISH, IDD_HELP_ABOUT_ENGLISH_3_0_WOW, IDS_HOMM_4_WOW // Heroes IV - 3.0WoW
#pragma endregion 
};

namespace Hooks
{
	const AddressSpace* hookSpace;
	HWND hWndMain;

	// ===============================================================
	SIZE adjustSize;

	BOOL __stdcall AdjustWindowRectExHook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
	{
		adjustSize.cx = lpRect->right - lpRect->left;
		adjustSize.cy = lpRect->bottom - lpRect->top;

		switch (dwStyle)
		{
		case STYLE_FULL_OLD:
			dwStyle = STYLE_FULL_NEW;
			break;
		case STYLE_WIN_OLD:
			dwStyle = STYLE_WIN_NEW;
			break;
		default:
			break;
		}

		return AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
	}

	BOOL __stdcall MoveWindowHook(HWND hWnd, INT X, INT Y, INT nWidth, INT nHeight, BOOL bRepaint)
	{
		if (GetWindowLong(hWnd, GWL_STYLE) & WS_MAXIMIZE)
			ShowWindow(hWnd, SW_SHOWNORMAL);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->ResetDisplayMode(adjustSize.cx, adjustSize.cy);

			if (ddraw->windowState != WinStateWindowed)
			{
				RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
				AdjustWindowRect(&rect, GetWindowLong(hWnd, GWL_STYLE), TRUE);
				return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
			}
		}

		return MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
	}

	VOID __fastcall LoadNewMenu(HMENU hMenu)
	{
		if (hMenu)
		{
			HMENU hSub;
			for (DWORD i = 0; hSub = GetSubMenu(hMenu, i); ++i)
			{
				if (GetMenuItemID(hSub, 0) == IDM_FILE_NEW_GAME && DeleteMenu(hMenu, i, MF_BYPOSITION))
				{
					HMENU hNew = LoadMenu(hDllModule, MAKEINTRESOURCE(IDM_MENU));
					if (hNew)
					{
						CHAR buffer[256];

						MENUITEMINFO info;
						MemoryZero(&info, sizeof(MENUITEMINFO));
						info.cbSize = sizeof(MENUITEMINFO);
						info.fMask = MIIM_TYPE;
						info.fType = MFT_STRING;
						info.dwTypeData = buffer;

						info.cch = sizeof(buffer);
						if (config.keys.windowedMode && GetMenuItemInfo(hNew, IDM_RES_FULL_SCREEN, FALSE, &info))
						{
							StrPrint(buffer, "%sF%d", buffer, config.keys.windowedMode);
							SetMenuItemInfo(hNew, IDM_RES_FULL_SCREEN, FALSE, &info);
						}

						info.cch = sizeof(buffer);
						if (config.keys.aspectRatio && GetMenuItemInfo(hNew, IDM_ASPECT_RATIO, FALSE, &info))
						{
							StrPrint(buffer, "%sF%d", buffer, config.keys.aspectRatio);
							SetMenuItemInfo(hNew, IDM_ASPECT_RATIO, FALSE, &info);
						}

						info.cch = sizeof(buffer);
						if (config.keys.vSync && GetMenuItemInfo(hNew, IDM_VSYNC, FALSE, &info))
						{
							StrPrint(buffer, "%sF%d", buffer, config.keys.vSync);
							SetMenuItemInfo(hNew, IDM_VSYNC, FALSE, &info);
						}

						MenuItemData mData;
						if (config.keys.imageFilter)
						{
							mData.childId = IDM_FILT_OFF;
							if (Window::GetMenuByChildID(hNew, &mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
							{
								StrPrint(buffer, "%sF%d", buffer, config.keys.imageFilter);
								SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
							}
						}

						for (DWORD j = 0; hSub = GetSubMenu(hNew, j); ++j)
						{
							GetMenuString(hNew, j, buffer, sizeof(buffer), MF_BYPOSITION);
							InsertMenu(hMenu, i + j, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSub, buffer);
						}

						Window::CheckMenu(hMenu);
					}

					return;
				}
			}
		}
	}

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		if (!config.isDDraw)
			dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;

		hWndMain = CreateWindow(lpClassName, config.title, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		if (hWndMain)
		{
			LoadNewMenu(GetMenu(hWndMain));
			Window::SetCaptureWindow(hWndMain);
			SetTimer(hWndMain, NULL, 10, NULL);
		}

		return hWndMain;
	}

	LONG __stdcall SetWindowLongHook(HWND hWnd, INT nIndex, LONG dwNewLong)
	{
		if (nIndex == GWL_STYLE)
		{
			switch (dwNewLong)
			{
			case STYLE_FULL_OLD:
				dwNewLong = STYLE_FULL_NEW;
				break;
			case STYLE_WIN_OLD:
				dwNewLong = STYLE_WIN_NEW;
				break;
			default:
				break;
			}
		}

		return SetWindowLong(hWnd, nIndex, dwNewLong);
	}

	HWND __stdcall SetActiveWindowHook(HWND hWnd) { return hWnd; }

	INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		INT res;
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = MessageBox(hWnd, lpText, lpCaption, uType);

		if (cookie)
			DeactivateActCtxC(0, cookie);

		return res;
	}

	BOOL __stdcall GetClientRectHook(HWND hWnd, LPRECT lpRect)
	{
		if (GetClientRect(hWnd, lpRect))
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->mode)
			{
				lpRect->right = ddraw->mode->width;
				lpRect->bottom = ddraw->mode->height;
			}

			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall GetCursorPosHook(LPPOINT lpPoint)
	{
		if (GetCursorPos(lpPoint))
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWndMain);
			if (ddraw && ddraw->mode)
			{
				ScreenToClient(hWndMain, lpPoint);
				ddraw->ScaleMouse(lpPoint);
				ClientToScreen(hWndMain, lpPoint);
			}

			return TRUE;
		}

		return FALSE;
	}

	HWND __stdcall WindowFromPointHook(POINT Point)
	{
		return hWndMain;
	}

	BOOL __stdcall GetWindowRectHook(HWND hWnd, LPRECT lpRect)
	{
		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw && ddraw->mode)
		{
			lpRect->left = 0;
			lpRect->top = 0;
			ClientToScreen(hWnd, (LPPOINT)&lpRect->left);
			lpRect->right = lpRect->left + ddraw->mode->width;
			lpRect->bottom = lpRect->top + ddraw->mode->height;
			AdjustWindowRect(lpRect, GetWindowLong(hWnd, GWL_STYLE), TRUE);
			return TRUE;
		}
		else
			return GetWindowRect(hWnd, lpRect);
	}

	HMENU __stdcall LoadMenuHook(HINSTANCE hInstance, LPCTSTR lpMenuName)
	{
		HMENU hMenu = LoadMenu(hInstance, lpMenuName);
		LoadNewMenu(hMenu);
		return hMenu;
	}

	BOOL __stdcall SetMenuHook(HWND hWnd, HMENU hMenu)
	{
		if (SetMenu(hWnd, hMenu))
		{
			Window::CheckMenu(hMenu);
			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall EnableMenuItemHook(HMENU, UINT, UINT)
	{
		return FALSE;
	}

	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		if (PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
			return TRUE;

		Sleep(config.coldCPU);

		return FALSE;
	}

#pragma region Registry
	struct {
		BOOL type;
		HKEY path;
		BOOL calc;
		CHAR sub[256];
		CHAR cls[16];
	} regKey;
	
	LSTATUS __stdcall RegCreateKeyExHook(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
	{
		regKey.type = TRUE;
		regKey.path = hKey;
		StrCopy(regKey.sub, lpSubKey);
		MemoryCopy(regKey.cls, lpClass, sizeof(regKey.cls));

		return ERROR_SUCCESS;
	}

	LSTATUS __stdcall RegOpenKeyExHook(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
	{
		regKey.type = FALSE;
		regKey.path = hKey;
		StrCopy(regKey.sub, lpSubKey);

		return ERROR_SUCCESS;
	}

	LSTATUS __stdcall RegCloseKeyHook(HKEY hKey)
	{
		return ERROR_SUCCESS;
	}

	LSTATUS __stdcall RegQueryValueExHook(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
	{
		DWORD size = *lpcbData;

		if (!Config::Check(CONFIG_APP, lpValueName))
		{
			DWORD dwDisposition;

			LSTATUS res = regKey.type ?
				RegCreateKeyEx(regKey.path, regKey.sub, NULL, regKey.cls, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) :
				RegOpenKeyEx(regKey.path, regKey.sub, NULL, KEY_EXECUTE, &hKey);

			if (!res)
			{
				res = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
				RegCloseKey(hKey);

				if (size)
				{
					if (size == sizeof(DWORD))
						Config::Set(CONFIG_APP, lpValueName, *(INT*)lpData);
					else
						Config::Set(CONFIG_APP, lpValueName, (CHAR*)lpData);
				}
			}

			return res;
		}
		else
		{
			if (!size)
			{
				*lpType = REG_SZ;

				CHAR buff[256];
				Config::Get(CONFIG_APP, lpValueName, "", buff, sizeof(buff));
				*lpcbData = StrLength(buff) + 1;

				regKey.calc = TRUE;
			}
			else
			{
				if (!regKey.calc && size == sizeof(DWORD))
				{
					*(INT*)lpData = Config::Get(CONFIG_APP, lpValueName, *(INT*)lpData);
					if (lpType)
						* lpType = REG_DWORD;
				}
				else
				{
					Config::Get(CONFIG_APP, lpValueName, "", (CHAR*)lpData, *lpcbData);
					if (lpType)
						* lpType = *lpcbData ? REG_BINARY : REG_SZ;
				}

				regKey.calc = FALSE;
			}

			return ERROR_SUCCESS;
		}
	}

	LSTATUS __stdcall RegSetValueExHook(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData)
	{
		if (dwType == REG_DWORD)
			Config::Set(CONFIG_APP, lpValueName, *(INT*)lpData);
		else
			Config::Set(CONFIG_APP, lpValueName, (CHAR*)lpData);

		return ERROR_SUCCESS;
	}
#pragma endregion

	BOOL Load()
	{
		const AddressSpace* defaultSpace = NULL;
		const AddressSpace* equalSpace = NULL;

		Hooker* mainHooker = new Hooker(GetModuleHandle(NULL));

		hookSpace = addressArray;
		DWORD hookCount = sizeof(addressArray) / sizeof(AddressSpace);
		do
		{
			DWORD check1, check2, equal;
			if (mainHooker->ReadDWord(hookSpace->check_1 + 1, &check1) && check1 == STYLE_FULL_OLD &&
				mainHooker->ReadDWord(hookSpace->check_2 + 1, &check2) && check2 == STYLE_FULL_OLD)
			{
				if (!hookSpace->equal_address)
					defaultSpace = hookSpace;
				else if (mainHooker->ReadDWord(hookSpace->equal_address, &equal) && equal == hookSpace->equal_value)
				{
					equalSpace = hookSpace;
					break;
				}
			}

			++hookSpace;
		} while (--hookCount);

		hookSpace = equalSpace ? equalSpace : defaultSpace;
		if (hookSpace)
		{
			Config::Load(mainHooker->hModule, hookSpace);

			{
				mainHooker->PatchImport("CreateWindowExA", CreateWindowExHook);
				mainHooker->PatchImport("MessageBoxA", MessageBoxHook);

				mainHooker->PatchImport("LoadMenuA", LoadMenuHook);
				mainHooker->PatchImport("SetMenu", SetMenuHook);
				mainHooker->PatchImport("EnableMenuItem", EnableMenuItemHook);
				mainHooker->PatchImport("PeekMessageA", PeekMessageHook);

				mainHooker->PatchImport("RegCreateKeyExA", RegCreateKeyExHook);
				mainHooker->PatchImport("RegOpenKeyExA", RegOpenKeyExHook);
				mainHooker->PatchImport("RegCloseKey", RegCloseKeyHook);
				mainHooker->PatchImport("RegQueryValueExA", RegQueryValueExHook);
				mainHooker->PatchImport("RegSetValueExA", RegSetValueExHook);

				mainHooker->PatchImport("DirectDrawCreateEx", Main::DirectDrawCreateEx);

				if (!config.isDDraw)
				{
					mainHooker->PatchImport("SetWindowLongA", SetWindowLongHook);
					mainHooker->PatchImport("AdjustWindowRectEx", AdjustWindowRectExHook);
					mainHooker->PatchImport("MoveWindow", MoveWindowHook);

					mainHooker->PatchImport("GetWindowRect", GetWindowRectHook);
					mainHooker->PatchImport("GetClientRect", GetClientRectHook);
					mainHooker->PatchImport("GetCursorPos", GetCursorPosHook);

					mainHooker->PatchImport("SetActiveWindow", SetActiveWindowHook);
				}

				mainHooker->UnmapFile();
			}

			// windowed limitations
			if (!config.isDDraw)
			{
				mainHooker->PatchNop(hookSpace->fullscr_nop[0], 20);
				mainHooker->PatchNop(hookSpace->fullscr_nop[1], 4);

				const DWORD* lpNop = hookSpace->clientrect_nop;
				DWORD count = sizeof(hookSpace->clientrect_nop) / sizeof(DWORD);
				do
					mainHooker->PatchNop(*lpNop++, 2);
				while (--count);

				count = sizeof(hookSpace->updateWindow_nop) / sizeof(DWORD);
				lpNop = hookSpace->updateWindow_nop;
				if (*lpNop)
				{
					do
						mainHooker->PatchNop(*lpNop++, 5);
					while (--count);
				}
				else
				{
					BYTE addEsp[5] = { 0x83, 0xC4, 0x08, 0x90, 0x90, };

					--count;
					do
						mainHooker->PatchBlock(*lpNop++, addEsp, sizeof(addEsp));
					while (--count);
				}
			}

			return TRUE;
		}

		return FALSE;
	}
}