/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in signed_code format from standard in,
 * and creates equivalent signatures by performing random switches.
 * This program is mainly intended for testing purposes.
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_random_equivalent -O4  signed_random_equivalent.c\
 *           shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int edgeCounter = 0;

int switchCount = 1000;

void performRandomSwitching(GRAPH graph, ADJACENCY adj, int order){
    int i, v;
    for(i = 0; i < switchCount; i++){
        v = (rand() % (order))+1; //select a vertex
        //not perfectly uniform way to get random integers, but close enough
        switchAtVertex(graph, adj, v);
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s creates equivalent signed graphs in signed_code\n", name);
    fprintf(stderr, "format for the input graphs by performing random switches at vertices.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] k\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs and give a larger value for MAXN.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -c n, --copies n\n");
    fprintf(stderr, "       Make n copies of each graph.\n");
    fprintf(stderr, "    -s n, --switches n\n");
    fprintf(stderr, "       Perform n switches per copy.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {
    int copies = 1;    
    
    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"copies", required_argument, NULL, 'c'},
        {"switches", required_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hc:s:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                copies = atoi(optarg);
                break;
            case 's':
                switchCount = atoi(optarg);
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
    
    int codeLength;
    unsigned short code[MAXCODELENGTH];
    GRAPH graph;
    ADJACENCY adj;
    int graphsRead, graphsWritten, i;

    graphsRead = graphsWritten = 0;

    while (readSignedCode(code, &codeLength, stdin)){

        graphsRead++;
        int order;
        
        for(i = 0; i < copies; i++){
            //we start with a fresh copy of the graph each time
            decodeSignedCode(code, codeLength, graph, adj, &order);
            performRandomSwitching(graph, adj, order);
            writeSignedCode(graph, adj, order, stdout);
        }
    }

    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Written %d randomly switched %s.\n",
            graphsWritten, graphsWritten==1 ? "copy" : "copies");


    return (EXIT_SUCCESS);

}


