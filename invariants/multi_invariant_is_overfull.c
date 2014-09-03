/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format is overfull
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_is_overfull -O4 -DINVARIANT=isOverfull \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_is_overfull.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean isOverfull(GRAPH graph, ADJACENCY adj){
    int i;
    int order = graph[0][0];
    if(!(order%2)){
        return FALSE;
    }
    
    int maxDegree = 0;
    int doubleEdgeCount = 0;
    for(i = 1; i <= order; i++){
        doubleEdgeCount += adj[i];
        if(adj[i] > maxDegree){
            maxDegree = adj[i];
        }
    }
    
    return doubleEdgeCount > (order - 1)*maxDegree;
}