/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a hamiltonian graph in multicode format from standard in,
 * embeds its vertices on a circle in the order of a hamiltonian cycle and 
 * writes the embedded graph to standard out in writegraph2d format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o hamiltonian_embed -O4  hamiltonian_embed.c \
 *     ../multicode/shared/multicode_base.c \
 *     ../multicode/shared/multicode_input.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_input.h"

boolean currentCycle[MAXN+1];
int cycleOrder[MAXN];

/**
  * 
  */
boolean continueCycle(GRAPH graph, ADJACENCY adj, int target, int next, int remaining) {
    int i;
    
    if(target==next){
        if(remaining==0){
            return TRUE;
        }
        return FALSE;
    }
    
    cycleOrder[graph[0][0] - remaining] = next;
    
    for(i = 0; i < adj[next]; i++){
        if(!currentCycle[graph[next][i]]){
            currentCycle[graph[next][i]]=TRUE;
            if(continueCycle(graph, adj, target, graph[next][i], remaining - 1)){
                return TRUE;
            }
            currentCycle[graph[next][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean findHamiltonianCycle(GRAPH graph, ADJACENCY adj){
    int i, j;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    
    if(order<3){
        return FALSE;
    }
    
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
            minDegreeVertex = i;
        }
    }
    
    if(minDegree == 1){
        return FALSE;
    }
    
    //just look for a hamiltonian cycle
    for(i=0; i<=MAXN; i++){
        currentCycle[i] = FALSE;
    }
    
    currentCycle[minDegreeVertex] = TRUE;
    cycleOrder[1] = minDegreeVertex;
    for(i = 1; i < adj[minDegreeVertex]; i++){
        currentCycle[graph[minDegreeVertex][i]] = TRUE;
        for(j = 0; j < i; j++){
            cycleOrder[0] = graph[minDegreeVertex][j];
            //search for cycle containing graph[minDegreeVertex][i], minDegreeVertex,  graph[minDegreeVertex][j]
            if(continueCycle(graph, adj, graph[minDegreeVertex][j], graph[minDegreeVertex][i], order - 2)){
                return TRUE;
            }
        }
        currentCycle[graph[minDegreeVertex][i]]=FALSE;
    }
    
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

void writeWritegraph2d_fixedCycle(GRAPH graph, ADJACENCY adj, FILE *f){
    static int first = TRUE;
    int i, j, pos;
    int nv = graph[0][0];
    
    if(first){
        first = FALSE;
        
        fprintf(f, ">>writegraph2d<<\n");
    }
    
    int cyclePosition[nv+1];
    
    for(i = 0; i <= nv; i++){
        cyclePosition[cycleOrder[i]] = i;
    }
    
    for(i = 1; i <= nv; i++){
        //current vertex
        fprintf(f, "%3d ", i);
        
        pos = cyclePosition[i];
        
        //coordinates
        fprintf(f, "%.4f %.4f ", cos(2*(pos-1)*M_PI/nv), sin(2*(pos-1)*M_PI/nv));
        
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
    fprintf(stderr, "The program %s embeds a hamiltonian graph in multicode format.\n\n", name);
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
        if(findHamiltonianCycle(graph, adj)){
            writeWritegraph2d_fixedCycle(graph, adj, stdout);
        } else {
            fprintf(stderr, "Error! Graph is not hamiltonian.\n");
        }
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

