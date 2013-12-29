/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format is hamiltonian
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_is_hamiltonian -O4 -DINVARIANT=isHamiltonian \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_hamiltonian_cycles.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean currentCycle[MAXN+1];

/**
  * 
  */
boolean continueCycle(GRAPH graph, ADJACENCY adj, int target, int next, int remaining) {
    int i;
    
    if(target==next){
        if(remaining==0){
            return TRUE;
        }
        return FALSE;
    }
    
    for(i = 0; i < adj[next]; i++){
        if(!currentCycle[graph[next][i]]){
            currentCycle[graph[next][i]]=TRUE;
            if(continueCycle(graph, adj, target, graph[next][i], remaining - 1)){
                return TRUE;
            }
            currentCycle[graph[next][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean isHamiltonian(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    
    if(order<3){
        return FALSE;
    }
    
    //check conditions for Dirac's theorem
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
            minDegreeVertex = i;
        }
    }
    
    if(2*minDegree >= order){
        return TRUE;
    } else if(minDegree == 1){
        return FALSE;
    }
    
    //just look for a hamiltonian cycle
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
    }
    
    currentCycle[minDegreeVertex] = TRUE;
    for(i = 1; i < adj[minDegreeVertex]; i++){
        currentCycle[graph[minDegreeVertex][i]]=TRUE;
        for(j = 0; j < i; j++){
            //search for cycle containing graph[minDegreeVertex][i], minDegreeVertex,  graph[minDegreeVertex][j]
            if(continueCycle(graph, adj, graph[minDegreeVertex][j], graph[minDegreeVertex][i], order - 2)){
                return TRUE;
            }
        }
        currentCycle[graph[minDegreeVertex][i]]=FALSE;
    }
    
    return FALSE;
}