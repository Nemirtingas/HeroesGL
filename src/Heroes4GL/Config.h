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

#pragma once

#include "ExtraTypes.h"

#define CONFIG_APP "Application"
#define CONFIG_WRAPPER "Wrapper"
#define CONFIG_COLORS "Colors"
#define CONFIG_KEYS "FunktionKeys"
#define RESOURCE_ICON 107

extern ConfigItems config;

extern Adjustment activeColors;
extern const Adjustment inactiveColors;
extern const Adjustment defaultColors;

namespace Config
{
	VOID Load(HMODULE hModule, const AddressSpace* hookSpace);
	BOOL Check(const CHAR* app, const CHAR* key);
	INT Get(const CHAR* app, const CHAR* key, INT default);
	DWORD Get(const CHAR* app, const CHAR* key, const CHAR* default, CHAR* returnString, DWORD nSize);
	BOOL Set(const CHAR* app, const CHAR* key, INT value);
	BOOL Set(const CHAR* app, const CHAR* key, CHAR* value);

	VOID SetProcessMask();
}