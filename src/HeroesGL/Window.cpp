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
#include "timeapi.h"
#include "Window.h"
#include "CommCtrl.h"
#include "Windowsx.h"
#include "Shellapi.h"
#include "Resource.h"
#include "Main.h"
#include "Config.h"
#include "Hooks.h"
#include "OpenDraw.h"

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	BOOL __fastcall GetMenuByChildID(HMENU hParent, MenuItemData* mData, INT index)
	{
		HMENU hMenu = GetSubMenu(hParent, index);

		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			UINT id = GetMenuItemID(hMenu, i);
			if (id == mData->childId)
			{
				mData->hParent = hParent;
				mData->hMenu = hMenu;
				mData->index = index;

				return TRUE;
			}
			else if (GetMenuByChildID(hMenu, mData, i))
				return TRUE;
		}

		return FALSE;
	}

	BOOL __fastcall GetMenuByChildID(HMENU hMenu, MenuItemData* mData)
	{
		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
			if (GetMenuByChildID(hMenu, mData, i))
				return TRUE;

		MemoryZero(mData, sizeof(MenuItemData));
		return FALSE;
	}

	VOID __fastcall CheckEnablePopup(HMENU hMenu, MenuItemData* mData, DWORD flags)
	{
		if (GetMenuByChildID(hMenu, mData))
		{
			EnableMenuItem(mData->hParent, mData->index, MF_BYPOSITION | flags);
			CheckMenuItem(mData->hParent, mData->index, MF_BYPOSITION | MF_UNCHECKED);

			UINT count = (UINT)GetMenuItemCount(mData->hMenu);
			for (UINT i = 0; i < count; ++i)
			{
				EnableMenuItem(mData->hMenu, i, MF_BYPOSITION | flags);
				CheckMenuItem(mData->hMenu, i, MF_BYPOSITION | MF_UNCHECKED);
			}
		}
	}

	VOID __fastcall CheckMenu(HMENU hMenu, MenuType type)
	{
		if (!hMenu)
			return;

		switch (type)
		{
		case MenuAspect: {
			EnableMenuItem(hMenu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (config.gl.version.value ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(hMenu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (config.gl.version.value && config.image.aspect ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuVSync: {
			EnableMenuItem(hMenu, IDM_VSYNC, MF_BYCOMMAND | (config.gl.version.value && WGLSwapInterval ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(hMenu, IDM_VSYNC, MF_BYCOMMAND | (config.gl.version.value && WGLSwapInterval && config.image.vSync ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuInterpolate: {
			CheckMenuItem(hMenu, IDM_FILT_OFF, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | (config.gl.version.value ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

			EnableMenuItem(hMenu, IDM_FILT_HERMITE, MF_BYCOMMAND | isFilters);
			CheckMenuItem(hMenu, IDM_FILT_HERMITE, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | isFilters);
			CheckMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.image.interpolation)
			{
			case InterpolateLinear:
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateHermite:
				CheckMenuItem(hMenu, isFilters == MF_ENABLED ? IDM_FILT_HERMITE : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateCubic:
				CheckMenuItem(hMenu, isFilters == MF_ENABLED ? IDM_FILT_CUBIC : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(hMenu, IDM_FILT_OFF, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}
		break;

		case MenuUpscale: {
			CheckMenuItem(hMenu, IDM_FILT_NONE, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = config.gl.version.value >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

			MenuItemData mScaleNx, mEagle, mXSal, mScaleHQ, mXBRZ;

			// ScaleNx
			mScaleNx.childId = IDM_FILT_SCALENX_2X;
			CheckEnablePopup(hMenu, &mScaleNx, isFilters);

			// Eagle
			mEagle.childId = IDM_FILT_EAGLE_2X;
			CheckEnablePopup(hMenu, &mEagle, isFilters);

			// XSal
			mXSal.childId = IDM_FILT_XSAL_2X;
			CheckEnablePopup(hMenu, &mXSal, isFilters);

			// ScaleHQ
			mScaleHQ.childId = IDM_FILT_SCALEHQ_2X;
			CheckEnablePopup(hMenu, &mScaleHQ, isFilters);

			// xBRz
			mXBRZ.childId = IDM_FILT_XRBZ_2X;
			CheckEnablePopup(hMenu, &mXBRZ, isFilters);

			if (config.image.upscaling != UpscaleNone && isFilters == MF_ENABLED)
			{
				switch (config.image.upscaling)
				{
				case UpscaleScaleNx:
					if (mScaleNx.hParent)
						CheckMenuItem(mScaleNx.hParent, mScaleNx.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.scaleNx)
					{
					case 3:
						CheckMenuItem(hMenu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(hMenu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				case UpscaleEagle:
					if (mEagle.hParent)
						CheckMenuItem(mEagle.hParent, mEagle.index, MF_BYPOSITION | MF_CHECKED);

					CheckMenuItem(hMenu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | MF_CHECKED);

					break;

				case UpscaleXSal:
					if (mXSal.hParent)
						CheckMenuItem(mXSal.hParent, mXSal.index, MF_BYPOSITION | MF_CHECKED);

					CheckMenuItem(hMenu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | MF_CHECKED);

					break;

				case UpscaleScaleHQ:
					if (mScaleHQ.hParent)
						CheckMenuItem(mScaleHQ.hParent, mScaleHQ.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.scaleHQ)
					{
					case 4:
						CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				case UpscaleXRBZ:
					if (mXBRZ.hParent)
						CheckMenuItem(mXBRZ.hParent, mXBRZ.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.xBRz)
					{
					case 3:
						CheckMenuItem(hMenu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 4:
						CheckMenuItem(hMenu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 5:
						CheckMenuItem(hMenu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 6:
						CheckMenuItem(hMenu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(hMenu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				default:
					break;
				}
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_NONE, MF_BYCOMMAND | MF_CHECKED);
		}
		break;

		case MenuCpu: {
			CheckMenuItem(hMenu, IDM_PATCH_CPU, MF_BYCOMMAND | (config.coldCPU ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuLanguage: {
			MenuItemData mData;
			mData.childId = IDM_LANG_ENGLISH;
			if (GetMenuByChildID(hMenu, &mData))
			{
				UINT count = (UINT)GetMenuItemCount(mData.hMenu);
				for (UINT i = 0; i < count; ++i)
					CheckMenuItem(mData.hMenu, i, MF_BYPOSITION | (GetMenuItemID(mData.hMenu, i) == config.language.futured ? MF_CHECKED : MF_UNCHECKED));
			}
		}
		break;

		case MenuRenderer: {
			EnableMenuItem(hMenu, IDM_REND_GL1, MF_BYCOMMAND | (config.gl.version.real >= GL_VER_1_1 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(hMenu, IDM_REND_GL2, MF_BYCOMMAND | (config.gl.version.real >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(hMenu, IDM_REND_GL3, MF_BYCOMMAND | (config.gl.version.real >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			CheckMenuItem(hMenu, IDM_REND_AUTO, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_REND_GL1, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_REND_GL2, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_REND_GL3, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.renderer)
			{
			case RendererOpenGL1:
				CheckMenuItem(hMenu, IDM_REND_GL1, MF_BYCOMMAND | MF_CHECKED);
				break;

			case RendererOpenGL2:
				CheckMenuItem(hMenu, IDM_REND_GL2, MF_BYCOMMAND | MF_CHECKED);
				break;

			case RendererOpenGL3:
				CheckMenuItem(hMenu, IDM_REND_GL3, MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(hMenu, IDM_REND_AUTO, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}

		default:
			break;
		}
	}

	VOID __fastcall CheckMenu(HWND hWnd, MenuType type)
	{
		CheckMenu(GetMenu(hWnd), type);
	}

	VOID __fastcall CheckMenu(HMENU hMenu)
	{
		CheckMenu(hMenu, MenuAspect);
		CheckMenu(hMenu, MenuVSync);
		CheckMenu(hMenu, MenuInterpolate);
		CheckMenu(hMenu, MenuUpscale);
		CheckMenu(hMenu, MenuCpu);
		CheckMenu(hMenu, MenuLanguage);
		CheckMenu(hMenu, MenuRenderer);
	}

	VOID __fastcall CheckMenu(HWND hWnd)
	{
		CheckMenu(GetMenu(hWnd));
	}

	VOID __fastcall FilterChanged(HWND hWnd, const CHAR* name, INT value)
	{
		Config::Set(CONFIG_WRAPPER, name, value);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->LoadFilterState();
			SetEvent(ddraw->hDrawEvent);
		}
	}

	VOID __fastcall InterpolationChanged(HWND hWnd, InterpolationFilter filter)
	{
		config.image.interpolation = config.gl.version.value >= GL_VER_2_0 || filter < InterpolateHermite ? filter : InterpolateLinear;

		FilterChanged(hWnd, "Interpolation", *(INT*)&config.image.interpolation);
		CheckMenu(hWnd, MenuInterpolate);
	}

	VOID __fastcall UpscalingChanged(HWND hWnd, UpscalingFilter filter)
	{
		config.image.upscaling = config.gl.version.value >= GL_VER_3_0 ? filter : UpscaleNone;

		FilterChanged(hWnd, "Upscaling", *(INT*)&config.image.upscaling);
		CheckMenu(hWnd, MenuUpscale);
	}

	VOID __fastcall SelectScaleNxMode(HWND hWnd, BYTE value)
	{
		config.image.scaleNx = value;
		Config::Set(CONFIG_WRAPPER, "ScaleNx", *(INT*)&config.image.scaleNx);
		UpscalingChanged(hWnd, UpscaleScaleNx);
	}

	VOID __fastcall SelectXSalMode(HWND hWnd, BYTE value)
	{
		config.image.xSal = value;
		Config::Set(CONFIG_WRAPPER, "XSal", *(INT*)&config.image.xSal);
		UpscalingChanged(hWnd, UpscaleXSal);
	}

	VOID __fastcall SelectEagleMode(HWND hWnd, BYTE value)
	{
		config.image.eagle = value;
		Config::Set(CONFIG_WRAPPER, "Eagle", *(INT*)&config.image.eagle);
		UpscalingChanged(hWnd, UpscaleEagle);
	}

	VOID __fastcall SelectScaleHQMode(HWND hWnd, BYTE value)
	{
		config.image.scaleHQ = value;
		Config::Set(CONFIG_WRAPPER, "ScaleHQ", *(INT*)&config.image.scaleHQ);
		UpscalingChanged(hWnd, UpscaleScaleHQ);
	}

	VOID __fastcall SelectXBRZMode(HWND hWnd, BYTE value)
	{
		config.image.xBRz = value;
		Config::Set(CONFIG_WRAPPER, "XBRZ", *(INT*)&config.image.xBRz);
		UpscalingChanged(hWnd, UpscaleXRBZ);
	}

	VOID __fastcall SelectRenderer(HWND hWnd, RendererType renderer)
	{
		config.renderer = renderer;
		Config::Set(CONFIG_WRAPPER, "Renderer", *(INT*)&config.renderer);

		CheckMenu(hWnd, MenuRenderer);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->RenderStop();
			ddraw->RenderStart();
		}
	}

	LRESULT __stdcall KeysHook(INT nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
		{
			KBDLLHOOKSTRUCT* phs = (KBDLLHOOKSTRUCT*)lParam;
			if (phs->vkCode == VK_SNAPSHOT)
			{
				HWND hWnd = GetActiveWindow();
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && ddraw->windowState != WinStateWindowed)
				{
					ddraw->isTakeSnapshot = TRUE;
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);
					MessageBeep(0);
					return TRUE;
				}
			}
		}

		return CallNextHookEx(OldKeysHook, nCode, wParam, lParam);
	}

	LRESULT __stdcall AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG: {
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, Hooks::EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(hDllModule, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)MemoryAlloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;

							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							StrPrint(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwFileVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}
					}
				}
				MemoryFree(verData);
			}

			if (GetDlgItemText(hDlg, IDC_COPYRIGHT, temp, sizeof(temp)))
			{
				StrPrint(path, temp, 2020, "Verok");
				SetDlgItemText(hDlg, IDC_COPYRIGHT, path);
			}

			if (lParam)
			{
				StrPrint(path, "<A HREF=\"mailto:%s\">%s</A>", "verokster@gmail.com", "verokster@gmail.com");
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);

				StrPrint(path, "<A HREF=\"%s\">%s</A>", "https://verokster.blogspot.com/2018/10/heroes-of-might-and-magic-i-iv-gl.html", "https://verokster.blogspot.com");
				SetDlgItemText(hDlg, IDC_LNK_WEB, path);

				StrPrint(path, "<A HREF=\"%s\">%s</A>", "https://www.patreon.com/join/verok", "https://www.patreon.com/join/verok");
				SetDlgItemText(hDlg, IDC_LNK_PATRON, path);
			}
			else
			{
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, "verokster@gmail.com");
				SetDlgItemText(hDlg, IDC_LNK_WEB, "https://verokster.blogspot.com");
				SetDlgItemText(hDlg, IDC_LNK_PATRON, "https://www.patreon.com/join/verok");
			}

			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CLICK)
			{
				switch (wParam)
				{
				case IDC_LNK_EMAIL:
				case IDC_LNK_WEB:
				case IDC_LNK_PATRON:
					SHELLEXECUTEINFOW shExecInfo;
					MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFOW));
					shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
					shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
					shExecInfo.nShow = SW_SHOW;
					ShellExecuteExW(&shExecInfo);
					break;

				default:
					break;
				}
			}

			break;
		}

		case WM_COMMAND: {
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ERASEBKGND: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->windowState != WinStateWindowed)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				return TRUE;
			}
			return NULL;
		}

		case WM_MOVE: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				SetEvent(ddraw->hDrawEvent);

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE: {
			Hooks::ScalePointer((FLOAT)LOWORD(lParam) / (FLOAT)RES_WIDTH, (FLOAT)HIWORD(lParam) / (FLOAT)RES_HEIGHT);

			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw && ddraw->hDraw != hWnd)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_GETMINMAXINFO: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->windowState == WinStateWindowed)
			{
				RECT rect = { 0, 0, MIN_WIDTH, MIN_HEIGHT };
				AdjustWindowRect(&rect, GetWindowLong(hWnd, GWL_STYLE), TRUE);

				MINMAXINFO* mmi = (MINMAXINFO*)lParam;
				mmi->ptMinTrackSize.x = rect.right - rect.left;
				mmi->ptMinTrackSize.y = rect.bottom - rect.top;
				;

				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ACTIVATEAPP: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->windowState != WinStateWindowed)
			{
				if ((BOOL)wParam)
					ddraw->RenderStart();
				else
					ddraw->RenderStop();
			}

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			if (!(HIWORD(lParam) & KF_ALTDOWN))
			{
				if (config.keys.imageFilter && config.keys.imageFilter + VK_F1 - 1 == wParam)
				{
					switch (config.image.interpolation)
					{
					case InterpolateNearest:
						InterpolationChanged(hWnd, InterpolateLinear);
						break;

					case InterpolateLinear:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateHermite : InterpolateNearest);
						break;

					case InterpolateHermite:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateCubic : InterpolateNearest);
						break;

					default:
						InterpolationChanged(hWnd, InterpolateNearest);
						break;
					}

					return NULL;
				}
				else if (config.keys.aspectRatio && config.keys.aspectRatio + VK_F1 - 1 == wParam)
				{
					WindowProc(hWnd, WM_COMMAND, IDM_ASPECT_RATIO, NULL);
					return NULL;
				}
				else if (config.keys.vSync && config.keys.vSync + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_VSYNC, NULL);
					return NULL;
				}
				else if (config.keys.windowedMode && config.keys.windowedMode + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);
					return NULL;
				}
				else if (wParam == VK_F4)
					return NULL;
			}

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:

		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:

		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (config.image.aspect)
				{
					POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					ddraw->ScaleMouse(&p);
					lParam = MAKELONG(p.x, p.y);
				}

				SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDM_PATCH_CPU: {
				config.coldCPU = !config.coldCPU;

				Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);

				CheckMenu(hWnd, MenuCpu);
				return NULL;
			}

			case IDM_HELP_WRAPPER: {
				ULONG_PTR cookie = NULL;
				if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
					cookie = NULL;

				DialogBoxParam(hDllModule, MAKEINTRESOURCE(cookie ? IDD_ABOUT : IDD_ABOUT_OLD), hWnd, (DLGPROC)AboutProc, cookie);

				if (cookie)
					DeactivateActCtxC(0, cookie);

				SetForegroundWindow(hWnd);
				return NULL;
			}

			case IDM_ASPECT_RATIO: {
				config.image.aspect = !config.image.aspect;
				Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					ddraw->viewport.refresh = TRUE;
					SetEvent(ddraw->hDrawEvent);
				}

				CheckMenu(hWnd, MenuAspect);

				return NULL;
			}

			case IDM_VSYNC: {
				config.image.vSync = !config.image.vSync;
				Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					SetEvent(ddraw->hDrawEvent);

				CheckMenu(hWnd, MenuVSync);

				return NULL;
			}

			case IDM_FILT_OFF: {
				InterpolationChanged(hWnd, InterpolateNearest);
				return NULL;
			}

			case IDM_FILT_LINEAR: {
				InterpolationChanged(hWnd, InterpolateLinear);
				return NULL;
			}

			case IDM_FILT_HERMITE: {
				InterpolationChanged(hWnd, InterpolateHermite);
				return NULL;
			}

			case IDM_FILT_CUBIC: {
				InterpolationChanged(hWnd, InterpolateCubic);
				return NULL;
			}

			case IDM_FILT_NONE: {
				UpscalingChanged(hWnd, UpscaleNone);
				return NULL;
			}

			case IDM_FILT_SCALENX_2X: {
				SelectScaleNxMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALENX_3X: {
				SelectScaleNxMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XSAL_2X: {
				SelectXSalMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_EAGLE_2X: {
				SelectEagleMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_2X: {
				SelectScaleHQMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_4X: {
				SelectScaleHQMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_2X: {
				SelectXBRZMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_XRBZ_3X: {
				SelectXBRZMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XRBZ_4X: {
				SelectXBRZMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_5X: {
				SelectXBRZMode(hWnd, 5);
				return NULL;
			}

			case IDM_FILT_XRBZ_6X: {
				SelectXBRZMode(hWnd, 6);
				return NULL;
			}

			case IDM_REND_AUTO: {
				SelectRenderer(hWnd, RendererAuto);
				return NULL;
			}

			case IDM_REND_GL1: {
				SelectRenderer(hWnd, RendererOpenGL1);
				return NULL;
			}

			case IDM_REND_GL2: {
				SelectRenderer(hWnd, RendererOpenGL2);
				return NULL;
			}

			case IDM_REND_GL3: {
				SelectRenderer(hWnd, RendererOpenGL3);
				return NULL;
			}

			default:
				HMENU hMenu = GetMenu(hWnd);
				if (hMenu)
				{
					MenuItemData mData;
					mData.childId = IDM_LANG_ENGLISH;
					if (GetMenuByChildID(hMenu, &mData))
					{
						UINT count = (UINT)GetMenuItemCount(mData.hMenu);
						for (UINT i = 0; i < count; ++i)
						{
							UINT id = GetMenuItemID(mData.hMenu, i);
							if (id == wParam)
							{
								if (config.language.futured != id)
								{
									config.language.futured = id;
									Config::Set(CONFIG_WRAPPER, "Language", *(INT*)&config.language.futured);

									if (config.language.current != id)
										Main::ShowInfo(IDS_INFO_RESTART);

									CheckMenu(hWnd, MenuLanguage);
								}

								return NULL;
							}
						}
					}
				}

				return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		case WM_SETCURSOR: {
			if (config.cursor.fix && LOWORD(lParam) == HTCLIENT)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					if (ddraw->windowState != WinStateWindowed || !config.image.aspect)
						SetCursor(config.cursor.game);
					else
					{
						POINT p;
						GetCursorPos(&p);
						ScreenToClient(hWnd, &p);

						if (p.x >= ddraw->viewport.rectangle.x && p.x < ddraw->viewport.rectangle.x + ddraw->viewport.rectangle.width && p.y >= ddraw->viewport.rectangle.y && p.y < ddraw->viewport.rectangle.y + ddraw->viewport.rectangle.height)
							SetCursor(config.cursor.game);
						else
							SetCursor(config.cursor.default);
					}

					return TRUE;
				}
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		default:
			if (uMsg == config.msgMenu)
			{
				CheckMenu(hWnd);
				return NULL;
			}

			return CallWindowProc(Window::OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_SYSCOMMAND:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_GETMINMAXINFO:
		case WM_SETCURSOR:
			return WindowProc(GetParent(hWnd), uMsg, wParam, lParam);

		default:
			return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
		}
	}

	VOID __fastcall SetCaptureKeys(BOOL state)
	{
		if (state)
		{
			if (!OldKeysHook)
				OldKeysHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeysHook, hDllModule, NULL);
		}
		else
		{
			if (OldKeysHook && UnhookWindowsHookEx(OldKeysHook))
				OldKeysHook = NULL;
		}
	}

	VOID __fastcall SetCaptureWindow(HWND hWnd)
	{
		OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
	}

	VOID __fastcall SetCapturePanel(HWND hWnd)
	{
		OldPanelProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)PanelProc);
	}
}