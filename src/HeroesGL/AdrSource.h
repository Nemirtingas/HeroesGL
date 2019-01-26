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

#pragma once

#include "Audiere.h"
#include "Allocation.h"
#include "ExtraTypes.h"

typedef audiere::SampleSource*(__stdcall *ADROPENSAMPLESOURCE)(CHAR* path);

class AdrSource : public Allocation
{
private:
	DWORD count;

public:
	audiere::SampleSource* source;
	TrackInfo* track;

	AdrSource(audiere::SampleSource*, TrackInfo*);
	~AdrSource();

	// Inherited via SampleSource
	virtual VOID __stdcall ref();
	virtual VOID __stdcall unref();
	virtual VOID __stdcall getFormat(INT&, INT&, audiere::SampleFormat&);
	virtual INT __stdcall read(INT, VOID*);
	virtual VOID __stdcall reset();
	virtual bool __stdcall isSeekable();
	virtual INT __stdcall getLength();
	virtual VOID __stdcall setPosition(INT);
	virtual INT __stdcall getPosition();
};

