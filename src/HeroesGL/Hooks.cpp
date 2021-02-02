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
#include "shlobj.h"
#include "Shellapi.h"
#include "mmsystem.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"
#include "Hooker.h"
#include "Mods.h"

#define STYLE_FULL_OLD (WS_VISIBLE | WS_CLIPSIBLINGS)
#define STYLE_FULL_NEW (WS_VISIBLE | WS_CLIPSIBLINGS | WS_SYSMENU | WS_POPUP)

const AddressSpace addressArray[] = {
#pragma region Heroes I Rus
	0x00442EFF, 1, 0x00479B13, MAKEINTRESOURCE(109), LNG_RUSSIAN, 0x0046827C, 0x004684AA, 0x0046833C, 0x004677A0, 0x00000000, 0x004A98D0, 0x0046F8EC, 0x38, // Heroes I - Buka
	(AppSettings*)0x004A9440, 0x00000000, 0x00000000,
	0x004A98EC, 0x0043DE33, 0x004CF438, 0x004CE178, 0x004CE880, 0x004CEF88, 0x004CF0B4, 0x004CF30C, 0x00000000, 0x004463EF, 0x004682F5, 0x0046FB39, 0x0046FB52, 0x0046FBBE, 0x0046FCAE, 0x0046FD2E,
	0x00000000, 0x00000000, IDS_HOMM_1,
	0x0040CBFA, 1, 0x0042A1D6, MAKEINTRESOURCE(109), LNG_RUSSIAN, 0x0041AFC2, 0x0041B1EA, 0x0041B082, 0x0041A51D, 0x00000000, 0x004528B0, 0x0042170C, 0x38, // Heroes I Editor - Buka
	(AppSettings*)0x00452490, 0x00000000, 0x00000000,
	0x004528A4, 0x00000000, 0x00454710, 0x00453450, 0x00453B58, 0x00454260, 0x0045438C, 0x004545E4, 0x00000000, 0x00408650, 0x0041B03B, 0x00421959, 0x00421972, 0x004219DE, 0x00421ACE, 0x00421B4E,
	0x00000000, 0x00000000, IDS_EDITOR_1,
#pragma endregion

#pragma region Heroes II Rus
	0x004710BE, 2, 0x004D8619, "HEROES", LNG_RUSSIAN, 0x004B193C, 0x004B1BB8, 0x004B1A25, 0x004B0E4D, 0x004BD200, 0x00526634, 0x004D468C, 0x38, // Heroes II Gold - Buka
	(AppSettings*)0x005261FC, 0x00497BD5, 0x00497B38,
	0x00000000, 0x00000000, 0x00535758, 0x005349D0, 0x00000000, 0x005352D4, 0x00535454, 0x00000000, 0x004BC572, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_HOMM_2_POL,
	0x0041535E, 2, 0x0044B923, "EDITOR", LNG_RUSSIAN, 0x0042CC58, 0x0042CED4, 0x0042CD41, 0x0042C19D, 0x00431530, 0x004A49F0, 0x00447E6C, 0x38, // Heroes II Gold Editor - Buka
	(AppSettings*)0x004A4670, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x004A6C00, 0x004A5E78, 0x00000000, 0x004A677C, 0x004A68FC, 0x00000000, 0x004308A2, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,
#pragma endregion

#pragma region Heroes I Pol
	// ----------------------------------------------------------------------------
	0x004436E6, 1, 0x0047A873, "HEROES", LNG_POLISH, 0x00468A69, 0x00468C97, 0x00468B29, 0x00467F8D, 0x00000000, 0x004A4FD8, 0x00470CDC, 0x38, // Heroes I v1.1
	(AppSettings*)0x004A4B40, 0x00000000, 0x00000000,
	0x004A4FF4, 0x0043E78A, 0x004C9E78, 0x004C8BB8, 0x004C92C0, 0x004C99C8, 0x004C9AF4, 0x004C9D4C, 0x00000000, 0x00446F0F, 0x00468AE2, 0x00470F4B, 0x00470F64, 0x00470FD0, 0x004710C0, 0x00471140,
	0x00000000, 0x00000000, IDS_HOMM_1,

	0x0040D866, 1, 0x0042BBCF, "EDITOR", LNG_POLISH, 0x0041BBD2, 0x0041BE00, 0x0041BC92, 0x0041B12D, 0x00000000, 0x0044EE70, 0x0042205C, 0x38, // Heroes I Editor v1.0
	(AppSettings*)0x0044EA30, 0x00000000, 0x00000000,
	0x0044EE64, 0x00000000, 0x00450CE0, 0x0044FA20, 0x00450128, 0x00450830, 0x0045095C, 0x00450BB4, 0x00000000, 0x004091A0, 0x0041BC4B, 0x004222CB, 0x004222E4, 0x00422350, 0x00422440, 0x004224C0,
	0x00000000, 0x00000000, IDS_EDITOR_1,
#pragma endregion

#pragma region Heroes II Pol
	0x00470C7C, 2, 0x004D22D9, "HEROES", LNG_POLISH, 0x004B0B7B, 0x004B0E3A, 0x004B0C90, 0x004B0040, 0x004BFCF0, 0x0051E2AC, 0x004CF172, 0x38, // Heroes II v2.1
	(AppSettings*)0x0051DE64, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x0052D928, 0x0052CBA0, 0x00000000, 0x0052D4A4, 0x0052D624, 0x00000000, 0x004BEEE2, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x004B52E4, 0x00000000, IDS_HOMM_2_POL,
	0x00414E0E, 2, 0x00441AC8, "EDITOR", LNG_POLISH, 0x0042C25B, 0x0042C4C9, 0x0042C342, 0x0042B7B0, 0x0042FE30, 0x00495F48, 0x0043FE02, 0x38, // Heroes II Editor v2.1
	(AppSettings*)0x00495B9C, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00498160, 0x004973D8, 0x00000000, 0x00497CDC, 0x00497E5C, 0x00000000, 0x0042F052, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,
#pragma endregion

#pragma region Heroes I Eng
	// ----------------------------------------------------------------------------
	0x0045B9AA, 1, 0x0048280B, "HEROES", LNG_ENGLISH, 0x00405547, 0x0040583D, 0x0040566A, 0x0040490E, 0x00000000, 0x00492E08, 0x004734D3, 0x40, // Heroes I v1.0
	(AppSettings*)0x004C6AC0, 0x00000000, 0x00000000,
	0x00492E28, 0x00450FE0, 0x004CBF58, 0x004CADB8, 0x004CB720, 0x004CB4C0, 0x004CC538, 0x004CAC88, 0x00000000, 0x004644E7, 0x004055FF, 0x004736D4, 0x004736E9, 0x0047372C, 0x004737C1, 0x004737FE,
	0x00000000, 0x00000000, IDS_HOMM_1,
	0x0041C62A, 1, 0x00429CBB, "EDITOR", LNG_ENGLISH, 0x0041BF70, 0x0041C266, 0x0041C093, 0x0041B398, 0x00000000, 0x00436E6C, 0x00421743, 0x40, // Heroes I Editor v1.0
	(AppSettings*)0x004480C0, 0x00000000, 0x00000000,
	0x00436E60, 0x00000000, 0x0044A6C8, 0x00449528, 0x00449E90, 0x00449C30, 0x0044ACA8, 0x004493F8, 0x00000000, 0x00409ADC, 0x0041C028, 0x00421964, 0x00421979, 0x004219BC, 0x00421A51, 0x00421A8E,
	0x00000000, 0x00000000, IDS_EDITOR_1,

	0x0045BAAA, 1, 0x00482DDB, "HEROES", LNG_ENGLISH, 0x00405547, 0x0040583D, 0x0040566A, 0x0040490E, 0x00000000, 0x00492E08, 0x00473AA3, 0x40, // Heroes I v1.1
	(AppSettings*)0x004C6F70, 0x00000000, 0x00000000,
	0x00492E28, 0x004510E1, 0x004CC408, 0x004CB268, 0x004CBBD0, 0x004CB970, 0x004CC9E8, 0x004CB138, 0x00000000, 0x00464607, 0x004055FF, 0x00473CA4, 0x00473CB9, 0x00473CFC, 0x00473D91, 0x00473DCE,
	0x00000000, 0x00000000, IDS_HOMM_1,
	0x0041C64A, 1, 0x00429D6B, "EDITOR", LNG_ENGLISH, 0x0041BF90, 0x0041C286, 0x0041C0B3, 0x0041B3B8, 0x00000000, 0x00436E6C, 0x00421773, 0x40, // Heroes I Editor v1.1
	(AppSettings*)0x004482A0, 0x00000000, 0x00000000,
	0x00436E60, 0x00000000, 0x0044A8A8, 0x00449708, 0x0044A070, 0x00449E10, 0x0044AE88, 0x004495D8, 0x00000000, 0x00409AEC, 0x0041C048, 0x00421994, 0x004219A9, 0x004219EC, 0x00421A81, 0x00421ABE,
	0x00000000, 0x00000000, IDS_EDITOR_1,

	0x00432B2A, 1, 0x004818BB, "HEROES", LNG_ENGLISH, 0x00436E67, 0x0043715D, 0x00436F8A, 0x0043622E, 0x00000000, 0x00492C20, 0x00475CC3, 0x40, // Heroes I v1.2
	(AppSettings*)0x004A9B88, 0x00000000, 0x00000000,
	0x00492C40, 0x00422571, 0x004CEB30, 0x004CD860, 0x004CDF68, 0x004CE670, 0x004CE7A0, 0x004CEA00, 0x00000000, 0x00439EA7, 0x00436F1F, 0x00475EC4, 0x00475ED9, 0x00475F1C, 0x00475FB1, 0x00475FEE,
	0x00000000, 0x00000000, IDS_HOMM_1,
	0x004012BA, 1, 0x00428F3B, "EDITOR", LNG_ENGLISH, 0x004113AF, 0x004116A5, 0x004114D2, 0x004107D8, 0x00000000, 0x0043967C, 0x00420CD3, 0x40, // Heroes I Editor v1.2
	(AppSettings*)0x0044A8B8, 0x00000000, 0x00000000,
	0x00439670, 0x00000000, 0x0044C2F0, 0x0044B020, 0x0044B728, 0x0044BE30, 0x0044BF60, 0x0044C1C0, 0x00000000, 0x0041AD4C, 0x00411467, 0x00420EF4, 0x00420F09, 0x00420F4C, 0x00420FE1, 0x0042101E,
	0x00000000, 0x00000000, IDS_EDITOR_1,
#pragma endregion

#pragma region Heroes II Eng
	// ----------------------------------------------------------------------------
	0x00409017, 2, 0x004D466B, "HEROES", LNG_ENGLISH, 0x004B32A0, 0x004B35F5, 0x004B33E8, 0x004B262A, 0x004BF5A0, 0x004F1FD0, 0x004CDF95, 0x3C, // Heroes II v1.0
	(AppSettings*)0x0051F344, 0x00472D87, 0x00472D33,
	0x00000000, 0x00000000, 0x00522650, 0x005218C8, 0x00000000, 0x005221C8, 0x00522350, 0x00000000, 0x004BEBF4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_HOMM_2_SW,
	0x004028B7, 2, 0x0043B25B, "EDITOR", LNG_ENGLISH, 0x00422160, 0x004224AF, 0x004222A8, 0x00421546, 0x00432040, 0x00446D9C, 0x00439135, 0x3C, // Heroes II Editor v1.0
	(AppSettings*)0x00481740, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00487E10, 0x00487088, 0x00000000, 0x00487988, 0x00487B10, 0x00000000, 0x004316B4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,

	0x00408FE7, 2, 0x004D5B5B, "HEROES", LNG_ENGLISH, 0x004B398B, 0x004B3CF2, 0x004B3AD3, 0x004B2D00, 0x004BFD70, 0x004EFE10, 0x004CE8B5, 0x3C, // Heroes II v1.1
	(AppSettings*)0x0051C2AC, 0x0047300C, 0x00472FB8,
	0x00000000, 0x00000000, 0x00520520, 0x0051F780, 0x00000000, 0x00520098, 0x00520CA0, 0x00000000, 0x004BF3C4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_HOMM_2_SW,
	0x00402987, 2, 0x0043FD1B, "EDITOR", LNG_ENGLISH, 0x0042213B, 0x0042249B, 0x00422283, 0x0042150C, 0x00435F00, 0x00449D9C, 0x0043CF55, 0x3C, // Heroes II Editor v1.1
	(AppSettings*)0x00484290, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x0048AF80, 0x0048A1E0, 0x00000000, 0x0048AAF8, 0x0048B700, 0x00000000, 0x00435574, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,

	0x0045DE87, 2, 0x004D5CDB, "HEROES", LNG_ENGLISH, 0x004A39EB, 0x004A3D52, 0x004A3B33, 0x004A2D60, 0x004C3440, 0x004EE960, 0x004D2C65, 0x3C, // Heroes II v1.2
	(AppSettings*)0x0051B464, 0x0042017C, 0x00420128,
	0x00000000, 0x00000000, 0x00520360, 0x0051F5C0, 0x00000000, 0x0051FED8, 0x00520AE0, 0x00000000, 0x004C2A94, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_HOMM_2_SW,
	0x00426667, 2, 0x0043FE8B, "EDITOR", LNG_ENGLISH, 0x0042194B, 0x00421CAC, 0x00421A93, 0x00420D1C, 0x004368F0, 0x0046753C, 0x0043CA45, 0x3C, // Heroes II Editor v1.2 - v1.3
	(AppSettings*)0x00488720, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x0048AEE0, 0x0048A140, 0x00000000, 0x0048AA58, 0x0048B660, 0x00000000, 0x00435F64, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,

	0x00434317, 2, 0x004D47DB, "HEROES", LNG_ENGLISH, 0x00437447, 0x004377AE, 0x0043758F, 0x004367BA, 0x004C5AF0, 0x004F1F38, 0x004CF745, 0x3C, // Heroes II v1.3
	(AppSettings*)0x00520E84, 0x00000000, 0x004B9AF1,
	0x00000000, 0x00000000, 0x00522458, 0x005216D0, 0x00000000, 0x00521FD0, 0x00522158, 0x00000000, 0x004C5144, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x004BEA6F, 0x004BF28E, IDS_HOMM_2_SW,

	0x0041BFB7, 2, 0x004E00AB, "HEROES", LNG_ENGLISH, 0x0043733B, 0x004376A2, 0x00437483, 0x004366B0, 0x004CE990, 0x004FBB30, 0x004D85C5, 0x3C, // Heroes II v2.0
	(AppSettings*)0x00528D3C, 0x00488B3F, 0x00488AEB,
	0x00000000, 0x00000000, 0x00533FE0, 0x00533240, 0x00000000, 0x00533B58, 0x00534760, 0x00000000, 0x004CDFD4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_HOMM_2_POL,
	0x0040FCD7, 2, 0x004412EB, "EDITOR", LNG_ENGLISH, 0x0042CA64, 0x0042CDB2, 0x0042CBAC, 0x0042BE4C, 0x00434350, 0x0046C29C, 0x00439E55, 0x3C, // Heroes II Editor v2.0
	(AppSettings*)0x0048EA28, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00490288, 0x0048F4E8, 0x00000000, 0x0048FE00, 0x00490A08, 0x00000000, 0x004339C4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, IDS_EDITOR_2,

	0x00484DC1, 2, 0x004DF27B, "HEROES", LNG_ENGLISH, 0x0049CEC3, 0x0049D26D, 0x0049D030, 0x0049C1E0, 0x004D1480, 0x004F2E78, 0x004D88B5, 0x3C, // Heroes II v2.1
	(AppSettings*)0x0052485C, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00533E40, 0x005330B8, 0x00000000, 0x005339B8, 0x00533B40, 0x00000000, 0x004D0AD4, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00402F7F, 0x004037BE, IDS_HOMM_2_POL,
	0x00423EF7, 2, 0x0044063B, "EDITOR", LNG_ENGLISH, 0x004187CE, 0x00418B1D, 0x00418916, 0x00417BB6, 0x00437C90, 0x0046AE2C, 0x0043F1F5, 0x3C, // Heroes II Editor v2.1
	(AppSettings*)0x0048EE40, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00491040, 0x004902B8, 0x00000000, 0x00490BB8, 0x00490D40, 0x00000000, 0x00437304, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00438A5F, 0x00000000, IDS_EDITOR_2
#pragma endregion
};

namespace Hooks
{
	const AddressSpace* hookSpace;
	HWND hWndMain;

#pragma region Fix paint rectangle on VM
	RECT rcPaint;

	VOID __fastcall CopyInvalidRect(DWORD* src)
	{
		rcPaint = { (LONG)src[2], (LONG)src[3], LONG(src[2] + src[0]), LONG(src[3] + src[1]) };

		RECT rc = { 0, 0, 1, 1 };
		InvalidateRect(hWndMain, &rc, FALSE);
	}

	DWORD invalidEsp;
	VOID __declspec(naked) hook_InvalidateRect()
	{
		_asm {
			mov ecx, invalidEsp
			lea ecx, [esp + ecx]
			call CopyInvalidRect
			retn 0xC
		}
	}

	HDC __stdcall BeginPaintHook(HWND hWnd, LPPAINTSTRUCT lpPaint)
	{
		config.update.rect = rcPaint;
		MemoryZero(&rcPaint, sizeof(RECT));

		if (config.update.rect.left < 0)
			config.update.rect.left = 0;
		if (config.update.rect.right > RES_WIDTH)
			config.update.rect.right = RES_WIDTH;
		if (config.update.rect.right - config.update.rect.left < 1)
			config.update.rect.right = 0;

		if (config.update.rect.top < 0)
			config.update.rect.top = 0;
		if (config.update.rect.bottom > RES_HEIGHT)
			config.update.rect.bottom = RES_HEIGHT;
		if (config.update.rect.bottom - config.update.rect.top < 1)
			config.update.rect.bottom = 0;

		HDC hDc = BeginPaint(hWnd, lpPaint);
		lpPaint->rcPaint = { 0, 0, 1, 1 };
		return hDc;
	}
#pragma endregion

#pragma region Set Full Screen
	VOID RepaintWindow()
	{
		rcPaint = { 0, 0, RES_WIDTH, RES_HEIGHT };

		RECT rc = { 0, 0, 1, 1 };
		InvalidateRect(hWndMain, &rc, FALSE);

		UpdateWindow(hWndMain);
	}

	DWORD ddSetFullScreenStatus;
	VOID __cdecl SwitchMode_v1(DWORD isFullscreen)
	{
		((VOID(__cdecl*)(DWORD))ddSetFullScreenStatus)(isFullscreen);
		RepaintWindow();
	}

	DWORD checkChangeCursor;
	VOID __fastcall SwitchMode_v2(DWORD isFullscreen)
	{
		((VOID(__fastcall*)(DWORD))ddSetFullScreenStatus)(isFullscreen);
		((VOID(__fastcall*)(DWORD, DWORD, BOOL))checkChangeCursor)(0, 0, TRUE);
		RepaintWindow();
	}
#pragma endregion

	BOOL __stdcall AdjustWindowRectHook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
	{
		if (config.cursor.fix && !config.isDDraw)
			Hooks::ScalePointer((FLOAT)(lpRect->right - lpRect->left + 1) / (FLOAT)RES_WIDTH, (FLOAT)(lpRect->bottom - lpRect->top + 1) / (FLOAT)RES_HEIGHT);

		if (dwStyle == STYLE_FULL_OLD)
			dwStyle = STYLE_FULL_NEW;

		return AdjustWindowRect(lpRect, dwStyle, bMenu);
	}

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		if (dwStyle == STYLE_FULL_OLD)
			dwStyle = STYLE_FULL_NEW;

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

			Mods::SetHWND(hWndMain);
		}

		return hWndMain;
	}

	LONG __stdcall SetWindowLongHook(HWND hWnd, INT nIndex, LONG dwNewLong)
	{
		if (dwNewLong == STYLE_FULL_OLD)
			dwNewLong = STYLE_FULL_NEW;

		return SetWindowLong(hWnd, nIndex, dwNewLong);
	}

	BOOL __stdcall ScreenToClientHook(HWND hWnd, LPPOINT lpPoint)
	{
		if (ScreenToClient(hWnd, lpPoint))
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				ddraw->ScaleMouse(lpPoint);

			return TRUE;
		}

		return FALSE;
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

	LPITEMIDLIST __stdcall SHBrowseForFolderHook(BROWSEINFO* info)
	{
		LPITEMIDLIST res;
		DialogParams params = { hWndMain, TRUE, NULL };
		Window::BeginDialog(&params);
		{
			res = SHBrowseForFolder(info);
		}
		Window::EndDialog(&params);
		return res;
	}

	BOOL __stdcall WinHelpHook(HWND hWndMain, LPCSTR lpszHelp, UINT uCommand, ULONG_PTR dwData)
	{
		CHAR filePath[MAX_PATH];
		GetModuleFileName(NULL, filePath, MAX_PATH - 1);
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
				for (i = 0; hSub = GetSubMenu(hMenu, i);)
				{
					DWORD itemId = GetMenuItemID(hSub, 0);
					if (itemId == IDM_RES_640_480 || itemId == IDM_HELP_MANUAL)
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

				for (DWORD j = 0; hSub = GetSubMenu(hNew, j); ++j)
				{
					GetMenuString(hNew, j, buffer, sizeof(buffer), MF_BYPOSITION);
					InsertMenu(hMenu, i + j, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSub, buffer);
				}

				Window::CheckMenu(hMenu);

				mData.childId = IDM_MODS;
				if (Window::GetMenuByChildID(hMenu, &mData) && DeleteMenu(hMenu, IDM_MODS, MF_BYCOMMAND))
				{
					BOOL added = FALSE;
					Mod* mod = mods;
					while (mod)
					{
						if (InsertMenu(mData.hMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)mod->GetMenu(), mod->GetName()))
							added = TRUE;

						mod = mod->last;
					}

					if (!added)
						DeleteMenu(hMenu, mData.index, MF_BYPOSITION);
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

#pragma region About dialog
	DLGPROC OldDialogProc;
	LRESULT __stdcall AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_INITDIALOG)
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
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

	HMODULE __stdcall LoadLibraryHook(LPCSTR lpLibFileName)
	{
		if (!StrCompareInsensitive(lpLibFileName, "DDRAW.dll"))
			return hDllModule;
		return LoadLibrary(lpLibFileName);
	}

	BOOL __stdcall FreeLibraryHook(HMODULE hLibModule)
	{
		if (hLibModule == hDllModule)
			return TRUE;
		return FreeLibrary(hLibModule);
	}

	FARPROC __stdcall GetProcAddressHook(HMODULE hModule, LPCSTR lpProcName)
	{
		if (hModule == hDllModule)
		{
			if (!StrCompareInsensitive(lpProcName, "DirectDrawCreate"))
				return (FARPROC)Main::DirectDrawCreate;
			else
				return NULL;
		}
		else
			return GetProcAddress(hModule, lpProcName);
	}

#pragma region Mouse Pointers
	const DWORD monoEntries[2] = { 0x00000000, 0x00ffffff };

	const DWORD palEntries[256] = {
		0x00000000,
		0x00800000,
		0x00008000,
		0x00808000,
		0x00000080,
		0x00800080,
		0x00008080,
		0x00c0c0c0,
		0x00c0dcc0,
		0x00a6caf0,
		0x00000000,
		0x00fcfcfc,
		0x00e8e8e8,
		0x00e0e0e0,
		0x00d0d0d0,
		0x00c8c8c8,
		0x00b8b8b8,
		0x00b0b0b0,
		0x009c9c9c,
		0x00949494,
		0x00848484,
		0x007c7c7c,
		0x006c6c6c,
		0x00646464,
		0x00545454,
		0x004c4c4c,
		0x003c3c3c,
		0x00343434,
		0x00202020,
		0x00181818,
		0x00080808,
		0x00000000,
		0x00fce8dc,
		0x00f4dcd0,
		0x00ecccb8,
		0x00e8c4ac,
		0x00dcb094,
		0x00d8a88c,
		0x00d09878,
		0x00cc906c,
		0x00c0845c,
		0x00bc7c54,
		0x00b47044,
		0x00ac6c40,
		0x009c6034,
		0x00945c30,
		0x00845028,
		0x007c4c24,
		0x00704420,
		0x0068401c,
		0x00583414,
		0x00503014,
		0x0040280c,
		0x003c240c,
		0x00d8ecfc,
		0x00c4e4fc,
		0x00bcdcfc,
		0x00b4d8fc,
		0x00a8d0fc,
		0x00a0c4fc,
		0x0098bcfc,
		0x008cb0fc,
		0x0084a4fc,
		0x007c98fc,
		0x00708cfc,
		0x00687cfc,
		0x006070fc,
		0x005460f0,
		0x004854e4,
		0x003c48d4,
		0x002c38c4,
		0x00242cb4,
		0x001820a8,
		0x00101498,
		0x00080c8c,
		0x00040880,
		0x00000070,
		0x00000064,
		0x00c4fcb0,
		0x00b4f0a0,
		0x00a8e894,
		0x009ce088,
		0x0090d87c,
		0x0084d070,
		0x0078c464,
		0x006cbc58,
		0x0060b450,
		0x0058ac48,
		0x004ca03c,
		0x00449834,
		0x0038902c,
		0x00308824,
		0x00288020,
		0x00207418,
		0x00186c14,
		0x0014640c,
		0x000c5c08,
		0x00085004,
		0x00044804,
		0x00044000,
		0x00003800,
		0x00003000,
		0x00fcfce4,
		0x00fcfcc8,
		0x00fcfcb0,
		0x00f8f894,
		0x00f8f87c,
		0x00f8f864,
		0x00f8f048,
		0x00f4e834,
		0x00f0e024,
		0x00e8d41c,
		0x00e0c814,
		0x00d8b810,
		0x00d0ac08,
		0x00c89c04,
		0x00c09000,
		0x00b88400,
		0x00b07800,
		0x00a46c00,
		0x00985c00,
		0x00885000,
		0x007c4400,
		0x00703800,
		0x00642c00,
		0x00582400,
		0x00f0d8fc,
		0x00e8c8f8,
		0x00e0b8f8,
		0x00d8acf4,
		0x00cc9cf4,
		0x00c48cf4,
		0x00b880f0,
		0x00b070f0,
		0x00a464f0,
		0x009858e0,
		0x008c4cd4,
		0x008040c8,
		0x007838bc,
		0x006c2cb0,
		0x006424a0,
		0x00581c94,
		0x00501888,
		0x0048107c,
		0x00400c70,
		0x00340860,
		0x002c0454,
		0x00240048,
		0x0020003c,
		0x00180030,
		0x00bcf8fc,
		0x00b0ecf4,
		0x00a8e4e8,
		0x009cdce0,
		0x0094d4d8,
		0x0088c8d0,
		0x0080c0c8,
		0x0078b8c0,
		0x0070b0b8,
		0x0068a8b0,
		0x0060a0a8,
		0x005894a0,
		0x00508c98,
		0x0048848c,
		0x00447c84,
		0x003c747c,
		0x00386c74,
		0x0030646c,
		0x002c5c64,
		0x0028545c,
		0x00204c54,
		0x001c444c,
		0x00183c44,
		0x0014343c,
		0x00fce4e4,
		0x00fcd0d0,
		0x00fcc0c0,
		0x00fcb0b0,
		0x00fca0a0,
		0x00fc9090,
		0x00fc8080,
		0x00fc7070,
		0x00fc6060,
		0x00f05454,
		0x00e44848,
		0x00d84040,
		0x00cc3434,
		0x00c02c2c,
		0x00b42424,
		0x00a82020,
		0x009c1818,
		0x00901010,
		0x00840c0c,
		0x00780808,
		0x006c0404,
		0x00600000,
		0x00540000,
		0x00480000,
		0x00fce4a0,
		0x00fcd890,
		0x00fccc88,
		0x00fcc07c,
		0x00fcb470,
		0x00fca464,
		0x00fc9854,
		0x00f88c40,
		0x00ec8028,
		0x00dc7820,
		0x00cc6c18,
		0x00b4600c,
		0x009c5000,
		0x00844400,
		0x006c3800,
		0x00643000,
		0x00fc580c,
		0x00dc3404,
		0x00c01400,
		0x00a40000,
		0x00fcfc00,
		0x00fccc00,
		0x00c08c00,
		0x008c4800,
		0x00bce800,
		0x00acd800,
		0x00a0c800,
		0x0094b800,
		0x0084a804,
		0x00789804,
		0x006c8804,
		0x00607c04,
		0x006068fc,
		0x004058f0,
		0x002850e4,
		0x001048d8,
		0x000048cc,
		0x00a8d0fc,
		0x0068b8fc,
		0x0084e0fc,
		0x000098fc,
		0x000050e4,
		0x000000a4,
		0x007c7ca8,
		0x0070709c,
		0x00606090,
		0x00585888,
		0x00fcfcfc,
		0x00fffbf0,
		0x00a0a0a4,
		0x00808080,
		0x00ff0000,
		0x0000ff00,
		0x00ffff00,
		0x000000ff,
		0x00ff00ff,
		0x0000ffff,
		0x00ffffff
	};

	struct
	{
		FLOAT cx;
		FLOAT cy;
	} scale = { 1.0f, 1.0f };

	VOID ScalePointer(FLOAT cx, FLOAT cy)
	{
		HICON* hIcon = (HICON*)hookSpace->icons_list;
		if (!hIcon)
			return;

		HBITMAP* hBmMask = (HBITMAP*)hookSpace->masks_list;
		HBITMAP* hBmColor = (HBITMAP*)hookSpace->colors_list;

		scale.cx = cx;
		scale.cy = cy;

		DWORD count = hookSpace->game_version == 1 ? 75 : 96;
		do
		{
			if (*hIcon)
			{
				DestroyIcon(*hIcon);
				*hIcon = NULL;
			}
			++hIcon;

			if (*hBmMask)
			{
				DeleteObject(*hBmMask);
				*hBmMask = NULL;
			}
			++hBmMask;

			if (hBmColor)
			{
				if (*hBmColor)
				{
					DeleteObject(*hBmColor);
					*hBmColor = NULL;
				}
				++hBmColor;
			}
		} while (--count);
	}

	HBITMAP __stdcall CreateBitmapIndirectHook(BITMAP* pbm)
	{
		if (config.isDDraw)
		{
			HBITMAP hBmp = NULL;

			HDC hDc = GetDC(hWndMain);
			if (hDc)
			{
				if (GetDeviceCaps(hDc, BITSPIXEL) == 32)
				{
					VOID* colorData;
					DWORD width = (DWORD)MathRound(scale.cx * pbm->bmWidth);
					DWORD height = (DWORD)MathRound(scale.cy * (pbm->bmBitsPixel != 8 ? pbm->bmHeight / 2 : pbm->bmHeight));

					{
						DWORD size = sizeof(BITMAPINFOHEADER);
						BITMAPINFO* bmi = (BITMAPINFO*)MemoryAlloc(size);
						{
							MemoryZero(bmi, size);

							BITMAPINFOHEADER* bmiHeader = &bmi->bmiHeader;
							bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
							bmiHeader->biPlanes = pbm->bmPlanes;
							bmiHeader->biXPelsPerMeter = 1;
							bmiHeader->biYPelsPerMeter = 1;
							bmiHeader->biCompression = BI_RGB;
							bmiHeader->biBitCount = 32;

							bmiHeader->biWidth = width;
							bmiHeader->biHeight = -*(LONG*)&height;
							hBmp = CreateDIBSection(hDc, bmi, 0, &colorData, 0, 0);
						}
						MemoryFree(bmi);

						if (hBmp)
						{
							BYTE* src = (BYTE*)pbm->bmBits;

							FLOAT* buffer = (FLOAT*)MemoryAlloc(pbm->bmWidth * pbm->bmHeight * sizeof(FLOAT) * sizeof(DWORD));
							{
								FLOAT* dst = buffer;
								DWORD count = pbm->bmWidth * pbm->bmHeight;
								DWORD checkHeight = pbm->bmHeight;
								if (pbm->bmBitsPixel == 8)
								{
									do
									{
										DWORD index = *src++;
										DWORD color = palEntries[index];

										BYTE* c = (BYTE*)&color;

										*dst++ = (FLOAT)*c++ / 255.0f;
										*dst++ = (FLOAT)*c++ / 255.0f;
										*dst++ = (FLOAT)*c / 255.0f;

										*dst++ = !index ? 0.0f : 1.0f;
									} while (--count);
								}
								else
								{
									count >>= 1;
									checkHeight >>= 1;

									BYTE* xor = src + count / 8;

									BYTE andMask = *src++;
									BYTE xorMask = *xor++;
									DWORD countMask = 8;

									do
									{
										FLOAT value = (xorMask & 0x80) ? 1.0f : 0.0f;
										*dst++ = value;
										*dst++ = value;
										*dst++ = value;

										value = (andMask & 0x80) ? 0.0f : 1.0f;
										*dst++ = value;

										if (--countMask)
										{
											andMask <<= 1;
											xorMask <<= 1;
										}
										else
										{
											countMask = 8;
											andMask = *src++;
											xorMask = *xor++;
										}
									} while (--count);
								}

								BYTE* dest = (BYTE*)colorData;
								for (DWORD j = 0; j < height; ++j)
								{
									FLOAT y = (FLOAT)j / scale.cy;

									FLOAT f = (FLOAT)MathFloor(y);
									FLOAT yFract = y - f;
									yFract = yFract * yFract * (3.0f - 2.0f * yFract);

									INT y0 = (INT)f;
									if (y0 < 0)
										y0 = 0;

									INT y1 = (INT)MathCeil(y);
									if (y1 >= (INT)checkHeight)
										y1 = (INT)checkHeight - 1;

									for (DWORD i = 0; i < width; ++i)
									{
										FLOAT x = (FLOAT)i / scale.cx;

										FLOAT f = (FLOAT)MathFloor(x);
										FLOAT xFract = x - f;
										xFract = xFract * xFract * (3.0f - 2.0f * xFract);

										INT x0 = (INT)f;
										if (x0 < 0)
											x0 = 0;

										INT x1 = (INT)MathCeil(x);
										if (x1 >= (INT)pbm->bmWidth)
											x1 = (INT)pbm->bmWidth - 1;

										FLOAT* p0 = buffer + (y0 * pbm->bmWidth + x0) * sizeof(DWORD);
										FLOAT* p1 = buffer + (y0 * pbm->bmWidth + x1) * sizeof(DWORD);

										FLOAT* p2 = buffer + (y1 * pbm->bmWidth + x0) * sizeof(DWORD);
										FLOAT* p3 = buffer + (y1 * pbm->bmWidth + x1) * sizeof(DWORD);

										DWORD k = sizeof(DWORD);
										do
										{
											FLOAT p01 = (*p1 - *p0) * xFract + *p0;
											FLOAT p23 = (*p3 - *p2) * xFract + *p2;

											FLOAT p = (p23 - p01) * yFract + p01;

											*dest++ = (BYTE)MathRound(p * 255.0f);

											++p0;
											++p1;
											++p2;
											++p3;
										} while (--k);
									}
								}
							}
							MemoryFree(buffer);
						}
					}
				}
				else
				{
					HBITMAP hBmp1 = NULL, hBmp2 = NULL;
					VOID *colorData1, *colorData2;

					DWORD width = (DWORD)MathRound(scale.cx * pbm->bmWidth);

					DWORD height;
					if (hookSpace->color_pointer)
						height = (DWORD)MathRound(scale.cy * pbm->bmHeight);
					else
						height = (DWORD)(MathRound(scale.cy * pbm->bmHeight * 0.5f)) << 1;

					DWORD size = sizeof(BITMAPINFOHEADER) + 256 * sizeof(DWORD);
					BITMAPINFO* bmi = (BITMAPINFO*)MemoryAlloc(size);
					{
						MemoryZero(bmi, size);

						BITMAPINFOHEADER* bmiHeader = &bmi->bmiHeader;
						bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
						bmiHeader->biPlanes = pbm->bmPlanes;
						bmiHeader->biXPelsPerMeter = 1;
						bmiHeader->biYPelsPerMeter = 1;
						bmiHeader->biCompression = BI_RGB;

						if (pbm->bmBitsPixel == 8)
						{
							bmiHeader->biBitCount = 8;
							bmiHeader->biClrUsed = 256;
							MemoryCopy(bmi->bmiColors, palEntries, 256 * sizeof(DWORD));
						}
						else
						{
							bmiHeader->biBitCount = 1;
							bmiHeader->biClrUsed = 2;
							MemoryCopy(bmi->bmiColors, monoEntries, 2 * sizeof(DWORD));
						}

						bmiHeader->biWidth = pbm->bmWidth;
						bmiHeader->biHeight = -pbm->bmHeight;
						hBmp1 = CreateDIBSection(hDc, bmi, 0, &colorData1, 0, 0);

						bmiHeader->biWidth = width;
						bmiHeader->biHeight = height;
						hBmp2 = CreateDIBSection(hDc, bmi, 0, &colorData2, 0, 0);
					}
					MemoryFree(bmi);

					if (hBmp1 && hBmp2)
					{
						if (pbm->bmBitsPixel == 8)
							MemoryCopy(colorData1, pbm->bmBits, pbm->bmWidthBytes * pbm->bmHeight);
						else
							MemoryCopy(colorData1, pbm->bmBits, (DWORD)MathCeil(pbm->bmWidthBytes * pbm->bmHeight));

						BOOL stretched = FALSE;
						HDC hDc1 = CreateCompatibleDC(hDc);
						if (hDc1)
						{
							SelectObject(hDc1, hBmp1);

							HDC hDc2 = CreateCompatibleDC(hDc);
							if (hDc2)
							{
								SelectObject(hDc2, hBmp2);
								stretched = StretchBlt(hDc2, 0, 0, width, height, hDc1, 0, 0, pbm->bmWidth, pbm->bmHeight, SRCCOPY);

								DeleteDC(hDc2);
							}

							DeleteDC(hDc1);
						}

						if (stretched)
						{
							DeleteObject(hBmp1);
							hBmp = hBmp2;
						}
						else
						{
							DeleteObject(hBmp2);
							hBmp = hBmp1;
						}
					}
					else
					{
						if (hBmp1)
							DeleteObject(hBmp1);

						if (hBmp2)
							DeleteObject(hBmp2);
					}
				}

				ReleaseDC(hWndMain, hDc);
			}

			return hBmp;
		}
		else
			return (HBITMAP)1;
	}

	HICON __stdcall CreateIconIndirectHook(PICONINFO piconinfo)
	{
		if (config.isDDraw)
		{
			piconinfo->xHotspot = (DWORD)MathRound(scale.cx * piconinfo->xHotspot);
			piconinfo->yHotspot = (DWORD)MathRound(scale.cy * piconinfo->yHotspot);

			HDC hDc = GetDC(hWndMain);
			if (hDc)
			{
				INT bpp = GetDeviceCaps(hDc, BITSPIXEL);
				ReleaseDC(hWndMain, hDc);

				if (bpp == 32)
				{
					if (hookSpace->game_version == 2)
						piconinfo->hbmColor = piconinfo->hbmMask;

					DWORD width = (DWORD)MathRound(scale.cx * 32);
					DWORD height = (DWORD)MathRound(scale.cy * 32);

					if (piconinfo->hbmMask = CreateBitmap(width, height, 1, 1, NULL))
					{
						HICON hIcon = CreateIconIndirect(piconinfo);
						DeleteObject(piconinfo->hbmMask);
						return hIcon;
					}
				}
			}

			return CreateIconIndirect(piconinfo);
		}
		else
			return (HICON)piconinfo;
	}

	HCURSOR __stdcall SetCursorHook(HCURSOR hCursor)
	{
		if (hCursor)
		{
			DWORD index = ((DWORD)hCursor - hookSpace->icons_info) / sizeof(ICONINFO);
			if (index >= 0 && index < (hookSpace->game_version == 1 ? 75u : 96u))
			{
				config.cursor.index = index + 1;
				config.cursor.game = NULL;
				return NULL;
			}
		}

		config.cursor.index = 0;
		config.cursor.game = hCursor;
		return NULL;
	}

	INT __stdcall ShowCursorHook(BOOL bShow)
	{
		config.cursor.hidden = !bShow;
		return bShow ? 1 : -1;
	}

	HCURSOR __stdcall LoadCursorHook(HINSTANCE hInstance, LPCSTR lpCursorName)
	{
		return NULL;
	}

#pragma endregion

#pragma region Smoth Screen Update
	DWORD savedTick = 0;
	VOID __cdecl SetTickCount()
	{
		savedTick = timeGetTime() + 20;
	}

	VOID __cdecl UpdatePaletteHook(VOID* pallete)
	{
		((VOID(__cdecl*)(VOID*))hookSpace->update_palette)(pallete); // UpdatePalette
		((VOID(__cdecl*)(DWORD))hookSpace->delay_til)(savedTick); // DelayTilMilli
	}
#pragma endregion

#pragma region Fix CD Audio detection with empty or non - cdda drive
	DWORD AIL_redbook_open, AIL_redbook_status, AIL_redbook_close;
	DWORD __stdcall AIL_redbook_openHook(DWORD drive)
	{
		DWORD res = ((DWORD(__stdcall*)(DWORD))AIL_redbook_open)(drive);
		if (res && !((DWORD(__stdcall*)(DWORD))AIL_redbook_status)(res))
		{
			((DWORD(__stdcall*)(DWORD))AIL_redbook_close)(res);
			res = NULL;
		}

		return res;
	}
#pragma endregion

#pragma region CPU patch
	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		if (PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
			return TRUE;

		Sleep(config.coldCPU);

		return FALSE;
	}
#pragma endregion

#pragma region Bordered window
	HICON __stdcall LoadIconHook(HINSTANCE hInstance, LPCSTR lpIconName)
	{
		return config.icon; // For PL Editor
	}

	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		hookSpace->appSettings->showMenu = !hookSpace->appSettings->fullScreen;
		return RegisterClass(lpWndClass);
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

#pragma region Increase Stack size for main thread, by creating new thread;
	struct EntryParams {
		HINSTANCE hInstance;
		HINSTANCE hPrevInstance;
		LPSTR lpCmdLine;
		INT nShowCmd;
	};

	DWORD realEntry;
	DWORD __stdcall MainThread(EntryParams* eParams)
	{
		if (SetThreadLanguage)
			SetThreadLanguage(config.language.current);
		Window::SetCaptureKeys(TRUE);
		DWORD res = ((DWORD(__stdcall*)(EntryParams))realEntry)(*eParams);
		Window::SetCaptureKeys(FALSE);
		return res;
	}

	INT __stdcall WinMain(EntryParams eParams)
	{
		DWORD threadId;
		SECURITY_ATTRIBUTES sAttribs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		HANDLE hThread = CreateThread(&sAttribs, STACK_SIZE, (LPTHREAD_START_ROUTINE)MainThread, &eParams, NORMAL_PRIORITY_CLASS, &threadId);
		WaitForSingleObject(hThread, INFINITE);

		DWORD code;
		if (GetExitCodeThread(hThread, &code))
			return code;

		return NULL;
	}
#pragma endregion

#pragma region check samples ending
	DWORD back_004037C6;
	DWORD back_004037DF;
	VOID __declspec(naked) hook_004037BE()
	{
		__asm {
			cmp [ebp - 0x4], 0x0
			jz lbl_exit
			jmp back_004037C6
			lbl_exit: jmp back_004037DF
		}
	}
#pragma endregion

#pragma optimize("s", on)
	BOOL Load()
	{
		BOOL res = FALSE;
		HOOKER hooker = CreateHooker(GetModuleHandle(NULL));
		{
			hookSpace = addressArray;
			DWORD hookCount = sizeof(addressArray) / sizeof(AddressSpace);
			do
			{
				DWORD check;
				if (ReadDWord(hooker, hookSpace->check + 6, &check) && check == STYLE_FULL_OLD)
				{
					Config::Load(GetHookerModule(hooker), hookSpace);

					HOOKER user = CreateHooker(GetModuleHandle("USER32.dll"));
					{
						PatchExport(user, "PeekMessageA", PeekMessageHook);
						PatchExport(user, "MessageBoxA", MessageBoxHook);
						PatchExport(user, "DialogBoxParamA", DialogBoxParamHook);
					}
					ReleaseHooker(user);

					HOOKER shell = CreateHooker(GetModuleHandle("SHELL32.dll"));
					{
						if (PatchExport(shell, "SHBrowseForFolderA", SHBrowseForFolderHook))
							PatchExport(shell, "SHBrowseForFolderA", SHBrowseForFolderHook);
					}
					ReleaseHooker(shell);

					DWORD baseOffset = GetBaseOffset(hooker);
					{
						PatchImportByName(hooker, "PeekMessageA", PeekMessageHook);
						PatchImportByName(hooker, "MessageBoxA", MessageBoxHook);
						PatchImportByName(hooker, "DialogBoxParamA", AboutBoxParamHook);

						PatchImportByName(hooker, "AdjustWindowRect", AdjustWindowRectHook);
						PatchImportByName(hooker, "CreateWindowExA", CreateWindowExHook);
						PatchImportByName(hooker, "SetWindowLongA", SetWindowLongHook);

						PatchImportByName(hooker, "WinHelpA", WinHelpHook);

						PatchImportByName(hooker, "LoadMenuA", LoadMenuHook);
						PatchImportByName(hooker, "SetMenu", SetMenuHook);
						PatchImportByName(hooker, "EnableMenuItem", EnableMenuItemHook);

						PatchImportByName(hooker, "Sleep", SleepHook);

						PatchImportByName(hooker, "LoadIconA", LoadIconHook);
						PatchImportByName(hooker, "RegisterClassA", RegisterClassHook);

						PatchImportByName(hooker, "RegCreateKeyA", RegCreateKeyHook);
						PatchImportByName(hooker, "RegOpenKeyExA", RegOpenKeyExHook);
						PatchImportByName(hooker, "RegCloseKey", RegCloseKeyHook);
						PatchImportByName(hooker, "RegQueryValueExA", RegQueryValueExHook);
						PatchImportByName(hooker, "RegSetValueExA", RegSetValueExHook);

						if (PatchImportByName(hooker, "_AIL_redbook_open@4", AIL_redbook_openHook, &AIL_redbook_open))
						{
							PatchImportByName(hooker, "_AIL_redbook_status@4", NULL, &AIL_redbook_status);
							PatchImportByName(hooker, "_AIL_redbook_close@4", NULL, &AIL_redbook_close);
						}

						if (!config.isDDraw)
						{
							PatchImportByName(hooker, "LoadLibraryA", LoadLibraryHook);
							PatchImportByName(hooker, "FreeLibrary", FreeLibraryHook);
							PatchImportByName(hooker, "GetProcAddress", GetProcAddressHook);

							PatchImportByName(hooker, "ScreenToClient", ScreenToClientHook);
							PatchImportByName(hooker, "InvalidateRect", hook_InvalidateRect);
							PatchImportByName(hooker, "BeginPaint", BeginPaintHook);
						}

						if (config.cursor.fix)
						{
							if (hookSpace->icons_list)
							{
								PatchImportByName(hooker, "CreateBitmapIndirect", CreateBitmapIndirectHook);
								PatchImportByName(hooker, "CreateIconIndirect", CreateIconIndirectHook);

								if (!config.isDDraw)
								{
									PatchImportByName(hooker, "SetCursor", SetCursorHook);
									PatchImportByName(hooker, "ShowCursor", ShowCursorHook);
									PatchImportByName(hooker, "LoadCursorA", LoadCursorHook);
								}
								else
									config.cursor.fix = FALSE;
							}
							else
								config.cursor.fix = FALSE;
						}
					}

					realEntry = RedirectCall(hooker, hookSpace->entry, WinMain);

					if (hookSpace->fadein_tick && hookSpace->fadein_update_1 && hookSpace->fadein_update_2)
					{
						PatchCall(hooker, hookSpace->fadein_tick, SetTickCount);
						PatchCall(hooker, hookSpace->fadein_update_1, UpdatePaletteHook);
						PatchCall(hooker, hookSpace->fadein_update_2, UpdatePaletteHook);
					}

					if (hookSpace->fadeout_tick && hookSpace->fadeout_update)
					{
						PatchCall(hooker, hookSpace->fadeout_tick, SetTickCount);
						PatchCall(hooker, hookSpace->fadeout_update, UpdatePaletteHook);
					}

					if (!config.isDDraw)
					{
						if (hookSpace->resLanguage == LNG_ENGLISH)
						{
							PatchNop(hooker, hookSpace->method2_nop, 6);
							PatchWord(hooker, hookSpace->method2_jmp, 0xE990);
							PatchByte(hooker, hookSpace->invalid_jmp, 0xEB);
						}
						else
						{
							PatchNop(hooker, hookSpace->method2_nop, 2);
							PatchByte(hooker, hookSpace->method2_jmp, 0xEB);
							PatchWord(hooker, hookSpace->invalid_jmp, 0xE990);
						}

						PatchHook(hooker, hookSpace->setFullScreenStatus, hookSpace->game_version == 2 ? (VOID*)SwitchMode_v2 : (VOID*)SwitchMode_v1);

						ddSetFullScreenStatus = hookSpace->ddSetFullScreenStatus + baseOffset;
						checkChangeCursor = hookSpace->checkChangeCursor + baseOffset;

						config.update.offset = (POINT*)(hookSpace->moveOffset + baseOffset);
						invalidEsp = hookSpace->invalid_esp;
					}

					if (hookSpace->icons_list && hookSpace->color_pointer && config.cursor.fix)
					{
						PatchDWord(hooker, hookSpace->color_pointer, TRUE);
						if (hookSpace->color_pointer_nop)
							PatchNop(hooker, hookSpace->color_pointer_nop, 10);
					}

					if (hookSpace->pointer_fs_nop)
						PatchNop(hooker, hookSpace->pointer_fs_nop, 2);

					if (hookSpace->dispelMagicSwitch)
					{
						BYTE caseList[15];
						if (ReadBlock(hooker, hookSpace->dispelMagicSwitch, &caseList, sizeof(caseList)))
						{
							BYTE norm = caseList[0];
							BYTE alt = caseList[1];

							caseList[1] = norm;
							caseList[2] = alt;
							caseList[3] = norm;
							caseList[4] = norm;
							caseList[5] = alt;
							caseList[6] = alt;
							caseList[7] = alt;
							caseList[8] = norm;
							caseList[9] = norm;
							caseList[10] = norm;
							caseList[11] = alt;

							caseList[13] = norm;
							caseList[14] = norm;

							PatchBlock(hooker, hookSpace->dispelMagicSwitch, &caseList, sizeof(caseList));

							WORD inst;
							if (ReadWord(hooker, hookSpace->dispelMagicFix, &inst))
								PatchWord(hooker, hookSpace->dispelMagicFix, inst == 0xC1DE ? 0xE1DE : 0x6DD8); // faddp -> fsubrp
						}
					}

					if (hookSpace->nt_check_nop)
					{
						if (hookSpace->resLanguage == LNG_ENGLISH)
							PatchWord(hooker, hookSpace->nt_check_nop, 0xE990);
						else
							PatchByte(hooker, hookSpace->nt_check_nop, 0xEB);
					}

					if (hookSpace->sample_end_check) // compare sample on end
					{
						PatchHook(hooker, 0x004037BE, hook_004037BE);
						back_004037C6 = hookSpace->sample_end_check + 8 + baseOffset;
						back_004037DF = hookSpace->sample_end_check + 33 + baseOffset;
					}

					res = TRUE;
					break;
				}

				++hookSpace;
			} while (--hookCount);
		}
		ReleaseHooker(hooker);
		return res;
	}
#pragma optimize("", on)
}