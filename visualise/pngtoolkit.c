/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "pngtoolkit.h"
#include <stdio.h>
#include <math.h>


void colourBackground(colour_t *backgroundColour, bitmap_t *image){
    int x, y;
    for(x = 0; x < image->width; x++){
        for(y = 0; y < image->height; y++){
            colourPixel(x, y, backgroundColour, image);
        }
    }
}

/* Given "bitmap", this returns the pixel of bitmap at the point 
   ("x", "y"). */

static pixel_t * pixel_at(bitmap_t * bitmap, int x, int y) {
    return bitmap->pixels + bitmap->width * y + x;
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */

int savePng(bitmap_t *bitmap, const char *path) {
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* "status" contains the return value of this function. At first
       it is set to a value which means 'failure'. When the routine
       has finished its work, it is set to a value which means
       'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
     */
    int pixel_size = 4;
    int depth = 8;

    fp = fopen(path, "wb");
    if (!fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    /* Set up error handling. */

    if (setjmp(png_jmpbuf(png_ptr))) {
        goto png_failure;
    }

    /* Set image attributes. */

    png_set_IHDR(png_ptr,
            info_ptr,
            bitmap->width,
            bitmap->height,
            depth,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */

    row_pointers = png_malloc(png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row =
                png_malloc(png_ptr, sizeof (uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at(bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
            *row++ = pixel->alpha;
        }
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */

    status = 0;

    for (y = 0; y < bitmap->height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);

png_failure:
    png_create_info_struct_failed :
            png_destroy_write_struct(&png_ptr, &info_ptr);
png_create_write_struct_failed:
    fclose(fp);
fopen_failed:
    return status;
}

void colourPixel(int x, int y, colour_t *colour, bitmap_t *image) {
    if (x < 0 || y < 0 || x >= image->width || y >= image->height){
        return;
    }
    pixel_t * pixel = pixel_at(image, x, y);
    pixel->red = colour->red;
    pixel->green = colour->green;
    pixel->blue = colour->blue;
    pixel->alpha = colour->alpha;
}

void drawLine(point_t p0, point_t p1, int width, colour_t *lineColour, bitmap_t *image);

void fillCircle(point_t center, int radius, colour_t *lineColour, bitmap_t *image) {
    int y;
    
    for (y = center.y - radius; y < center.y + radius + 1; y++) {
        point_t p0, p1;
        p0.x = center.x - (int)(sqrt((double) (radius * radius) - (-center.y + y)*(-center.y + y)));
        p1.x = center.x + (int)(sqrt((double) (radius * radius) - (-center.y + y)*(-center.y + y)));
        p0.y = p1.y = y;

        drawLine(p0, p1, 1, lineColour, image);
    }
}

void drawCircle(point_t center, int radius, colour_t *lineColour, bitmap_t *image) {
    // Midpoint circle algorithm: see http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x0, y0;
    x0 = center.x;
    y0 = center.y;

    int error = 1 - radius;
    int errorY = 1;
    int errorX = -2 * radius;
    int x = radius, y = 0;

    colourPixel(x0, y0 + radius, lineColour, image);
    colourPixel(x0, y0 - radius, lineColour, image);
    colourPixel(x0 + radius, y0, lineColour, image);
    colourPixel(x0 - radius, y0, lineColour, image);

    while (y < x) {
        if (error > 0) {
            x--;
            errorX += 2;
            error += errorX;
        }
        y++;
        errorY += 2;
        error += errorY;
        colourPixel(x0 + x, y0 + y, lineColour, image);
        colourPixel(x0 - x, y0 + y, lineColour, image);
        colourPixel(x0 + x, y0 - y, lineColour, image);
        colourPixel(x0 - x, y0 - y, lineColour, image);
        colourPixel(x0 + y, y0 + x, lineColour, image);
        colourPixel(x0 - y, y0 + x, lineColour, image);
        colourPixel(x0 + y, y0 - x, lineColour, image);
        colourPixel(x0 - y, y0 - x, lineColour, image);
    }
}


////////////////////////////////////////////////////////////////////////////

//based on GNU hp2xx

static struct {
    int u, v; /* delta x , delta y */
    int ku, kt, kv, kd; /* loop constants */
    int oct2;
    int quad4;
    point_t temp;
} murphy;

#define my_lrint(a) ((long)(a+0.5))

void murphy_paraline(point_t pt, int d1, colour_t *lineColour, bitmap_t *image) {
    /* implements Figure 5B */

    int p; /* pel counter, p=along line */
    d1 = -d1;

    for (p = 0; p <= murphy.u; p++) { /* test for end of parallel line */

        colourPixel(pt.x, pt.y, lineColour, image);

        if (d1 <= murphy.kt) { /* square move */
            if (murphy.oct2 == 0) {
                pt.x++;
            } else {
                if (murphy.quad4 == 0) {
                    pt.y++;
                } else {
                    pt.y--;
                }
            }
            d1 += murphy.kv;
        } else { /* diagonal move */
            pt.x++;
            if (murphy.quad4 == 0) {
                pt.y++;
            } else {
                pt.y--;
            }
            d1 += murphy.kd;
        }
    }
    murphy.temp = pt;
}

void murphy_wideline(point_t p0, point_t p1, int width, colour_t *lineColour, bitmap_t *image) {
    /* implements figure 5A - draws lines parallel to ideal line */

    float offset = width / 2.;

    point_t pt, ptx, ml1, ml2, ml1b, ml2b;

    int d0, d1; /* difference terms d0=perpendicular to line, d1=along line */

    int q; /* pel counter,q=perpendicular to line */
    int tmp;

    int dd; /* distance along line */
    int tk; /* thickness threshold */
    double ang; /* angle for initial point calculation */
    /* Initialisation */
    murphy.u = p1.x - p0.x; /* delta x */
    murphy.v = p1.y - p0.y; /* delta y */

    if (murphy.u < 0) { /* swap to make sure we are in quadrants 1 or 4 */
        pt = p0;
        p0 = p1;
        p1 = pt;
        murphy.u *= -1;
        murphy.v *= -1;
    }

    if (murphy.v < 0) { /* swap to 1st quadrant and flag */
        murphy.v *= -1;
        murphy.quad4 = 1;
    } else {
        murphy.quad4 = 0;
    }

    if (murphy.v > murphy.u) { /* swap things if in 2 octant */
        tmp = murphy.u;
        murphy.u = murphy.v;
        murphy.v = tmp;
        murphy.oct2 = 1;
    } else {
        murphy.oct2 = 0;
    }

    murphy.ku = murphy.u + murphy.u; /* change in l for square shift */
    murphy.kv = murphy.v + murphy.v; /* change in d for square shift */
    murphy.kd = murphy.kv - murphy.ku; /* change in d for diagonal shift */
    murphy.kt = murphy.u - murphy.kv; /* diag/square decision threshold */

    d0 = 0;
    d1 = 0;
    dd = 0;

    ang = atan((double) murphy.v / (double) murphy.u); /* calc new initial point - offset both sides of ideal */

    if (murphy.oct2 == 0) {
        pt.x = p0.x + my_lrint(offset * sin(ang));
        if (murphy.quad4 == 0) {
            pt.y = p0.y - my_lrint(offset * cos(ang));
        } else {
            pt.y = p0.y + my_lrint(offset * cos(ang));
        }
    } else {
        pt.x = p0.x - my_lrint(offset * cos(ang));
        if (murphy.quad4 == 0) {
            pt.y = p0.y + my_lrint(offset * sin(ang));
        } else {
            pt.y = p0.y - my_lrint(offset * sin(ang));
        }
    }

    tk = (int) (4. * hypot(pt.x - p0.x, pt.y - p0.y) * hypot(murphy.u, murphy.v)); /* used here for constant thickness line */

    ptx = pt;

    for (q = 0; dd <= tk; q++) { /* outer loop, stepping perpendicular to line */

        murphy_paraline(pt, d1, lineColour, image); /* call to inner loop - right edge */
        if (q == 0) {
            ml1 = pt;
            ml1b = murphy.temp;
        } else {
            ml2 = pt;
            ml2b = murphy.temp;
        }
        if (d0 < murphy.kt) { /* square move  - M2 */
            if (murphy.oct2 == 0) {
                if (murphy.quad4 == 0) {
                    pt.y++;
                } else {
                    pt.y--;
                }
            } else {
                pt.x++;
            }
        } else { /* diagonal move */
            dd += murphy.kv;
            d0 -= murphy.ku;
            if (d1 < murphy.kt) { /* normal diagonal - M3 */
                if (murphy.oct2 == 0) {
                    pt.x--;
                    if (murphy.quad4 == 0) {
                        pt.y++;
                    } else {
                        pt.y--;
                    }
                } else {
                    pt.x++;
                    if (murphy.quad4 == 0) {
                        pt.y--;
                    } else {
                        pt.y++;
                    }
                }
                d1 += murphy.kv;
            } else { /* double square move, extra parallel line */
                if (murphy.oct2 == 0) {
                    pt.x--;
                } else {
                    if (murphy.quad4 == 0) {
                        pt.y--;
                    } else {
                        pt.y++;
                    }
                }
                d1 += murphy.kd;
                if (dd > tk) {
                    return; /* breakout on the extra line */
                }
                murphy_paraline(pt, d1, lineColour, image);
                if (murphy.oct2 == 0) {
                    if (murphy.quad4 == 0) {
                        pt.y++;
                    } else {

                        pt.y--;
                    }
                } else {
                    pt.x++;
                }
            }
        }
        dd += murphy.ku;
        d0 += murphy.kv;
    }

}

////////////////////////////////////////////////////////////////////////////

void bresenham_line(int x0, int y0, int x1, int y1, colour_t *lineColour, bitmap_t *image) {
    // Bresenham's line algorithm: see http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    int dx = x0 > x1 ? x0 - x1 : x1 - x0;
    int dy = y0 > y1 ? y0 - y1 : y1 - y0;

    int sx = x0 > x1 ? -1 : 1;
    int sy = y0 > y1 ? -1 : 1;

    int err = dx - dy;

    while (x0 != x1 || y0 != y1) {

        colourPixel(x0, y0, lineColour, image);

        int e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (x0 == x1 && y0 == y1) break;
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    colourPixel(x0, y0, lineColour, image);
}

void drawLine(point_t p0, point_t p1, int width, colour_t *lineColour, bitmap_t *image) {
    if (width == 1) {
        bresenham_line(p0.x, p0.y, p1.x, p1.y, lineColour, image);
    } else {
        murphy_wideline(p0, p1, width, lineColour, image);
    }
}
