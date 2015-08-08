/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format has the property that removing any
 * vertex gives a graph containing a hamiltonian path
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_has_hypo_hamiltonian_path -O4 -DINVARIANT=hasHypoHamiltonianPath \
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
    int i;
    //mark the start vertex as being in the path
    currentPath[startVertex]=TRUE;
    for(i = 0; i < adj[startVertex]; i++){
        if(!currentPath[graph[startVertex][i]]){
            currentPath[graph[startVertex][i]]=TRUE;
            //search for path containing the edge (v, graph[v][i])
            if(continuePath(graph, adj, graph[startVertex][i], order - 2)){
                return TRUE;
            }
            currentPath[graph[startVertex][i]]=FALSE;
        }
    }
    currentPath[startVertex]=FALSE;
    
    return FALSE;
}

boolean remainingGraphHasHamiltonianPath(GRAPH graph, ADJACENCY adj, int remainingOrder, int removedVertex){
    int i;
    
    for(i = 1; i <= remainingOrder; i++){
        if(i!=removedVertex){
            //we try to start the path from each vertex (except the last)
            if(startPath(graph, adj, i, remainingOrder)){
                return TRUE;
            }
        }
    }
    return FALSE;
}

boolean hasHypoHamiltonianPath(GRAPH graph, ADJACENCY adj){
    int i, v;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    int minDegreeCount;
    
    if(order < 3){
        //not well defined for graphs with less than 3 vertices
        return FALSE;
    }
    
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
    
    if(minDegree == 1){
        //the graph obtained by removing the neighbour of the degree 1 vertex
        //is not connected and so cannot contain a hamiltonian path
        return FALSE;
    }
    
    if(2*(minDegree - 1) >= order - 1){
        //the graph has a hamiltonian cycle after removing any vertex,
        //so also a hamiltonian path
        return TRUE;
    }
    
    //just look for a hamiltonian path in all graphs
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    
    for(v = 1; v <= order; v++){
        currentPath[v] = TRUE;
        //we mark v as visited, so it is as if it got removed
        if(!remainingGraphHasHamiltonianPath(graph, adj, order-1, v)){
            return FALSE;
        }
        currentPath[v] = FALSE;
    }
    return TRUE;
    
}