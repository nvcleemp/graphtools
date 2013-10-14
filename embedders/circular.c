/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * embeds its vertices on a circle and writes the new graph to standard out
 * in writegraph2d format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o circular -O4  circular.c ../multicode/shared/multicode_base.c \
 *     ../multicode/shared/multicode_input.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_input.h"

//////////////////////////////////////////////////////////////////////////////

void writeWritegraph2d(GRAPH graph, ADJACENCY adj, FILE *f){
    static int first = TRUE;
    int i, j;
    int nv = graph[0][0];
    
    if(first){
        first = FALSE;
        
        fprintf(f, ">>writegraph2d<<\n");
    }
    
    for(i = 1; i <= nv; i++){
        //current vertex
        fprintf(f, "%3d ", i);
        
        //coordinates
        fprintf(f, "%.4f %.4f ", cos(2*(i-1)*M_PI/nv), sin(2*(i-1)*M_PI/nv));
        
        //neighbours
        for(j = 0; j<adj[i]; j++){
            fprintf(f, "%3d ", graph[i][j]);
        }
        
        //next line
        fprintf(f, "\n");
    }
    //end of graph
    fprintf(f, "0\n");
}

//////////////////////////////////////////////////////////////////////////////
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s embeds a graph in multicode format.\n\n", name);
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
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        writeWritegraph2d(graph, adj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

