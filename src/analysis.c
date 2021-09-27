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

#include <analysis.h>
#include <stdio.h>

VIDEO *video;
uint64_t current_frame = 0;
void (*pete_request_next_frame)() = NULL;
void (*pete_notify_flash)(FLASH* flash, uint16_t x, uint16_t y, bool is_red) = NULL;
void (*pete_notify_three_flashes)(uint64_t start, uint64_t end, uint16_t x, uint16_t y, bool is_red) = NULL;

void pete_set_metadata(uint16_t width, uint16_t height, uint8_t fps, bool has_alpha)
{
	video = (VIDEO*)malloc(sizeof(VIDEO));
	video->width = width;
	video->height = height;
	video->fps = fps;
	video->has_alpha = has_alpha;
	alloc_nodes(video);
}

void pete_receive_frame(uint8_t *data)
{
	uint64_t channels = video->has_alpha ? 4 : 3;
	// Process each pixel
	for(uint64_t y = 0; y < video->height; y++)
	{
		for(uint64_t x = 0; x < video->width; x++)
		{
			uint64_t node_index = (y * (uint64_t)video->width) + x;
			uint64_t data_index = node_index * channels;

			process_pixel(
				data[data_index+RED],
				data[data_index+GREEN],
				data[data_index+BLUE],
				node_index
			);
		}
	
	}

	current_frame++;
	if(pete_request_next_frame != NULL)
		pete_request_next_frame();
}

void process_pixel(uint8_t red, uint8_t green, uint8_t blue, uint64_t idx)
{
	double R = rgb8_to_gamma_corrected_rgb(red);
	double G = rgb8_to_gamma_corrected_rgb(green);
	double B = rgb8_to_gamma_corrected_rgb(blue);

	// General flashes
	double relative_luminance = rgb_to_luminance(R, G, B);
	
	NODE *current_inc = &(video->inc_nodes_gen[idx]);
	NODE *current_dec = &(video->dec_nodes_gen[idx]);
	
	

	if(is_luminance_transition(current_dec->value, relative_luminance))
	{	
		if(is_flash(INC, false, idx))
		{
			push_flash(video->last_transitions_gen[idx].start_frame, current_frame, false, idx);
		}

		push_transition(current_dec->frame, current_frame, INC, false, idx);
		reset_nodes(current_frame, relative_luminance, false, idx);
	}
	else if(is_luminance_transition(relative_luminance, current_inc->value))
	{
		if(is_flash(DEC, false, idx))
		{
			push_flash(video->last_transitions_gen[idx].start_frame, current_frame, false, idx);
		}

		push_transition(current_inc->frame, current_frame, DEC, false, idx);
		reset_nodes(current_frame, relative_luminance, false, idx);
	}

	if(relative_luminance >= current_inc->value)
	{
		current_inc->frame = current_frame;
		current_inc->value = relative_luminance;
	}

	if(relative_luminance <= current_dec->value)
	{
		current_dec->frame = current_frame;
		current_dec->value = relative_luminance;
	}
}

bool is_luminance_transition(double low_val, double high_val)
{
	if(high_val == 0.0) return false;
	return high_val - low_val >= 0.1*high_val && low_val < 0.8;
}

bool is_flash(DIRECTION current_transition_direction, bool is_red, uint64_t idx)
{
	TRANSITION last_transition = is_red ? video->last_transitions_red[idx] : video->last_transitions_gen[idx];

	if(last_transition.direction == current_transition_direction)
		return false;
		
	if(last_transition.start_frame == -1)
		return false;

	return true;
}

void push_flash(int start, int end, bool is_red, uint64_t idx)
{
	FLASH * (*flashes)[3] = is_red ? &(video->flashes_red) : &(video->flashes_gen);

	(*flashes)[2][idx] = (*flashes)[1][idx];
	(*flashes)[1][idx] = (*flashes)[0][idx];
	(*flashes)[0][idx].start_frame = start;
	(*flashes)[0][idx].end_frame = end;

	if(pete_notify_flash != NULL)
	{
		uint16_t x = idx % video->width;
		uint16_t y = (idx - x) / video->width;
		FLASH *flash = &((*flashes)[0][idx]);
		pete_notify_flash(flash, x, y, is_red);
	}

	if(are_three_flashes_in_one_second(flashes, idx) && pete_notify_three_flashes != NULL)
	{
		uint16_t x = idx % video->width;
		uint16_t y = (idx - x) / video->width;
		pete_notify_three_flashes((*flashes)[2][idx].start_frame, (*flashes)[0][idx].end_frame, x, y, is_red);
	}
}

bool are_three_flashes_in_one_second(FLASH * (*flashes)[3], uint64_t idx)
{
	// Check if there have been 3 flashes before checking if they happened in one second
	for(int i = 0; i < 3; i++)
	{
		// If start frame is negative for any flash, it means that there
		// haven't been 3 flashes yet
		if((*flashes)[i][idx].start_frame < 0) return false;
	}

	int time_span = (*flashes)[0][idx].end_frame - (*flashes)[2][idx].start_frame;

	return time_span <= video->fps;
}

void push_transition(int start_frame, int end_frame, DIRECTION dir, bool is_red, uint64_t idx)
{
	TRANSITION *last_transitions = is_red ? video->last_transitions_red : video->last_transitions_gen;

	last_transitions[idx].start_frame = start_frame;
	last_transitions[idx].end_frame = end_frame;
	last_transitions[idx].direction = dir;
}

void reset_nodes(int frame, double val, bool is_red, uint64_t idx)
{
	NODE *inc_nodes = is_red ? video->inc_nodes_red : video->inc_nodes_gen;
	NODE *dec_nodes = is_red ? video->dec_nodes_red : video->dec_nodes_gen;

	inc_nodes[idx].frame = frame;
	inc_nodes[idx].value = val;
	dec_nodes[idx].frame = frame;
	dec_nodes[idx].value = val;
}