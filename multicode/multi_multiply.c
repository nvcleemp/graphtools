/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in,
 * and writes the specified number of copies to standard out in 
 * multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_multiply -O4  multi_multiply.c shared/multicode_base.c \
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
    fprintf(stderr, "The program %s reads graphs in multicode format and writes the\n", name);
    fprintf(stderr, "specified number of copies.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] n\n", name);
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
    
    int copies = 0;
    
    if(argc - optind != 1){
        usage(name);
        return EXIT_FAILURE;
    }
    
    if(sscanf(argv[optind], "%d", &copies) != 1){
        fprintf(stderr, "Error while reading the number of needed copies.\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        for(i = 0; i < copies; i++){
            writeMultiCode(graph, adj, stdout);
        }
    }
    
    return (EXIT_SUCCESS);
}

