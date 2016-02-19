/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANAR_INPUT_H
#define	PLANAR_INPUT_H

#include "planar_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct __plane_graph_input_options PG_INPUT_OPTIONS;
    
    struct __plane_graph_input_options {
        boolean containsHeader;
        boolean removeInternalHeaders;
        
        int initialCodeLength;
        
        int maxn;
        int maxnFactor;
        int maxe;
        
        boolean computeDual;
    };
    
//the default code length is sufficient to store any graph with less than 100 vertices
#define DEFAULT_PG_INPUT_OPTIONS(options) PG_INPUT_OPTIONS options = {TRUE, TRUE, 700, 0, 1, 0, FALSE}

    PLANE_GRAPH *decodePlanarCode(unsigned short* code, PG_INPUT_OPTIONS *options);

    unsigned short *readPlanarCode(FILE *file, PG_INPUT_OPTIONS *options);
    
    PLANE_GRAPH *readAndDecodePlanarCode(FILE *f, PG_INPUT_OPTIONS *options);

#ifdef	__cplusplus
}
#endif

#endif	/* PLANAR_INPUT_H */

