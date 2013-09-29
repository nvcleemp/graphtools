/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef MULTICODE_OUTPUT_H
#define	MULTICODE_OUTPUT_H

#include "multicode_base.h"
#include<stdio.h>
 
#ifdef	__cplusplus
extern "C" {
#endif

void writeMultiCode(GRAPH graph, ADJACENCY adj, FILE *f);

#ifdef	__cplusplus
}
#endif

#endif	/* MULTICODE_OUTPUT_H */

