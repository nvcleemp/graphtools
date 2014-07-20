/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format contains a wheel as a subgraph
 * (This version supports large graphs, but will be slower for small graphs)
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_contains_wheel_large_graphs -O4
 *     -DINVARIANT=containsWheel \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_contains_wheel_large_graphs.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean verticesInCycle[MAXN+1];
boolean neighbourhoods[MAXN+1][MAXN+1];
boolean universalNeighbours[MAXN+1][MAXN+1];

boolean handleSimpleCycle(int universalNeighbourCount){
    //if there is a universal neighbour then we have a wheel
    return universalNeighbourCount > 0 ? TRUE : FALSE;
}

boolean checkSimpleCycles_impl(
            GRAPH graph, ADJACENCY adj, int firstVertex,
            int secondVertex, int currentVertex, int size,
            int universalNeighbourCount){
    int i, w;
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
            if(currentVertex < secondVertex){
                //cycle not in canonical form
                continue;
            }
            if(handleSimpleCycle(universalNeighbourCount)){
                return TRUE;
            }
        } else {
            //we continue the cycle
            verticesInCycle[neighbour] = TRUE;
            
            //compute the new universal neighbours
            int universalNeighbourCount = 0;
            for(w = 1; w <= graph[0][0]; w++){
                universalNeighbours[size + 1][w] = universalNeighbours[size][w] &&
                        neighbourhoods[neighbour][w];
                if(universalNeighbours[size + 1][w]){
                    universalNeighbourCount++;
                }
            }
                
            if(universalNeighbourCount && 
                    checkSimpleCycles_impl(graph, adj, firstVertex, secondVertex,
                    neighbour, size+1, universalNeighbourCount)){
                return TRUE;
            }
            verticesInCycle[neighbour] = FALSE;
        }
    }
    return FALSE;
}

/* Returns TRUE if the graph contains a wheel. Check all cycles and check whether
 * there is a central vertex adjacent to each vertex of the cycle.
 */
boolean containsWheel(GRAPH graph, ADJACENCY adj){
    int v, w, i;
    int order = graph[0][0];
    
    for(v = 1; v <= order; v++){
        for(w=1; w <= order; w++){
            neighbourhoods[v][w] = FALSE;
        }
        for(i=0; i<adj[v]; i++){
            neighbourhoods[v][graph[v][i]] = TRUE;
        }
    }
    
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
                
                int universalNeighbourCount = 0;
                for(w = 1; w <= order; w++){
                    universalNeighbours[2][w] = neighbourhoods[v][w] &&
                            neighbourhoods[neighbour][w];
                    if(universalNeighbours[2][w]){
                        universalNeighbourCount++;
                    }
                }
                if(universalNeighbourCount && 
                        checkSimpleCycles_impl(graph, adj, v, neighbour, neighbour,
                        2, universalNeighbourCount)){
                    return TRUE;
                }
                verticesInCycle[neighbour] = FALSE;
            }
        }
        verticesInCycle[v] = FALSE;
    }
    return FALSE;
}
