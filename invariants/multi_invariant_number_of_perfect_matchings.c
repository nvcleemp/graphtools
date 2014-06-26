/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the number of perfect matchings in a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_number_of_perfect_matchings -O4 -DINVARIANT=numberPM \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_number_of_perfect_matchings.c
 */

#include "../multicode/shared/multicode_base.h"

unsigned long long int perfectMatchingCount;

boolean available[MAXN + 1];

void handlePerfectMatching(){
    perfectMatchingCount++;
}

void extendPerfectMatching(GRAPH graph, ADJACENCY adj){
    int i = 1, j;
    
    while(i <= graph[0][0] && !available[i]){
        i++;
    }
    
    if(i > graph[0][0]){
        handlePerfectMatching();
    } else {
        available[i] = FALSE;
        for(j = 0; j < adj[i]; j++){
            if(available[graph[i][j]]){
                available[graph[i][j]] = FALSE;
                extendPerfectMatching(graph, adj);
                available[graph[i][j]] = TRUE;
            }
        }
        available[i] = TRUE;
    }
}

int numberPM(GRAPH graph, ADJACENCY adj){
    int i;
    
    if(graph[0][0]%2){
        //an graph with an odd number of vertices does not have a perfect matching
        return 0;
    }
    
    perfectMatchingCount = 0;
    
    for(i = 1; i <= MAXN; i++){
        available[i] = TRUE;
    }
    
    extendPerfectMatching(graph, adj);
    
    return perfectMatchingCount;
}