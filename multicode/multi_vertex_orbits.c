/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads simple graphs from standard in and
 * computes the vertex orbits.
 * 
 * 
 * Compile with:
 * 
 *     cc -o multi_vertex_orbits -O4 multi_vertex_orbits.c \
 *          shared/multicode_base.c shared/multicode_input.c \
 *          ../nauty/nauty.c ../nauty/nautil.c ../nauty/naugraph.c\
 *          ../nauty/schreier.c ../nauty/naurng.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "../nauty/nauty.h"

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"

/* Nauty worksize */
#define WORKSIZE 50 * MAXM

/** Nauty variables */
int lab[MAXN], ptn[MAXN], orbits[MAXN];
static DEFAULTOPTIONS_GRAPH(options);
statsblk stats;
setword workspace[WORKSIZE];

graph ng[MAXN*MAXM]; /* nauty graph datastructure */

int m;

int graphCount = 0;


/* This method translates the internal data structure to nauty's dense graph
 * data structure, so the graph can be passed to nauty.
 */
inline void translateGraphToNautyDenseGraph(GRAPH graph, ADJACENCY adj){
    int n, i, j;
    
    n = graph[0][0];
    
    if(n > MAXN){
        fprintf(stderr, "We only support graphs with up to %d vertices - exiting!\n", MAXN);
        exit(EXIT_FAILURE);
    }
    
    m = SETWORDSNEEDED(n);
    
    nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
    
    EMPTYGRAPH(ng,m,n);
    
    for(i = 1; i <= graph[0][0]; i++){
        for(j = 0; j < adj[i]; j++){
            if(i < graph[i][j]){
                ADDONEEDGE(ng, i - 1, graph[i][j] - 1, m);
            }
        }
    }
}

void findVertexOrbits(GRAPH g, ADJACENCY adj){
    int i;
    
    translateGraphToNautyDenseGraph(g, adj);
    
    //call Nauty so we have the automorphism group
    nauty((graph*) &ng, lab, ptn, NULL, orbits, &options, &stats, workspace, WORKSIZE, m, g[0][0], NULL);
        
    fprintf(stderr, "Graph %d has %d vertex orbit%s.\n", graphCount, stats.numorbits,
            stats.numorbits == 1 ? "" : "s");
    
    int orbitCount = 0;
    int orbitSizes[MAXN];
    for(i = 0; i < g[0][0]; i++){
        orbitSizes[i] = 0;
    }
    for(i = 0; i < g[0][0]; i++){
        orbitSizes[orbits[i]]++;
    }
    
    for(i = 0; i < g[0][0]; i++){
        if(orbits[i] == i){
            fprintf(stderr, "Orbit %d (representative %d) contains %d %s.\n",
                    ++orbitCount, i + 1, orbitSizes[i],
                    orbitSizes[i] == 1 ? "vertex" : "vertices");
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s computes the vertex orbits of graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices and degrees up to %d.\n\n", MAXN, MAXVAL);
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

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

    GRAPH graph;
    ADJACENCY adj;
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        graphCount++;
        
        findVertexOrbits(graph, adj);
    }
    
    return EXIT_SUCCESS;
}
