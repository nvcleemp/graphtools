/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */


#include "connect_general.h"

/**
 * After calling this function copyGraph will contain the given number of
 * copies of originalGraph. These copies will not be connected.
 */
void initializeCopies(int copies, GRAPH originalGraph, ADJACENCY originalAdj,
                                  GRAPH copyGraph, ADJACENCY copyAdj){
    int i, j, k;
    int originalSize = originalGraph[0][0];
    
    if (copies*originalSize > MAXN) {
        fprintf(stderr, "MAXN too small (%d)!\n", MAXN);
        exit(0);
    }
    
    copyGraph[0][0] = copies*originalSize;
    
    prepareGraph(copyGraph, copyAdj, copyGraph[0][0]);
    
    for (i=1; i<=originalSize; i++) {
        for (j=0; j<originalAdj[i]; j++){
            if (i<originalGraph[i][j]){
                int neighbour = originalGraph[i][j];
                for (k=0; k<copies; k++){
                    addEdge(copyGraph, copyAdj, i+k*originalSize, neighbour+k*originalSize);
                }
            }
        }
    }
}

void makeConnection(GRAPH graph, ADJACENCY adj, int originalSize, int from, int fromCopy, int to, int toCopy){
    addEdge(graph, adj, from + fromCopy*originalSize, to + toCopy*originalSize);
}
