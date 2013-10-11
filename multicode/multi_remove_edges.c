/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * removes the specified edges and writes the new graph to standard out
 * in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_remove_edges -O4  multi_remove_edges.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s removes edges from a graph in multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] f1,t1 f2,t2 f3,t3 ...\n\n", name);
    fprintf(stderr, "This removes the edge from f1 to t1, the edge from f2 to t2,...\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] f1,t1 f2,t2 f3,t3 ...\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH graph;
    ADJACENCY adj;
    
    boolean all = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"all", no_argument, NULL, 'a'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "ha", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'a':
                all = TRUE;
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
    
    int edgeCount = argc - optind;
    
    int edgesToRemove[edgeCount][2];
    
    for (i = 0; i < edgeCount; i++){
        if(sscanf(argv[optind + i], "%d,%d", edgesToRemove[i], edgesToRemove[i]+1)!=2){
            fprintf(stderr, "Error while reading edges to be removed.\n", c);
            usage(name);
            return EXIT_FAILURE;
        }
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        for (i=0; i<edgeCount; i++){
            removeEdge(graph, adj, edgesToRemove[i][0], edgesToRemove[i][1], all);
        }
        
        writeMultiCode(graph, adj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

