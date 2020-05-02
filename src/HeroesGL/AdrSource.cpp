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
#include "AdrSource.h"

AdrSource::AdrSource(audiere::SampleSource* source, TrackInfo* track)
{
	this->count = 0;
	this->source = source;
	this->source->ref();
	this->track = track;
}

AdrSource::~AdrSource()
{
	this->source->unref();
}

VOID __stdcall AdrSource::ref()
{
	++this->count;
}

VOID __stdcall AdrSource::unref()
{
	if (!--this->count)
		delete this;
}

VOID __stdcall AdrSource::getFormat(INT& channel_count, INT& sample_rate, audiere::SampleFormat& sample_format)
{
	this->source->getFormat(channel_count, sample_rate, sample_format);
}

INT __stdcall AdrSource::read(INT frame_count, VOID* buffer)
{
	return this->source->read(frame_count, buffer);
}

VOID __stdcall AdrSource::reset()
{
	this->source->reset();
}

bool __stdcall AdrSource::isSeekable()
{
	return this->source->isSeekable();
}

INT __stdcall AdrSource::getLength()
{
	return this->source->getLength();
}

VOID __stdcall AdrSource::setPosition(INT position)
{
	this->source->setPosition(position);
}

INT __stdcall AdrSource::getPosition()
{
	return this->source->getPosition();
}