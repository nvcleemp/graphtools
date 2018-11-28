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

GRAPH *new_graph(int maxn, int maxval);

void free_graph(GRAPH *graph);

void add_edge(GRAPH *graph, int v, int w);

void remove_edge(GRAPH *graph, int v, int w, boolean all);

void prepare_graph(GRAPH *graph, int vertex_count);

boolean are_adjacent(GRAPH *graph, int v, int w);

int get_graph_order(GRAPH *graph);
int get_graph_size(GRAPH *graph);
int get_maximum_degree(GRAPH *graph);
int get_minimum_degree(GRAPH *graph);

#endif	/* MULTICODE_BASE_H */

