/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes the ones that are 2-factor hamiltonian to standard out.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_is_2_factor_hamiltonian -O4  cubic_is_2_factor_hamiltonian.c \
 *     shared/cubic_base.c shared/cubic_input.c shared/cubic_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/cubic_base.h"
#include "shared/cubic_input.h"
#include "shared/cubic_output.h"

int vertexCount;
GRAPH graph;
int twoFactorCount;

boolean isHamiltonian2Factor(int factor[]){
    int i, v;
    int currentCount;
    
    boolean available[MAXN];
    
    for(i=0; i<MAXN; i++){
        available[i] = TRUE;
    }
    
    v = 0;

    //start cycle from v
    available[v] = FALSE;
    currentCount = 1;
    i = 0;
    while(factor[v]==i || !available[graph[v][i]]) i++;
    int start = v;
    v = graph[v][i];
    while(v!=start){
        available[v] = FALSE;
        currentCount++;
        i = 0;
        while(i<3 && (factor[v]==i || !available[graph[v][i]])) i++;
        if(i==3) {
            v = start;
        } else {
            v = graph[v][i];
        }
    }
    
    return currentCount == vertexCount;
}

boolean handle2Factor(int factor[]){
    twoFactorCount++;
    return isHamiltonian2Factor(factor);
}

boolean is2FactorHamiltonianImpl(boolean available[], boolean positionAdjacencyList[][MAXN], int factor[]){
    int v, i;
    
    v = 0;
    while(v<vertexCount && !available[v]) v++;
    
    if(v==vertexCount){
        if(!handle2Factor(factor)){
            return FALSE;
        }
    } else {
        available[v] = FALSE;
        for(i=0; i<3; i++){
            int n = graph[v][i];
            if(available[n]){
                factor[v] = i;
                factor[n] = positionAdjacencyList[n][v];
                available[n] = FALSE;
                if(!is2FactorHamiltonianImpl(available, positionAdjacencyList, factor)){
                    return FALSE;
                }
                available[n] = TRUE;
            }
        }
        available[v] = TRUE;
    }
    return TRUE;
}

boolean is2FactorHamiltonian(){
    int i, j;
    
    twoFactorCount = 0;
    
    boolean available[MAXN];
    int positionAdjacencyList[MAXN][MAXN];
    int factor[MAXN];
    
    for(i=0; i<MAXN; i++){
        available[i] = TRUE;
    }
    
    for(i=0; i<vertexCount; i++){
        for(j=0; j<3; j++){
            positionAdjacencyList[i][graph[i][j]] = j;
        }
    }
    
    available[0] = FALSE;
    for(i=0; i<3; i++){
        int neighbour = graph[0][i];
        factor[0] = i;
        factor[neighbour] = positionAdjacencyList[neighbour][0];
        available[neighbour] = FALSE;
        if(!is2FactorHamiltonianImpl(available, positionAdjacencyList, factor)){
            return FALSE;
        }
        available[neighbour] = TRUE;
    }
    
    return twoFactorCount > 0;
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters the cubic graphs that are 2-factor hamiltonian.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -v, --verbose\n");
    fprintf(stderr, "       Print extra text to stderr for each 2-factor hamiltonian graph.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    int graphsRead = 0, graphsFiltered = 0;
    boolean verbose = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case 'v':
                verbose = TRUE;
                break;
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
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;
        
        if(is2FactorHamiltonian()){
            if(verbose){
                fprintf(stderr, "Graph %d is 2-factor hamiltonian.\n", graphsRead);
            }
            writeCubicMultiCode(graph, vertexCount, stdout);
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Written %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

