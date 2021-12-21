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
PETE_CTX *pete_create_context(uint16_t width, uint16_t height, uint8_t fps, bool has_alpha)
{
	PETE_CTX *ctx = (PETE_CTX*)malloc(sizeof(PETE_CTX));
	ctx->width = width;
	ctx->height = height;
	ctx->fps = fps;
	ctx->has_alpha = has_alpha;
	
	// Alocate as many nodes and flashes as pixels per frame.
	ctx->inc_nodes_gen           = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->dec_nodes_gen           = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->inc_nodes_red           = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->dec_nodes_red           = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->inc_nodes_saturated_red = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->dec_nodes_saturated_red = (struct PETE_NODE*)       malloc(ctx->width * ctx->height * sizeof(struct PETE_NODE));
	ctx->last_transitions_gen    = (struct PETE_TRANSITION*) malloc(ctx->width * ctx->height * sizeof(struct PETE_TRANSITION));
	ctx->last_transitions_red    = (struct PETE_TRANSITION*) malloc(ctx->width * ctx->height * sizeof(struct PETE_TRANSITION));

	for(uint8_t i = 0; i < 4; i++)
	{
		ctx->flashes_gen[i] = (struct PETE_FLASH*) malloc(ctx->width * ctx->height * sizeof(struct PETE_FLASH));
		ctx->flashes_red[i] = (struct PETE_FLASH*) malloc(ctx->width * ctx->height * sizeof(struct PETE_FLASH));
	}

	for (uint64_t i = 0; i < ctx->width * ctx->height; i++)
	{
		// Ensure that any valid node is lower than the
		// down nodes at the start
		ctx->dec_nodes_gen[i].value = 1.1;
		ctx->dec_nodes_red[i].value = 1.1;
		ctx->dec_nodes_saturated_red[i].value = 1.1;

		ctx->last_transitions_gen[i].start_frame = -1;
		ctx->last_transitions_red[i].start_frame = -1;

		// Initialize flashes with negative start frames
		for(int j = 0; j < 3; j++)
		{
			ctx->flashes_gen[j][i].start_frame = -1;
			ctx->flashes_red[j][i].start_frame = -1;
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
	for(uint8_t i = 0; i < 4; i++)
	{
		free(ctx->flashes_gen[i]);
		free(ctx->flashes_red[i]);
	}
	free(ctx->inc_nodes_gen);
	free(ctx->dec_nodes_gen);
	free(ctx->inc_nodes_red);
	free(ctx->dec_nodes_red);
	free(ctx->inc_nodes_saturated_red);
	free(ctx->dec_nodes_saturated_red);
	free(ctx->last_transitions_gen);
	free(ctx->last_transitions_red);
	free(ctx);
}