/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef MULTICODE_BASE_H
#define	MULTICODE_BASE_H

#include<stdlib.h>
#include<limits.h>

#ifndef MAXN
#define MAXN 4000
#endif
#ifndef MAXVAL
#define MAXVAL 100
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef int boolean;

#define MAXCODELENGTH (MAXN * MAXVAL + MAXN)
#define EMPTY USHRT_MAX

typedef unsigned short GRAPH[MAXN + 1][MAXVAL + 1];
typedef unsigned short ADJACENCY[MAXN + 1];

#ifdef	__cplusplus
extern "C" {
#endif

void addEdge(GRAPH graph, ADJACENCY adj, int v, int w);

void removeEdge(GRAPH graph, ADJACENCY adj, int v, int w, boolean all);

void prepareGraph(GRAPH graph, ADJACENCY adj, int vertexCount);

boolean areAdjacent(GRAPH graph, ADJACENCY adj, int v, int w);

#ifdef	__cplusplus
}
#endif

#endif	/* MULTICODE_BASE_H */

