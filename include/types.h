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

#ifndef PETE_TYPES_H
#define PETE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

enum
{
	PETE_CHANNEL_R,
	PETE_CHANNEL_G,
	PETE_CHANNEL_B,
	PETE_CHANNEL_A // Unused
};

// Define PETE_DIR type as enum
enum
{
	PETE_DIR_DEC,
	PETE_DIR_INC
};
typedef uint8_t PETE_DIR;

/*----------------------------------------------------------------------------*/

struct PETE_NODE
{
	int frame;

	double value;

	// Unused for general flashes
	bool saturated_red;
};

struct PETE_TRANSITION
{
	int start_frame, end_frame;

	PETE_DIR direction;
};

struct PETE_FLASH
{
	int start_frame, end_frame;
};

struct PETE_PIX
{
	// Nodes used as running counters of the highest
	// and lowest points since the las transition
	struct PETE_NODE inc_node_gen, dec_node_gen;
	struct PETE_NODE inc_node_red, dec_node_red;
	// Red nodes exclusively for saturated reds
	struct PETE_NODE inc_node_sat_red, dec_node_sat_red;

	// The last transition
	// If its direction opposes a new transition, it's a flash
	struct PETE_TRANSITION last_trans_gen, last_trans_red;

	// The last 4 general and red flashes
	struct PETE_FLASH flashes_gen[4], flashes_red[4];
};

// Typedef PETE_CTX as it's user facing
typedef struct PETE_CTX
{
	uint16_t width, height;

	// For non-integer fps, round up to the nearest integer
	uint8_t fps;

	// Whether the frames will include an alpha channel or not
	bool has_alpha;

	// The current frame
	uint64_t current_frame;

	// Dynamic array of pixels
	struct PETE_PIX *pixels;
} PETE_CTX;

#endif