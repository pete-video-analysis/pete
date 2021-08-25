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

#include<utils.h>

/*----------------------------------------------------------------------------*/

extern uint64_t current_frame;

extern void (*request_next_frame)();
extern void (*send_message)(char*);

extern VIDEO *video;

/*----------------------------------------------------------------------------*/

// API
void pete_set_metadata(uint16_t width, uint16_t height, uint8_t fps, bool has_alpha);
void pete_set_callbacks(void (*request_next_frame), void (*send_message)(char*));
void pete_receive_frame(uint8_t *data);

// Internal
void process_pixel(uint8_t red, uint8_t green, uint8_t blue, uint64_t idx);
bool is_luminance_transition(double low_val, double high_val);
// bool is_red_transition(stuff lol);
bool is_flash(DIRECTION current_transition_direction, bool is_red, uint64_t idx);
void push_flash(int start, int end, bool is_red, uint64_t idx);
void push_transition(int start_frame, int end_frame, DIRECTION dir, bool is_red, uint64_t idx);
void reset_nodes(int frame, double val, bool is_red, uint64_t idx);

#endif