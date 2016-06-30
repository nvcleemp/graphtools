/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef MULTICODE_INPUT_H
#define	MULTICODE_INPUT_H

#include "multicode_base.h"
#include<stdio.h>

typedef struct __graph_input_options GRAPH_INPUT_OPTIONS;

struct __graph_input_options {
    boolean containsHeader;
    boolean removeInternalHeaders;

    int initialCodeLength;

    int maxn;
    int maxnFactor;
    int maxnOffset;

    int maxval;
    int maxvalFactor;
    int maxvalOffset;
};
    
//the default code length is sufficient to store any graph with less than 100 vertices
#define DEFAULT_GRAPH_INPUT_OPTIONS(options) GRAPH_INPUT_OPTIONS options = {TRUE, TRUE, 700, 0, 1, 0, 0, 1, 0}

GRAPH *decodeMultiCode(unsigned short* code, GRAPH_INPUT_OPTIONS *options);

unsigned short *readMultiCode(FILE *file, GRAPH_INPUT_OPTIONS *options);

GRAPH *readAndDecodeMultiCode(FILE *f, GRAPH_INPUT_OPTIONS *options);

#endif	/* MULTICODE_INPUT_H */

