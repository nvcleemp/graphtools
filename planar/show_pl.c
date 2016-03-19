/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads planar graphs from standard in and
 * writes formatted adjacency lists to standard out.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o show_pl -O4 show_pl.c shared/planar_base.c shared/planar_input.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "shared/planar_base.h"
#include "shared/planar_input.h"

void printAdjacencyList(PLANE_GRAPH *pg){
    int i;
    PG_EDGE *e, *elast;
    
    //write the number of vertices
    fprintf(stdout, "Graph with %d %s\n", pg->nv, pg->nv == 1 ? "vertex" : "vertices");
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        fprintf(stdout, "%d) ", i + 1);
        do {
            fprintf(stdout, "%d ", e->end + 1);
            e = e->next;
        } while (e != elast);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
}

void printCode(unsigned short* code) {
    int i, j, codePosition;

    int nv = code[0];
    codePosition = 1;
    
    fprintf(stdout, "%d\n", nv);

    for (i = 0; i < nv; i++) {
        fprintf(stdout, "%d ", code[codePosition]);
        codePosition++;
        for (j = 1; code[codePosition]; j++, codePosition++) {
            fprintf(stdout, "%d ", code[codePosition]);
        }
        fprintf(stdout, "0\n");
        codePosition++; /* read the closing 0 */
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s print adjacency lists of planar graphs from a file.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s\n\n", name);
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, " -c, --code\n");
    fprintf(stderr, "    Print a structured form of the planar_code instead of an adjacency list.\n");
    fprintf(stderr, " -h, --help\n");
    fprintf(stderr, "    Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] \n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char** argv) {
    
    int graphsRead = 0;
    boolean showCode = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"code", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hc", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                showCode = TRUE;
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
    
    unsigned short *code;
    DEFAULT_PG_INPUT_OPTIONS(options);
    while ((code = readPlanarCode(stdin, &options))) {
        if(showCode){
            printCode(code);
        } else {
            PLANE_GRAPH *pg = decodePlanarCode(code, &options);
            printAdjacencyList(pg);
            freePlaneGraph(pg);
        }
        free(code);
        graphsRead++;
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");

    return (EXIT_SUCCESS);
}
