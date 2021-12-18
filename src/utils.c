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

#include <utils.h>

/*
	Allocates the nodes pointers in a ctx struct.
	parameters:
		ctx: pointer to the context struct
*/
void alloc_nodes(PETE_CTX *ctx)
{
	// Alocate as many nodes and flashes as pixels per frame.
	ctx->inc_nodes_gen           = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->dec_nodes_gen           = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->inc_nodes_red           = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->dec_nodes_red           = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->inc_nodes_saturated_red = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->dec_nodes_saturated_red = (struct NODE*)       malloc(ctx->width * ctx->height * sizeof(struct NODE));
	ctx->last_transitions_gen    = (struct TRANSITION*) malloc(ctx->width * ctx->height * sizeof(struct TRANSITION));
	ctx->last_transitions_red    = (struct TRANSITION*) malloc(ctx->width * ctx->height * sizeof(struct TRANSITION));

	for(uint8_t i = 0; i < 4; i++)
	{
		ctx->flashes_gen[i] = (struct FLASH*) malloc(ctx->width * ctx->height * sizeof(struct FLASH));
		ctx->flashes_red[i] = (struct FLASH*) malloc(ctx->width * ctx->height * sizeof(struct FLASH));
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
}

/*
	Frees the nodes pointers in a ctx struct before freeing the ctx itself.
	parameters:
		ctx: pointer to the context struct
*/
void free_ctx(PETE_CTX *ctx)
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

/*
	Gamma corrects an 8-bit (0-255) R, G or B value.
	parameters:
		value: the 8-bit R, G or B value to correct
	returns: the gamma corrected R, G or B value (0-1)
*/
double rgb8_to_gamma_corrected_rgb(uint8_t value)
{
	double value01 = (double)value / 255.0;

	double gamma_corrected = value01 <= 0.04045 ? value01 / 12.92 : pow((value01 + 0.055) / 1.055, 2.4);

	return gamma_corrected;
}

/*
	Calculates relative luminance from gamma corrected R, G and B values.
	parameters:
		r: gamma corrected R value
		g: gamma corrected G value
		b: gamma corrected B value
	returns: the relative luminance value (0-1)
*/
double rgb_to_luminance(double r, double g, double b)
{
	return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

/*
	Calculates the value whose change is measured in testing for red flashes.
	parameters:
		r: gamma corrected R value
		g: gamma corrected G value
		b: gamma corrected B value
	returns: the aforementioned value, max(0, (r-g-b)*320))
*/
double rgb_to_red_flash_val(double r, double g, double b)
{
	return fmax(0, (r-g-b)*320);
}

/*
	Returns whether the given RGB color counts as a saturated red
	parameters:
		r: gamma corrected R value
		g: gamma corrected G value
		b: gamma corrected B value
	returns: true if it's a saturated red and false if it's not
*/
bool is_saturated_red(double r, double g, double b)
{
	return r / (r+g+b) >= 0.8;
}