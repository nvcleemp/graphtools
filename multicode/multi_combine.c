/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads two graphs in multicode format from standard in,
 * combines them by identifying the specified vertices and writes the 
 * new graph to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_combine -O4  multi_combine.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

void combineGraphs(GRAPH graph1, ADJACENCY adj1, GRAPH graph2, ADJACENCY adj2,
                   GRAPH combinedGraph, ADJACENCY combinedAdj, int newOrder, int *translations){
    int i, j;
    
    prepareGraph(combinedGraph, combinedAdj, newOrder);
    
    for(i=1; i<=graph1[0][0]; i++){
        for(j=0; j<adj1[i]; j++){
            if(i < graph1[i][j]){
                addEdge(combinedGraph, combinedAdj, i, graph1[i][j]);
            }
        }
    }
    
    for(i=1; i<=graph2[0][0]; i++){
        for(j=0; j<adj2[i]; j++){
            if(i < graph2[i][j]){
                addEdge(combinedGraph, combinedAdj, translations[i], translations[graph2[i][j]]);
            }
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s combines two graphs in multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] u1,v1 u2,v2 u3,v3 ...\n\n", name);
    fprintf(stderr, "This identifies vertex u1 in graph 1 with vertex v1 in graph 2, vertex u2 in \ngraph 1 with vertex v2 in graph 2,...\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] u1,v1 u2,v2 u3,v3 ...\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH graph1;
    ADJACENCY adj1;
    
    GRAPH graph2;
    ADJACENCY adj2;
    
    GRAPH combinedGraph;
    ADJACENCY combinedAdj;

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
    
    int identificationCount = argc - optind;
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph1, adj1);
        
    } else {
        fprintf(stderr, "Error! Could not read first graph.\n");
        return (EXIT_FAILURE);
    }
    
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph2, adj2);
        
    } else {
        fprintf(stderr, "Error! Could not read second graph.\n");
        return (EXIT_FAILURE);
    }
    
    int translation[graph2[0][0]+1];
    
    for (i = 0; i <= graph2[0][0]; i++){
        translation[i] = 0;
    }
    
    for (i = 0; i < identificationCount; i++){
        int g1;
        int g2;
        if(sscanf(argv[optind + i], "%d,%d", &g1, &g2)!=2){
            fprintf(stderr, "Error while reading vertices to be identified.\n", c);
            usage(name);
            return EXIT_FAILURE;
        }
        translation[g2] = g1;
    }
    int vertexCounter = graph1[0][0];
    for (i = 1; i <= graph2[0][0]; i++){
        if(!translation[i]){
            vertexCounter++;
            translation[i] = vertexCounter;
        }
    }
    if(vertexCounter != graph1[0][0] + graph2[0][0] - identificationCount){
        fprintf(stderr, "Something went wrong -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
    combineGraphs(graph1, adj1, graph2, adj2, combinedGraph, combinedAdj, vertexCounter, translation);

    writeMultiCode(combinedGraph, combinedAdj, stdout);

    return (EXIT_SUCCESS);
}

