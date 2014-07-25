/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * suppresses the vertices of degree 2 and writes the new graph to 
 * standard out in multicode format. Suppressing the vertices of degree
 * 2 can lead to multi-edges if the vertex of degree 2 was part of a
 * triangle.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_suppress_degree_2 -O4  multi_suppress_degree_2.c\
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

void suppressDegree2(GRAPH oldGraph, ADJACENCY oldAdj, GRAPH newGraph, ADJACENCY newAdj){
    int old2new[MAXN+1];
    int vertexCounter, i, j;
    
    vertexCounter = 0;
    for(i = 1; i <= oldGraph[0][0]; i++){
        if(oldAdj[i]!=2){
            vertexCounter++;
            old2new[i] = vertexCounter;
        }
    }
    
    prepareGraph(newGraph, newAdj, vertexCounter);
    
    for(i = 1; i <= oldGraph[0][0]; i++){
        if(oldAdj[i]!=2){
            for(j = 0; j < oldAdj[i]; j++){
                if(oldGraph[i][j] >= i && oldAdj[oldGraph[i][j]]!=2){
                    addEdge(newGraph, newAdj, old2new[i], old2new[oldGraph[i][j]]);
                }
            }
        } else {
            addEdge(newGraph, newAdj, old2new[oldGraph[i][0]], old2new[oldGraph[i][1]]);
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s suppresses vertices of degree 2 in graphs\n", name);
    fprintf(stderr, "in multicode format. At the moment this program does not support neighbouring\n");
    fprintf(stderr, "vertices of degree 2.\n\n");
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
        
        suppressDegree2(graph, adj, newGraph, newAdj);
        
        writeMultiCode(newGraph, newAdj, stdout);
    }

    return (EXIT_SUCCESS);
}

