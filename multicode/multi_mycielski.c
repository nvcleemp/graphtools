/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in, and 
 * writes the mycielskian of that graph to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_mycielski -O4  multi_mycielski.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

void createMycielski(GRAPH graph, ADJACENCY adj, GRAPH newGraph, ADJACENCY newAdj){
    int i, j;
    
    int order = graph[0][0];
    
    prepareGraph(newGraph, newAdj, 2*order + 1);
    
    for(i=1; i<=graph[0][0]; i++){
        for(j=0; j<adj[i]; j++){
            if(i < graph[i][j]){
                addEdge(newGraph, newAdj, i, graph[i][j]);
            }
            addEdge(newGraph, newAdj, i + order, graph[i][j]);
        }
        addEdge(newGraph, newAdj, i + order, 2*order + 1);
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s creates the mycielskian of a graph in multicode format.\n\n", name);
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
    
    GRAPH newGraph;
    ADJACENCY newAdj;

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
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }
    
    createMycielski(graph, adj, newGraph, newAdj);

    writeMultiCode(newGraph, newAdj, stdout);

    return (EXIT_SUCCESS);
}

