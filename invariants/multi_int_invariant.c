/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * adds the requested edges and writes the new graph to standard out
 * in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_invariant_invariantname -O4 \
 *     -DINVARIANT=invariantmethod \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c \
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_invariantname.c
 * 
 * or:
 *     
 *     cc -o multi_invariant_invariantname -O4 \
 *     -DINVARIANT=invariantmethod \
 *     -DINVARIANTNAME=invariantname \
 *     multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c \
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_invariantname.c
 * 
 */

#ifndef INVARIANT
    #error "INVARIANT must be defined"
#endif

#ifndef INVARIANTNAME
#define INVARIANTNAME INVARIANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_input.h"
#include "../multicode/shared/multicode_output.h"

#define XSTR(s) STR(s)
#define STR(s) #s

int INVARIANT(GRAPH graph, ADJACENCY adj);

int graphCount = 0;
int graphsFiltered = 0;

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s calculates the %s \nfor graphs in multicode format.\n\n", name, XSTR(INVARIANTNAME));
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -f #, --filter #\n");
    fprintf(stderr, "       Filter graphs that have the specified value for the invariant.\n");
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
    int i;
    
    GRAPH graph;
    ADJACENCY adj;
    
    int filterValue;
    boolean doFiltering = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"filter", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hf:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'f':
                doFiltering = TRUE;
                filterValue = atoi(optarg);
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
        graphCount++;
        
        int value = INVARIANT(graph, adj);
        if(doFiltering){
            if(filterValue == value){
                graphsFiltered++;
                writeMultiCode(graph, adj, stdout);
            }
        } else {
            fprintf(stdout, "Graph %d has " XSTR(INVARIANTNAME) " equal to %d.\n", graphCount, value);
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    if(doFiltering){
        fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");
    }

    return (EXIT_SUCCESS);
}

