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
#include "Hooker.h"

Hooker::Hooker(HMODULE hModule)
{
	this->hModule = hModule;
	this->headNT = (PIMAGE_NT_HEADERS)((BYTE*)this->hModule + ((PIMAGE_DOS_HEADER)this->hModule)->e_lfanew);
	this->baseOffset = (INT)this->hModule - (INT)headNT->OptionalHeader.ImageBase;

	this->hFile = INVALID_HANDLE_VALUE;
	this->hMap = NULL;
	this->mapAddress = NULL;
}

Hooker::~Hooker()
{
	this->UnmapFile();
}

BOOL Hooker::MapFile()
{
	if (!this->mapAddress)
	{
		if (this->hFile == INVALID_HANDLE_VALUE)
		{
			CHAR filePath[MAX_PATH];
			GetModuleFileName(hModule, filePath, MAX_PATH);
			this->hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (this->hFile == INVALID_HANDLE_VALUE)
				return FALSE;
		}

		if (!this->hMap)
		{
			this->hMap = CreateFileMapping(this->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
			if (!this->hMap)
				return FALSE;
		}

		this->mapAddress = MapViewOfFile(this->hMap, FILE_MAP_READ, 0, 0, 0);
	}

	return (BOOL)this->mapAddress;
}

VOID Hooker::UnmapFile()
{
	if (this->mapAddress && UnmapViewOfFile(this->mapAddress))
		this->mapAddress = NULL;

	if (this->hMap && CloseHandle(this->hMap))
		this->hMap = NULL;

	if (this->hFile != INVALID_HANDLE_VALUE && CloseHandle(this->hFile))
		this->hFile = INVALID_HANDLE_VALUE;
}

BOOL Hooker::ReadBlock(DWORD addr, VOID* block, DWORD size)
{
	DWORD address = addr + this->baseOffset;

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

BOOL Hooker::ReadWord(DWORD addr, WORD* value)
{
	return this->ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::ReadDWord(DWORD addr, DWORD* value)
{
	return this->ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::ReadByte(DWORD addr, BYTE* value)
{
	return this->ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::PatchRedirect(DWORD addr, DWORD dest, BYTE instruction, DWORD nop)
{
	DWORD address = addr + this->baseOffset;

	DWORD size = instruction == 0xEB ? 2 : 5;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size + nop, PAGE_EXECUTE_READWRITE, &old_prot))
	{
		BYTE* jump = (BYTE*)address;
		*jump = instruction;
		++jump;
		*(DWORD*)jump = dest - address - size;

		if (nop)
			MemorySet((VOID*)(address + size), 0x90, nop);

		VirtualProtect((VOID*)address, size + nop, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::PatchJump(DWORD addr, DWORD dest)
{
	INT relative = dest - addr - this->baseOffset - 2;
	return this->PatchRedirect(addr, dest, relative >= -128 && relative <= 127 ? 0xEB : 0xE9, 0);
}

BOOL Hooker::PatchHook(DWORD addr, VOID* hook, DWORD nop)
{
	return this->PatchRedirect(addr, (DWORD)hook, 0xE9, nop);
}

BOOL Hooker::PatchCall(DWORD addr, VOID* hook, DWORD nop)
{
	return this->PatchRedirect(addr, (DWORD)hook, 0xE8, nop);
}

DWORD Hooker::RedirectCall(DWORD addr, VOID* hook)
{
	BYTE block[5];
	return this->ReadBlock(addr, block, sizeof(block)) && block[0] == 0xE8 && PatchRedirect(addr, (DWORD)hook, 0xE8, 0)
		? addr + 5 + this->baseOffset + *(DWORD*)&block[1]
		: NULL;
}

BOOL Hooker::PatchNop(DWORD addr, DWORD size)
{
	DWORD address = addr + this->baseOffset;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
	{
		MemorySet((VOID*)address, 0x90, size);
		VirtualProtect((VOID*)address, size, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::PatchBlock(DWORD addr, VOID* block, DWORD size)
{
	DWORD address = addr + this->baseOffset;

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

BOOL Hooker::PatchWord(DWORD addr, WORD value)
{
	return this->PatchBlock(addr, &value, sizeof(value));
}

BOOL Hooker::PatchDWord(DWORD addr, DWORD value)
{
	return this->PatchBlock(addr, &value, sizeof(value));
}

BOOL Hooker::PatchByte(DWORD addr, BYTE value)
{
	return this->PatchBlock(addr, &value, sizeof(value));
}

DWORD Hooker::PatchImport(const CHAR* function, VOID* addr)
{
	DWORD res = NULL;

	PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (dataDir->Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)this->hModule + dataDir->VirtualAddress);
		for (DWORD idx = 0; imports->Name; ++idx, ++imports)
		{
			PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)((DWORD)this->hModule + imports->FirstThunk);
			PIMAGE_THUNK_DATA nameThunk;
			if (imports->OriginalFirstThunk)
				nameThunk = (PIMAGE_THUNK_DATA)((DWORD)this->hModule + imports->OriginalFirstThunk);
			else if (this->MapFile())
			{
				PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)this->mapAddress + ((PIMAGE_DOS_HEADER)this->mapAddress)->e_lfanew);
				PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

				nameThunk = NULL;
				DWORD sCount = headNT->FileHeader.NumberOfSections;
				while (sCount--)
				{
					if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
					{
						nameThunk = PIMAGE_THUNK_DATA((DWORD)this->mapAddress + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
						break;
					}

					++sh;
				}

				if (!nameThunk)
					return res;
			}
			else
				return res;

			for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
			{
				PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME((DWORD)this->hModule + nameThunk->u1.AddressOfData);

				WORD hint;
				if (this->ReadWord((INT)name - this->baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
				{
					if (this->ReadDWord((INT)&addressThunk->u1.AddressOfData - this->baseOffset, &res))
						this->PatchDWord((INT)&addressThunk->u1.AddressOfData - this->baseOffset, (DWORD)addr);

					return res;
				}
			}
		}
	}

	return res;
}

DWORD Hooker::PatchExport(DWORD function, VOID* addr)
{
	PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (dataDir->Size)
	{
		PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((DWORD)this->hModule + dataDir->VirtualAddress);
		{
			DWORD* functions = (DWORD*)((DWORD)this->hModule + exports->AddressOfFunctions);

			for (DWORD i = 0; i < exports->NumberOfFunctions; ++i)
				if (function == (DWORD)this->hModule + functions[i])
					return this->PatchDWord((DWORD)&functions[i] - this->baseOffset, (DWORD)addr - (DWORD)this->hModule);
		}
	}

	return FALSE;
}