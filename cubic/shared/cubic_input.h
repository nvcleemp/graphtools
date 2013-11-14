/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef CUBIC_INPUT_H
#define	CUBIC_INPUT_H

#include "cubic_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void decodeCubicMultiCode(unsigned short* code, int length, GRAPH graph, int *vertexCount);

int readCubicMultiCode(unsigned short code[], int *length, FILE *file);

#ifdef	__cplusplus
}
#endif

#endif	/* CUBIC_INPUT_H */

