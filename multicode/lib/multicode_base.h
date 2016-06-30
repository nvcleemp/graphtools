/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef MULTICODE_BASE_H
#define	MULTICODE_BASE_H

#include<stdlib.h>
#include<limits.h>
#include "boolean.h"

#define MAXCODELENGTH(g) (g->maxn * (g->maxval + 1))
#define EMPTY USHRT_MAX

typedef struct __graph GRAPH;

struct __graph {
    int n;
    
    int maxn;
    int maxval;
    
    unsigned short *graph;
    unsigned short *degrees;
};

#define NEIGHBOUR(g,v,i) (g->graph[v*(g->maxval)+i])

GRAPH *newGraph(int maxn, int maxval);

void freeGraph(GRAPH *graph);

void addEdge(GRAPH *graph, int v, int w);

void removeEdge(GRAPH *graph, int v, int w, boolean all);

void prepareGraph(GRAPH *graph, int vertexCount);

boolean areAdjacent(GRAPH *graph, int v, int w);

#endif	/* MULTICODE_BASE_H */

