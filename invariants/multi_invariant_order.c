/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * This file serves mainly as an example on how to use the multi_int_invariant.c
 * program to calculate an arbitrary invariant.
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_order -O4 -DINVARIANT=order \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_order.c
 */

#include "../multicode/shared/multicode_base.h"

int order(GRAPH graph, ADJACENCY adj){
    return graph[0][0];
}