/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * writes the complement of those graphs to standard out in multicode format.
 * 
 * Compile with:
 *     
 *     cc -o multi_complement -O4  multi_complement.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"


void constructComplement(GRAPH graph, ADJACENCY adj, GRAPH complementGraph, ADJACENCY complementAdj){
    int i, j;
    
    int order = graph[0][0];
    
    boolean neighbours[MAXN + 1];
    
    prepareGraph(complementGraph, complementAdj, order);
    
    for(i=1; i<=order; i++){
        for(j=1; j<=MAXN; j++){
            neighbours[j] = FALSE;
        }
        for(j=0; j<adj[i]; j++){
            neighbours[graph[i][j]] = TRUE;
        }
        neighbours[i] = TRUE;
        for(j=1; j<=MAXN; j++){
            if(!neighbours[j]){
                addEdge(complementGraph, complementAdj, i, j);
            }
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s constructs the complement of graphs in multicode format.\n\n", name);
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
    
    GRAPH graph;
    ADJACENCY adj;
    GRAPH complementGraph;
    ADJACENCY complementAdj;

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
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        constructComplement(graph, adj, complementGraph, complementAdj);
        
        writeMultiCode(complementGraph, complementAdj, stdout);
    }

    return (EXIT_SUCCESS);
}

