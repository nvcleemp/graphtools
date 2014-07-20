/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format contains a wheel as a subgraph
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_contains_wheel -O4 -DINVARIANT=containsWheel \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_contains_wheel.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

typedef unsigned long long int bitset;

#define ZERO 0ULL
#define ONE 1ULL
#define EMPTY_SET 0ULL
#define SINGLETON(el) (ONE << (el))
#define IS_SINGLETON(s) ((s) && (!((s) & ((s)-1))))
#define HAS_MORE_THAN_ONE_ELEMENT(s) ((s) & ((s)-1))
#define IS_NOT_EMPTY(s) (s)
#define IS_EMPTY(s) (!(s))
#define CONTAINS(s, el) ((s) & SINGLETON(el))
#define CONTAINS_ALL(s, elements) (((s) & (elements)) == (elements))
#define ADD(s, el) ((s) |= SINGLETON(el))
#define ADD_ALL(s, elements) ((s) |= (elements))
#define UNION(s1, s2) ((s1) | (s2))
#define INTERSECTION(s1, s2) ((s1) & (s2))
//these will only work if the element is actually in the set
#define REMOVE(s, el) ((s) ^= SINGLETON(el))
#define REMOVE_ALL(s, elements) ((s) ^= (elements))
#define MINUS(s, el) ((s) ^ SINGLETON(el))
#define MINUS_ALL(s, elements) ((s) ^ (elements))
//the following macros perform an extra step, but will work even if the element is not in the set
#define SAFE_REMOVE(s, el) ADD(s, el); REMOVE(s, el)
#define SAFE_REMOVE_ALL(s, elements) ADD_ALL(s, elements); REMOVE_ALL(s, elements)


boolean handleSimpleCycle(bitset verticesInCycle, bitset universalNeighbours){
    //if there is a universal neighbour then we have a wheel
    return universalNeighbours ? TRUE : FALSE;
}

boolean checkSimpleCycles_impl(
            GRAPH graph, ADJACENCY adj, int firstVertex,
            int secondVertex, int currentVertex, bitset *verticesInCycle,
            bitset universalNeighbours, bitset neighbourhoods[]){
    int i;
    for(i=0; i<adj[currentVertex]; i++){
        int neighbour = graph[currentVertex][i];
        if((neighbour != firstVertex) && CONTAINS(*verticesInCycle, neighbour)){
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
            if(handleSimpleCycle(*verticesInCycle, universalNeighbours)){
                return TRUE;
            }
        } else if(IS_NOT_EMPTY(
                INTERSECTION(universalNeighbours, neighbourhoods[neighbour]))){
            //we continue the cycle
            ADD(*verticesInCycle, neighbour);
            if(checkSimpleCycles_impl(
                    graph, adj, firstVertex, secondVertex, neighbour,
                    verticesInCycle, 
                    INTERSECTION(universalNeighbours, neighbourhoods[neighbour]),
                    neighbourhoods)){
                return TRUE;
            }
            REMOVE(*verticesInCycle, neighbour);
        }
    }
    return FALSE;
}

/* Returns TRUE if the graph contains a wheel. Check all cycles and check whether
 * there is a central vertex adjacent to each vertex of the cycle.
 */
boolean containsWheel(GRAPH graph, ADJACENCY adj){
    int v, i;
    int order = graph[0][0];
    bitset neighbourhoods[order+1];
    
    if(order > 63){
        fprintf(stderr, "Currently only graphs up to 63 vertices supported -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
    for(v = 1; v <= order; v++){
        neighbourhoods[v] = EMPTY_SET; 
        for(i=0; i<adj[v]; i++){
            ADD(neighbourhoods[v], graph[v][i]);
        }
    }
    
    for(v = 1; v < order; v++){ //intentionally skip v==order!
        for(i=0; i<adj[v]; i++){
            int neighbour = graph[v][i];
            if(neighbour < v){
                //cycle not in canonical form: we have seen this cycle already
                continue;
            } else {
                //start a cycle
                bitset verticesInCycle = UNION(SINGLETON(v), SINGLETON(neighbour));
                bitset universalNeighbours = 
                        INTERSECTION(neighbourhoods[v], neighbourhoods[neighbour]);
                if(checkSimpleCycles_impl(graph, adj, v, neighbour, neighbour,
                        &verticesInCycle, universalNeighbours, neighbourhoods)){
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
