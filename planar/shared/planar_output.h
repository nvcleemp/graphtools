/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANAR_OUTPUT_H
#define	PLANAR_OUTPUT_H

#include "planar_base.h"
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void writePlanarCode(PLANE_GRAPH *pg, FILE *f);

void writeEdgeCode(PLANE_GRAPH *pg, FILE *f);

#ifdef	__cplusplus
}
#endif

#endif	/* PLANAR_OUTPUT_H */

