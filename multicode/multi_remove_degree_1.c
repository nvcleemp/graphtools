/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * recursively removes vertices of degree 1 and writes the new graph to 
 * standard out in multicode format.
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_remove_degree_1 -O4  multi_remove_degree_1.c\
 *     shared/multicode_base.c shared/multicode_input.c\
 *     shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

void removeDegree1(GRAPH oldGraph, ADJACENCY oldAdj, GRAPH newGraph, ADJACENCY newAdj){
    int old2new[MAXN+1];
    ADJACENCY degrees;
    int vertexCounter, i, j;
    boolean containsDegree1 = FALSE;
    
    for(i = 1; i <= oldGraph[0][0]; i++){
        old2new[i] = i;
        degrees[i] = oldAdj[i];
        if(degrees[i] == 1){
            containsDegree1 = TRUE;
        }
    }
    
    while(containsDegree1){
        containsDegree1 = FALSE;
        for(i = 1; i <= oldGraph[0][0]; i++){
            if(old2new[i] && degrees[i] == 1){
                old2new[i] = 0;
                for(j = 0; j < oldAdj[i]; j++){
                    if(old2new[oldGraph[i][j]]){
                        degrees[oldGraph[i][j]]--;
                        if(degrees[oldGraph[i][j]] == 1){
                            containsDegree1 = TRUE;
                        }
                    }
                }
            }
        }
    }
    
    vertexCounter = 0;
    for(i = 1; i <= oldGraph[0][0]; i++){
        if(old2new[i]){
            vertexCounter++;
            old2new[i] = vertexCounter;
        }
    }
    prepareGraph(newGraph, newAdj, vertexCounter);
    
    for(i = 1; i <= oldGraph[0][0]; i++){
        if(old2new[i]){
            for(j = 0; j < oldAdj[i]; j++){
                if(oldGraph[i][j] >= i && old2new[oldGraph[i][j]]){
                    addEdge(newGraph, newAdj, old2new[i], old2new[oldGraph[i][j]]);
                }
            }
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s recursively removes vertices of degree 1 in graphs\n", name);
    fprintf(stderr, "in multicode format.\n\n");
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
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        removeDegree1(graph, adj, newGraph, newAdj);
        
        writeMultiCode(newGraph, newAdj, stdout);
    }

    return (EXIT_SUCCESS);
}

