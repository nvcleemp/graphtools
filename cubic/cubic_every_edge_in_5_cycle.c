/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes those for which each edge lies in a 5-cycle to standard out in 
 * multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_every_edge_in_5_cycle -O4  cubic_every_edge_in_5_cycle.c \
 *     shared/cubic_base.c shared/cubic_input.c shared/cubic_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/cubic_base.h"
#include "shared/cubic_input.h"
#include "shared/cubic_output.h"


//bit vectors

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
#define ALL_UP_TO(el) (SINGLETON((el)+1)-1)
#define TOGGLE(s, el) ((s) ^= SINGLETON(el))
#define TOGGLE_ALL(s, elements) ((s) ^= (elements))


int vertexCount;
GRAPH graph;

bitset neighbours[MAXN];

boolean edgeLiesOn5Cycle(int v1, int v2){
    int i, j;
    
    for(i = 0; i < 3; i++){
        int n1 = graph[v1][i];
        if(n1 != v2){
            for(j = 0; j < 3; j++){
                int n2 = graph[v2][j];
                if(n2 != v1 && n1 != n2){
                    bitset u = INTERSECTION(neighbours[n1], neighbours[n2]);
                    SAFE_REMOVE(u, v1);
                    SAFE_REMOVE(u, v2);
                    SAFE_REMOVE(u, n1);
                    SAFE_REMOVE(u, n2);
                    if(IS_NOT_EMPTY(u)){
                        return TRUE;
                    }
                }
            }
        }
    }
    
    return FALSE;
}

boolean eachEdgeLiesOn5Cycle(){
    int i, j;
    
    //construct bitset with neighbourhood for each vertex
    for(i=0; i<vertexCount; i++){
        neighbours[i] = EMPTY_SET;
        for(j=0; j<3; j++){
            ADD(neighbours[i], graph[i][j]);
        }
    }
    
    //check each edge
    for(i=0; i<vertexCount; i++){
        for(j=0; j<3; j++){
            if(i < graph[i][j] && !edgeLiesOn5Cycle(i, graph[i][j])){
                return FALSE;
            }
        }
    }
    
    return TRUE;
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s .\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    int graphsRead = 0;
    int graphsFiltered = 0;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;
        
        if(vertexCount>64){
            fprintf(stderr, "Only up to 64 vertices -- exiting!\n");
            return EXIT_FAILURE;
        }
        
        if(eachEdgeLiesOn5Cycle()){
            writeCubicMultiCode(graph, vertexCount, stdout);
            graphsFiltered++;
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

