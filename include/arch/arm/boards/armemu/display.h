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

#ifndef __DISPLAY_H
#define __DISPLAY_H

// display
void clear_display(unsigned int color);
int test_display(void);
void draw_pixel(int x, int y, unsigned int color);
void fill_rect(int x, int y, int w, int h, unsigned int color);
void copy_rect(int x, int y, int w, int h, int x2, int y2);
unsigned int *get_fb(void);

// screen specs
#define SCREEN_X 640
#define SCREEN_Y 480
#define SCREEN_BITDEPTH 32
#define SCREEN_FB_SIZE (4*1024*1024)

// there is more buffer available for blitting to/from
#define SCREEN_BUF_Y (SCREEN_FB_SIZE / SCREEN_X / (SCREEN_BITDEPTH / 8)) 

#endif
