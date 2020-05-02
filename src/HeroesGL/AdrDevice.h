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

#include "Audiere.h"
#include "Allocation.h"

typedef audiere::AudioDevice*(__stdcall *ADROPENDEVICE)(CHAR* name, CHAR* parameters);

class AdrDevice : public Allocation
{
private:
	DWORD count;

public:
	audiere::AudioDevice* device;

	AdrDevice(audiere::AudioDevice*);
	~AdrDevice();

	// Inherited via AudioDevice
	virtual VOID __stdcall ref();
	virtual VOID __stdcall unref();
	virtual VOID __stdcall update();
	virtual audiere::OutputStream* __stdcall openStream(audiere::SampleSource*);
	virtual audiere::OutputStream* __stdcall openBuffer(VOID*, INT, INT, INT, audiere::SampleFormat);
};

