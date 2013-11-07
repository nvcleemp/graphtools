/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the girth of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_girth -O4 -DINVARIANT=girth \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_order.c
 */

#include "../multicode/shared/multicode_base.h"

/**
  * Return the minimum of the given girth and the length of the shortest
  * cycle through the given vertex.
  */
int pseudoGirth(GRAPH graph, ADJACENCY adj, int v, int girth) {
    int i;
    
    int n = graph[0][0];
        
    int queue[n];
    int head = 0;
    int tail = 0;

    int levels[n+1];
    for(i=1; i<=n; i++){
        levels[i]=-1;
    }
    queue[head] = v;

    int lim = girth / 2;

    head ++;
    levels[v] = 0;
    while (head > tail) {
        int vertex = queue[tail];
        int d = levels[vertex];
        if (d >= lim)
            return girth; // can never improve current girth
        tail ++;
        for (i=0; i<adj[vertex]; i++) {
            int nb = graph[vertex][i];
            int e = levels[nb];
            if (e < 0) {
                // not yet encountered
                levels[nb] = d+1;
                queue[head] = nb;
                head ++;
            } else if (e == d) {
                // odd cycle
                return 2*d + 1;
            } else if (e > d) {
                // even cycle
                if (girth > 2*e) {
                    girth = 2*e; // == 2*d+2
                }
            } //else {
                // returns towards v, ignore
            //}
        }
    }
    return girth;
}

int girth(GRAPH graph, ADJACENCY adj){
    int i;
    int girth = graph[0][0];
    
    for(i=1; i<=graph[0][0]; i++){
        girth = pseudoGirth(graph, adj, i, girth);
    }
    
    return girth;
}