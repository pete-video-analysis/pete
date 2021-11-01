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
	Allocates the nodes pointers in a video struct.
	parameters:
		video: pointer to the video struct
*/
void alloc_nodes(VIDEO *video)
{
	// Alocate as many nodes and flashes as pixels per frame.
	video->inc_nodes_gen        = (NODE*)       malloc(video->width * video->height * sizeof(NODE));
	video->dec_nodes_gen        = (NODE*)       malloc(video->width * video->height * sizeof(NODE));
	video->inc_nodes_red        = (NODE*)       malloc(video->width * video->height * sizeof(NODE));
	video->dec_nodes_red        = (NODE*)       malloc(video->width * video->height * sizeof(NODE));
	video->last_transitions_gen = (TRANSITION*) malloc(video->width * video->height * sizeof(TRANSITION));
	video->last_transitions_red = (TRANSITION*) malloc(video->width * video->height * sizeof(TRANSITION));

	for(uint8_t i = 0; i < 4; i++)
	{
		video->flashes_gen[i] = (FLASH*) malloc(video->width * video->height * sizeof(FLASH));
		video->flashes_red[i] = (FLASH*) malloc(video->width * video->height * sizeof(FLASH));
	}

	for (uint64_t i = 0; i < video->width * video->height; i++)
	{
		// Ensure that any valid node is lower than the
		// down nodes at the start
		video->dec_nodes_gen[i].value = 1.1;
		video->dec_nodes_red[i].value = 1.1;

		video->last_transitions_gen[i].start_frame = -1;
		video->last_transitions_red[i].start_frame = -1;

		// Initialize flashes with negative start frames
		for(int j = 0; j < 3; j++)
		{
			video->flashes_gen[j][i].start_frame = -1;
			video->flashes_red[j][i].start_frame = -1;
		}
	}
}

/*
	Frees the nodes pointers in a video struct before freeing the video itself.
	parameters:
		video: pointer to the video struct
*/
void free_video(VIDEO *video)
{
	for(uint8_t i = 0; i < 4; i++)
	{
		free(video->flashes_gen[i]);
		free(video->flashes_red[i]);
	}
	free(video->inc_nodes_gen);
	free(video->dec_nodes_gen);
	free(video->inc_nodes_red);
	free(video->dec_nodes_red);
	free(video->last_transitions_gen);
	free(video->last_transitions_red);
	free(video);
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