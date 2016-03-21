/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format is traceable (i.e., has a 
 * hamiltonian path)
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_is_traceable -O4 -DINVARIANT=isTraceable \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_hamiltonian_cycles.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>
#include <stdlib.h>

boolean currentPath[MAXN+1];
int pathSequence[MAXN];
int pathLength;
boolean *adjacency;
boolean *connected;

void foundPath(){
    int i;
    int order = pathLength;
    int start = pathSequence[0];
    int end = pathSequence[pathLength-1];
    if(adjacency[start*(order+1) + end]){
        //we found a hamiltonian cycle
        //all adjacent vertices on the cycle are hamiltonian connected
        connected[start*(order+1) + end] = connected[end*(order+1) + start] = TRUE;
        for(i = 1; i < pathLength; i++){
            int v1 = pathSequence[i-1];
            int v2 = pathSequence[i];
            connected[v1*(order+1) + v2] = connected[v2*(order+1) + v1] = TRUE;
        }
    } else {
        connected[start*(order+1) + end] = connected[end*(order+1) + start] = TRUE;
    }
}

/**
  * 
  */
boolean continuePath(GRAPH graph, ADJACENCY adj, int last, int targetVertex, int remaining) {
    int i;
    if(remaining > 0 && currentPath[targetVertex]){
        return FALSE;
    }
    
    if(remaining==0){
        foundPath();
        if(last==targetVertex){
            return TRUE;
        } else {
            return FALSE;
        }
    }
    
    for(i = 0; i < adj[last]; i++){
        if(!currentPath[graph[last][i]]){
            currentPath[graph[last][i]]=TRUE;
            pathSequence[pathLength] = graph[last][i];
            pathLength++;
            if(continuePath(graph, adj, graph[last][i], targetVertex, remaining - 1)){
                return TRUE;
            }
            pathLength--;
            currentPath[graph[last][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean startPath(GRAPH graph, ADJACENCY adj, int startVertex, int targetVertex, int order){
    int i;
    
    currentPath[startVertex]=TRUE;
    for(i = 1; i < adj[startVertex]; i++){
        currentPath[graph[startVertex][i]]=TRUE;
        pathSequence[0] = startVertex;
        pathSequence[1] = graph[startVertex][i];
        pathLength = 2;
        //search for path containing the edge (v, graph[v][i])
        if(continuePath(graph, adj, graph[startVertex][i], targetVertex, order - 2)){
            return TRUE;
        }
        currentPath[graph[startVertex][i]]=FALSE;
    }
    currentPath[startVertex]=FALSE;
    
    return FALSE;
}

boolean checkHamiltonianConnected(GRAPH graph, ADJACENCY adj){
    int i, j, k;
    int order = graph[0][0];
    
    for(i=1; i<= order-1; i++){
        for(j=i+1; j<= order; j++){
            if(!connected[i*(order+1)+j]){
                for(k = 0; k < MAXN + 1; k++){
                    currentPath[k] = FALSE;
                }
                if(!startPath(graph, adj, i, j, order)){
                    fprintf(stderr, "not connected: %d - %d\n", i, j);
                    return FALSE;
                }
            }
        }
    }
    
    return TRUE;
}

boolean isHamiltonianConnected(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
            minDegreeVertex = i;
        }
    }
    
    if(minDegree == 1 && order > 2){
        //this graph cannot be hamiltonian-connected
        return FALSE;
    }
    
    //just look for a hamiltonian paths
    //TODO: just a quick and dirty version: really should be optimised
    adjacency = (boolean *)malloc(sizeof(boolean)*(order+1)*(order+1));
    connected = (boolean *)malloc(sizeof(boolean)*(order+1)*(order+1));
    
    for(i=1; i <= order; i++){
        for(j=1; j <= order; j++){
            adjacency[i*(order+1)+j] = FALSE;
            connected[i*(order+1)+j] = FALSE;
        }
        for(j=0; j<adj[i]; j++){
            adjacency[i*(order+1)+graph[i][j]] = TRUE;
        }
    }
    
    boolean hamiltonianConnected = checkHamiltonianConnected(graph, adj);
    
    free(adjacency);
    free(connected);
    
    return hamiltonianConnected;
}
