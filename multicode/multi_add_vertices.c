/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2018 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * adds the requested number of vertices and writes the new graph to standard out
 * in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_add_vertices -O4  multi_add_vertices.c shared/multicode_base.c \
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
    fprintf(stderr, "The program %s adds vertices to a graph in multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] k\n\n", name);
    fprintf(stderr, "This adds k isolated vertices.\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] k\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH graph;
    ADJACENCY adj;

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
    
    if(argc - optind != 1){
        fprintf(stderr, "Illegal number of arguments: %d.\n", argc - optind);
        usage(name);
        return EXIT_FAILURE;
    }
    
    int extras = atoi(argv[optind]);
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        for(i = graph[0][0] + 1; i <= graph[0][0] + extras; i++){
            adj[i] = 0;
        }
        graph[0][0] += extras;
        
        writeMultiCode(graph, adj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

