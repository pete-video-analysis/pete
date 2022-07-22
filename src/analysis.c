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

#include <stddef.h>
#include "analysis.h"
#include "utils.h"

void (*pete_request_next_frame)(const PETE_CTX *const ctx) = NULL;
void (*pete_notify_flash)(const int start, const int end, const int x, const int y, const bool is_red, const PETE_CTX *const ctx) = NULL;
void (*pete_notify_over_three_flashes)(const int start, const int end, const int x, const int y, const PETE_CTX *const ctx) = NULL;

/*
	Processes the next frame in a video.
	parameters:
		data: pointer to the frame buffer for the received frame, in RGB8 or RGBA8 format. POINTER IS NOT FREED INSIDE THIS METHOD!!
		ctx: pointer to the context allocated for the analysis of the video
*/
void pete_receive_frame(uint8_t *const data, PETE_CTX *const ctx)
{
	uint64_t channels = ctx->has_alpha ? 4 : 3;
	// Process each pixel
	for(uint64_t y = 0; y < ctx->height; y++)
	{
		for(uint64_t x = 0; x < ctx->width; x++)
		{
			uint64_t pixel_index = (y * (uint64_t)ctx->width) + x;
			uint64_t data_index = pixel_index * channels;

			process_pixel(
				data[data_index + PETE_CHANNEL_R],
				data[data_index + PETE_CHANNEL_G],
				data[data_index + PETE_CHANNEL_B],
				pixel_index,
				ctx
			);
		}
	
	}

	ctx->current_frame++;
	if(pete_request_next_frame != NULL)
		pete_request_next_frame(ctx);
}

static void process_pixel(const uint8_t red, const uint8_t green, const uint8_t blue, const uint64_t idx, PETE_CTX *const ctx)
{
	double R = rgb8_to_gamma_corrected_rgb(red);
	double G = rgb8_to_gamma_corrected_rgb(green);
	double B = rgb8_to_gamma_corrected_rgb(blue);

	// General flashes
	double relative_luminance = rgb_to_luminance(R, G, B);
	
	struct PETE_PIX *const pixel = &(ctx->pixels[idx]);

	if(is_luminance_transition(pixel->dec_node_gen.value, relative_luminance))
	{	
		if(is_flash(PETE_DIR_INC, pixel->last_trans_gen))
		{
			push_flash(pixel->last_trans_gen.start_frame, ctx->current_frame, pixel->flashes_gen, false, idx, ctx);
		}

		push_transition(pixel->dec_node_gen.frame, ctx->current_frame, PETE_DIR_INC, &pixel->last_trans_gen);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = relative_luminance,
			.saturated_red = false // unused
		};
		pixel->inc_node_gen = pixel->dec_node_gen = current;
	}
	else if(is_luminance_transition(relative_luminance, pixel->inc_node_gen.value))
	{
		if(is_flash(PETE_DIR_DEC, pixel->last_trans_gen))
		{
			push_flash(pixel->last_trans_gen.start_frame, ctx->current_frame, pixel->flashes_gen, false, idx, ctx);
		}

		push_transition(pixel->inc_node_gen.frame, ctx->current_frame, PETE_DIR_DEC, &pixel->last_trans_gen);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = relative_luminance,
			.saturated_red = false // unused
		};
		pixel->inc_node_gen = pixel->dec_node_gen = current;
	}

	if(relative_luminance >= pixel->inc_node_gen.value)
	{
		pixel->inc_node_gen.frame = ctx->current_frame;
		pixel->inc_node_gen.value = relative_luminance;
	}

	if(relative_luminance <= pixel->dec_node_gen.value)
	{
		pixel->dec_node_gen.frame = ctx->current_frame;
		pixel->dec_node_gen.value = relative_luminance;
	}

	// Red flashes
	double red_flash_val = rgb_to_red_flash_val(R, G, B);
	bool is_saturated = is_saturated_red(R, G, B);

	if(is_red_transition(pixel->dec_node_red.value, pixel->dec_node_red.saturated_red, red_flash_val, is_saturated))
	{
		if(is_flash(PETE_DIR_INC, pixel->last_trans_red))
		{
			push_flash(pixel->last_trans_red.start_frame, ctx->current_frame, pixel->flashes_red, true, idx, ctx);
		}

		push_transition(pixel->dec_node_red.frame, ctx->current_frame, PETE_DIR_INC, &pixel->last_trans_red);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		pixel->dec_node_red = pixel->inc_node_red = current;
		pixel->dec_node_sat_red = pixel->inc_node_sat_red = current;
		pixel->dec_node_sat_red.saturated_red = true;
		pixel->inc_node_sat_red.saturated_red = true;
	}
	else if(is_red_transition(red_flash_val, is_saturated, pixel->inc_node_red.value, pixel->inc_node_red.saturated_red))
	{
		if(is_flash(PETE_DIR_DEC, pixel->last_trans_red))
		{
			push_flash(pixel->last_trans_red.start_frame, ctx->current_frame, pixel->flashes_red, true, idx, ctx);
		}

		push_transition(pixel->inc_node_red.frame, ctx->current_frame, PETE_DIR_DEC, &pixel->last_trans_red);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		pixel->dec_node_red = pixel->inc_node_red = current;
		pixel->dec_node_sat_red = pixel->inc_node_sat_red = current;
		pixel->dec_node_sat_red.saturated_red = true;
		pixel->inc_node_sat_red.saturated_red = true;
	}
	else if(is_red_transition(pixel->dec_node_sat_red.value, true, red_flash_val, is_saturated))
	{
		if(is_flash(PETE_DIR_INC, pixel->last_trans_red))
		{
			push_flash(pixel->last_trans_red.start_frame, ctx->current_frame, pixel->flashes_red, true, idx, ctx);
		}

		push_transition(pixel->dec_node_sat_red.frame, ctx->current_frame, PETE_DIR_INC, &pixel->last_trans_red);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		pixel->dec_node_red = pixel->inc_node_red = current;
		pixel->dec_node_sat_red = pixel->inc_node_sat_red = current;
		pixel->dec_node_sat_red.saturated_red = true;
		pixel->inc_node_sat_red.saturated_red = true;
	}
	else if(is_red_transition(red_flash_val, is_saturated, pixel->inc_node_sat_red.value, true))
	{
		if(is_flash(PETE_DIR_DEC, pixel->last_trans_red))
		{
			push_flash(pixel->last_trans_red.start_frame, ctx->current_frame, pixel->flashes_red, true, idx, ctx);
		}

		push_transition(pixel->inc_node_sat_red.frame, ctx->current_frame, PETE_DIR_DEC, &pixel->last_trans_red);
		// Reset nodes
		struct PETE_NODE current = {
			.frame = ctx->current_frame,
			.value = red_flash_val,
			.saturated_red = is_saturated
		};
		pixel->dec_node_red = pixel->inc_node_red = current;
		pixel->dec_node_sat_red = pixel->inc_node_sat_red = current;
		pixel->dec_node_sat_red.saturated_red = true;
		pixel->inc_node_sat_red.saturated_red = true;
	}

	if(red_flash_val >= pixel->inc_node_red.value)
	{
		pixel->inc_node_red.frame = ctx->current_frame;
		pixel->inc_node_red.value = red_flash_val;
	}

	if(red_flash_val <= pixel->dec_node_red.value)
	{
		pixel->dec_node_red.frame = ctx->current_frame;
		pixel->dec_node_red.value = red_flash_val;
	}

	if(red_flash_val >= pixel->inc_node_sat_red.value && is_saturated)
	{
		pixel->inc_node_sat_red.frame = ctx->current_frame;
		pixel->inc_node_sat_red.value = red_flash_val;
	}

	if(red_flash_val <= pixel->dec_node_sat_red.value && is_saturated)
	{
		pixel->dec_node_sat_red.frame = ctx->current_frame;
		pixel->dec_node_sat_red.value = red_flash_val;
	}
}

static bool is_luminance_transition(const double low_val, const double high_val)
{
	if(high_val == 0.0) return false;
	return high_val - low_val >= 0.1 && low_val < 0.8;
}

static bool is_red_transition(const double low_val, const bool low_sat, const double high_val, const bool high_sat)
{
	if(high_val == 0.0) return false;
	if(!low_sat && !high_sat) return false;
	return high_val - low_val > 20.0;
}

static bool is_flash(const PETE_DIR current_transition_direction, const struct PETE_TRANSITION last_trans)
{
	if(last_trans.direction == current_transition_direction)
		return false;
		
	if(last_trans.start_frame == -1)
		return false;

	return true;
}

static void push_flash(const int start, const int end, struct PETE_FLASH flashes[4], const bool is_red, const int idx, const PETE_CTX *const ctx)
{
	flashes[3] = flashes[2];
	flashes[2] = flashes[1];
	flashes[1] = flashes[0];
	flashes[0].start_frame = start;
	flashes[0].end_frame = end;

	if(pete_notify_flash != NULL)
	{
		uint16_t x = idx % ctx->width;
		uint16_t y = (idx - x) / ctx->width;
		struct PETE_FLASH flash = flashes[0];
		pete_notify_flash(flash.start_frame, flash.end_frame, x, y, is_red, ctx);
	}

	if(are_over_three_flashes_in_one_second(flashes, ctx) && pete_notify_over_three_flashes != NULL)
	{
		uint16_t x = idx % ctx->width;
		uint16_t y = (idx - x) / ctx->width;
		pete_notify_over_three_flashes(flashes[3].start_frame, flashes[0].end_frame, x, y, ctx);
	}
}

static bool are_over_three_flashes_in_one_second(struct PETE_FLASH flashes[4], const PETE_CTX *const ctx)
{
	// Check if there have been 3 flashes before checking if they happened in one second
	for(int i = 0; i < 4; ++i)
	{
		// If start frame is negative for any flash, it means that there
		// haven't been 3 flashes yet
		if(flashes[i].start_frame < 0) return false;
	}

	int time_span = flashes[0].end_frame - flashes[3].start_frame;
	
	return time_span <= ctx->fps;
}

static void push_transition(const int start_frame, const int end_frame, const PETE_DIR dir, struct PETE_TRANSITION *const last_trans)
{
	last_trans->start_frame = start_frame;
	last_trans->end_frame = end_frame;
	last_trans->direction = dir;
}