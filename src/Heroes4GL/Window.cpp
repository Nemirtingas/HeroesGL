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

#define WM_REDRAW_CANVAS 0x8001

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	VOID BeginDialog(DialogParams* params)
	{
		params->ddraw = Main::FindOpenDrawByWindow(params->hWnd);
		if (params->ddraw)
		{
			params->isFullscreen = params->ddraw->windowState == WinStateFullScreen;
			if (params->isFullscreen)
			{
				params->ddraw->RenderStop();
				if (params->isGrayed)
					config.colors.current = &inactiveColors;
				params->ddraw->windowState = WinStateWindowed;
				params->ddraw->RenderStart();
			}
			else
			{
				if (params->isGrayed)
					config.colors.current = &inactiveColors;
				SetEvent(params->ddraw->hDrawEvent);
			}
		}

		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &params->cookie))
			params->cookie = NULL;
	}

	VOID EndDialog(DialogParams* params)
	{
		if (params->cookie)
			DeactivateActCtxC(0, params->cookie);

		if (params->ddraw)
		{
			if (params->isFullscreen)
			{
				params->ddraw->RenderStop();
				if (params->isGrayed)
					config.colors.current = &config.colors.active;
				params->ddraw->windowState = WinStateFullScreen;
				params->ddraw->RenderStart();
			}
			else
			{
				if (params->isGrayed)
					config.colors.current = &config.colors.active;
				SetEvent(params->ddraw->hDrawEvent);
			}
		}

		if (params->hWnd)
			SetForegroundWindow(params->hWnd);
	}

	BOOL GetMenuByChildID(HMENU hParent, MenuItemData* mData, INT index)
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

	BOOL GetMenuByChildID(HMENU hMenu, MenuItemData* mData)
	{
		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
			if (GetMenuByChildID(hMenu, mData, i))
				return TRUE;

		*mData = {};
		return FALSE;
	}

	VOID CheckMenu(HMENU hMenu, MenuType type)
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

		case MenuFps: {
			DWORD menuId;
			switch (config.fps)
			{
			case FpsNormal:
				menuId = IDM_FPS_NORMAL;
				break;
			case FpsBenchmark:
				menuId = IDM_FPS_BENCHMARK;
				break;
			default:
				menuId = IDM_FPS_OFF;
				break;
			}

			CheckMenuItem(hMenu, IDM_FPS_OFF, MF_BYCOMMAND | (menuId == IDM_FPS_OFF ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FPS_NORMAL, MF_BYCOMMAND | (menuId == IDM_FPS_NORMAL ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FPS_BENCHMARK, MF_BYCOMMAND | (menuId == IDM_FPS_BENCHMARK ? MF_CHECKED : MF_UNCHECKED));

			MenuItemData mData;
			mData.childId = IDM_FPS_OFF;
			if (GetMenuByChildID(hMenu, &mData))
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (menuId != IDM_FPS_OFF ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuInterpolate: {
			EnableMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | (config.gl.version.value ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(hMenu, IDM_FILT_HERMITE, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(hMenu, IDM_FILT_LANCZOS, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			DWORD menuId;
			switch (config.image.interpolation)
			{
			case InterpolateLinear:
				menuId = config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF;
				break;
			case InterpolateHermite:
				menuId = config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_HERMITE : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF);
				break;
			case InterpolateCubic:
				menuId = config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_CUBIC : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF);
				break;
			case InterpolateLanczos:
				menuId = config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_LANCZOS : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF);
				break;
			default:
				menuId = IDM_FILT_OFF;
				break;
			}

			CheckMenuItem(hMenu, IDM_FILT_OFF, MF_BYCOMMAND | (menuId == IDM_FILT_OFF ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | (menuId == IDM_FILT_LINEAR ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FILT_HERMITE, MF_BYCOMMAND | (menuId == IDM_FILT_HERMITE ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | (menuId == IDM_FILT_CUBIC ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDM_FILT_LANCZOS, MF_BYCOMMAND | (menuId == IDM_FILT_LANCZOS ? MF_CHECKED : MF_UNCHECKED));

			MenuItemData mData;
			mData.childId = IDM_FILT_OFF;
			if (GetMenuByChildID(hMenu, &mData))
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (menuId != IDM_FILT_OFF ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuUpscale: {
			CheckMenuItem(hMenu, IDM_FILT_NONE, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD menuId;
			BOOL isFilters = config.gl.version.value >= GL_VER_3_0;
			if (isFilters)
			{
				switch (config.image.upscaling)
				{
				case UpscaleScaleNx:
					menuId = config.image.scaleNx == 3 ? IDM_FILT_SCALENX_3X : IDM_FILT_SCALENX_2X;
					break;
				case UpscaleEagle:
					menuId = IDM_FILT_EAGLE_2X;
					break;
				case UpscaleXSal:
					menuId = IDM_FILT_XSAL_2X;
					break;
				case UpscaleScaleHQ:
					menuId = config.image.scaleHQ == 4 ? IDM_FILT_SCALEHQ_4X : IDM_FILT_SCALEHQ_2X;
					break;
				case UpscaleXRBZ:
					switch (config.image.xBRz)
					{
					case 3:
						menuId = IDM_FILT_XRBZ_3X;
						break;
					case 4:
						menuId = IDM_FILT_XRBZ_4X;
						break;
					case 5:
						menuId = IDM_FILT_XRBZ_5X;
						break;
					case 6:
						menuId = IDM_FILT_XRBZ_6X;
						break;
					default:
						menuId = IDM_FILT_XRBZ_2X;
						break;
					}
					break;
				default:
					menuId = IDM_FILT_NONE;
					break;
				}

				CheckMenuItem(hMenu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | (menuId == IDM_FILT_SCALENX_2X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | (menuId == IDM_FILT_SCALENX_3X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | (menuId == IDM_FILT_EAGLE_2X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | (menuId == IDM_FILT_XSAL_2X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | (menuId == IDM_FILT_SCALEHQ_2X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | (menuId == IDM_FILT_SCALEHQ_4X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | (menuId == IDM_FILT_XRBZ_2X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | (menuId == IDM_FILT_XRBZ_3X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | (menuId == IDM_FILT_XRBZ_4X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | (menuId == IDM_FILT_XRBZ_5X ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(hMenu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | (menuId == IDM_FILT_XRBZ_6X ? MF_CHECKED : MF_UNCHECKED));
			}
			else
				menuId = IDM_FILT_NONE;

			CheckMenuItem(hMenu, IDM_FILT_NONE, MF_BYCOMMAND | (menuId == IDM_FILT_NONE ? MF_CHECKED : MF_UNCHECKED));

			MenuItemData mData;
			mData.childId = IDM_FILT_SCALENX_2X;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters && config.image.upscaling == UpscaleScaleNx ? MF_CHECKED : MF_UNCHECKED));
			}

			mData.childId = IDM_FILT_EAGLE_2X;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters && config.image.upscaling == UpscaleEagle ? MF_CHECKED : MF_UNCHECKED));
			}

			mData.childId = IDM_FILT_XSAL_2X;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters && config.image.upscaling == UpscaleXSal ? MF_CHECKED : MF_UNCHECKED));
			}

			mData.childId = IDM_FILT_SCALEHQ_2X;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters && config.image.upscaling == UpscaleScaleHQ ? MF_CHECKED : MF_UNCHECKED));
			}

			mData.childId = IDM_FILT_XRBZ_2X;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (isFilters && config.image.upscaling == UpscaleXRBZ ? MF_CHECKED : MF_UNCHECKED));
			}

			mData.childId = IDM_FILT_NONE;
			if (GetMenuByChildID(hMenu, &mData))
				CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (menuId != IDM_FILT_NONE ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuColors: {
			EnableMenuItem(hMenu, IDM_COLOR_ADJUST, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		}
		break;

		case MenuCpu: {
			CheckMenuItem(hMenu, IDM_PATCH_CPU, MF_BYCOMMAND | (config.coldCPU ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuCore: {
			if (config.singleCore.allowed)
			{
				EnableMenuItem(hMenu, IDM_SINGLE_CORE, MF_BYCOMMAND | MF_ENABLED);
				CheckMenuItem(hMenu, IDM_SINGLE_CORE, MF_BYCOMMAND | (config.singleCore.enabled ? MF_CHECKED : MF_UNCHECKED));
			}
			else
			{
				EnableMenuItem(hMenu, IDM_SINGLE_CORE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				CheckMenuItem(hMenu, IDM_SINGLE_CORE, MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		break;

		case MenuLanguage: {
			MenuItemData mData;
			mData.childId = IDM_LANG_ENGLISH;
			if (GetMenuByChildID(hMenu, &mData))
			{
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (SetThreadLanguage ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				if (SetThreadLanguage)
				{
					UINT count = (UINT)GetMenuItemCount(mData.hMenu);
					for (UINT i = 0; i < count; ++i)
						CheckMenuItem(mData.hMenu, i, MF_BYPOSITION | (SetThreadLanguage && GetMenuItemID(mData.hMenu, i) == config.language.futured ? MF_CHECKED : MF_UNCHECKED));
				}
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

	VOID CheckMenu(HWND hWnd, MenuType type)
	{
		CheckMenu(GetMenu(hWnd), type);
	}

	VOID CheckMenu(HMENU hMenu)
	{
		CheckMenu(hMenu, MenuAspect);
		CheckMenu(hMenu, MenuVSync);
		CheckMenu(hMenu, MenuFps);
		CheckMenu(hMenu, MenuInterpolate);
		CheckMenu(hMenu, MenuUpscale);
		CheckMenu(hMenu, MenuColors);
		CheckMenu(hMenu, MenuCpu);
		CheckMenu(hMenu, MenuCore);
		CheckMenu(hMenu, MenuLanguage);
		CheckMenu(hMenu, MenuRenderer);
	}

	VOID CheckMenu(HWND hWnd)
	{
		CheckMenu(GetMenu(hWnd));
	}

	VOID FpsChanged(HWND hWnd, FpsState state)
	{
		if (config.fps != state)
		{
			config.fps = state;
			Config::Set(CONFIG_WRAPPER, "FpsCounter", *(INT*)&config.fps);

			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				ddraw->isFpsChanged = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			CheckMenu(hWnd, MenuFps);
		}
	}

	VOID FilterChanged(HWND hWnd, const CHAR* name, INT value)
	{
		Config::Set(CONFIG_WRAPPER, name, value);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->LoadFilterState();
			SetEvent(ddraw->hDrawEvent);
		}
	}

	VOID InterpolationChanged(HWND hWnd, InterpolationFilter filter)
	{
		config.image.interpolation = config.gl.version.value >= GL_VER_2_0 || filter < InterpolateHermite ? filter : InterpolateLinear;

		FilterChanged(hWnd, "Interpolation", *(INT*)&config.image.interpolation);
		CheckMenu(hWnd, MenuInterpolate);
	}

	VOID UpscalingChanged(HWND hWnd, UpscalingFilter filter)
	{
		config.image.upscaling = config.gl.version.value >= GL_VER_3_0 ? filter : UpscaleNone;

		FilterChanged(hWnd, "Upscaling", *(INT*)&config.image.upscaling);
		CheckMenu(hWnd, MenuUpscale);
	}

	VOID SelectScaleNxMode(HWND hWnd, BYTE value)
	{
		config.image.scaleNx = value;
		Config::Set(CONFIG_WRAPPER, "ScaleNx", *(INT*)&config.image.scaleNx);
		UpscalingChanged(hWnd, UpscaleScaleNx);
	}

	VOID SelectXSalMode(HWND hWnd, BYTE value)
	{
		config.image.xSal = value;
		Config::Set(CONFIG_WRAPPER, "XSal", *(INT*)&config.image.xSal);
		UpscalingChanged(hWnd, UpscaleXSal);
	}

	VOID SelectEagleMode(HWND hWnd, BYTE value)
	{
		config.image.eagle = value;
		Config::Set(CONFIG_WRAPPER, "Eagle", *(INT*)&config.image.eagle);
		UpscalingChanged(hWnd, UpscaleEagle);
	}

	VOID SelectScaleHQMode(HWND hWnd, BYTE value)
	{
		config.image.scaleHQ = value;
		Config::Set(CONFIG_WRAPPER, "ScaleHQ", *(INT*)&config.image.scaleHQ);
		UpscalingChanged(hWnd, UpscaleScaleHQ);
	}

	VOID SelectXBRZMode(HWND hWnd, BYTE value)
	{
		config.image.xBRz = value;
		Config::Set(CONFIG_WRAPPER, "XBRZ", *(INT*)&config.image.xBRz);
		UpscalingChanged(hWnd, UpscaleXRBZ);
	}

	VOID SelectRenderer(HWND hWnd, RendererType renderer)
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

	BOOL __stdcall EnumChildProc(HWND hDlg, LPARAM lParam)
	{
		if ((GetWindowLong(hDlg, GWL_STYLE) & SS_ICON) == SS_ICON)
			SendMessage(hDlg, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)config.icon);
		else
			SendMessage(hDlg, WM_SETFONT, (WPARAM)config.font, TRUE);

		return TRUE;
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
			RECT rect, offset;
			GetClientRect(hDlg, &rect);
			GetWindowRect(hDlg, &offset);
			OffsetRect(&rect, offset.left, offset.top);
			AdjustWindowRect(&rect, STYLE_DIALOG, FALSE);
			SetWindowLong(hDlg, GWL_STYLE, STYLE_DIALOG);
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

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
				StrPrint(path, temp, 2021, "Verok");
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
				case IDC_LNK_PATRON: {
					SHELLEXECUTEINFOW shExecInfo = {};
					shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
					shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
					shExecInfo.nShow = SW_SHOW;
					ShellExecuteExW(&shExecInfo);
					break;
				}

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

	LRESULT __stdcall AboutAppProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_INITDIALOG)
		{
			RECT rect, offset;
			GetClientRect(hDlg, &rect);
			GetWindowRect(hDlg, &offset);
			OffsetRect(&rect, offset.left, offset.top);
			AdjustWindowRect(&rect, STYLE_DIALOG, FALSE);
			SetWindowLong(hDlg, GWL_STYLE, STYLE_DIALOG);
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
			EnumChildWindows(hDlg, EnumChildProc, NULL);
		}
		else if (uMsg == WM_COMMAND && wParam == IDC_BTN_OK)
			EndDialog(hDlg, TRUE);
		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall ColorAdjustmentProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG: {
			SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETRANGE, FALSE, MAKELPARAM(0, 360));
			SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));

			SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

			SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, DWORD(config.colors.active.satHue.hueShift * 360.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, DWORD(config.colors.active.satHue.saturation * 1000.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.left.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.right.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, DWORD(config.colors.active.gamma.rgb * 1000.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.left.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.right.rgb * 255.0f));

			CHAR text[16];
			FLOAT val = 360.0f * config.colors.active.satHue.hueShift - 180.0f;
			StrPrint(text, val ? "%+0.f" : "%0.f", val);
			SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%.2f", 4.0f * config.colors.active.satHue.saturation * config.colors.active.satHue.saturation);
			SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.input.left.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.input.right.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%.2f", MathPower(2.0f * config.colors.active.gamma.rgb, 3.32f));
			SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.output.left.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.output.right.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

			LevelsData* levelsData = (LevelsData*)MemoryAlloc(sizeof(LevelsData));
			if (levelsData)
			{
				SetWindowLong(hDlg, GWLP_USERDATA, (LONG)levelsData);
				*levelsData = {};
				levelsData->delta = 0.7f;
				levelsData->values = config.colors.active;

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw && ddraw->attachedSurface && ddraw->attachedSurface->indexBuffer)
				{
					OpenDrawSurface* surface = ddraw->attachedSurface;

					struct {
						DWORD width;
						DWORD height;
					} size = {
						surface->width,
						surface->height
					};

					LevelColors levels[256] = {};

					WORD* data = (WORD*)surface->indexBuffer;
					DWORD count = size.width * size.height;
					do
					{
						WORD p = *data++;
						DWORD px = ((p & 0xF800) >> 8) | ((p & 0x07E0) << 5) | ((p & 0x001F) << 19);

						BYTE* b = (BYTE*)&px;
						++levels[*b++].red;
						++levels[*b++].green;
						++levels[*b].blue;
					} while (--count);

					levelsData->hDc = CreateCompatibleDC(NULL);
					if (levelsData->hDc)
					{
						HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);
						RECT rc;
						GetClientRect(hImg, &rc);

						BITMAPINFO bmi = {};
						bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
						bmi.bmiHeader.biWidth = rc.right;
						bmi.bmiHeader.biHeight = 100;
						bmi.bmiHeader.biPlanes = 1;
						bmi.bmiHeader.biBitCount = 32;
						bmi.bmiHeader.biXPelsPerMeter = 1;
						bmi.bmiHeader.biYPelsPerMeter = 1;

						levelsData->hBmp = CreateDIBSection(levelsData->hDc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (VOID**)&levelsData->data, NULL, 0);
						if (levelsData->hBmp)
						{
							SelectObject(levelsData->hDc, levelsData->hBmp);

							DWORD total = size.width * size.height;

							LevelColors* src = levels;
							LevelColorsFloat* dst = levelsData->colors;
							DWORD count = 256;
							do
							{
								dst->red = (FLOAT)src->red / total;
								dst->green = (FLOAT)src->green / total;
								dst->blue = (FLOAT)src->blue / total;

								++src;
								++dst;
							} while (--count);
						}
					}
				}
			}

			EnumChildWindows(hDlg, EnumChildProc, NULL);
			UpdateWindow(hDlg);
			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CUSTOMDRAW)
			{
				switch (wParam)
				{
				case IDC_TRK_HUE:
				case IDC_TRK_SAT:
				case IDC_TRK_IN_LEFT:
				case IDC_TRK_IN_RIGHT:
				case IDC_TRK_GAMMA:
				case IDC_TRK_OUT_LEFT:
				case IDC_TRK_OUT_RIGHT: {
					NMCUSTOMDRAW* lpDraw = (NMCUSTOMDRAW*)lParam;

					switch (lpDraw->dwDrawStage)
					{
					case CDDS_PREPAINT: {
						SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
						return CDRF_NOTIFYITEMDRAW;
					}

					case CDDS_ITEMPREPAINT: {

						switch (lpDraw->dwItemSpec)
						{
						case TBCD_THUMB: {
							lpDraw->rc.left += 1;
							lpDraw->rc.top += 2;
							lpDraw->rc.right -= 1;
							lpDraw->rc.bottom -= 2;
							break;
						}

						default:
							break;
						}

						return CDRF_DODEFAULT;
					}

					default:
						break;
					}

					break;
				}

				default:
					break;
				}
			}

			break;
		}

		case WM_REDRAW_CANVAS: {
			HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

			RECT rc;
			GetWindowRect(hImg, &rc);

			POINT pt = *(POINT*)&rc;
			ScreenToClient(hDlg, &pt);

			rc = { pt.x, pt.y, pt.x + (rc.right - rc.left), pt.y + (rc.bottom - rc.top) };
			InvalidateRect(hDlg, &rc, FALSE);
			return NULL;
		}

		case WM_DRAWITEM: {
			if (wParam == IDC_CANVAS)
			{
				DRAWITEMSTRUCT* paint = (DRAWITEMSTRUCT*)lParam;

				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					LevelColorsFloat prep[260] = {};
					{
						FLOAT h = 2.0f * config.colors.active.satHue.hueShift - 1.0f;
						FLOAT s = 4.0f * config.colors.active.satHue.saturation * config.colors.active.satHue.saturation;

						struct {
							Levels input;
							Levels gamma;
							Levels output;
						} levels;

						for (DWORD i = 0; i < 4; ++i)
						{
							levels.input.chanel[i] = config.colors.active.input.right.chanel[i] - config.colors.active.input.left.chanel[i];
							levels.gamma.chanel[i] = 1.0f / (FLOAT)MathPower(2.0f * config.colors.active.gamma.chanel[i], 3.32f);
							levels.output.chanel[i] = config.colors.active.output.right.chanel[i] - config.colors.active.output.left.chanel[i];
						}

						INT sh = 0;
						if (h < 0.0f)
						{
							sh++;
							h = 1.0f + h;
						}

						LevelColorsFloat* src = levelsData->colors;
						for (DWORD i = 0; i < 256; ++i, ++src)
						{
							FLOAT dx = (FLOAT)i / 255.0f;

							LevelColorsFloat ex;
							FLOAT sss = 0;
							for (DWORD j = 0; j < 3; ++j)
							{
								FLOAT p0 = src->chanel[(j - sh + 2) % 3];
								FLOAT p1 = src->chanel[(j - sh + 3) % 3];
								FLOAT p2 = src->chanel[(j - sh + 4) % 3];
								FLOAT c = p1 + 0.5 * h * (p2 - p0 + h * (p0 - 5.0 * p1 + 4.0 * p2 + h * (3.0 * (p1 - p2))));
								ex.chanel[j] = c;
								sss += c;
							}
							sss /= 3;

							for (DWORD j = 0; j < 3; ++j)
							{
								FLOAT k = dx;
								for (DWORD s = 2, idx = j + 1; s; --s, idx = 0)
								{
									k = (k - config.colors.active.input.left.chanel[idx]) / levels.input.chanel[idx];
									k = min(1.0f, max(0.0f, k));
									k = (FLOAT)MathPower(k, levels.gamma.chanel[idx]);
									k = k * levels.output.chanel[idx] + config.colors.active.output.left.chanel[idx];
								}

								prep[(DWORD)(k * 255.0f) + 2].chanel[j] += sss - (sss - ex.chanel[j]) * s;
							}
						}
					}

					prep[1] = prep[2];
					prep[258] = prep[257];

					LevelColorsFloat floats[259];
					for (DWORD y = 4; y; --y)
					{
						{
							LevelColorsFloat* src = prep;
							LevelColorsFloat* dst = floats + 1;
							DWORD count = 257;
							do
							{
								for (DWORD i = 0; i < 3; ++i)
									dst->chanel[i] = FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0));

								++src;
								++dst;
							} while (--count);
						}
						floats[0] = floats[1];
						floats[258] = floats[257];

						{
							LevelColorsFloat* src = floats;
							LevelColorsFloat* dst = prep + 2;
							DWORD count = 256;
							do
							{
								for (DWORD i = 0; i < 3; ++i)
									dst->chanel[i] = FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0));

								++src;
								++dst;
							} while (--count);
						}
						prep[1] = prep[2];
						prep[258] = prep[257];
					}

					{
						LevelColorsFloat* src = prep;
						LevelColorsFloat* dst = floats + 1;
						DWORD count = 257;
						do
						{
							for (DWORD i = 0; i < 3; ++i)
								dst->chanel[i] = min(1.0f, max(0.0f, FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0))));

							++src;
							++dst;
						} while (--count);
					}

					FLOAT max = 0.0;
					{
						FLOAT* data = (FLOAT*)(floats + 1);
						DWORD count = 257 * 3;
						do
						{
							*data = (FLOAT)MathPower(*data, levelsData->delta);
							max += *data++;
						} while (--count);
					}

					if (max > 0.0)
					{
						LevelColors levels[259] = {};

						{
							max /= FLOAT(256 / 4 * 3);

							FLOAT* src = (FLOAT*)(floats + 1);
							DWORD* dst = (DWORD*)(levels + 1);
							DWORD count = 257 * 3;
							do
								*dst++ = DWORD(*src++ / max * 100.0f);
							while (--count);

							levels[0] = levels[1];
							levels[258] = levels[257];
						}

						{
							MemoryZero(levelsData->bmpData, sizeof(levelsData->bmpData));

							INT index;
							struct {
								DWORD line;
								DWORD back;
							} light, dark;

							if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 0;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 1;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 2;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else
							{
								index = -1;
								dark = { 0xD0, 0x50 };
							}

							DWORD* dst = levelsData->bmpData + 1;
							for (DWORD i = 0; i < 100; ++i)
							{
								LevelColors* src = levels + 1;
								DWORD count = 257;
								do
								{
									LevelColors* neighbor = src - 1;
									DWORD iter = 2;
									do
									{
										DWORD pix = 0;
										for (DWORD j = 0, x = 16; j < 3; ++j, x -= 8)
										{
											if (j != index)
											{
												if (i < src->chanel[j])
												{
													DWORD min = neighbor->chanel[j] + 1;
													if (min > src->chanel[j])
														min = src->chanel[j];

													pix |= (i + 1 >= min ? dark.line : dark.back) << x;
												}
												else
													pix |= 0x20 << x;
											}
										}

										if (index + 1)
										{
											DWORD shift = (2 - index) * 8;
											if (i < src->chanel[index])
											{
												DWORD min = neighbor->chanel[index] + 1;
												if (min > src->chanel[index])
													min = src->chanel[index];

												if (i + 1 >= min)
													pix = light.line << shift;
												else
													pix |= light.back << shift;
											}
											else
												pix |= 0x20 << shift;
										}

										*dst++ = pix;
										neighbor = src + 1;
									} while (--iter);

									++src;
								} while (--count);

								dst += 2;
							}

							HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

							RECT rc;
							GetClientRect(hImg, &rc);

							for (LONG i = 0; i < rc.right; ++i)
							{
								FLOAT pos = (FLOAT)i / rc.right * 514.0f;
								DWORD index = DWORD(pos);
								pos -= (FLOAT)index;

								DWORD* dest = levelsData->data + i;
								for (DWORD j = 0; j < 100; ++j)
								{
									BYTE* src = (BYTE*)&levelsData->bmpData[j * 516 + index];
									BYTE* dst = (BYTE*)dest;

									for (DWORD j = 0; j < 3; ++j, ++src)
									{
										INT d = INT(src[4] + 0.5 * pos * (src[8] - src[0] + pos * (2 * src[0] - 5 * src[4] + 4 * src[8] - src[12] + pos * (3 * (src[4] - src[8]) + src[12] - src[0]))));
										dst[j] = LOBYTE(min(255, max(0, d)));
									}

									dest += rc.right;
								}
							}

							if (levelsData->hBmp)
								BitBlt(paint->hDC, 0, 0, rc.right, 100, levelsData->hDc, 0, 0, SRCCOPY);
						}

						return TRUE;
					}
				}

				FillRect(paint->hDC, &paint->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
				return TRUE;
			}

			break;
		}

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDC_BTN_OK: {
				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
					levelsData->values = config.colors.active;

				Config::Set(CONFIG_COLORS, "HueSat", MAKELONG(DWORD(config.colors.active.satHue.hueShift * 1000.0f), DWORD(config.colors.active.satHue.saturation * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RgbInput", MAKELONG(DWORD(config.colors.active.input.left.rgb * 1000.0f), DWORD(config.colors.active.input.right.rgb * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RedInput", MAKELONG(DWORD(config.colors.active.input.left.red * 1000.0f), DWORD(config.colors.active.input.right.red * 1000.0f)));
				Config::Set(CONFIG_COLORS, "GreenInput", MAKELONG(DWORD(config.colors.active.input.left.green * 1000.0f), DWORD(config.colors.active.input.right.green * 1000.0f)));
				Config::Set(CONFIG_COLORS, "BlueInput", MAKELONG(DWORD(config.colors.active.input.left.blue * 1000.0f), DWORD(config.colors.active.input.right.blue * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RgbGamma", DWORD(config.colors.active.gamma.rgb * 1000.0f));
				Config::Set(CONFIG_COLORS, "RedGamma", DWORD(config.colors.active.gamma.red * 1000.0f));
				Config::Set(CONFIG_COLORS, "GreenGamma", DWORD(config.colors.active.gamma.green * 1000.0f));
				Config::Set(CONFIG_COLORS, "BlueGamma", DWORD(config.colors.active.gamma.blue * 1000.0f));
				Config::Set(CONFIG_COLORS, "RgbOutput", MAKELONG(DWORD(config.colors.active.output.left.rgb * 1000.0f), DWORD(config.colors.active.output.right.rgb * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RedOutput", MAKELONG(DWORD(config.colors.active.output.left.red * 1000.0f), DWORD(config.colors.active.output.right.red * 1000.0f)));
				Config::Set(CONFIG_COLORS, "GreenOutput", MAKELONG(DWORD(config.colors.active.output.left.green * 1000.0f), DWORD(config.colors.active.output.right.green * 1000.0f)));
				Config::Set(CONFIG_COLORS, "BlueOutput", MAKELONG(DWORD(config.colors.active.output.left.blue * 1000.0f), DWORD(config.colors.active.output.right.blue * 1000.0f)));
			}

			case IDC_BTN_CANCEL: {
				EndDialog(hDlg, TRUE);
				return NULL;
			}

			case IDC_BTN_RESET: {
				config.colors.active.satHue.hueShift = 0.5f;
				config.colors.active.satHue.saturation = 0.5f;

				for (DWORD i = 0; i < 4; ++i)
				{
					config.colors.active.input.left.chanel[i] = 0.0f;
					config.colors.active.input.right.chanel[i] = 1.0f;
					config.colors.active.gamma.chanel[i] = 0.5f;
					config.colors.active.output.left.chanel[i] = 0.0f;
					config.colors.active.output.right.chanel[i] = 1.0f;
				}

				SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

				SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, 180);
				SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, 255);
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, 255);

				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw)
				{
					SetEvent(ddraw->hDrawEvent);
					Sleep(0);
				}

				return NULL;
			}

			case IDC_BTN_AUTO: {
				config.colors.active.satHue.hueShift = 0.5f;
				config.colors.active.satHue.saturation = 0.5f;

				for (DWORD i = 0; i < 4; ++i)
				{
					config.colors.active.input.left.chanel[i] = 0.0f;
					config.colors.active.input.right.chanel[i] = 1.0f;
					config.colors.active.gamma.chanel[i] = 0.5f;
					config.colors.active.output.left.chanel[i] = 0.0f;
					config.colors.active.output.right.chanel[i] = 1.0f;
				}

				SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

				SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, 180);
				SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, 255);
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, 255);

				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");

				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					{
						LevelColorsFloat found = { 0.0, 0.0, 0.0 };
						BOOL success[3] = { FALSE, FALSE, FALSE };
						DWORD exit = 0;
						LevelColorsFloat* data = levelsData->colors;

						for (DWORD i = 0; i < 256; ++i, ++data)
						{
							for (DWORD j = 0; j < 3; ++j)
							{
								if (!success[j])
								{
									found.chanel[j] += data->chanel[j];
									if (found.chanel[j] > 0.007f)
									{
										success[j] = TRUE;
										++exit;
										config.colors.active.input.left.chanel[j + 1] = (FLOAT)i / 255.0f;
									}
								}
							}

							if (exit == 3)
								break;
						};
					}

					{
						LevelColorsFloat found = { 0.0, 0.0, 0.0 };
						BOOL success[3] = { FALSE, FALSE, FALSE };
						DWORD exit = 0;
						LevelColorsFloat* data = &levelsData->colors[255];

						for (DWORD i = 255; i >= 0; --i, --data)
						{
							for (DWORD j = 0; j < 3; ++j)
							{
								if (!success[j])
								{
									found.chanel[j] += data->chanel[j];
									if (found.chanel[j] > 0.007f)
									{
										success[j] = TRUE;
										++exit;
										config.colors.active.input.right.chanel[j + 1] = (FLOAT)i / 255.0f;
									}
								}
							}

							if (exit == 3)
								break;
						};
					}
				}

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw)
				{
					SetEvent(ddraw->hDrawEvent);
					Sleep(0);
				}

				return NULL;
			}

			case IDC_RAD_RGB:
			case IDC_RAD_RED:
			case IDC_RAD_GREEN:
			case IDC_RAD_BLUE: {
				DWORD index = (DWORD)wParam - IDC_RAD_RGB;

				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.left.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.right.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, DWORD(config.colors.active.gamma.chanel[index] * 1000.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.left.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.right.chanel[index] * 255.0f));

				CHAR text[16];

				StrPrint(text, "%0.f", 255.0f * config.colors.active.input.left.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.input.right.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%.2f", MathPower(2.0f * config.colors.active.gamma.chanel[index], 3.32f));
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.output.left.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.output.right.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				return NULL;
			}

			default:
				break;
			}

			break;
		}

		case WM_MOUSEWHEEL: {
			HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

			RECT rc;
			GetClientRect(hImg, &rc);

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hImg, &pt);
			if (PtInRect(&rc, pt))
			{
				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					FLOAT dlt = levelsData->delta + 0.025f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
					if (dlt > 0.0f && dlt < 1.0f)
					{
						levelsData->delta = dlt;
						SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
					}
				}
			}

			break;
		}

		case WM_HSCROLL: {
			DWORD value = LOWORD(wParam) == SB_THUMBTRACK || LOWORD(wParam) == SB_THUMBPOSITION ? HIWORD(wParam) : SendMessage((HWND)lParam, TBM_GETPOS, NULL, NULL);
			CHAR text[16];
			INT id = GetDlgCtrlID((HWND)lParam);
			switch (id)
			{
			case IDC_TRK_HUE: {
				config.colors.active.satHue.hueShift = (FLOAT)value / 360.0f;

				DWORD val = value - 180;
				StrPrint(text, val ? "%+d" : "%d", val);
				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_SAT: {
				config.colors.active.satHue.saturation = 0.001f * value;

				StrPrint(text, "%.2f", 0.000004f * (value * value));
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_IN_LEFT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				DWORD comp = SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_GETPOS, NULL, NULL);
				if (value > comp)
				{
					value = comp;
					SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, value);
				}

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.input.left.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_IN_RIGHT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				DWORD comp = SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_GETPOS, NULL, NULL);
				if (value < comp)
				{
					value = comp;
					SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, value);
				}

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.input.right.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_GAMMA: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				config.colors.active.gamma.chanel[idx] = 0.001f * value;

				StrPrint(text, "%.2f", MathPower(0.002f * value, 3.32f));
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_OUT_LEFT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.output.left.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_OUT_RIGHT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.output.right.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			default:
				break;
			}

			SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
			if (ddraw)
			{
				SetEvent(ddraw->hDrawEvent);
				Sleep(0);
			}

			return NULL;
		}

		case WM_CLOSE: {
			EndDialog(hDlg, TRUE);
			break;
		}

		case WM_DESTROY: {
			LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
			if (levelsData)
			{
				SetWindowLong(hDlg, GWLP_USERDATA, NULL);
				if (levelsData->hDc)
				{
					DeleteDC(levelsData->hDc);
					if (levelsData->hBmp)
						DeleteObject(levelsData->hBmp);
				}

				config.colors.active = levelsData->values;
				MemoryFree(levelsData);
			}
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

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw && ddraw->hDraw != hWnd)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);

				if (ddraw->mode)
					lParam = ddraw->mode->width | (ddraw->mode->height << 16);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ACTIVATEAPP: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->windowState != WinStateWindowed)
				{
					if ((BOOL)wParam)
						ddraw->RenderStart();
					else
						ddraw->RenderStop();
				}
				else
				{
					config.colors.current = (BOOL)wParam ? &config.colors.active : &inactiveColors;
					SetEvent(ddraw->hDrawEvent);
				}
			}

			CallWindowProc(OldWindowProc, hWnd, WM_ACTIVATE, wParam, lParam);
			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ACTIVATE:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			if (config.keys.fpsCounter && config.keys.fpsCounter + VK_F1 - 1 == wParam)
			{
				switch (config.fps)
				{
				case FpsNormal:
					FpsChanged(hWnd, FpsBenchmark);
					break;
				case FpsBenchmark:
					FpsChanged(hWnd, FpsDisabled);
					break;
				default:
					FpsChanged(hWnd, FpsNormal);
					break;
				}

				return NULL;
			}
			else if (!(HIWORD(lParam) & KF_ALTDOWN))
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

					case InterpolateCubic:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateLanczos : InterpolateNearest);
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

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
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

		case WM_MOUSEHOVER:

		case WM_MOUSEWHEEL:

		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->mode)
			{
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ddraw->ScaleMouse(&p);
				lParam = MAKELONG(p.x, p.y);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOUSEMOVE: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->mode)
			{
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ddraw->ScaleMouse(&p);
				lParam = MAKELONG(p.x, p.y);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOUSELEAVE:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDM_PATCH_CPU: {
				config.coldCPU = !config.coldCPU;
				Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);
				Window::CheckMenu(hWnd, MenuCpu);
				return NULL;
			}

			case IDM_SINGLE_CORE: {
				config.singleCore.enabled = !config.singleCore.enabled;
				Config::Set(CONFIG_WRAPPER, "SingleCPU", config.singleCore.enabled);
				Config::SetProcessMask();
				Window::CheckMenu(hWnd, MenuCore);
				return NULL;
			}

			case IDM_RES_FULL_SCREEN:
				return CallWindowProc(OldWindowProc, hWnd, WM_KEYDOWN, VK_F4, lParam);

			case IDM_HELP_WRAPPER: {
				DialogParams params = { hWnd, TRUE, NULL };
				BeginDialog(&params);
				{
					DialogBoxParam(hDllModule, MAKEINTRESOURCE(params.cookie ? IDD_ABOUT : IDD_ABOUT_OLD), hWnd, (DLGPROC)AboutProc, params.cookie);
				}
				EndDialog(&params);
				return NULL;
			}

			case IDM_HELP_ABOUT: {
				Hooks::DialogBoxParamHook(hDllModule, MAKEINTRESOURCE(config.dialog), hWnd, (DLGPROC)AboutAppProc, NULL);
				return NULL;
			}

			case IDM_COLOR_ADJUST: {
				DialogParams params = { hWnd, FALSE, NULL };
				BeginDialog(&params);
				{
					DialogBoxParam(hDllModule, MAKEINTRESOURCE(IDD_COLOR_ADJUSTMENT), hWnd, (DLGPROC)ColorAdjustmentProc, NULL);
				}
				EndDialog(&params);
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

			case IDM_FPS_OFF: {
				FpsChanged(hWnd, FpsDisabled);
				return NULL;
			}

			case IDM_FPS_NORMAL: {
				FpsChanged(hWnd, FpsNormal);
				return NULL;
			}

			case IDM_FPS_BENCHMARK: {
				FpsChanged(hWnd, FpsBenchmark);
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

			case IDM_FILT_LANCZOS: {
				InterpolationChanged(hWnd, InterpolateLanczos);
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
						INT count = GetMenuItemCount(mData.hMenu);
						for (INT i = 0; i < count; ++i)
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

				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		case WM_SETCURSOR: {
			if (LOWORD(lParam) == HTCLIENT)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					if (ddraw->windowState != WinStateWindowed || !config.image.aspect)
						SetCursor(NULL);
					else
					{
						POINT p;
						GetCursorPos(&p);
						ScreenToClient(hWnd, &p);

						if (p.x >= ddraw->viewport.rectangle.x && p.x < ddraw->viewport.rectangle.x + ddraw->viewport.rectangle.width && p.y >= ddraw->viewport.rectangle.y && p.y < ddraw->viewport.rectangle.y + ddraw->viewport.rectangle.height)
							SetCursor(NULL);
						else
							SetCursor(config.cursor);
					}

					return TRUE;
				}
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_TIMER: {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);

			RECT rect;
			GetClientRect(hWnd, &rect);

			if (pt.x < rect.left || pt.x >= rect.right || pt.y < rect.top || pt.y >= rect.bottom)
				SendMessage(hWnd, WM_MOUSEMOVE, NULL, MAKELONG(pt.x, pt.y));

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		default:
			if (uMsg == config.msgMenu)
			{
				CheckMenu(hWnd);
				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	VOID SetCaptureKeys(BOOL state)
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
		case WM_SETCURSOR: {
			HWND hParent = GetParent(hWnd);
			WNDPROC proc = (WNDPROC)GetWindowLong(hParent, GWL_WNDPROC);
			return CallWindowProc(proc, hParent, uMsg, wParam, lParam);
		}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	VOID SetCaptureWindow(HWND hWnd)
	{
		OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
	}

	VOID SetCapturePanel(HWND hWnd)
	{
		OldPanelProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)PanelProc);
	}
}