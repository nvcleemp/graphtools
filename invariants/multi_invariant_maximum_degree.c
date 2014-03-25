/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * 'Computes' the maximum degree of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_maximum_degree -O4 -DINVARIANT=maximumDegree \
 *     -DINVARIANTNAME="maximum degree"
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_maximum_degree.c
 */

#include "../multicode/shared/multicode_base.h"

int maximumDegree(GRAPH graph, ADJACENCY adj){
    int i, maximumDegree = 0;
    
    for(i = 1; i <= graph[0][0]; i++){
        if(adj[i] > maximumDegree){
            maximumDegree = adj[i];
        }
    }
    
    return maximumDegree;
}