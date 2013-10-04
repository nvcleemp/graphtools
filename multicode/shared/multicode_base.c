/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */


#include "multicode_base.h"

/* This method adds the edge (v,w) to graph. This assumes that adj contains
 * the current degree of the vertices v and w. This degrees are then updated.
 */
void addEdge(GRAPH graph, ADJACENCY adj, int v, int w) {
    graph[v][adj[v]] = w;
    graph[w][adj[w]] = v;
    adj[v]++;
    adj[w]++;
}

void prepareGraph(GRAPH graph, ADJACENCY adj, int vertexCount) {
    int i, j;
    
    //mark all vertices as having degree 0
    for (i = 1; i <= vertexCount; i++) {
        adj[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = EMPTY;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = 0;
    }
    
    graph[0][0] = vertexCount;
}
