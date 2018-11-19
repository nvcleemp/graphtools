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
    boolean contains_header;
    boolean remove_internal_headers;

    int initial_code_length;

    int maxn;
    int maxn_factor;
    int maxn_offset;

    int maxval;
    int maxval_factor;
    int maxval_offset;
};
    
//the default code length is sufficient to store any graph with less than 100 vertices
#define DEFAULT_GRAPH_INPUT_OPTIONS(options) GRAPH_INPUT_OPTIONS options = {TRUE, TRUE, 700, 0, 1, 0, 0, 1, 0}

GRAPH *decode_multi_code(unsigned short* code, GRAPH_INPUT_OPTIONS *options);

unsigned short *read_multi_code(FILE *file, GRAPH_INPUT_OPTIONS *options);

GRAPH *read_and_decode_multi_code(FILE *f, GRAPH_INPUT_OPTIONS *options);

#endif	/* MULTICODE_INPUT_H */

