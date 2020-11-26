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
#include "Shellapi.h"
#include "Mmsystem.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"
#include "hooker.h"

#define STYLE_FULL_OLD (WS_VISIBLE | WS_POPUP)
#define STYLE_FULL_NEW (WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS)

#define STYLE_WIN_OLD (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define STYLE_WIN_NEW (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_CLIPSIBLINGS)

#define STYLE_DIALOG (DS_MODALFRAME | WS_POPUP)

const AddressSpace addressArray[] = {
// === RUS ======================================================================================================================================
#pragma region RUS
	0x004D4A5F, 0x0059308B, 0x00000000, 0x00000000, 0x00592D72, 0x004E6BAF, 0x004E6C1D, 0x005B88F0, 0x004759CB, 0x00592A2C, 0x00000000, 0x005E3D48, 50,
	LNG_RUSSIAN, 0x0059D2B5, 0x00418DD4, 0x00418E74, 0x004D5390, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.0 Buka

	0x004F2533, 0x005F9649, 0x00000000, 0x00000000, 0x005F9336, 0x005074CF, 0x0050753B, 0x00635688, 0x0047E004, 0x005F901D, 0x00635AF4, 0x006793D8, 142,
	LNG_RUSSIAN, 0x00610806, 0x00418A27, 0x00418AC3, 0x004F2CD0, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.1 Buka

	0x004F2863, 0x005F9609, 0x00000000, 0x00000000, 0x005F92F6, 0x00507A5F, 0x00507ACB, 0x00635688, 0x0047DCC4, 0x005F8FDD, 0x00635AF4, 0x00679400, 142,
	LNG_RUSSIAN, 0x006107D6, 0x004189C7, 0x00418A63, 0x004F3000, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.2 Buka

	0x004F7EB3, 0x00602379, 0x00000000, 0x00000000, 0x00602066, 0x0050D58F, 0x0050D5FB, 0x0063C6A8, 0x0048052C, 0x00601D4D, 0x0063CB0C, 0x00682A10, 142,
	LNG_RUSSIAN, 0x006183F6, 0x00419787, 0x00419823, 0x004F86A0, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.1 Buka

	0x004F7EB3, 0x006021A9, 0x00000000, 0x00000000, 0x00601E96, 0x0050DB1F, 0x0050DB8B, 0x0063E6D8, 0x0048017C, 0x00601B7D, 0x0063EB3C, 0x00684D60, 142,
	LNG_RUSSIAN, 0x006193B6, 0x00419707, 0x004197A3, 0x004F86A0, IDS_HOMM_3_COMP, // Heroes III Complete - v4.0 Buka

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004EBA34, 0x005AF329, 0x00000000, 0x00000000, 0x005AF016, 0x004FFFAF, 0x0050001B, 0x005EB1B8, 0x0047AF94, 0x005AECFD, 0x005EB578, 0x00620D58, 146,
	LNG_RUSSIAN, 0x005C62E6, 0x00418A77, 0x00418B13, 0x004EC1D0, IDS_HOMM_CHR, // Heroes Chronicles Warlords & Underworld & Elements & Dragons - v1.0

	0x004EB494, 0x005AF2D9, 0x004027E9, 0x00643234, 0x005AEFC6, 0x004FF98F, 0x004FF9FB, 0x005EB1B8, 0x0047AF44, 0x005AECAD, 0x005EB578, 0x00620D80, 146,
	LNG_RUSSIAN, 0x005C62A6, 0x00418AF7, 0x00418B93, 0x004EBC30, IDS_HOMM_CHR_ROB, // Heroes Chronicles Beastmaster - v1.0

	0x004EB494, 0x005AF2D9, 0x004027E9, 0x0064322C, 0x005AEFC6, 0x004FF98F, 0x004FF9FB, 0x005EB1B8, 0x0047AF44, 0x005AECAD, 0x005EB578, 0x00620D80, 146,
	LNG_RUSSIAN, 0x005C62A6, 0x00418AF7, 0x00418B93, 0x004EBC30, IDS_HOMM_CHR_SOF, // Heroes Chronicles Sword - v1.0
#pragma endregion

// === ENG ======================================================================================================================================
#pragma region ENG
	0x004D3363, 0x0058E558, 0x00000000, 0x00000000, 0x0058E246, 0x004E568F, 0x004E56FB, 0x005B58A0, 0x004753E0, 0x0058DF2F, 0x00000000, 0x005E0D48, 50,
	LNG_ENGLISH, 0x00598885, 0x00418BD9, 0x00418C79, 0x004D3CE0, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.0

	0x0041E573, 0x004210B8, 0x00000000, 0x00000000, 0x00420DA6, 0x004179AF, 0x00417A1B, 0x005B7D50, 0x0048B7B0, 0x00420A8F, 0x00000000, 0x005E3900, 50,
	LNG_ENGLISH, 0x0059A427, 0x00438229, 0x004382C9, 0x0041EE70, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.1

	0x0041E523, 0x00421078, 0x00000000, 0x00000000, 0x00420D66, 0x0041797F, 0x004179EB, 0x005B6D40, 0x0048BB20, 0x00420A4F, 0x00000000, 0x005E2900, 50,
	LNG_ENGLISH, 0x0059A747, 0x00438119, 0x004381B9, 0x0041EE30, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.2

	0x004F58F3, 0x005D9679, 0x00000000, 0x00000000, 0x005D9366, 0x0050ABEF, 0x0050AC5B, 0x00613630, 0x0047F82C, 0x005D904D, 0x00613A94, 0x006559A0, 141,
	LNG_ENGLISH, 0x005F0846, 0x004194B7, 0x00419553, 0x004F60C0, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.3

	0x004F5583, 0x005D8F69, 0x00000000, 0x00000000, 0x005D8C56, 0x0050AAAF, 0x0050AB1B, 0x00613630, 0x0047FC4C, 0x005D893D, 0x00613A94, 0x006559B0, 141,
	LNG_ENGLISH, 0x005F0136, 0x00419587, 0x00419623, 0x004F5D50, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.4

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004EB283, 0x005EEFD8, 0x00000000, 0x00000000, 0x005EECC6, 0x0050041F, 0x0050048B, 0x00628F14, 0x0047A480, 0x005EE9AF, 0x006292C4, 0x006692D0, 90,
	LNG_ENGLISH, 0x00605F06, 0x00419299, 0x00419339, 0x004EBA50, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.0

	0x004F5C43, 0x00600299, 0x00000000, 0x00000000, 0x005FFF86, 0x0050B6AF, 0x0050B71B, 0x0063B678, 0x0048024C, 0x005FFC6D, 0x0063BADC, 0x006808F0, 141,
	LNG_ENGLISH, 0x00617476, 0x00419557, 0x004195F3, 0x004F6410, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.1

	0x004F5963, 0x005FFBF9, 0x00000000, 0x00000000, 0x005FF8E6, 0x0050B1BF, 0x0050B22B, 0x0063B678, 0x0048053C, 0x005FF5CD, 0x0063BADC, 0x006808F8, 141,
	LNG_ENGLISH, 0x00616DD6, 0x00419507, 0x004195A3, 0x004F6130, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.2

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004F7D73, 0x006027E9, 0x00000000, 0x00000000, 0x006024D6, 0x0050D93F, 0x0050D9AB, 0x0063E6C8, 0x004802EC, 0x006021BD, 0x0063EB2C, 0x00684A08, 141,
	LNG_ENGLISH, 0x006199F6, 0x00419727, 0x004197C3, 0x004F8560, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.0

	0x004F85B3, 0x006027E9, 0x00000000, 0x00000000, 0x006024D6, 0x0050DE4F, 0x0050DEBB, 0x0063E6C8, 0x004802FC, 0x006021BD, 0x0063EB2C, 0x00684A08, 141,
	LNG_ENGLISH, 0x00619A06, 0x00419707, 0x004197A3, 0x004F8DA0, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.1

	0x004F8193, 0x00602149, 0x00000000, 0x00000000, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0063D6C8, 0x0047FEAC, 0x00601B1D, 0x0063DB2C, 0x00683A10, 141,
	LNG_ENGLISH, 0x00619366, 0x00419657, 0x004196F3, 0x004F8980, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.2

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004F7B03, 0x00601B89, 0x00000000, 0x00000000, 0x00601876, 0x0050D6CF, 0x0050D73B, 0x0063D6C8, 0x0048058C, 0x0060155D, 0x0063DB2C, 0x006839C0, 141,
	LNG_ENGLISH, 0x00618D96, 0x004196F7, 0x00419793, 0x004F82F0, IDS_HOMM_3_COMP, // Heroes III Complete - v4.0

	// ==============================================================================================================================================

	0x004F0033, 0x005B5129, 0x004022FD, 0x0063B6C0, 0x005B4E16, 0x00504C8F, 0x00504CFB, 0x005EFE28, 0x0047DF5C, 0x005B4AFD, 0x005F01E4, 0x00625B78, 145,
	LNG_ENGLISH, 0x005CC0F6, 0x00419697, 0x00419733, 0x004F07F0, IDS_HOMM_CHR_WOW, // Heroes Chronicles Warlords - v1.0

	0x004F0033, 0x005B5129, 0x004022FD, 0x0063B6B8, 0x005B4E16, 0x00504C8F, 0x00504CFB, 0x005EFE28, 0x0047DF5C, 0x005B4AFD, 0x005F01E4, 0x00625B78, 145,
	LNG_ENGLISH, 0x005CC0F6, 0x00419697, 0x00419733, 0x004F07F0, IDS_HOMM_CHR_COU, // Heroes Chronicles Underworld - v1.0

	0x004EFE04, 0x005B5469, 0x004022FD, 0x0063B6C0, 0x005B5156, 0x00504B6F, 0x00504BDB, 0x005EFE28, 0x0047DA1C, 0x005B4E3D, 0x005F01E4, 0x00625B68, 145,
	LNG_ENGLISH, 0x005CC436, 0x00419687, 0x00419723, 0x004F05D0, IDS_HOMM_CHR_MOE, // Heroes Chronicles Elements - v1.0

	0x004EFE04, 0x005B5469, 0x004022FD, 0x0063B6B8, 0x005B5156, 0x00504B6F, 0x00504BDB, 0x005EFE28, 0x0047DA1C, 0x005B4E3D, 0x005F01E4, 0x00625B70, 145,
	LNG_ENGLISH, 0x005CC436, 0x00419687, 0x00419723, 0x004F05D0, IDS_HOMM_CHR_COD, // Heroes Chronicles Dragons - v1.0

	0x004EFA84, 0x005B51B9, 0x00000000, 0x00000000, 0x005B4EA6, 0x00504B6F, 0x00504BDB, 0x005EFE28, 0x0047D8EC, 0x005B4B8D, 0x005F01E4, 0x00625B70, 145,
	LNG_ENGLISH, 0x005CC186, 0x00419637, 0x004196D3, 0x004F02B0, IDS_HOMM_CHR_WT, // Heroes Chronicles WorldTree - v1.0

	0x004EF824, 0x005B5249, 0x00000000, 0x00000000, 0x005B4F36, 0x0050418F, 0x005041FB, 0x005F11A8, 0x0047D37C, 0x005B4C1D, 0x005F1564, 0x00627F60, 145,
	LNG_ENGLISH, 0x005CC226, 0x00419657, 0x004196F3, 0x004F0050, IDS_HOMM_CHR_FM, // Heroes Chronicles FieryMoon - v1.0

	0x004EF874, 0x005B4C09, 0x00401050, 0x00636868, 0x005B48F6, 0x0050460F, 0x0050467B, 0x005F11B8, 0x0047D60C, 0x005B45DD, 0x005F1574, 0x00626FA0, 145,
	LNG_ENGLISH, 0x005CBBD6, 0x004196B7, 0x00419753, 0x004F0040, IDS_HOMM_CHR_ROB, // Heroes Chronicles Beastmaster - v1.0

	0x004EF874, 0x005B4C09, 0x00401050, 0x00636878, 0x005B48F6, 0x0050460F, 0x0050467B, 0x005F11B8, 0x0047D60C, 0x005B45DD, 0x005F1574, 0x00626FA8, 145,
	LNG_ENGLISH, 0x005CBBD6, 0x004196B7, 0x00419753, 0x004F0040, IDS_HOMM_CHR_SOF, // Heroes Chronicles Sword - v1.0
#pragma endregion

// === USA ======================================================================================================================================
#pragma region USA
	0x004EF914, 0x005B51B9, 0x005AFABC, 0x0064A494, 0x005B4EA6, 0x0050459F, 0x0050460B, 0x005F11B8, 0x0047DB5C, 0x005B4B8D, 0x005F1574, 0x00627F98, 145,
	LNG_ENGLISH, 0x005CC186, 0x004196E7, 0x00419783, 0x004F00E0, IDS_HOMM_CHR_ROB, // Heroes Chronicles Beastmaster - v1.0

	0x004EF914, 0x005B51B9, 0x005AFABC, 0x0064A484, 0x005B4EA6, 0x0050459F, 0x0050460B, 0x005F11B8, 0x0047DB5C, 0x005B4B8D, 0x005F1574, 0x00627F98, 145,
	LNG_ENGLISH, 0x005CC186, 0x004196E7, 0x00419783, 0x004F00E0, IDS_HOMM_CHR_SOF, // Heroes Chronicles Sword - v1.0
#pragma endregion

// === GER ======================================================================================================================================
#pragma region GER
	0x004D5253, 0x00591B29, 0x00000000, 0x00000000, 0x00591816, 0x004E714F, 0x004E71BB, 0x005B88A0, 0x004761DC, 0x005914FD, 0x00000000, 0x005E3E68, 50,
	LNG_ENGLISH, 0x0059BF65, 0x00418B77, 0x00418C13, 0x004D5B60, IDS_HOMM_3_ROE, // Heroes III Erathia - v1.2

	// ==============================================================================================================================================

	0x004EFA04, 0x005B51C9, 0x00000000, 0x00000000, 0x005B4EB6, 0x0050475F, 0x005047CB, 0x005F11A8, 0x0047D26C, 0x005B4B9D, 0x005F1564, 0x00627FA8, 145,
	LNG_ENGLISH, 0x005CC1A6, 0x004195C7, 0x00419663, 0x004F01D0, IDS_HOMM_CHR_COD, // Heroes Chronicles Dragons - GOG - v1.0
#pragma endregion

// === FRA ======================================================================================================================================
#pragma region FRA
	0x004F61C3, 0x006003D9, 0x00000000, 0x00000000, 0x006000C6, 0x0050BA6F, 0x0050BADB, 0x0063B678, 0x004804DC, 0x005FFDAD, 0x0063BADC, 0x006809D0, 141,
	LNG_ENGLISH, 0x006175B6, 0x004195A7, 0x00419643, 0x004F6990, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.1

	0x004F8163, 0x006028F9, 0x00000000, 0x00000000, 0x006025E6, 0x0050DBBF, 0x0050DC2B, 0x0063E6D8, 0x0048073C, 0x006022CD, 0x0063EB3C, 0x00684AD0, 141,
	LNG_ENGLISH, 0x00619B26, 0x00419707, 0x004197A3, 0x004F8950, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.1
#pragma endregion

// === POL ======================================================================================================================================
#pragma region POL
	0x004F5723, 0x005FED57, 0x00000000, 0x00000000, 0x006000B6, 0x0050AFAF, 0x0050B01B, 0x0063C678, 0x0047FDBC, 0x005FFD9D, 0x0063CADC, 0x006818F0, 141,
	LNG_ENGLISH, 0x00617596, 0x00419617, 0x004196B3, 0x004F5EF0, IDS_HOMM_3_AB, // Heroes III Armageddon - v2.1

	0x004F7AF3, 0x00600ED7, 0x00000000, 0x00000000, 0x00602236, 0x0050D66F, 0x0050D6DB, 0x0063E6D8, 0x0047FF6C, 0x00601F1D, 0x0063EB3C, 0x00684A20, 141,
	LNG_ENGLISH, 0x00619766, 0x00419707, 0x004197A3, 0x004F82E0, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.1

	0x004F78D3, 0x00602179, 0x00000000, 0x00000000, 0x00601E66, 0x0050D50F, 0x0050D57B, 0x0063E6D8, 0x0047FD3C, 0x00601B4D, 0x0063EB3C, 0x00684A18, 141,
	LNG_ENGLISH, 0x00619386, 0x00419837, 0x004198D3, 0x004F80C0, IDS_HOMM_3_SOD, // Heroes III Shadow - v3.2

	0x004F5993, 0x005FE337, 0x00000000, 0x00000000, 0x005FF696, 0x0050B70F, 0x0050B77B, 0x0063B678, 0x0047FD9C, 0x005FF37D, 0x0063BADC, 0x00680900, 141,
	LNG_ENGLISH, 0x00616B86, 0x00419657, 0x004196F3, 0x004F6160, IDS_HOMM_3_AB, // Heroes III Shadow - v3.2 / Armageddon - v2.2
#pragma endregion

// ==============================================================================================================================================
#pragma region Others
	0x004F8193, 0x00602149, 0x0067FEB2, 0x00352E33, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0063D6C8, 0x0047FEAC, 0x00601B1D, 0x0063DB2C, 0x00683A10, 141,
	LNG_ENGLISH, 0x00619366, 0x00419657, 0x004196F3, 0x004F8980, IDS_HOMM_3_WOW, // Heroes III WoG - v3.52f - v3.58f

	0x004F8193, 0x00602149, 0x0067FEB2, 0x00506F4D, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0063D6C8, 0x0047FEAC, 0x00601B1D, 0x0063DB2C, 0x00683A10, 141,
	LNG_ENGLISH, 0x00619366, 0x00419657, 0x004196F3, 0x004F8980, IDS_HOMM_3_MOP, // Heroes III MoP - v3.7.2.7

	0x004F8193, 0x00602149, 0x00639C01, 0x00639BF4, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0063D6C8, 0x0047FEAC, 0x00601B1D, 0x0063DB2C, 0x00683A10, 141,
	LNG_ENGLISH, 0x00619366, 0x00419657, 0x004196F3, 0x004F8980, IDS_HOMM_3_HOA // Heroes III HotA - v1.5.3
#pragma endregion
};

namespace Hooks
{
	const AddressSpace* hookSpace;
	HOOKER hooker;
	HWND hWndMain;

	// ===============================================================
	BOOL __stdcall AdjustWindowRectHook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
	{
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

		return AdjustWindowRect(lpRect, dwStyle, bMenu);
	}

	BOOL __stdcall AdjustWindowRectExHook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
	{
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

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
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

		hWndMain = CreateWindow(lpClassName, config.title, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		if (hWndMain)
		{
			Window::SetCaptureWindow(hWndMain);

			HDC hDc = GetDC(hWndMain);
			if (hDc)
			{
				RECT rc;
				GetClientRect(hWndMain, &rc);
				FillRect(hDc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				ReleaseDC(hWndMain, hDc);
			}
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

	BOOL isModeReaded;
	INT __stdcall GetDeviceCapsHook(HDC hdc, INT index)
	{
		if (!isModeReaded)
		{
			isModeReaded = TRUE;
			ReadByte(hooker, hookSpace->bpp_address + 1, (BYTE*)&displayMode.bpp);
			ReadDWord(hooker, hookSpace->bpp_address + 2 + 1, &displayMode.height);
			ReadDWord(hooker, hookSpace->bpp_address + 2 + 5 + 1, &displayMode.width);
		}

		return index != BITSPIXEL ? GetDeviceCaps(hdc, index) : displayMode.bpp;
	}

	BOOL __stdcall GetCursorPosHook(LPPOINT lpPoint)
	{
		return TRUE;
	}

	BOOL __stdcall ScreenToClientHook(HWND hWnd, LPPOINT lpPoint)
	{
		GetCursorPos(lpPoint);
		if (ScreenToClient(hWnd, lpPoint))
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				ddraw->ScaleMouse(lpPoint);

			return TRUE;
		}

		return FALSE;
	}

	INT __stdcall ShowCursorHook(BOOL bShow) { return bShow ? 1 : -1; }

	HCURSOR __stdcall SetCursorHook(HCURSOR hCursor) { return NULL; }

	BOOL __stdcall WinHelpHook(HWND hWndMain, LPCSTR lpszHelp, UINT uCommand, ULONG_PTR dwData)
	{
		CHAR filePath[MAX_PATH];
		GetModuleFileName(GetHookerModule(hooker), filePath, MAX_PATH - 1);
		CHAR* p = StrLastChar(filePath, '\\');
		*p = NULL;
		StrCopy(p, "\\winhlp32.exe");

		FILE* file = FileOpen(filePath, "rb");
		if (file)
		{
			FileClose(file);

			SHELLEXECUTEINFO shExecInfo;
			MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFO));
			shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			shExecInfo.lpFile = filePath;
			shExecInfo.lpParameters = lpszHelp;
			shExecInfo.nShow = SW_SHOW;

			ShellExecuteEx(&shExecInfo);

			return TRUE;
		}
		else
			return WinHelp(hWndMain, lpszHelp, uCommand, dwData);
	}

	HMENU __stdcall LoadMenuHook(HINSTANCE hInstance, LPCTSTR lpMenuName)
	{
		HMENU hMenu = LoadMenu(hInstance, lpMenuName);
		if (hMenu)
		{
			HMENU hNew = LoadMenu(hDllModule, MAKEINTRESOURCE(IDM_MENU));
			if (hNew)
			{
				DWORD i, index = 0;

				HMENU hSub;
				for (DWORD i = 0; hSub = GetSubMenu(hMenu, i);)
				{
					DWORD itemId = GetMenuItemID(hSub, 0);
					if (itemId == IDM_FILE_QUIT || itemId == IDM_RES_FULL_SCREEN || itemId == IDM_HELP_MANUAL
						|| itemId == IDM_HELP_ABOUT) // for GOG releases
					{
						DeleteMenu(hMenu, i, MF_BYPOSITION);
						index = i;
					}
					else
						++i;
				}

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

				for (i = GetMenuItemCount(hNew); i; --i)
				{
					hSub = GetSubMenu(hNew, i - 1);

					GetMenuString(hNew, i - 1, buffer, sizeof(buffer), MF_BYPOSITION);
					InsertMenu(hMenu, index, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSub, buffer);
				}
			}
		}

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

	VOID __stdcall SleepHook(DWORD dwMilliseconds)
	{
		if (dwMilliseconds != 3000)
			Sleep(dwMilliseconds);
	}

	BOOL __stdcall EnumChildProc(HWND hDlg, LPARAM lParam)
	{
		if ((GetWindowLong(hDlg, GWL_STYLE) & SS_ICON) == SS_ICON)
			SendMessage(hDlg, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)config.icon);
		else
			SendMessage(hDlg, WM_SETFONT, (WPARAM)config.font, TRUE);

		return TRUE;
	}

	INT_PTR __stdcall DialogBoxParamHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND, DLGPROC lpDialogFunc, LPARAM dwInitParam)
	{
		INT_PTR res;
		DialogParams params = { hWndMain, TRUE, NULL };
		Window::BeginDialog(&params);
		{
			res = DialogBoxParam(hInstance, lpTemplateName, params.hWnd, lpDialogFunc, dwInitParam);
		}
		Window::EndDialog(&params);
		return res;
	}

	INT __stdcall MessageBoxHook(HWND, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		INT res;
		DialogParams params = { hWndMain, TRUE, NULL };
		Window::BeginDialog(&params);
		{
			res = MessageBox(params.hWnd, lpText, lpCaption, uType);
		}
		Window::EndDialog(&params);
		return res;
	}

#pragma region About dialog
	DLGPROC OldDialogProc;
	LRESULT __stdcall AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

		return OldDialogProc(hDlg, uMsg, wParam, lParam);
	}

	INT_PTR __stdcall AboutBoxParamHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
	{
		OldDialogProc = lpDialogFunc;
		return DialogBoxParamHook(hInstance, lpTemplateName, hWndParent, (DLGPROC)AboutProc, dwInitParam);
	}
#pragma endregion

#pragma region Music
	AIL_WAVEOUTOPEN AIL_waveOutOpen;
	AIL_OPEN_STREAM AIL_open_stream;
	AIL_STREAM_POSITION AIL_stream_position;
	AIL_SET_STREAM_POSITION AIL_set_stream_position;
	CHAR* audioExtList[] = { ".wav", ".mp3" };
	TrackInfo *tracksList, *trackInfo;

	DWORD __stdcall AIL_waveOutOpenHook(LPVOID driver, DWORD a1, DWORD a2, LPPCMWAVEFORMAT pcmFormat)
	{
		pcmFormat->wf.wFormatTag = WAVE_FORMAT_PCM;
		pcmFormat->wf.nChannels = 2;
		pcmFormat->wf.nSamplesPerSec = 44100;

		pcmFormat->wBitsPerSample = 16;

		pcmFormat->wf.nBlockAlign = pcmFormat->wf.nChannels * pcmFormat->wBitsPerSample / 8;
		pcmFormat->wf.nAvgBytesPerSec = pcmFormat->wf.nSamplesPerSec * pcmFormat->wf.nBlockAlign;

		return AIL_waveOutOpen(driver, a1, a2, pcmFormat);
	}

	LPVOID __fastcall OpenTrack(LPVOID driver, CHAR* group, CHAR* path, DWORD unknown)
	{
		trackInfo = tracksList;
		while (trackInfo)
		{
			if (!StrCompare(trackInfo->path, path))
				goto lbl_return;

			trackInfo = trackInfo->last;
		}

		trackInfo = (TrackInfo*)MemoryAlloc(sizeof(TrackInfo));
		trackInfo->last = tracksList;
		tracksList = trackInfo;

		trackInfo->position = 0;
		trackInfo->group = StrDuplicate(group);
		trackInfo->path = StrDuplicate(path);

	lbl_return:
		return AIL_open_stream(driver, trackInfo->path, unknown);
	}

	LPVOID __stdcall AIL_open_streamHook(LPVOID driver, CHAR* path, DWORD unknown)
	{
		if (trackInfo && !StrCompare(trackInfo->group, path))
			return AIL_open_stream(driver, trackInfo->path, unknown);

		if (StrLastChar(path, '.'))
		{
			DWORD total = 0;
			CHAR filePath[MAX_PATH];

			CHAR** extension = audioExtList;
			DWORD count = sizeof(audioExtList) / sizeof(CHAR*);
			do
			{
				StrCopy(filePath, path);
				CHAR* p = StrLastChar(filePath, '.');
				*p = NULL;

				StrCat(filePath, "*");
				StrCat(filePath, *extension);

				WIN32_FIND_DATA findData;
				MemoryZero(&findData, sizeof(WIN32_FIND_DATA));

				HANDLE hFind = FindFirstFile(filePath, &findData);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					do
						++total;
					while (FindNextFile(hFind, &findData));
					FindClose(hFind);
				}
				++extension;
			} while (--count);

			if (total)
			{
				SeedRandom(timeGetTime());
				DWORD random = total != 1 ? Random() % total : 0;

				DWORD index = 0;
				extension = audioExtList;
				count = sizeof(audioExtList) / sizeof(CHAR*);
				do
				{
					StrCopy(filePath, path);
					CHAR* p = StrLastChar(filePath, '.');
					*p = NULL;

					StrCat(filePath, "*");
					StrCat(filePath, *extension);

					WIN32_FIND_DATA findData;
					HANDLE hFind = FindFirstFile(filePath, &findData);
					if (hFind != INVALID_HANDLE_VALUE)
					{
						do
						{
							if (index++ == random)
							{
								FindClose(hFind);

								p = StrLastChar(filePath, '\\');
								*(++p) = NULL;
								StrCat(filePath, findData.cFileName);

								return OpenTrack(driver, path, filePath, unknown);
							}
						} while (FindNextFile(hFind, &findData));
						FindClose(hFind);
					}

					++extension;
				} while (--count);
			}
		}

		return OpenTrack(driver, path, path, unknown);
	}

	DWORD __stdcall AIL_stream_positionHook(LPVOID stream)
	{
		trackInfo->position = AIL_stream_position(stream);
		return trackInfo->position;
	}

	VOID __stdcall AIL_set_stream_positionHook(LPVOID stream, DWORD position)
	{
		AIL_set_stream_position(stream, trackInfo->position);
	}

#pragma endregion

	BOOL notSleep;
	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		if (PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
			return TRUE;

		if (!notSleep)
			Sleep(config.coldCPU);

		return FALSE;
	}

#pragma region Video
	HANDLE __stdcall CreateFileHook(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	{
		HANDLE hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		if (hFile && hFile != INVALID_HANDLE_VALUE)
		{
			CHAR* p = StrLastChar((CHAR*)lpFileName, '.');
			if (p && !StrCompareInsensitive(p, ".vid"))
			{
				DWORD countInFile, readed;
				if (ReadFile(hFile, &countInFile, sizeof(DWORD), &readed, NULL) && readed)
				{
					VideoFile* infoList = (VideoFile*)MemoryAlloc(sizeof(VideoFile) * countInFile);
					if (ReadFile(hFile, infoList, sizeof(VideoFile) * countInFile, &readed, NULL))
					{
						CHAR name[40];
						{
							VideoInfo* gameInfo = (VideoInfo*)(hookSpace->video_address + GetBaseOffset(hooker));
							DWORD countInGame = hookSpace->video_count;

							do
							{
								if (!gameInfo->bink)
								{
									StrCopy(name, gameInfo->fileName);
									StrCat(name, ".bik");

									VideoFile* fileInfo = infoList;

									DWORD count = countInFile;
									do
									{
										if (!StrCompareInsensitive(name, fileInfo->name))
										{
											SetFilePointer(hFile, fileInfo->stride, 0, FILE_BEGIN);
											DWORD signature;
											if (ReadFile(hFile, &signature, 4, &readed, NULL))
												gameInfo->bink = signature == 'bKIB';

											break;
										}

										++fileInfo;
									} while (--count);
								}

								++gameInfo;
							} while (--countInGame);
						}
					}
					MemoryFree(infoList);
				}

				SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			}
		}

		return hFile;
	}
#pragma endregion

#pragma region Move Hero
	DWORD moveCounter;
	VOID __stdcall CalcRunPos(DWORD* obj, DWORD idx)
	{
		if (config.smooth.move)
		{
			// legs correction
			static DWORD posIndex;
			if (posIndex % moveCounter)
			{
				DWORD* pos = &obj[128];
				if (*pos)
					--*pos;
				else
					*pos = 7;
			}

			++posIndex;

			// position correction
			if (idx == 1)
			{
				POINT* pos = (POINT*)&obj[61];
				POINT offset = { pos->x % 32, pos->y % 32 };

				if (offset.x)
				{
					LONG c = offset.x > 0 ? 32 : -32;
					LONG d = (c - offset.x) / 2;
					pos->x -= c % d;
				}

				if (offset.y)
				{
					LONG c = offset.y > 0 ? 32 : -32;
					LONG d = (c - offset.y) / 2;
					pos->y -= c % d;
				}
			}
		}
	}

	DWORD sub_DrawSizedRect_1;
	VOID __declspec(naked) hook_0048017C()
	{
		__asm
		{
			PUSH ECX
			PUSH EAX

			PUSH EDI
			PUSH ESI
			CALL CalcRunPos

			POP EAX
			POP ECX
			JMP sub_DrawSizedRect_1
		}
	}
#pragma endregion

	DWORD cursorTime = 16;
	VOID __fastcall CheckRefreshRate()
	{
		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);

		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && devMode.dmDisplayFrequency)
			cursorTime = 1000 / devMode.dmDisplayFrequency;
		else
			cursorTime = 16;

		// ==========================================================
		PatchByte(hooker, hookSpace->cursor_time_1 + 2, (BYTE)cursorTime);
		PatchByte(hooker, hookSpace->cursor_time_2 + 2, (BYTE)cursorTime);
		PatchDWord(hooker, hookSpace->cursor_time_2 + 5 + 1, cursorTime);

		// ==========================================================
		if (config.smooth.move)
		{
			moveCounter = DWORD(50.0f / cursorTime);

			FLOAT dist = 0.02f * cursorTime;
			MoveObject moveObject = {
				2, DWORD(dist * 8), DWORD(dist * 10), DWORD(dist * 16), 32,
				100, cursorTime, cursorTime, cursorTime, 100,
				-32, 0, 32
			};

			PatchBlock(hooker, hookSpace->move_object, &moveObject, sizeof(MoveObject));
		}
		else
		{
			const MoveObject moveObject = {
				2, 8, 10, 16, 32,
				100, 50, 50, 50, 100,
				-32, 0, 32
			};

			PatchBlock(hooker, hookSpace->move_object, (VOID*)&moveObject, sizeof(MoveObject));
		}
		// ==========================================================
	}

#pragma region Move Map
	struct MapPosition {
		SHORT x;
		SHORT y;
	} oldCenter;

	DWORD sub_SetMapCenter, sub_DrawSizedRect_2;

	VOID __declspec(naked) hook_00419707()
	{
		__asm
		{
			MOV EAX, [ESI+0xE4]
			MOV oldCenter, EAX
			RETN
		}
	}

	VOID __stdcall DrawMapRect(DWORD mapObject, DWORD object, RECT rc)
	{
		MapPosition* newCenter = (MapPosition*)(mapObject + 228);
		POINT newc = { SHORT(newCenter->x << 6) >> 6, SHORT(newCenter->y << 6) >> 6 };
		LONG newUnk = SHORT(newCenter->y << 2) >> 12;

		if (config.smooth.scroll)
		{
			POINT oldc = { SHORT(oldCenter.x << 6) >> 6, SHORT(oldCenter.y << 6) >> 6 };
			LONG oldUnk = SHORT(oldCenter.y << 2) >> 12;

			POINT speed = { newc.x - oldc.x, newc.y - oldc.y };
			if (speed.x || speed.y)
			{
				DWORD sleep = timeGetTime();

				POINTFLOAT start = { (FLOAT)speed.x, (FLOAT)speed.y };
				POINTFLOAT offset = { (FLOAT)start.x, (FLOAT)start.y };

				FLOAT mult = (FLOAT)cursorTime / 70;
				POINTFLOAT diff = { 0.0f, 0.0f };
				POINT step = { 0, 0 };

				if (speed.x)
				{
					if (speed.x > 0)
						step.x = 1;
					else
						step.x = -1;

					start.x += step.x;
					diff.x = mult * speed.x;
				}

				if (speed.y)
				{
					if (speed.y > 0)
						step.y = 1;
					else
						step.y = -1;

					start.y += step.y;
					diff.y = mult * speed.y;
				}

				POINT* shift = (POINT*)(mapObject + 244);
				POINT pos = oldc;
				do
				{
					if (speed.x)
					{
						offset.x -= diff.x;

						if (speed.x > 0)
						{
							if (offset.x < 0.0f)
								break;
						}
						else
						{
							if (offset.x > 0.0f)
								break;
						}

						pos.x = oldc.x + LONG(start.x - offset.x);
						shift->x = (LONG)(offset.x * 32.0f) % 32;
					}

					if (speed.y)
					{
						offset.y -= diff.y;

						if (speed.y > 0)
						{
							if (offset.y < 0.0f)
								break;
						}
						else
						{
							if (offset.y > 0.0f)
								break;
						}

						pos.y = oldc.y + LONG(start.y - offset.y);
						shift->y = (LONG)(offset.y * 32.0f) % 32;
					}

					((VOID(__thiscall*)(DWORD, POINT, LONG, DWORD, DWORD))sub_SetMapCenter)(mapObject, pos, newUnk, 0, 1);
					((VOID(__thiscall*)(DWORD, RECT))sub_DrawSizedRect_2)(object, rc);

					notSleep = TRUE;
					{
						sleep += cursorTime;
						((VOID(__thiscall*)(DWORD))hookSpace->move_lifeCycle)(sleep);
					}
					notSleep = FALSE;
				} while (TRUE);

				if (shift->x || shift->y || pos.x != newc.x || pos.x != newc.x)
				{
					*shift = { 0, 0 };

					((VOID(__thiscall*)(DWORD, POINT, LONG, DWORD, DWORD))sub_SetMapCenter)(mapObject, newc, newUnk, 0, 1);
					((VOID(__thiscall*)(DWORD, RECT))sub_DrawSizedRect_2)(object, rc);
				}
			}
			else
			{
				((VOID(__thiscall*)(DWORD, POINT, LONG, DWORD, DWORD))sub_SetMapCenter)(mapObject, newc, newUnk, 0, 1);
				((VOID(__thiscall*)(DWORD, RECT))sub_DrawSizedRect_2)(object, rc);
			}
		}
		else
		{
			((VOID(__thiscall*)(DWORD, POINT, LONG, DWORD, DWORD))sub_SetMapCenter)(mapObject, newc, newUnk, 0, 1);
			((VOID(__thiscall*)(DWORD, RECT))sub_DrawSizedRect_2)(object, rc);
		}
	}

	VOID __declspec(naked) hook_004197A3()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH ESI
			PUSH EAX
			JMP DrawMapRect
		}
	}

	VOID __stdcall FakeSetCenter(LONG, LONG, LONG, DWORD, DWORD)
	{
	}
#pragma endregion

#pragma region Registry
	LSTATUS __stdcall RegCreateKeyHook(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
	{
		return config.isExist ? ERROR_SUCCESS : RegCreateKey(hKey, lpSubKey, phkResult);
	}

	LSTATUS __stdcall RegOpenKeyExHook(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
	{
		return config.isExist ? ERROR_SUCCESS : RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	}

	LSTATUS __stdcall RegCloseKeyHook(HKEY hKey)
	{
		return config.isExist ? ERROR_SUCCESS : RegCloseKey(hKey);
	}

	LSTATUS __stdcall RegQueryValueExHook(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
	{
		DWORD size = *lpcbData;

		if (!config.isExist)
		{
			LSTATUS res = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

			if (size == sizeof(DWORD))
				Config::Set(CONFIG_APP, lpValueName, *(INT*)lpData);
			else
				Config::Set(CONFIG_APP, lpValueName, (CHAR*)lpData);

			return res;
		}
		else
		{
			if (size == sizeof(DWORD))
			{
				*(INT*)lpData = Config::Get(CONFIG_APP, lpValueName, *(INT*)lpData);
				if (lpType)
					*lpType = REG_DWORD;
			}
			else
			{
				Config::Get(CONFIG_APP, lpValueName, "", (CHAR*)lpData, *lpcbData);
				if (lpType)
					*lpType = *lpcbData ? REG_BINARY : REG_SZ;
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

	DOUBLE __cdecl StrToDoubleHook(const CHAR* str)
	{
		BOOL isComma = FALSE;
		DWORD length = 0;
		const CHAR* src = str;
		while (*src)
		{
			if (*src == ',')
				isComma = TRUE;
			++length;
			++src;
		}

		if (!isComma)
			return StrToDouble(str);

		CHAR stack[64];
		CHAR* buffer = stack;
		if (length >= 64)
			buffer = (CHAR*)malloc(length + 1);

		src = str;
		CHAR* dst = buffer;
		while (*src)
		{
			CHAR c = *src++;
			*dst++ = c != ',' ? c : '.';
		}

		buffer[length] = NULL;

		DOUBLE res = StrToDouble(buffer);

		if (length >= 64)
			free(buffer);

		return res;
	}

	BOOL __stdcall GetDiskFreeSpaceHook(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
	{
		if (GetDiskFreeSpace(lpRootPathName, lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters))
		{
			LARGE_INTEGER total;
			total.QuadPart = (LONGLONG)*lpSectorsPerCluster * (LONGLONG)*lpBytesPerSector * (LONGLONG)*lpNumberOfFreeClusters;

			*lpSectorsPerCluster = 1;
			*lpBytesPerSector = 1;
			*lpNumberOfFreeClusters = !total.HighPart ? total.LowPart : 0xFFFFFFFF;

			return TRUE;
		}

		return FALSE;
	}

#pragma region Start bonus fix
	VOID __declspec(naked) hook_00457CC9()
	{
		__asm {
			mov eax, [esp+0x4] // bones cell index
			test eax,eax
			jge lbl_cont
			xor eax,eax
			lbl_cont: mov ecx, [ecx+0x8]
			mov eax, [eax*0x8+ecx]
			retn 4
		}
	}
#pragma endregion

#pragma optimize("s", on)
	BOOL Load()
	{
		const AddressSpace* defaultSpace = NULL;
		const AddressSpace* equalSpace = NULL;

		hooker = CreateHooker(GetModuleHandle(NULL));

		hookSpace = addressArray;
		DWORD hookCount = sizeof(addressArray) / sizeof(AddressSpace);
		do
		{
			DWORD check1, check2, equal;
			if (ReadDWord(hooker, hookSpace->check_1 + 1, &check1) && check1 == STYLE_FULL_OLD && ReadDWord(hooker, hookSpace->check_2 + 1, &check2) && check2 == STYLE_FULL_OLD)
			{
				if (!hookSpace->equal_address)
					defaultSpace = hookSpace;
				else if (ReadDWord(hooker, hookSpace->equal_address, &equal) && equal == hookSpace->equal_value)
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
			Config::Load(GetHookerModule(hooker), hookSpace);

			HOOKER user = CreateHooker(GetModuleHandle("USER32.dll"));
			{
				PatchExport(user, "PeekMessageA", PeekMessageHook);
				PatchExport(user, "MessageBoxA", MessageBoxHook);
				PatchExport(user, "DialogBoxParamA", DialogBoxParamHook);
			}
			ReleaseHooker(user);

			{
				PatchImportByName(hooker, "PeekMessageA", PeekMessageHook);
				PatchImportByName(hooker, "MessageBoxA", MessageBoxHook);
				PatchImportByName(hooker, "DialogBoxParamA", AboutBoxParamHook);

				PatchImportByName(hooker, "WinHelpA", WinHelpHook);

				PatchImportByName(hooker, "LoadMenuA", LoadMenuHook);
				PatchImportByName(hooker, "SetMenu", SetMenuHook);
				PatchImportByName(hooker, "EnableMenuItem", EnableMenuItemHook);

				PatchImportByName(hooker, "Sleep", SleepHook);

				PatchImportByName(hooker, "RegCreateKeyA", RegCreateKeyHook);
				PatchImportByName(hooker, "RegOpenKeyExA", RegOpenKeyExHook);
				PatchImportByName(hooker, "RegCloseKey", RegCloseKeyHook);
				PatchImportByName(hooker, "RegQueryValueExA", RegQueryValueExHook);
				PatchImportByName(hooker, "RegSetValueExA", RegSetValueExHook);

				PatchImportByName(hooker, "GetDiskFreeSpaceA", GetDiskFreeSpaceHook);

				PatchImportByName(hooker, "DirectDrawCreate", Main::DirectDrawCreate);

				PatchImportByName(hooker, "_AIL_waveOutOpen@16", AIL_waveOutOpenHook, (DWORD*)&AIL_waveOutOpen);
				PatchImportByName(hooker, "_AIL_stream_position@4", AIL_stream_positionHook, (DWORD*)&AIL_stream_position);
				PatchImportByName(hooker, "_AIL_open_stream@12", AIL_open_streamHook, (DWORD*)&AIL_open_stream);
				PatchImportByName(hooker, "_AIL_set_stream_position@8", AIL_set_stream_positionHook, (DWORD*)&AIL_set_stream_position);

				PatchImportByName(hooker, "CreateFileA", CreateFileHook);

				if (!config.isDDraw)
				{
					PatchImportByName(hooker, "AdjustWindowRect", AdjustWindowRectHook);
					PatchImportByName(hooker, "AdjustWindowRectEx", AdjustWindowRectExHook);
					PatchImportByName(hooker, "CreateWindowExA", CreateWindowExHook);
					PatchImportByName(hooker, "SetWindowLongA", SetWindowLongHook);

					PatchImportByName(hooker, "GetDeviceCaps", GetDeviceCapsHook);
					PatchImportByName(hooker, "GetCursorPos", GetCursorPosHook);
					PatchImportByName(hooker, "ScreenToClient", ScreenToClientHook);
					PatchImportByName(hooker, "ShowCursor", ShowCursorHook);
					PatchImportByName(hooker, "SetCursor", SetCursorHook);
				}

				UnmapFile(hooker);
			}

			if (!config.isDDraw)
				PatchNop(hooker, hookSpace->renderNop, 5); // prevent on WM_PAINT

			// Smooth hero move
			sub_DrawSizedRect_1 = RedirectCall(hooker, hookSpace->move_address, hook_0048017C);

			// Smooth map move
			{
				PatchCall(hooker, hookSpace->move_oldCenter, hook_00419707, 2);
				sub_SetMapCenter = RedirectCall(hooker, hookSpace->move_drawRect - 25, FakeSetCenter);
				sub_DrawSizedRect_2 = RedirectCall(hooker, hookSpace->move_drawRect, hook_004197A3);
			}

			// replace ',' by '.' for atof
			PatchHook(hooker, hookSpace->atof, StrToDoubleHook);

			// Start scenario bonus selection fix
			if (hookSpace->start_bonus_fix)
				PatchDWord(hooker, hookSpace->start_bonus_fix, (DWORD)hook_00457CC9);

			// Reset video table
			{
				VideoInfo* gameInfo = (VideoInfo*)(hookSpace->video_address + GetBaseOffset(hooker));
				DWORD countInGame = hookSpace->video_count;
				do
				{
					gameInfo->bink = FALSE;
					++gameInfo;
				} while (--countInGame);
			}

			return TRUE;
		}

		return FALSE;
	}
#pragma optimize("", on)
}