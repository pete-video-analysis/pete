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
void (*pete_notify_over_three_flashes)(uint64_t start, uint64_t end, uint16_t x, uint16_t y, bool is_red) = NULL;

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
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = relative_luminance,
			.saturated_red = false // unused
		};
		*current_dec = *current_inc = current;
	}
	else if(is_luminance_transition(relative_luminance, current_inc->value))
	{
		if(is_flash(DEC, false, idx))
		{
			push_flash(video->last_transitions_gen[idx].start_frame, current_frame, false, idx);
		}

		push_transition(current_inc->frame, current_frame, DEC, false, idx);
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = relative_luminance,
			.saturated_red = false // unused
		};
		*current_dec = *current_inc = current;
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

	// Red flashes
	double red_flash_val = rgb_to_red_flash_val(R, G, B);
	bool is_saturated = is_saturated_red(R, G, B);

	current_inc = &(video->inc_nodes_red[idx]);
	current_dec = &(video->dec_nodes_red[idx]);
	NODE *current_saturated_inc = &(video->inc_nodes_saturated_red[idx]);
	NODE *current_saturated_dec = &(video->dec_nodes_saturated_red[idx]);

	if(is_red_transition(current_dec->value, current_dec->saturated_red, red_flash_val, is_saturated))
	{
		if(is_flash(INC, true, idx))
		{
			push_flash(video->last_transitions_red[idx].start_frame, current_frame, true, idx);
		}

		push_transition(current_dec->frame, current_frame, INC, true, idx);
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		*current_dec = *current_inc = current;
		*current_saturated_dec = *current_saturated_inc = current;
		current_saturated_dec->saturated_red = true;
		current_saturated_inc->saturated_red = true;
	}
	else if(is_red_transition(red_flash_val, is_saturated, current_inc->value, current_inc->saturated_red))
	{
		if(is_flash(DEC, true, idx))
		{
			push_flash(video->last_transitions_red[idx].start_frame, current_frame, true, idx);
		}

		push_transition(current_dec->frame, current_frame, DEC, true, idx);
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		*current_dec = *current_inc = current;
		*current_saturated_dec = *current_saturated_inc = current;
		current_saturated_dec->saturated_red = true;
		current_saturated_inc->saturated_red = true;
	}
	else if(is_red_transition(current_saturated_dec->value, true, red_flash_val, is_saturated))
	{
		if(is_flash(INC, true, idx))
		{
			push_flash(video->last_transitions_red[idx].start_frame, current_frame, true, idx);
		}

		push_transition(current_dec->frame, current_frame, INC, true, idx);
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		*current_dec = *current_inc = current;
		*current_saturated_dec = *current_saturated_inc = current;
		current_saturated_dec->saturated_red = true;
		current_saturated_inc->saturated_red = true;
	}
	else if(is_red_transition(red_flash_val, is_saturated, current_saturated_inc->value, true))
	{
		if(is_flash(DEC, true, idx))
		{
			push_flash(video->last_transitions_red[idx].start_frame, current_frame, true, idx);
		}

		push_transition(current_saturated_dec->frame, current_frame, DEC, true, idx);
		// Reset nodes
		NODE current = {
			.frame = current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		*current_dec = *current_inc = current;
		*current_saturated_dec = *current_saturated_inc = current;
		current_saturated_dec->saturated_red = true;
		current_saturated_inc->saturated_red = true;
	}

	if(red_flash_val >= current_inc->value)
	{
		current_inc->frame = current_frame;
		current_inc->value = red_flash_val;
	}

	if(red_flash_val <= current_dec->value)
	{
		current_dec->frame = current_frame;
		current_dec->value = red_flash_val;
	}

	if(red_flash_val >= current_saturated_inc->value && is_saturated)
	{
		current_saturated_inc->frame = current_frame;
		current_saturated_inc->value = red_flash_val;
	}

	if(red_flash_val <= current_saturated_dec->value && is_saturated)
	{
		current_saturated_dec->frame = current_frame;
		current_saturated_dec->value = red_flash_val;
	}
}

bool is_luminance_transition(double low_val, double high_val)
{
	if(high_val == 0.0) return false;
	return high_val - low_val >= 0.1 && low_val < 0.8;
}

bool is_red_transition(double low_val, bool low_sat, double high_val, bool high_sat)
{
	if(high_val == 0.0) return false;
	if(!low_sat && !high_sat) return false;
	return high_val - low_val > 20.0;
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
	FLASH * (*flashes)[4] = is_red ? &(video->flashes_red) : &(video->flashes_gen);

	(*flashes)[3][idx] = (*flashes)[2][idx];
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

	if(are_over_three_flashes_in_one_second(flashes, idx) && pete_notify_over_three_flashes != NULL)
	{
		uint16_t x = idx % video->width;
		uint16_t y = (idx - x) / video->width;
		pete_notify_over_three_flashes((*flashes)[3][idx].start_frame, (*flashes)[0][idx].end_frame, x, y, is_red);
	}
}

bool are_over_three_flashes_in_one_second(FLASH * (*flashes)[4], uint64_t idx)
{
	// Check if there have been 3 flashes before checking if they happened in one second
	for(int i = 0; i < 4; i++)
	{
		// If start frame is negative for any flash, it means that there
		// haven't been 3 flashes yet
		if((*flashes)[i][idx].start_frame < 0) return false;
	}

	int time_span = (*flashes)[0][idx].end_frame - (*flashes)[3][idx].start_frame;
	
	return time_span <= video->fps;
}

void push_transition(int start_frame, int end_frame, DIRECTION dir, bool is_red, uint64_t idx)
{
	TRANSITION *last_transitions = is_red ? video->last_transitions_red : video->last_transitions_gen;

	last_transitions[idx].start_frame = start_frame;
	last_transitions[idx].end_frame = end_frame;
	last_transitions[idx].direction = dir;
}