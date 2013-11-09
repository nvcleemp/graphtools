/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in and
 * writes the graph induced by the specified vertices to standard out
 * in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_induced_subgraph -O4  multi_induced_subgraph.c \
 *     shared/multicode_base.c shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"
    
void constructInducedSubgraph(GRAPH graph, ADJACENCY adj, GRAPH newGraph, ADJACENCY newAdj, int newOrder, int *vertices){
    int i, j;
    
    //mapping of vertices
    int mapping[graph[0][0]+1];
    
    for(i=1; i<=graph[0][0]; i++){
        mapping[i] = 0;
    }
    
    for(i=1; i<= newOrder; i++){
        if(mapping[vertices[i]]){
            fprintf(stderr, "Vertex %d is given twice -- exiting!\n", vertices[i]);
            exit(EXIT_FAILURE);
        } else {
            mapping[vertices[i]]=i;
        }
    }
    
    //construct induced subgraph
    prepareGraph(newGraph, newAdj, newOrder);
    
    for(i=1; i<=graph[0][0]; i++){
        if(mapping[i]){
            for(j=0; j<=adj[i]; j++){
                if(mapping[i] < mapping[graph[i][j]]){
                    /* This test will also give false if graph[i][j] is unmapped,
                     * i.e., mapping[graph[i][j]] == 0
                     */
                    addEdge(newGraph, newAdj, mapping[i], mapping[graph[i][j]]);
                }
            }
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s constructs an induced subgraph for a graph\nin multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] v1 v2 v3 ...\n\n", name);
    fprintf(stderr, "where v1, v2, v3, ... are the vertices of the subgraph.\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] v1 v2 v3 ...\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
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
            case 0:
                break;
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
    
    int newOrder = argc - optind;
    
    int vertices[newOrder+1];
    
    for (i = 1; i <= newOrder; i++){
        vertices[i] = atoi(argv[optind+i-1]);
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        constructInducedSubgraph(graph, adj, newGraph, newAdj, newOrder, vertices);
        
        fprintf(stderr, "Graph with order %d\n", newOrder);
        
        writeMultiCode(newGraph, newAdj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

