/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format is pancyclic.
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_is pancyclic -O4
 *     -DINVARIANT=isPancyclic \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_is_pancyclic.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean verticesInCycle[MAXN + 1];

boolean observedCycleSizes[MAXN + 1];
int largestMissingCycle, smallestMissingCycle;

boolean handleSimpleCycle(int size){
    if(largestMissingCycle == size && smallestMissingCycle == size){
        return TRUE;
    } else {
        observedCycleSizes[size] = TRUE;
        if(largestMissingCycle == size){
            while(observedCycleSizes[largestMissingCycle]){
                largestMissingCycle--;
            }
        } else if(smallestMissingCycle == size){
            while(observedCycleSizes[smallestMissingCycle]){
                smallestMissingCycle++;
            }
        }
        return FALSE;
    }
}

boolean checkSimpleCycles_impl(
            GRAPH graph, ADJACENCY adj, int firstVertex,
            int secondVertex, int currentVertex, int size){
    int i;
    for(i=0; i<adj[currentVertex]; i++){
        int neighbour = graph[currentVertex][i];
        if((neighbour != firstVertex) && verticesInCycle[neighbour]){
            //vertex already in cycle (and not first vertex)
            continue;
        } else if(neighbour < firstVertex){
            //cycle not in canonical form
            continue;
        } else if(neighbour == firstVertex){
            //we have returned to the first vertex
            if(currentVertex <= secondVertex){
                //cycle not in canonical form
                continue;
            }
            if(!observedCycleSizes[size] && handleSimpleCycle(size)){
                return TRUE;
            }
        } else if (size < largestMissingCycle) {
            //we continue the cycle
            verticesInCycle[neighbour] = TRUE;
                
            if(checkSimpleCycles_impl(graph, adj, firstVertex, secondVertex,
                    neighbour, size+1)){
                return TRUE;
            }
            
            verticesInCycle[neighbour] = FALSE;
        }
    }
    return FALSE;
}

/* Returns TRUE if the graph is pancyclic.
 */
boolean isPancyclic(GRAPH graph, ADJACENCY adj){
    int v, i;
    int order = graph[0][0];
    
    for(v = 3; v <= order; v++){
        observedCycleSizes[v] = FALSE;
    }
    smallestMissingCycle = 3;
    largestMissingCycle = order;
    
    for(v = 1; v < order; v++){ //intentionally skip v==order!
        verticesInCycle[v] = TRUE;
        for(i=0; i<adj[v]; i++){
            int neighbour = graph[v][i];
            if(neighbour < v){
                //cycle not in canonical form: we have seen this cycle already
                continue;
            } else {
                //start a cycle
                verticesInCycle[neighbour] = TRUE;
                if(checkSimpleCycles_impl(graph, adj, v, neighbour, neighbour, 2)){
                    return TRUE;
                }
                verticesInCycle[neighbour] = FALSE;
            }
        }
        verticesInCycle[v] = FALSE;
    }
    return FALSE;
}
