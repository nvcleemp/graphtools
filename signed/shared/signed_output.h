/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef SIGNED_OUTPUT_H
#define	SIGNED_OUTPUT_H

#include "signed_base.h"
#include<stdio.h>
 
#ifdef	__cplusplus
extern "C" {
#endif

void writeSignedCode(GRAPH graph, ADJACENCY adj, int order, FILE *f);

void writeAsMultiCode(GRAPH graph, ADJACENCY adj, int order, FILE *f);

#ifdef	__cplusplus
}
#endif

#endif	/* SIGNED_OUTPUT_H */

