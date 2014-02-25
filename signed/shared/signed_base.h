/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef SIGNED_BASE_H
#define	SIGNED_BASE_H

#include<stdlib.h>
#include<limits.h>

#ifndef MAXN
#define MAXN 4000
#endif
#ifndef MAXVAL
#define MAXVAL 10
#endif
#ifndef MAXE
#define MAXE (MAXVAL*MAXN/2)
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define POSITIVE 1
#define NEGATIVE 0

typedef int boolean;

#define MAXCODELENGTH (2 * MAXN * MAXVAL + MAXN)
#define EMPTY USHRT_MAX


typedef struct e /* The data type used for edges */ {
    int smallest; /* smallest vertex incident to this edge */
    int largest; /* largest vertex incident to this edge */
    
    boolean isNegative; /* FALSE if this edge is positive, and TRUE otherwise*/
    
    int mark, index; /* two ints for temporary use;
                          Only access mark via the MARK macros. */
} EDGE;

typedef EDGE *GRAPH[MAXN + 1][MAXVAL + 1];
typedef unsigned short ADJACENCY[MAXN + 1];
    
EDGE edges[MAXE];
extern int edgeCounter;

#ifdef	__cplusplus
extern "C" {
#endif

void addEdge(GRAPH graph, ADJACENCY adj, int v, int w, int sign);

void removeEdge(GRAPH graph, ADJACENCY adj, int v, int w, boolean all);

void prepareGraph(GRAPH graph, ADJACENCY adj, int vertexCount);

boolean areAdjacent(GRAPH graph, ADJACENCY adj, int v, int w);

boolean areAdjacentWithSign(GRAPH graph, ADJACENCY adj, int v, int w, int sign);

#ifdef	__cplusplus
}
#endif

#endif	/* SIGNED_BASE_H */

