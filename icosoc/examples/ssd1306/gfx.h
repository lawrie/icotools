/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

This is a cut down version of the Adafruit GFX library for BlackSoc.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "font.h"

static unsigned char buffer [1024];

static const int16_t WIDTH = 128;
static const int16_t HEIGHT = 64;

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return;

        uint8_t   *ptr  = &buffer[(y/8)*WIDTH + x];
        if(color) *ptr |=   0x1 << (y & 7);
        else      *ptr &= ~(0x1 << (y & 7));
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
        uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void drawBitmap(int16_t x, int16_t y,
  const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {

    int16_t byteWidth = (w + 7) / 8; 
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte >>= 1;
            else      byte   = bitmap[j * byteWidth + i / 8];
            drawPixel(x+i, y, (byte & 0x80));
        }
    }
}

void drawCircle(int16_t x0, int16_t y0, int16_t r,
        uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {
    for (int16_t i=x; i<x+w; i++) {
        drawLine(i, y, i, y+h-1, color);
    }
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {
    drawLine(x, y, x+w-1, y, color);
    drawLine(x, y+h-1, x+w-1, y+h-1, color);
    drawLine(x, y, x, y+h-1, color);
    drawLine(x+w-1, y, x+w-1, y+h-1, color);
}

void drawTriangle(int16_t x0, int16_t y0,
        int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void drawChar(int16_t x, int16_t y, unsigned char c,
  uint16_t color) {

    if((x >= WIDTH)            || // Clip right
       (y >= HEIGHT)           || // Clip bottom
       ((x + 6 - 1) < 0) || // Clip left
       ((y + 8 - 1) < 0))   // Clip top
        return;

    for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
        uint8_t line = font[c * 5 + i];
        for(int8_t j=0; j<8; j++, line >>= 1) {
            if(line & 1) {
                drawPixel(x+i, y+j, color);
            } else {
                drawPixel(x+i, y+j, 0);
            }
        }
    }
}

void drawText(int16_t x, int16_t y, const char *text, int16_t c) {
    for(int i=0;text[i];i++) {
        drawChar(x + i*6,y,text[i],c);
    }
}

