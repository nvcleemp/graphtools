/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format from standard in and
 * writes the underlying graphs to standard out in multi_code format.
 * 
 * Compile with:
 *     
 *     cc -o signed_underlying -O4  signed_underlying.c \
 *     shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int edgeCounter = 0;

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s writes the underlying graphs for given signed graphs.\n\n", name);
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
    
    int graphsRead = 0;
    
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
    while (readSignedCode(code, &length, stdin)) {
        int order;
        decodeSignedCode(code, length, graph, adj, &order);
        writeAsMultiCode(graph, adj, order, stdout);
        graphsRead++;
    }
    
    fprintf(stderr, "Written underlying graphs for %d signed graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

