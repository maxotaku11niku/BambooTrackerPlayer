/*
 * Copyright (C) 2023 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <stdexcept>

/// Repeat type
enum SampleRepeatFlag : int
{
	ShouldRewriteStart	= 0b001,
	ShouldRewriteStop	= 0b010,
	ShouldRepeat		= 0b100,

	/*
	 * |---| repeat range
	 * Disabled:		|        |
	 * LeftPartial:		|-----|  |
	 * RightPartial:	|  |-----|
	 * MiddlePartial:	| |----| |
	 * Simple:			|--------|
	 */
	Disabled		= 0,
	LeftPartial		= ShouldRepeat | ShouldRewriteStop,
	RightPartial	= ShouldRepeat | ShouldRewriteStart,
	MiddlePartial	= ShouldRepeat | ShouldRewriteStart | ShouldRewriteStop,
	Simple			= ShouldRepeat,
};

/// Immutable safe range class
/// handles position as ADPCM memory address by 32 bytes
class SampleRepeatRange
{
public:
	SampleRepeatRange(size_t first, size_t last)
	{
		if (last < first) {
			//throw std::invalid_argument("Invalid range"); MH - Godot disables exceptions
			last_ = first; //MH - Clearly Bamboo Tracker itself should probably correct this while saving the file, but just in case
			first_ = last;
		}
		else
		{
			first_ = first;
			last_ = last;
		}
	}

	size_t first() const noexcept { return first_; }
	size_t last() const noexcept { return last_; }
	SampleRepeatRange clampLast(size_t last) const noexcept
	{
		return SampleRepeatRange(std::min(first_, last), std::min(last_, last));
	}

private:
	size_t first_, last_;
};
