/*
	MIT License

	Copyright (c) 2021 pete-video-analysis

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

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "types.h"

/*----------------------------------------------------------------------------*/

static void process_pixel(uint8_t red, uint8_t green, uint8_t blue, uint64_t idx, PETE_CTX *ctx);
static bool is_luminance_transition(double low_val, double high_val);
static bool is_red_transition(double low_val, bool low_sat, double high_val, bool high_sat);
static bool is_flash(PETE_DIR current_transition_direction, bool is_red, uint64_t idx, PETE_CTX *ctx);
static void push_flash(int start, int end, bool is_red, uint64_t idx, PETE_CTX *ctx);
static bool are_over_three_flashes_in_one_second(struct PETE_FLASH * (*flashes)[4], uint64_t idx, PETE_CTX *ctx);
static void push_transition(int start_frame, int end_frame, PETE_DIR dir, bool is_red, uint64_t idx, PETE_CTX *ctx);

#endif