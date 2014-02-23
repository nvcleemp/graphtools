/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef SIGNED_INPUT_H
#define	SIGNED_INPUT_H

#include "signed_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void decodeSignedCode(unsigned short* code, int length, GRAPH graph, ADJACENCY adj, int* order);

int readSignedCode(unsigned short code[], int *length, FILE *file);

void decodeMultiCode(unsigned short* code, int length, GRAPH graph, ADJACENCY adj, int* order);

int readMultiCode(unsigned short code[], int *length, FILE *file);

#ifdef	__cplusplus
}
#endif

#endif	/* SIGNED_INPUT_H */

