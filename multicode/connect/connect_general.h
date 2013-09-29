/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */


#ifndef CONNECT_GENERAL_H
#define	CONNECT_GENERAL_H

#include "../shared/multicode_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void initializeCopies(int copies, GRAPH originalGraph, ADJACENCY originalAdj,
                                  GRAPH copyGraph, ADJACENCY copyAdj);

void makeConnection(GRAPH graph, ADJACENCY adj, int originalSize, int from, int fromCopy, int to, int toCopy);


#ifdef	__cplusplus
}
#endif

#endif	/* CONNECT_GENERAL_H */

