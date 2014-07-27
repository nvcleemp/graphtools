/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in,
 * replaces each multi-edge by a single edge and writes the 
 * resulting graph to standard out.
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_simplify -O4  multi_simplify.c shared/multicode_base.c \
 *     shared/multicode_output.c shared/multicode_input.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

boolean adjacencyMatrix[MAXN+1][MAXN+1];

void removeMultiEdges(GRAPH graph, ADJACENCY adj, GRAPH newGraph, ADJACENCY newAdj){
    int i, j;
    
    //clean array
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 1; j <= graph[0][0]; j++){
            adjacencyMatrix[i][j] = FALSE;
        }
    }
    
    prepareGraph(newGraph, newAdj, graph[0][0]);
    
    //copy graph structure and remove multi-edges
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(i < graph[i][j] && !adjacencyMatrix[i][graph[i][j]]){
                adjacencyMatrix[i][graph[i][j]] = TRUE;
                addEdge(newGraph, newAdj, i, graph[i][j]);
            }
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s reads graphs in multicode format, replaces\n", name);
    fprintf(stderr, "multi-edges by simple edges and writes the resulting graph to standard out.\n\n");
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
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        removeMultiEdges(graph, adj, newGraph, newAdj);
        
        writeMultiCode(newGraph, newAdj, stdout);
    }
    
    return (EXIT_SUCCESS);
}

