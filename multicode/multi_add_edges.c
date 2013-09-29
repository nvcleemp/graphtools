/* 
 * File:   multi_add_edges.c
 * Author: nvcleemp
 *
 * Created on September 27, 2013, 6:47 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s adds edges to a graph in multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] f1,t1 f2,t2 f3,t3 ...\n\n", name);
    fprintf(stderr, "This connects vertex f1 with vertex t1, vertex f2 with vertex t2,...\n");
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
    
    int connectionCount = argc - optind;
    
    int connections[connectionCount][2];
    
    for (i = 0; i < connectionCount; i++){
        if(sscanf(argv[optind + i], "%d,%d", connections[i], connections[i]+1)!=2){
            fprintf(stderr, "Error while reading edges to be added.\n", c);
            usage(name);
            return EXIT_FAILURE;
        }
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        for (i=0; i<connectionCount; i++){
            addEdge(graph, adj, connections[i][0], connections[i][1]);
        }
        
        writeMultiCode(graph, adj, stdout);
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

