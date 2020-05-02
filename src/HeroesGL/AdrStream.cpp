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
#include "AdrStream.h"

AdrStream::AdrStream(audiere::OutputStream* stream, TrackInfo* track)
{
	this->count = 0;
	this->stream = stream;
	this->stream->ref();
	this->track = track;
}

AdrStream::~AdrStream()
{
	this->stream->unref();
}

VOID __stdcall AdrStream::ref()
{
	++this->count;
}

VOID __stdcall AdrStream::unref()
{
	if (!--this->count)
		delete this;
}

VOID __stdcall AdrStream::play()
{
	if (track->position)
		this->stream->setPosition(track->position);

	this->stream->play();
}

VOID __stdcall AdrStream::stop()
{
	this->stream->stop();
}

bool __stdcall AdrStream::isPlaying()
{
	return this->stream->isPlaying();
}

VOID __stdcall AdrStream::reset()
{
	this->stream->reset();
}

VOID __stdcall AdrStream::setRepeat(bool repeat)
{
	this->stream->setRepeat(repeat);
}

bool __stdcall AdrStream::getRepeat()
{
	return this->stream->getRepeat();;
}

VOID __stdcall AdrStream::setVolume(FLOAT volume)
{
	this->stream->setVolume(volume);
}

FLOAT __stdcall AdrStream::getVolume()
{
	return this->stream->getVolume();
}

VOID __stdcall AdrStream::setPan(FLOAT pan)
{
	this->stream->setPan(pan);
}

FLOAT __stdcall AdrStream::getPan()
{
	return this->stream->getPan();
}

VOID __stdcall AdrStream::setPitchShift(FLOAT shift)
{
	this->stream->setPitchShift(shift);
}

FLOAT __stdcall AdrStream::getPitchShift()
{
	return this->stream->getPitchShift();
}

bool __stdcall AdrStream::isSeekable()
{
	return this->stream->isSeekable();
}

INT __stdcall AdrStream::getLength()
{
	return this->stream->getLength();
}

VOID __stdcall AdrStream::setPosition(INT position)
{
	this->stream->setPosition(position);
}

INT __stdcall AdrStream::getPosition()
{
	if (track->isPositional)
		track->position = this->stream->getPosition();

	return track->position;
}
