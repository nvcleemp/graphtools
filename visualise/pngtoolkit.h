/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PNGTOOLKIT_H
#define	PNGTOOLKIT_H

#include <png.h>
#include <stdint.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} colour_t;

/* A coloured pixel. */

typedef colour_t pixel_t;

/* A picture. */

typedef struct {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;

typedef struct {
    int x;
    int y;
} point_t;

void colourBackground(colour_t *backgroundColour, bitmap_t *image);

int savePng(bitmap_t *bitmap, const char *path);

void colourPixel(int x, int y, colour_t *colour, bitmap_t *image);

void drawLine(point_t p0, point_t p1, int width, colour_t *lineColour, bitmap_t *image);

void fillCircle(point_t center, int radius, colour_t *lineColour, bitmap_t *image);

void drawCircle(point_t center, int radius, colour_t *lineColour, bitmap_t *image);

#endif	/* PNGTOOLKIT_H */

