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
#include "Shellapi.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"

#define STYLE_FULL_OLD (WS_VISIBLE | WS_POPUP)
#define STYLE_FULL_NEW (WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS)

#define STYLE_WIN_OLD (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define STYLE_WIN_NEW (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_CLIPSIBLINGS)

#define STYLE_DIALOG (DS_MODALFRAME | WS_POPUP)

const AddressSpace addressArray[] = {
	// === RUS ======================================================================================================================================
#pragma region RUS
	0x004D4A5F, 0x0059308B, 0x00000000, 0x00000000, 0x00592D72, 0x004E6BAF, 0x004E6C1D, 0x004E6C22, 0x005B88F0, 0x005B8904, 0x004759BD, 0x00592A2C, 0x005E3D48, 50, LNG_RUSSIAN, // Heroes III Erathia - v1.0 Buka
	"Герои Меча и Магии III: Возрождение Эрафии",

	0x004F2533, 0x005F9649, 0x00000000, 0x00000000, 0x005F9336, 0x005074CF, 0x0050753B, 0x00507540, 0x0063569C, 0x00635688, 0x0047DFF6, 0x005F901D, 0x006793D8, 142, LNG_RUSSIAN, // Heroes III Armageddon - v2.1 Buka
	"Герои Меча и Магии III: Клинок Армагеддона",

	0x004F2863, 0x005F9609, 0x00000000, 0x00000000, 0x005F92F6, 0x00507A5F, 0x00507ACB, 0x00507AD0, 0x0063569C, 0x00635688, 0x0047DCB6, 0x005F8FDD, 0x00679400, 142, LNG_RUSSIAN, // Heroes III Armageddon - v2.2 Buka
	"Герои Меча и Магии III: Клинок Армагеддона",

	0x004F7EB3, 0x00602379, 0x00000000, 0x00000000, 0x00602066, 0x0050D58F, 0x0050D5FB, 0x0050D600, 0x0063C6BC, 0x0063C6A8, 0x0048051E, 0x00601D4D, 0x00682A10, 142, LNG_RUSSIAN, // Heroes III Shadow - v3.1 Buka
	"Герои Меча и Магии III: Дыхание Cмерти",

	0x004F7EB3, 0x006021A9, 0x00000000, 0x00000000, 0x00601E96, 0x0050DB1F, 0x0050DB8B, 0x0050DB90, 0x0063E6EC, 0x0063E6D8, 0x0048016E, 0x00601B7D, 0x00684D60, 142, LNG_RUSSIAN, // Heroes III Complete - v4.0 Buka
	"Герои Меча и Магии III: Полное Собрание",

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004EBA34, 0x005AF329, 0x00000000, 0x00000000, 0x005AF016, 0x004FFFAF, 0x0050001B, 0x00500020, 0x005EB1CC, 0x005EB1B8, 0x0047AF86, 0x005AECFD, 0x00620D58, 146, LNG_RUSSIAN, // Heroes Chronicles Warlords & Underworld & Elements & Dragons - v1.0
	"Хроники героев",

	0x004EB494, 0x005AF2D9, 0x004027E9, 0x00643234, 0x005AEFC6, 0x004FF98F, 0x004FF9FB, 0x004FFA00, 0x005EB1CC, 0x005EB1B8, 0x0047AF36, 0x005AECAD, 0x00620D80, 146, LNG_RUSSIAN, // Heroes Chronicles Beastmaster - v1.0
	"Хроники героев: Восстание",

	0x004EB494, 0x005AF2D9, 0x004027E9, 0x0064322C, 0x005AEFC6, 0x004FF98F, 0x004FF9FB, 0x004FFA00, 0x005EB1CC, 0x005EB1B8, 0x0047AF36, 0x005AECAD, 0x00620D80, 146, LNG_RUSSIAN, // Heroes Chronicles Sword - v1.0
	"Хроники героев: Ледяной Клинок",
#pragma endregion 

	// === ENG ======================================================================================================================================
#pragma region ENG
	0x004D3363, 0x0058E558, 0x00000000, 0x00000000, 0x0058E246, 0x004E568F, 0x004E56FB, 0x004E5700, 0x005B58B4, 0x005B58A0, 0x004753D2, 0x0058DF2F, 0x005E0D48, 50, LNG_ENGLISH, // Heroes III Erathia - v1.0
	"Heroes of Might and Magic III: The Restoration of Erathia",

	0x0041E573, 0x004210B8, 0x00000000, 0x00000000, 0x00420DA6, 0x004179AF, 0x00417A1B, 0x00417A20, 0x005B7D64, 0x005B7D50, 0x0048B7A2, 0x00420A8F, 0x005E3900, 50, LNG_ENGLISH, // Heroes III Erathia - v1.1
	"Heroes of Might and Magic III: The Restoration of Erathia",

	0x0041E523, 0x00421078, 0x00000000, 0x00000000, 0x00420D66, 0x0041797F, 0x004179EB, 0x004179F0, 0x005B6D54, 0x005B6D40, 0x0048BB12, 0x00420A4F, 0x005E2900, 50, LNG_ENGLISH, // Heroes III Erathia - v1.2
	"Heroes of Might and Magic III: The Restoration of Erathia",

	0x004F58F3, 0x005D9679, 0x00000000, 0x00000000, 0x005D9366, 0x0050ABEF, 0x0050AC5B, 0x0050AC60, 0x00613644, 0x00613630, 0x0047F81E, 0x005D904D, 0x006559A0, 141, LNG_ENGLISH, // Heroes III Erathia - v1.3
	"Heroes of Might and Magic III: The Restoration of Erathia",

	0x004F5583, 0x005D8F69, 0x00000000, 0x00000000, 0x005D8C56, 0x0050AAAF, 0x0050AB1B, 0x0050AB20, 0x00613644, 0x00613630, 0x0047FC3E, 0x005D893D, 0x006559B0, 141, LNG_ENGLISH, // Heroes III Erathia - v1.4
	"Heroes of Might and Magic III: The Restoration of Erathia",

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004EB283, 0x005EEFD8, 0x00000000, 0x00000000, 0x005EECC6, 0x0050041F, 0x0050048B, 0x00500490, 0x00628F28, 0x00628F14, 0x0047A472, 0x005EE9AF, 0x006692D0, 90, LNG_ENGLISH, // Heroes III Armageddon - v2.0
	"Heroes of Might and Magic III: Armageddon’s Blade",

	0x004F5C43, 0x00600299, 0x00000000, 0x00000000, 0x005FFF86, 0x0050B6AF, 0x0050B71B, 0x0050B720, 0x0063B68C, 0x0063B678, 0x0048023E, 0x005FFC6D, 0x006808F0, 141, LNG_ENGLISH, // Heroes III Armageddon - v2.1
	"Heroes of Might and Magic III: Armageddon’s Blade",

	0x004F5963, 0x005FFBF9, 0x00000000, 0x00000000, 0x005FF8E6, 0x0050B1BF, 0x0050B22B, 0x0050B230, 0x0063B68C, 0x0063B678, 0x0048052E, 0x005FF5CD, 0x006808F8, 141, LNG_ENGLISH, // Heroes III Armageddon - v2.2
	"Heroes of Might and Magic III: Armageddon’s Blade",

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004F7D73, 0x006027E9, 0x00000000, 0x00000000, 0x006024D6, 0x0050D93F, 0x0050D9AB, 0x0050D9B0, 0x0063E6DC, 0x0063E6C8, 0x004802DE, 0x006021BD, 0x00684A08, 141, LNG_ENGLISH, // Heroes III Shadow - v3.0
	"Heroes of Might and Magic III: The Shadow of Death",

	0x004F85B3, 0x006027E9, 0x00000000, 0x00000000, 0x006024D6, 0x0050DE4F, 0x0050DEBB, 0x0050DEC0, 0x0063E6DC, 0x0063E6C8, 0x004802EE, 0x006021BD, 0x00684A08, 141, LNG_ENGLISH, // Heroes III Shadow - v3.1
	"Heroes of Might and Magic III: The Shadow of Death",

	0x004F8193, 0x00602149, 0x00000000, 0x00000000, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0050D940, 0x0063D6DC, 0x0063D6C8, 0x0047FE9E, 0x00601B1D, 0x00683A10, 141, LNG_ENGLISH, // Heroes III Shadow - v3.2
	"Heroes of Might and Magic III: The Shadow of Death",

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	0x004F7B03, 0x00601B89, 0x00000000, 0x00000000, 0x00601876, 0x0050D6CF, 0x0050D73B, 0x0050D740, 0x0063D6DC, 0x0063D6C8, 0x0048057E, 0x0060155D, 0x006839C0, 141, LNG_ENGLISH, // Heroes III Complete - v4.0
	"Heroes of Might and Magic III: Complete",

	// ==============================================================================================================================================

	0x004F0033, 0x005B5129, 0x004022FD, 0x0063B6C0, 0x005B4E16, 0x00504C8F, 0x00504CFB, 0x00504D00, 0x005EFE3C, 0x005EFE28, 0x0047DF4E, 0x005B4AFD, 0x00625B78, 145, LNG_ENGLISH, // Heroes Chronicles Warlords - v1.0
	"Heroes Chronicles: Warlords of the Wasteland",

	0x004F0033, 0x005B5129, 0x004022FD, 0x0063B6B8, 0x005B4E16, 0x00504C8F, 0x00504CFB, 0x00504D00, 0x005EFE3C, 0x005EFE28, 0x0047DF4E, 0x005B4AFD, 0x00625B78, 145, LNG_ENGLISH, // Heroes Chronicles Underworld - v1.0
	"Heroes Chronicles: Conquest of the Underworld",

	0x004EFE04, 0x005B5469, 0x004022FD, 0x0063B6C0, 0x005B5156, 0x00504B6F, 0x00504BDB, 0x00504BE0, 0x005EFE3C, 0x005EFE28, 0x0047DA0E, 0x005B4E3D, 0x00625B68, 145, LNG_ENGLISH, // Heroes Chronicles Elements - v1.0
	"Heroes Chronicles: Masters of the Elements",

	0x004EFE04, 0x005B5469, 0x004022FD, 0x0063B6B8, 0x005B5156, 0x00504B6F, 0x00504BDB, 0x00504BE0, 0x005EFE3C, 0x005EFE28, 0x0047DA0E, 0x005B4E3D, 0x00625B70, 145, LNG_ENGLISH, // Heroes Chronicles Dragons - v1.0
	"Heroes Chronicles: Clash of the Dragons",

	0x004EFA84, 0x005B51B9, 0x00000000, 0x00000000, 0x005B4EA6, 0x00504B6F, 0x00504BDB, 0x00504BE0, 0x005EFE3C, 0x005EFE28, 0x0047D8DE, 0x005B4B8D, 0x00625B70, 145, LNG_ENGLISH, // Heroes Chronicles WorldTree - v1.0
	"Heroes Chronicles: The World Tree",

	0x004EF824, 0x005B5249, 0x00000000, 0x00000000, 0x005B4F36, 0x0050418F, 0x005041FB, 0x00504200, 0x005F11BC, 0x005F11A8, 0x0047D36E, 0x005B4C1D, 0x00627F60, 145, LNG_ENGLISH, // Heroes Chronicles FieryMoon - v1.0
	"Heroes Chronicles: The Fiery Moon",

	0x004EF874, 0x005B4C09, 0x00401050, 0x00636868, 0x005B48F6, 0x0050460F, 0x0050467B, 0x00504680, 0x005F11CC, 0x005F11B8, 0x0047D5FE, 0x005B45DD, 0x00626FA0, 145, LNG_ENGLISH, // Heroes Chronicles Beastmaster - v1.0
	"Heroes Chronicles: Revolt of the Beastmasters",

	0x004EF914, 0x005B51B9, 0x005AFABC, 0x0064A494, 0x005B4EA6, 0x0050459F, 0x0050460B, 0x00504610, 0x005F11CC, 0x005F11B8, 0x0047DB4E, 0x005B4B8D, 0x00627F98, 145, LNG_ENGLISH, // Heroes Chronicles Beastmaster - v1.0
	"Heroes Chronicles: Revolt of the Beastmasters",

	0x004EF874, 0x005B4C09, 0x00401050, 0x00636878, 0x005B48F6, 0x0050460F, 0x0050467B, 0x00504680, 0x005F11CC, 0x005F11B8, 0x0047D5FE, 0x005AECAD, 0x00626FA8, 145, LNG_ENGLISH, // Heroes Chronicles Sword - v1.0
	"Heroes Chronicles: The Sword of Frost",

	0x004EF914, 0x005B51B9, 0x005AFABC, 0x0064A484, 0x005B4EA6, 0x0050459F, 0x0050460B, 0x00504610, 0x005F11CC, 0x005F11B8, 0x0047DB4E, 0x005B4B8D, 0x00627F98, 145, LNG_ENGLISH, // Heroes Chronicles Beastmaster - v1.0
	"Heroes Chronicles: The Sword of Frost",
#pragma endregion

	// === GER ======================================================================================================================================
#pragma region GER
	0x004D5253, 0x00591B29, 0x00000000, 0x00000000, 0x00591816, 0x004E714F, 0x004E71BB, 0x004E71C0, 0x005B88B4, 0x005B88A0, 0x004761CE, 0x005914FD, 0x005E3E68, 50, LNG_ENGLISH, // Heroes III Erathia - v1.2
	"Heroes of Might and Magic III: The Restoration of Erathia",

	// ==============================================================================================================================================

	0x004EFA04, 0x005B51C9, 0x00000000, 0x00000000, 0x005B4EB6, 0x0050475F, 0x005047CB, 0x005047D0, 0x005F11BC, 0x005F11A8, 0x0047D25E, 0x005B4B9D, 0x00627FA8, 145, LNG_ENGLISH, // Heroes Chronicles Dragons - GOG - v1.0
	"Heroes Chronicles: Clash of the Dragons",
#pragma endregion

	// === FRA ======================================================================================================================================
#pragma region FRA
	0x004F61C3, 0x006003D9, 0x00000000, 0x00000000, 0x006000C6, 0x0050BA6F, 0x0050BADB, 0x0050BAE0, 0x0063B68C, 0x0063B678, 0x004804CE, 0x005FFDAD, 0x006809D0, 141, LNG_ENGLISH, // Heroes III Armageddon - v2.1
	"Heroes of Might and Magic III: Armageddon’s Blade",

	0x004F8163, 0x006028F9, 0x00000000, 0x00000000, 0x006025E6, 0x0050DBBF, 0x0050DC2B, 0x0050DC30, 0x0063E6EC, 0x0063E6D8, 0x0048072E, 0x006022CD, 0x00684AD0, 141, LNG_ENGLISH, // Heroes III Shadow - v3.1
	"Heroes of Might and Magic III: The Shadow of Death",
#pragma endregion

	// === POL ======================================================================================================================================
#pragma region POL
	0x004F78D3, 0x00602179, 0x00000000, 0x00000000, 0x00601E66, 0x0050D50F, 0x0050D57B, 0x0050D580, 0x0063E6EC, 0x0063E6D8, 0x0047FD2E, 0x00601B4D, 0x00684A18, 141, LNG_ENGLISH, // Heroes III Shadow - v3.2
	"Heroes of Might and Magic III: The Shadow of Death",
#pragma endregion

	// ==============================================================================================================================================
#pragma region Others
	0x004F8193, 0x00602149, 0x0067FEB2, 0x00352E33, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0050D940, 0x0063D6DC, 0x0063D6C8, 0x0047FE9E, 0x00601B1D, 0x00683A10, 141, LNG_ENGLISH, // Heroes III WoG - v3.52f - v3.58f
	"Heroes of Might and Magic III: In the Wake of Gods",

	0x004F8193, 0x00602149, 0x0067FEB2, 0x00506F4D, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0050D940, 0x0063D6DC, 0x0063D6C8, 0x0047FE9E, 0x00601B1D, 0x00683A10, 141, LNG_ENGLISH, // Heroes III MoP - v3.7.2.7
	"Heroes of Might and Magic III: Master of Puppets",

	0x004F8193, 0x00602149, 0x00639C01, 0x00639BF4, 0x00601E36, 0x0050D8CF, 0x0050D93B, 0x0050D940, 0x0063D6DC, 0x0063D6C8, 0x0047FE9E, 0x00601B1D, 0x00683A10, 141, LNG_ENGLISH, // Heroes III HotA - v1.5.3
	"Heroes of Might and Magic III: Horn of the Abyss"
#pragma endregion
};

const UINT menuIds[] = { IDM_FILT_OFF, IDM_FILT_LINEAR, IDM_FILT_HERMITE, IDM_FILT_CUBIC, IDM_ASPECT_RATIO, IDM_VSYNC, IDM_HELP_WRAPPER,
	IDM_FILT_NONE, IDM_FILT_XRBZ_2X, IDM_FILT_XRBZ_3X, IDM_FILT_XRBZ_4X, IDM_FILT_XRBZ_5X, IDM_FILT_XRBZ_6X,
	IDM_FILT_SCALEHQ_2X, IDM_FILT_SCALEHQ_4X,
	IDM_FILT_XSAL_2X,
	IDM_FILT_EAGLE_2X,
	IDM_FILT_SCALENX_2X, IDM_FILT_SCALENX_3X,
	IDM_PATCH_CPU
};

namespace Hooks
{
	const AddressSpace* hookSpace;
	HMODULE hModule;
	INT baseOffset;

#pragma region Hook helpers
	BOOL __fastcall PatchRedirect(DWORD addr, VOID* hook, BYTE instruction)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, 5, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)address;
			*jump = instruction;
			++jump;
			*(DWORD*)jump = (DWORD)hook - (DWORD)address - 5;

			VirtualProtect((VOID*)address, 5, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE9);
	}

	BOOL __fastcall PatchCall(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE8);
	}

	BOOL __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			MemorySet((VOID*)address, 0x90, size);
			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)address = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)address = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)address = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)address, block, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)address;
				break;
			case 2:
				*(WORD*)block = *(WORD*)address;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)address;
				break;
			default:
				MemoryCopy(block, (VOID*)address, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchWord(DWORD addr, WORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchByte(DWORD addr, BYTE value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall ReadWord(DWORD addr, WORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadDWord(DWORD addr, DWORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadByte(DWORD addr, BYTE* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	DWORD __fastcall PatchFunction(MappedFile* file, const CHAR* function, VOID* addr)
	{
		DWORD res = NULL;

		DWORD base = (DWORD)file->hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)file->hModule)->e_lfanew);

		PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (dataDir->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(base + dataDir->VirtualAddress);
			for (DWORD idx = 0; imports->Name; ++idx, ++imports)
			{
				CHAR* libraryName = (CHAR*)(base + imports->Name);

				PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)(base + imports->FirstThunk);
				PIMAGE_THUNK_DATA nameThunk;
				if (imports->OriginalFirstThunk)
					nameThunk = (PIMAGE_THUNK_DATA)(base + imports->OriginalFirstThunk);
				else
				{
					if (!file->hFile)
					{
						CHAR filePath[MAX_PATH];
						GetModuleFileName(file->hModule, filePath, MAX_PATH);
						file->hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (!file->hFile)
							return res;
					}

					if (!file->hMap)
					{
						file->hMap = CreateFileMapping(file->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
						if (!file->hMap)
							return res;
					}

					if (!file->address)
					{
						file->address = MapViewOfFile(file->hMap, FILE_MAP_READ, 0, 0, 0);;
						if (!file->address)
							return res;
					}

					headNT = (PIMAGE_NT_HEADERS)((BYTE*)file->address + ((PIMAGE_DOS_HEADER)file->address)->e_lfanew);
					PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

					nameThunk = NULL;
					DWORD sCount = headNT->FileHeader.NumberOfSections;
					while (sCount--)
					{
						if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
						{
							nameThunk = PIMAGE_THUNK_DATA((DWORD)file->address + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
							break;
						}

						++sh;
					}

					if (!nameThunk)
						return res;
				}

				for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
				{
					PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME(base + nameThunk->u1.AddressOfData);

					WORD hint;
					if (ReadWord((INT)name - baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
					{
						if (ReadDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, &res))
							PatchDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, (DWORD)addr);

						return res;
					}
				}
			}
		}

		return res;
	}
#pragma endregion

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

		HWND hWnd = CreateWindow(lpClassName, hookSpace->windowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (hWnd)
			Window::SetCaptureWindow(hWnd);

		return hWnd;
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
			ReadByte(hookSpace->bpp_address + 1, (BYTE*)&displayMode.bpp);
			ReadDWord(hookSpace->bpp_address + 2 + 1, &displayMode.height);
			ReadDWord(hookSpace->bpp_address + 2 + 5 + 1, &displayMode.width);
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

	BOOL __stdcall WinHelpHook(HWND hWndMain, LPCSTR lpszHelp, UINT uCommand, ULONG_PTR dwData)
	{
		CHAR filePath[MAX_PATH];
		GetModuleFileName(hModule, filePath, MAX_PATH - 1);
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
			HMENU hNew = LoadMenu(hDllModule, MAKEINTRESOURCE(config.language == LNG_ENGLISH ? IDM_ENGLISH : IDM_RUSSIAN));
			if (hNew)
			{
				DWORD i, index = 0;

				HMENU hSub;
				for (i = 0; hSub = GetSubMenu(hMenu, i);)
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
					StrPrint(buffer, "%s (F%d)", buffer, config.keys.windowedMode);
					SetMenuItemInfo(hNew, IDM_RES_FULL_SCREEN, FALSE, &info);
				}

				info.cch = sizeof(buffer);
				if (config.keys.aspectRatio && GetMenuItemInfo(hNew, IDM_ASPECT_RATIO, FALSE, &info))
				{
					StrPrint(buffer, "%s (F%d)", buffer, config.keys.aspectRatio);
					SetMenuItemInfo(hNew, IDM_ASPECT_RATIO, FALSE, &info);
				}

				info.cch = sizeof(buffer);
				if (config.keys.vSync && GetMenuItemInfo(hNew, IDM_VSYNC, FALSE, &info))
				{
					StrPrint(buffer, "%s (F%d)", buffer, config.keys.vSync);
					SetMenuItemInfo(hNew, IDM_VSYNC, FALSE, &info);
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

	BOOL __stdcall EnableMenuItemHook(HMENU hMenu, UINT uIDEnableItem, UINT uEnable)
	{
		BOOL found = FALSE;
		const UINT* menu = menuIds;
		DWORD count = sizeof(menuIds) / sizeof(UINT);
		do
		{
			if (*menu++ == uIDEnableItem)
			{
				found = TRUE;
				break;
			}
		} while (--count);

		return !found && EnableMenuItem(hMenu, uIDEnableItem, uEnable);
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

	INT_PTR __stdcall DialogBoxParamHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
	{
		OldDialogProc = lpDialogFunc;

		INT_PTR res;
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = DialogBoxParam(hInstance, lpTemplateName, hWndParent, (DLGPROC)AboutProc, dwInitParam);

		if (cookie)
			DeactivateActCtxC(0, cookie);

		return res;
	}

#pragma region Music
	AIL_WAVEOUTOPEN AIL_waveOutOpen;
	AIL_OPEN_STREAM AIL_open_stream;
	AIL_STREAM_POSITION AIL_stream_position;
	AIL_SET_STREAM_POSITION AIL_set_stream_position;
	CHAR* audioExtList[] = { ".wav", ".mp3" };
	TrackInfo* tracksList, *trackInfo;

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
				SeedRandom(GetTickCount());
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

	DWORD posIndex;
	DWORD moveCounter;
	VOID __stdcall CalcRunPos(VOID* obj)
	{
		if (posIndex % moveCounter)
		{
			DWORD* pos = (DWORD*)obj + 128;
			if (*pos)
				--*pos;
			else
				*pos = 7;
		}

		++posIndex;
	}

	DWORD sub_move;
	VOID __declspec(naked) hook_move()
	{
		__asm
		{
			PUSH ECX
			PUSH EAX

			PUSH ESI
			CALL CalcRunPos

			POP EAX
			POP ECX
			JMP sub_move
		}
	}

	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		if (PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
			return TRUE;

		Sleep(config.coldCPU);

		return FALSE;
	}

	DWORD video_address;
	VOID __fastcall ResetVideoTable()
	{
		VideoInfo* gameInfo = (VideoInfo*)video_address;
		DWORD countInGame = hookSpace->video_count;
		do
		{
			gameInfo->isBink = FALSE;
			++gameInfo;
		} while (--countInGame);
	}

	HANDLE __stdcall CreateFileHook(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	{
		HANDLE hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		if (hFile)
		{
			CHAR* p = StrLastChar((CHAR*)lpFileName, '.');
			if (p && !StrCompareInsensitive(p, ".vid"))
			{
				DWORD countInFile, readed;
				if (ReadFile(hFile, &countInFile, sizeof(DWORD), &readed, NULL))
				{
					VideoFile* infoList = (VideoFile*)MemoryAlloc(sizeof(VideoFile) * countInFile);
					if (ReadFile(hFile, infoList, sizeof(VideoFile) * countInFile, &readed, NULL))
					{
						CHAR name[40];

						VideoInfo* gameInfo = (VideoInfo*)video_address;
						DWORD countInGame = hookSpace->video_count;
						do
						{
							StrCopy(name, gameInfo->fileName);
							StrCat(name, ".bik");

							VideoFile* fileInfo = infoList;

							DWORD count = countInFile;
							do
							{
								if (!StrCompareInsensitive(name, fileInfo->name))
								{
									gameInfo->isBink = TRUE;
									break;
								}

								++fileInfo;
							} while (--count);

							++gameInfo;
						} while (--countInGame);
					}
					MemoryFree(infoList);
				}

				SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			}
		}

		return hFile;
	}

	VOID __fastcall CheckRefreshRate()
	{
		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
		DWORD cursorTime;
		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && devMode.dmDisplayFrequency)
			cursorTime = 1000 / devMode.dmDisplayFrequency;
		else
			cursorTime = 16;

		// ==========================================================
		PatchByte(hookSpace->cursor_time_1 + 2, (BYTE)cursorTime);
		PatchByte(hookSpace->cursor_time_2 + 2, (BYTE)cursorTime);
		PatchDWord(hookSpace->cursor_time_3 + 1, cursorTime);

		// ==========================================================
		moveCounter = DWORD(50.0f / cursorTime);

		PatchDWord(hookSpace->move_timeout + 4, cursorTime);
		PatchDWord(hookSpace->move_timeout + 8, cursorTime);
		PatchDWord(hookSpace->move_timeout + 12, cursorTime);

		PatchDWord(hookSpace->move_distance + 4, DWORD((FLOAT)cursorTime / 50.0f * 8.0f));
		PatchDWord(hookSpace->move_distance + 8, DWORD((FLOAT)cursorTime / 50.0f * 12.0f));
		PatchDWord(hookSpace->move_distance + 12, DWORD((FLOAT)cursorTime / 50.0f * 16.0f));
		// ==========================================================
	}

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

	BOOL Load()
	{
		hookSpace = addressArray;
		hModule = GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		baseOffset = (INT)hModule - (INT)headNT->OptionalHeader.ImageBase;

		const AddressSpace* defaultSpace = NULL;
		const AddressSpace* equalSpace = NULL;

		DWORD hookCount = sizeof(addressArray) / sizeof(AddressSpace);
		do
		{
			DWORD check1, check2, equal;
			if (ReadDWord(hookSpace->check_1 + 1, &check1) && check1 == STYLE_FULL_OLD &&
				ReadDWord(hookSpace->check_2 + 1, &check2) && check2 == STYLE_FULL_OLD)
			{
				if (!hookSpace->equal_address)
					defaultSpace = hookSpace;
				else if (ReadDWord(hookSpace->equal_address, &equal) && equal == hookSpace->equal_value)
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
			Config::Load(hModule, hookSpace);

			{
				MappedFile file = { hModule, NULL, NULL, NULL };
				{
					PatchFunction(&file, "MessageBoxA", MessageBoxHook);
					PatchFunction(&file, "WinHelpA", WinHelpHook);

					PatchFunction(&file, "LoadMenuA", LoadMenuHook);
					PatchFunction(&file, "SetMenu", SetMenuHook);
					PatchFunction(&file, "EnableMenuItem", EnableMenuItemHook);

					PatchFunction(&file, "Sleep", SleepHook);
					PatchFunction(&file, "DialogBoxParamA", DialogBoxParamHook);
					PatchFunction(&file, "PeekMessageA", PeekMessageHook);

					PatchFunction(&file, "CreateFileA", CreateFileHook);

					PatchFunction(&file, "RegCreateKeyA", RegCreateKeyHook);
					PatchFunction(&file, "RegOpenKeyExA", RegOpenKeyExHook);
					PatchFunction(&file, "RegCloseKey", RegCloseKeyHook);
					PatchFunction(&file, "RegQueryValueExA", RegQueryValueExHook);
					PatchFunction(&file, "RegSetValueExA", RegSetValueExHook);

					PatchFunction(&file, "DirectDrawCreate", Main::DirectDrawCreate);

					AIL_waveOutOpen = (AIL_WAVEOUTOPEN)PatchFunction(&file, "_AIL_waveOutOpen@16", AIL_waveOutOpenHook);
					AIL_stream_position = (AIL_STREAM_POSITION)PatchFunction(&file, "_AIL_stream_position@4", AIL_stream_positionHook);
					AIL_open_stream = (AIL_OPEN_STREAM)PatchFunction(&file, "_AIL_open_stream@12", AIL_open_streamHook);
					AIL_set_stream_position = (AIL_SET_STREAM_POSITION)PatchFunction(&file, "_AIL_set_stream_position@8", AIL_set_stream_positionHook);

					if (!config.isNoGL)
					{
						PatchFunction(&file, "AdjustWindowRect", AdjustWindowRectHook);
						PatchFunction(&file, "AdjustWindowRectEx", AdjustWindowRectExHook);
						PatchFunction(&file, "CreateWindowExA", CreateWindowExHook);
						PatchFunction(&file, "SetWindowLongA", SetWindowLongHook);

						PatchFunction(&file, "GetDeviceCaps", GetDeviceCapsHook);
						PatchFunction(&file, "GetCursorPos", GetCursorPosHook);
						PatchFunction(&file, "ScreenToClient", ScreenToClientHook);
					}
				}

				if (file.address)
					UnmapViewOfFile(file.address);

				if (file.hMap)
					CloseHandle(file.hMap);

				if (file.hFile)
					CloseHandle(file.hFile);
			}

			if (!config.isNoGL)
				PatchNop(hookSpace->renderNop, 5); // prevent on WM_PAINT

			// Smotth move
			{
				DWORD address = hookSpace->move_address + 5 + 5 + 2 + 2;
				DWORD offset;
				ReadDWord(address + 1, &offset);

				DWORD newAddress = address + 5 + baseOffset;
				sub_move = newAddress + offset;
				PatchDWord(address + 1, (DWORD)hook_move - newAddress);
			}

			video_address = hookSpace->video_address + baseOffset;
			ResetVideoTable();

			return TRUE;
		}

		return FALSE;
	}
}