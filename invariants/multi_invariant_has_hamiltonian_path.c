/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format has a hamiltonian path
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_has_hamiltonian_path -O4 -DINVARIANT=hasHamiltonianPath \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_hamiltonian_cycles.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean currentPath[MAXN+1];

/**
  * 
  */
boolean continuePath(GRAPH graph, ADJACENCY adj, int last, int remaining) {
    int i;
    
    if(remaining==0){
        return TRUE;
    }
    
    for(i = 0; i < adj[last]; i++){
        if(!currentPath[graph[last][i]]){
            currentPath[graph[last][i]]=TRUE;
            if(continuePath(graph, adj, graph[last][i], remaining - 1)){
                return TRUE;
            }
            currentPath[graph[last][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean startPath(GRAPH graph, ADJACENCY adj, int startVertex, int order){
    int i, j;
    
    currentPath[startVertex]=TRUE;
    for(i = 1; i < adj[startVertex]; i++){
        currentPath[graph[startVertex][i]]=TRUE;
        for(j = 0; j < i; j++){
            //search for path containing the edge (v, graph[v][i])
            if(continuePath(graph, adj, graph[startVertex][i], order - 2)){
                return TRUE;
            }
        }
        currentPath[graph[startVertex][i]]=FALSE;
    }
    currentPath[startVertex]=FALSE;
    
    return FALSE;
}

boolean hasHamiltonianPath(GRAPH graph, ADJACENCY adj){
    int i, j, v;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    int minDegreeCount;
    
    //check conditions for Dirac's theorem
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
            minDegreeVertex = i;
            minDegreeCount = 1;
        } else if(adj[i] == minDegree){
            minDegreeCount++;
        }
    }
    
    if(minDegree == 1 && minDegreeCount > 2){
        return FALSE;
    }
    
    if(2*minDegree >= order){
        //the graph has a hamiltonian cycle, so also a hamiltonian path
        return TRUE;
    }
    
    //just look for a hamiltonian path
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    
    if(minDegree > 1){
        for(v = 0; v < order-1; v++){
            //we try to start the path from each vertex (except the last)
            if(startPath(graph, adj, v, order)){
                return TRUE;
            }
        }
        return FALSE;
    } else {
        //if there is a degree 1 vertex, then any hamiltonian path will start with that vertex
        return startPath(graph, adj, minDegreeVertex, order);
    }
    
}