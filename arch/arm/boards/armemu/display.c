/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "arch/display.h"
#include "arch/memmap.h"

static unsigned int *fb = (unsigned int *)DISPLAY_BASE;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void clear_display(unsigned int color)
{
	int i;

	fill_rect(0, 0, SCREEN_X, SCREEN_Y, color);
}

void draw_pixel(int x, int y, unsigned int color)
{
	fb[x*y] = color;
}

unsigned int *get_fb(void)
{
	return fb;
}

// fills a rect from x,y width w and height h with color
void fill_rect(int x, int y, int w, int h, unsigned int color)
{
	int i, j;
	unsigned int *dest;
	int stride;
	
	if(x < 0 || x > SCREEN_X)
		return;
	if(y < 0 || y > SCREEN_BUF_Y)
		return;		
	if(w < 0 || h < 0)
		return;

	// clips the width in case the source rect goes off the side
	if(x + w > SCREEN_X)
		w = SCREEN_X - x; // clip the source rect

	// clips the height to the full framebuffer size
	if(y + h > SCREEN_BUF_Y)
		h = SCREEN_BUF_Y - y; // clip the source rect

	// start the copy
	dest = &fb[x + y * SCREEN_X];
	stride = SCREEN_X - w;
	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			*dest = color;
			dest++;
		}
		// increment the pointer by stride
		dest += stride;
	}
}

// copies a rect from x,y width w and height h to x2,y2
void copy_rect(int x, int y, int w, int h, int x2, int y2)
{
	int i, j;
	unsigned int *src, *dest;
	int stride;
	
	if(x < 0 || x > SCREEN_X)
		return;
	if(y < 0 || y > SCREEN_BUF_Y)
		return;		
	if(x2 < 0 || x2 > SCREEN_X)
		return;
	if(y2 < 0 || y2 > SCREEN_BUF_Y)
		return;		
	if(w < 0 || h < 0)
		return;

	// clips the width, in case the source or dest rect goes off the side
	if(x + w > SCREEN_X)
		w = SCREEN_X - x; // clip the source rect
	if(x2 + w > SCREEN_X)
		w = SCREEN_X - x2; // clip the dest rect

	// clips the height to the full framebuffer size
	if(y + h > SCREEN_BUF_Y)
		h = SCREEN_BUF_Y - y; // clip the source rect
	if(y2 + h > SCREEN_BUF_Y)
		h = SCREEN_BUF_Y - y2; // clip the dest rect

	// start the copy
	src = &fb[x + y * SCREEN_X];
	dest = &fb[x2 + y2 * SCREEN_X];
	stride = SCREEN_X - w;
	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			*dest = *src;
			dest++;
			src++;
		}
		// increment both pointers by stride
		dest += stride;
		src += stride;
	}
}

// blits a rect to the screen from memory
void blit(unsigned int *buf, int x, int y, int w, int h)
{
	int i, j;
	unsigned int *src, *dest;
	int src_stride = 0;
	int dest_stride;

	if(x < 0 || x > SCREEN_X)
		return;
	if(y < 0 || y > SCREEN_BUF_Y)
		return;		
	if(w < 0 || h < 0)
		return;

	// clips the width, in case the source or dest rect goes off the side
	if(x + w > SCREEN_X) {
		src_stride = w - (SCREEN_X - x);
		w = SCREEN_X - x; // clip the source rect
	}

	// clips the height to the full framebuffer size
	if(y + h > SCREEN_BUF_Y)
		h = SCREEN_BUF_Y - y; // clip the source rect

	// start the copy
	src = buf;
	dest = &fb[x + y * SCREEN_X];
	dest_stride = SCREEN_X - w;
	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			*dest = *src;
			dest++;
			src++;
		}
		// increment both pointers by stride
		dest += dest_stride;
		src += src_stride;
	}
}

int test_display(void)
{
	int i;
	static foo = 0;

	for(i=0; i < SCREEN_X * SCREEN_Y; i++) {
		fb[i] = i + foo;
	}

	foo++;

	return 0;
}
