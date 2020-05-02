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
#include "AdrDevice.h"
#include "AdrStream.h"
#include "AdrSource.h"

AdrDevice::AdrDevice(audiere::AudioDevice* device)
{
	this->count = 0;
	this->device = device;
	this->device->ref();
}

AdrDevice::~AdrDevice()
{
	this->device->unref();
}

VOID __stdcall AdrDevice::ref()
{
	++this->count;
}

VOID __stdcall AdrDevice::unref()
{
	if (!--this->count)
		delete this;
}

VOID __stdcall AdrDevice::update()
{
	this->device->update();
}

audiere::OutputStream* __stdcall AdrDevice::openStream(audiere::SampleSource* source)
{
	audiere::OutputStream* stream = this->device->openStream(source);
	return stream ? (audiere::OutputStream*)new AdrStream(stream, ((AdrSource*)source)->track) : NULL;
}

audiere::OutputStream* __stdcall AdrDevice::openBuffer(VOID* samples, INT frame_count, INT channel_count, INT sample_rate, audiere::SampleFormat sample_format)
{
	return this->device->openBuffer(samples, frame_count, channel_count, sample_rate, sample_format);
}
