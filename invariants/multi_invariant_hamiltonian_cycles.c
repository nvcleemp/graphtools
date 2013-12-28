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
#include <stdio.h>

#if INVARIANT == hamiltonianCyclesEdgeIncidence
#define CONSTRUCT_CYCLE
#define HANDLE_CYCLE countCycleEdgeIncidence
#endif

#if INVARIANT == hamiltonianCyclesUniversalEdges
#define CONSTRUCT_CYCLE
#define HANDLE_CYCLE countCycleEdgeIncidence
#endif

#if INVARIANT == hamiltonianCyclesUncoveredEdges
#define CONSTRUCT_CYCLE
#define HANDLE_CYCLE countCycleEdgeIncidence
#endif

#ifndef HANDLE_CYCLE
#define HANDLE_CYCLE countCycle
#endif

unsigned int cycleCount;
boolean currentCycle[MAXN+1];

//this array will only be filled if the macro CONSTRUCT_CYCLE is defined
int currentCycleVertexOrder[MAXN];

int cycleEdgeIncidence[MAXN][MAXN];

void countCycle(GRAPH graph, ADJACENCY adj){
    cycleCount++;
}

void countCycleEdgeIncidence(GRAPH graph, ADJACENCY adj){
    int i;
    
    cycleCount++;
    
    for(i = 1; i < graph[0][0]; i++){
        cycleEdgeIncidence[currentCycleVertexOrder[i-1]][currentCycleVertexOrder[i]]++;
        cycleEdgeIncidence[currentCycleVertexOrder[i]][currentCycleVertexOrder[i-1]]++;
    }
    cycleEdgeIncidence[currentCycleVertexOrder[0]][currentCycleVertexOrder[graph[0][0]-1]]++;
    cycleEdgeIncidence[currentCycleVertexOrder[graph[0][0]-1]][currentCycleVertexOrder[0]]++;
    
}

double processCycleEdgeIncidence(GRAPH graph, ADJACENCY adj){
    int i, j;
    int a = 0, b = 0;
    
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(cycleEdgeIncidence[i][graph[i][j]] == cycleCount){
                a++;
            } else if(cycleEdgeIncidence[i][graph[i][j]] == 0){
                b++;
            }
        }
    }
    
    return 1.0*b/a;
}

int processCycleUniversalEdges(GRAPH graph, ADJACENCY adj){
    int i, j;
    int a = 0;
    
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(cycleEdgeIncidence[i][graph[i][j]] == cycleCount){
                a++;
            }
        }
    }
    
    return a/2;
}

int processCycleUncoveredEdges(GRAPH graph, ADJACENCY adj){
    int i, j;
    int b = 0;
    
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(cycleEdgeIncidence[i][graph[i][j]] == 0){
                b++;
            }
        }
    }
    
    return b/2;
}

/**
  * 
  */
void continueCycle(GRAPH graph, ADJACENCY adj, int target, int next, int remaining) {
    int i;
    
    if(target==next){
        if(remaining==0){
            HANDLE_CYCLE(graph, adj);
        }
        return;
    }
    
#ifdef CONSTRUCT_CYCLE
    currentCycleVertexOrder[graph[0][0]-remaining] = next;
#endif
    
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
    cycleCount = 0;
    
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
    }
    
#ifdef CONSTRUCT_CYCLE
    currentCycleVertexOrder[1] = 1;
#endif
    
    currentCycle[1] = TRUE;
    for(i = 1; i < adj[1]; i++){
        currentCycle[graph[1][i]]=TRUE;
        for(j = 0; j < i; j++){
#ifdef CONSTRUCT_CYCLE
            currentCycleVertexOrder[0] = graph[1][j];
#endif
            //search for cycle containing graph[1][i], 1,  graph[1][j]
            continueCycle(graph, adj, graph[1][j], graph[1][i], order - 2);
        }
        currentCycle[graph[1][i]]=FALSE;
    }
    
    return cycleCount;
}

double hamiltonianCyclesEdgeIncidence(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    cycleCount = 0;
    
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
        for(j = 0; j <= MAXN; j++){
            cycleEdgeIncidence[i][j] = 0;
        }
    }
    
    currentCycleVertexOrder[1] = 1;
    
    currentCycle[1] = TRUE;
    for(i = 1; i < adj[1]; i++){
        currentCycle[graph[1][i]]=TRUE;
        for(j = 0; j < i; j++){
            currentCycleVertexOrder[0] = graph[1][j];
            //search for cycle containing graph[1][i], 1,  graph[1][j]
            continueCycle(graph, adj, graph[1][j], graph[1][i], order - 2);
        }
        currentCycle[graph[1][i]]=FALSE;
    }
    
    if(cycleCount>0){
        return processCycleEdgeIncidence(graph, adj);
    } else {
        return 0;
    }
}

int hamiltonianCyclesUniversalEdges(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    cycleCount = 0;
    
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
        for(j = 0; j <= MAXN; j++){
            cycleEdgeIncidence[i][j] = 0;
        }
    }
    
    currentCycleVertexOrder[1] = 1;
    
    currentCycle[1] = TRUE;
    for(i = 1; i < adj[1]; i++){
        currentCycle[graph[1][i]]=TRUE;
        for(j = 0; j < i; j++){
            currentCycleVertexOrder[0] = graph[1][j];
            //search for cycle containing graph[1][i], 1,  graph[1][j]
            continueCycle(graph, adj, graph[1][j], graph[1][i], order - 2);
        }
        currentCycle[graph[1][i]]=FALSE;
    }
    
    if(cycleCount>0){
        return processCycleUniversalEdges(graph, adj);
    } else {
        return 0;
    }
}

int hamiltonianCyclesUncoveredEdges(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    cycleCount = 0;
    
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
        for(j = 0; j <= MAXN; j++){
            cycleEdgeIncidence[i][j] = 0;
        }
    }
    
    currentCycleVertexOrder[1] = 1;
    
    currentCycle[1] = TRUE;
    for(i = 1; i < adj[1]; i++){
        currentCycle[graph[1][i]]=TRUE;
        for(j = 0; j < i; j++){
            currentCycleVertexOrder[0] = graph[1][j];
            //search for cycle containing graph[1][i], 1,  graph[1][j]
            continueCycle(graph, adj, graph[1][j], graph[1][i], order - 2);
        }
        currentCycle[graph[1][i]]=FALSE;
    }
    
    if(cycleCount>0){
        return processCycleUncoveredEdges(graph, adj);
    } else {
        int edgeCount = 0;
        for(i = 1; i < order; i++){
            edgeCount += adj[i];
        }
        return edgeCount/2;
    }
}