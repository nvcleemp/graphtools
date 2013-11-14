/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef CUBIC_OUTPUT_H
#define	CUBIC_OUTPUT_H

#include "cubic_base.h"
#include <stdio.h>
 
#ifdef	__cplusplus
extern "C" {
#endif

void writeCubicMultiCode(GRAPH graph, int vertexCount, FILE *f);

#ifdef	__cplusplus
}
#endif

#endif	/* CUBIC_OUTPUT_H */

