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

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// Used define bool instead of enum for performance
#define DIRECTION bool
#define INC true
#define DEC false

enum channels
{
	RED,
	GREEN,
	BLUE,
	ALPHA // Unused
};

/*----------------------------------------------------------------------------*/

typedef struct NODE
{
	int frame;

	double value;
} NODE;

typedef struct TRANSITION
{
	int start_frame, end_frame;

	DIRECTION direction;
} TRANSITION;

typedef struct FLASH
{
	int start_frame, end_frame;
} FLASH;

typedef struct VIDEO
{
	uint16_t width, height;

	// For non-integer fps, round up to the nearest integer
	uint8_t fps;

	// Whether the frames will include an alpha channel or not
	bool has_alpha;

	// Nodes used as running counters of the highest
	// and lowest points since the las transition
	NODE *inc_nodes_gen, *dec_nodes_gen;
	NODE *inc_nodes_red, *dec_nodes_red;

	// The last transition
	// If its direction opposes a new transition, it's a flash
	TRANSITION *last_transitions_gen;
	TRANSITION *last_transitions_red;

	// The last 3 general flashes
	FLASH *flashes_gen[3];
	// The last 3 red flashes
	FLASH *flashes_red[3];
} VIDEO;

/*----------------------------------------------------------------------------*/

// Video methods
void alloc_nodes(VIDEO *video);
void free_video(VIDEO *video);

// Color methods
double rgb8_to_gamma_corrected_rgb(uint8_t value);
double rgb_to_luminance(double r, double g, double b);

// JSON methods
char* make_flash_json(int start_frame, int end_frame, bool is_red, int x, int y);

#endif