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

#include "pete.h"
#include "utils.h"
#include "types.h"
#include <stdlib.h>

/*
	Creates a context struct, allocates the pointers within it and initializes necessary elements
	parameters:
		width: the width of the video being analyzed, in pixels
		height: the height of the video being analyzed, in pixels
		fps: the fps (frames per second) of the video being analyzed (for non-integer fps, round to nearest integer)
		has_alpha: whether the frame buffers corresponding to the video being analyzed include an alpha channel
	returns:
		the created and initialized PETE_CTX struct
*/
PETE_CTX *pete_create_context(const uint16_t width, const uint16_t height, const uint8_t fps, const bool has_alpha)
{
	PETE_CTX *ctx = (PETE_CTX*)malloc(sizeof(PETE_CTX));
	ctx->width = width;
	ctx->height = height;
	ctx->fps = fps;
	ctx->has_alpha = has_alpha;
	
	// Alocate pixels
	ctx->pixels = (struct PETE_PIX*) malloc(width * height * sizeof(struct PETE_PIX));

	for (uint64_t i = 0; i < ctx->width * ctx->height; ++i)
	{
		struct PETE_PIX *pixel = &(ctx->pixels[i]);
		// Ensure that any valid node is lower than the
		// down nodes at the start
		pixel->dec_node_gen.value = 1.1;
		pixel->dec_node_red.value = 1.1;
		pixel->dec_node_sat_red.value = 1.1;

		pixel->last_trans_gen.start_frame = -1;
		pixel->last_trans_red.start_frame = -1;

		// Initialize flashes with negative start frames
		for(int j = 0; j < 3; j++)
		{
			pixel->flashes_gen[j].start_frame = -1;
			pixel->flashes_red[j].start_frame = -1;
		}
	}

	return ctx;
}

/*
	Frees the nodes pointers in a context struct before freeing the context itself.
	parameters:
		ctx: pointer to the context struct
*/
void pete_free_ctx(PETE_CTX *ctx)
{
	free(ctx->pixels);
	free(ctx);
}