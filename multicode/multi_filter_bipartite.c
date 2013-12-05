/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * writes those that are bipartite to standard out in multicode format.
 * 
 * Compile with:
 *     
 *     cc -o multi_filter_bipartite -O4  multi_filter_bipartite.c \
 *     shared/multicode_base.c shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

#define WHITE 1
#define BLACK 0
#define GRAY -1

boolean onlyCount = FALSE;

boolean isBipartite(GRAPH graph, ADJACENCY adj){
    int i;
    int colours[MAXN+1];
    for(i = 0; i < MAXN + 1; i++) {
        colours[i] = GRAY;
    }
    
    int queueHead, queueTail;
    int queue[MAXN];

    queueHead = 0;
    queueTail = 1;
    queue[0] = 1;
    colours[1] = WHITE;

    while(queueTail > queueHead){
        int currentVertex = queue[queueHead++];

        for(i=0; i<adj[currentVertex]; i++){
            int neighbour = graph[currentVertex][i];
            if(colours[neighbour]==GRAY){
                //neighbour is not yet coloured
                colours[neighbour]=!colours[currentVertex];
                queue[queueTail++] = neighbour;
            } else if (colours[neighbour] == colours[currentVertex]){
                //neighbour is coloured, but has conflicting colour
                return FALSE;
            }
        }
    }
    
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters out graphs that are bipartite.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -c, --count\n");
    fprintf(stderr, "       Only count the number of graphs that are bipartite.\n");
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
    int graphsFiltered = 0;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"count", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hc", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                onlyCount = TRUE;
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
        graphsRead++;
        
        if(isBipartite(graph, adj)){
            if(!onlyCount){
                writeMultiCode(graph, adj, stdout);
            }
            graphsFiltered++;
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

