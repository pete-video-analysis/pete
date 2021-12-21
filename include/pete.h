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

// API header file

#ifndef PETE_H
#define PETE_H

#include "types.h"

/*----------------------------------------------------------------------------*/

// User can define these functions as "listeners" for pete "events"

/*
	Called when a frame has finished being processed.
	parameters:
		ctx: pointer to the context in which the frame was processed. Can be used to distinguish between contexts.
*/
extern void (*pete_request_next_frame)(PETE_CTX *ctx);

/*
	Called when a flash is detected in a video. Flashes are common and don't mean a video is unsafe.
	parameters:
		flash: pointer to the flash struct, containing the start and end frame.
		x: the x position of the pixel in which the flash was detected.
		y: the y position of the pixel in which the flash was detected.
		is_red: whether the flash is a general flash or red flash.
		ctx: pointer to the context in which the flash was detected. Can be used to distinguish between contexts.
*/
extern void (*pete_notify_flash)(struct PETE_FLASH* flash, uint16_t x, uint16_t y, bool is_red, PETE_CTX *ctx);

/*
	Called when more than three flashes are detected in a video within the span of a single second. This means a video is unsafe.
	parameters:
		start: the start frame of the oldest flash.
		end: the end frame of the newest flash.
		x: the x position of the pixel in which the flashes were detected.
		y: the y position of the pixel in which the flashes were detected.
		ctx: pointer to the context in which the flashes were detected. Can be used to distinguish between contexts.
*/
extern void (*pete_notify_over_three_flashes)(uint64_t start, uint64_t end, uint16_t x, uint16_t y, bool is_red, PETE_CTX *ctx);

/*----------------------------------------------------------------------------*/

PETE_CTX *pete_create_context(uint16_t width, uint16_t height, uint8_t fps, bool has_alpha);
void pete_free_ctx(PETE_CTX *ctx);

// Defined in analysis.c
void pete_receive_frame(uint8_t *data, PETE_CTX *ctx);

#endif