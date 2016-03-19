/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * and creates a new graph by taking the requested number of copies
 * and adding the requested connections between the copies.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_cyclic_connect -O4  multi_cyclic_connect.c \
 *     ../shared/multicode_base.c ../shared/multicode_input.c \
 *     ../shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "connect_general.h"
#include "../shared/multicode_base.h"
#include "../shared/multicode_input.h"
#include "../shared/multicode_output.h"

void makeCyclicConnections(GRAPH graph, ADJACENCY adj, int originalSize, int copies, int from, int to){
    int i;
    for (i=0; i<copies; i++) {
        makeConnection(graph, adj, originalSize, from, i, to, (i+1)%copies);
    }
}

void makeAllCyclicConnections(GRAPH graph, ADJACENCY adj, int originalSize, int copies){
    int i;
    for (i=1; i<=originalSize; i++) {
        makeCyclicConnections(graph, adj, originalSize, copies, i, i);
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s generates a new graph based on the provided graph.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n f1,t1 f2,t2 f3,t3 ...\n\n", name);
    fprintf(stderr, "A graph is read from standard in, n copies are made and for each connection fi,ti\n");
    fprintf(stderr, "and for j<n the vertex fi in copy j is connected to ti in copy (j+1)mod n.\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -a, --all\n");
    fprintf(stderr, "       For each vertex v also make the connection v,v.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] n f1,t1 f2,t2 f3,t3 ...\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH originalGraph;
    ADJACENCY originalAdj;
    GRAPH copyGraph;
    ADJACENCY copyAdj;

    boolean connectAll = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"all", no_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "ha", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'a':
                connectAll = TRUE;
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
    
    if (optind == argc) {
        fprintf(stderr, "Error: number of copies omitted.\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    int copies = atoi(argv[optind]);
    
    int connectionCount = argc - optind - 1;
    
    int connections[connectionCount][2];
    
    for (i = 0; i < connectionCount; i++){
        if(sscanf(argv[optind + 1 + i], "%d,%d", connections[i], connections[i]+1)!=2){
            fprintf(stderr, "Error while reading connections to be made.\n");
            usage(name);
            return EXIT_FAILURE;
        }
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, originalGraph, originalAdj);
        
        initializeCopies(copies, originalGraph, originalAdj, copyGraph, copyAdj);
        
        if (connectAll) {
            makeAllCyclicConnections(copyGraph, copyAdj, originalGraph[0][0], copies);
        }
        for (i=0; i<connectionCount; i++){
            makeCyclicConnections(copyGraph, copyAdj, originalGraph[0][0], 
                                  copies, connections[i][0], connections[i][1]);
        }
        
        writeMultiCode(copyGraph, copyAdj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

