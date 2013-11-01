/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the edge connectivity of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_edge_connectivity -O4 -DINVARIANT=edge_connectivity \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     connectivity/multi_connectivity.c \
 *     multi_invariant_order.c
 */

#include "../multicode/shared/multicode_base.h"
#include "connectivity/multi_connectivity.h"

int edge_connectivity(GRAPH graph, ADJACENCY adj){
    return findEdgeConnectivity(graph, adj);
}