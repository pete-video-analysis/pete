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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/*----------------------------------------------------------------------------*/

// Color functionss

/*
	Gamma corrects an 8-bit (0-255) R, G or B value.
	parameters:
		value: the 8-bit R, G or B value to correct
	returns: the gamma corrected R, G or B value (0-1)
*/
static double rgb8_to_gamma_corrected_rgb(const uint8_t value)
{
	const double value01 = (double)value / 255.0;

	const double gamma_corrected = value01 <= 0.04045 ? value01 / 12.92 : pow((value01 + 0.055) / 1.055, 2.4);

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
static double rgb_to_luminance(const double r, const double g, const double b)
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
static double rgb_to_red_flash_val(const double r, const double g, const double b)
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
static bool is_saturated_red(const double r, const double g, const double b)
{
	return r / (r+g+b) >= 0.8;
}

#endif