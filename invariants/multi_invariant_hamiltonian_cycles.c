/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the number of hamiltonian cycles of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_hamiltonian_cycles -O4 -DINVARIANT=hamiltonianCycles \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_hamiltonian_cycles.c
 */

#include "../multicode/shared/multicode_base.h"

unsigned int cycleCount;
boolean currentCycle[MAXN+1];

#ifndef HANDLE_CYCLE
#define HANDLE_CYCLE countCycle
#endif

void countCycle(){
    cycleCount++;
}

/**
  * 
  */
void continueCycle(GRAPH graph, ADJACENCY adj, int target, int next, int remaining) {
    int i;
    
    if(target==next){
        if(remaining==0){
            HANDLE_CYCLE();
        }
        return;
    }
    
    for(i = 0; i < adj[next]; i++){
        if(!currentCycle[graph[next][i]]){
            currentCycle[graph[next][i]]=TRUE;
            continueCycle(graph, adj, target, graph[next][i], remaining - 1);
            currentCycle[graph[next][i]]=FALSE;
        }
    }
    
}

int hamiltonianCycles(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
    }
    
    currentCycle[1] = TRUE;
    for(i = 0; i < adj[1]; i++){
        currentCycle[graph[1][i]]=TRUE;
        for(j = 0; j < i; j++){
            //search for cycle containing graph[1][i], 1,  graph[1][j]
            continueCycle(graph, adj, graph[1][j], graph[1][i], order - 2);
        }
        currentCycle[graph[1][i]]=FALSE;
    }
    
    return cycleCount;
}