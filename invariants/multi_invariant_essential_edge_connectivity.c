/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the edge connectivity of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_edge_connectivity -O4 -DINVARIANT=edge_connectivity \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     connectivity/multi_connectivity.c \
 *     multi_invariant_order.c
 */

#include "../multicode/shared/multicode_base.h"
#include "connectivity/multi_connectivity.h"

#include <stdio.h>

/*
 * Contract the edges (from1, to1) and (from2, to2). This function assumes that the edges
 * are disjoint. The result is stored in the new graph. The newly formed vertices
 * have number 1 and 2.
 */
void contractEdges(GRAPH oldGraph, ADJACENCY oldAdj, GRAPH newGraph, ADJACENCY newAdj, int from1, int to1, int from2, int to2){
    int i, j;
    int order = oldGraph[0][0];
    int old2new[order+1];
    for(i=1; i<=order; i++){
        old2new[i]=-1;
    }
    
    int newVertexCounter = 1;
    old2new[from1] = old2new[to1] = newVertexCounter++;
    old2new[from2] = old2new[to2] = newVertexCounter++;
    for(i=1; i<=order; i++){
        if(old2new[i]==-1){
            old2new[i] = newVertexCounter++;
        }
    }
    
    prepareGraph(newGraph, newAdj, order-2);
    
    for(i=1; i<=order; i++){
        for(j=0; j<oldAdj[i]; j++){
            int neighbour = oldGraph[i][j];
            if((i==from1 && neighbour==to1) || (i==to1 && neighbour==from1)) continue;
            if((i==from2 && neighbour==to2) || (i==to2 && neighbour==from2)) continue;
            if(i < neighbour){
                addEdge(newGraph, newAdj, old2new[i], old2new[neighbour]);
            }
        }
    }
}

int findEssentialEdgeConnectivity(GRAPH graph, ADJACENCY adj){
    int i, j, i2, j2;
    GRAPH modifiedGraph;
    ADJACENCY modifiedAdj;
    int minimumCutSize;
    
    minimumCutSize = graph[0][0]*(graph[0][0]-1)/2;
    
    for (i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(adj[i] + adj[graph[i][j]] < minimumCutSize){
                minimumCutSize = adj[i] + adj[graph[i][j]];
            }
        }
    }
    
    for (i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(i<graph[i][j]){
                for (i2 = i+1; i2 <= graph[0][0]; i2++){
                    for(j2 = 0; j2 < adj[i2]; j2++){
                        if(i2<graph[i2][j2] && i < i2 && graph[i][j]!=i2 && graph[i][j]!=graph[i2][j2]){
                            contractEdges(graph, adj, modifiedGraph, modifiedAdj, i, graph[i][j], i2, graph[i2][j2]);
                            minimumCutSize = findMaxFlowInSTNetwork(modifiedGraph, modifiedAdj, 1, 2, minimumCutSize);
                        }
                    }
                }
            }
        }
    }
    
    return minimumCutSize;
}

int essential_edge_connectivity(GRAPH graph, ADJACENCY adj){
    return findEssentialEdgeConnectivity(graph, adj);
}