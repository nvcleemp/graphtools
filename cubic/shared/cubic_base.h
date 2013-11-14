/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef CUBIC_BASE_H
#define	CUBIC_BASE_H

#include<stdlib.h>
#include<limits.h>

#ifndef MAXN
#define MAXN 100
#endif
#ifdef REG
#undef REG
#endif
#define REG 3
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef int boolean;

#define MAXCODELENGTH (MAXN * REG + MAXN)
#define EMPTY USHRT_MAX

typedef unsigned short GRAPH[MAXN][REG + 1]; //a bit more efficient to do +1
typedef unsigned short ADJACENCY[MAXN];

#ifdef	__cplusplus
extern "C" {
#endif

void addEdge(GRAPH graph, ADJACENCY adj, int v, int w);

#ifdef	__cplusplus
}
#endif

#endif	/* CUBIC_BASE_H */

