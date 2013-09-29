/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef MULTICODE_INPUT_H
#define	MULTICODE_INPUT_H

#include "multicode_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void decodeMultiCode(unsigned short* code, int length, GRAPH graph, ADJACENCY adj);

int readMultiCode(unsigned short code[], int *length, FILE *file);

#ifdef	__cplusplus
}
#endif

#endif	/* MULTICODE_INPUT_H */

