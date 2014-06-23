/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes all 2-factors to standard error.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_all_2_factors -O4  cubic_all_2_factors.c \
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

boolean printSizes = FALSE;

void print2Factor(int factor[]){
    int i, v;
    int totalCount = 0;
    int currentCount;
    
    boolean available[MAXN];
    
    for(i=0; i<MAXN; i++){
        available[i] = TRUE;
    }
    
    while(totalCount < vertexCount){
        v = 0;
        while(!available[v]) v++;
        
        //start cycle from v
        available[v] = FALSE;
        currentCount = 1;
        i = 0;
        while(factor[v]==i || !available[graph[v][i]]) i++;
        int start = v;
        fprintf(stderr, "(%d", v);
        v = graph[v][i];
        while(v!=start){
            available[v] = FALSE;
            currentCount++;
            fprintf(stderr, ", %d", v);
            i = 0;
            while(i<3 && (factor[v]==i || !available[graph[v][i]])) i++;
            if(i==3) {
                v = start;
            } else {
                v = graph[v][i];
            }
        }
        fprintf(stderr, ")");
        totalCount += currentCount;
    }
    fprintf(stderr, "\n");
}

void print2FactorSizes(int factor[]){
    int i, v;
    int totalCount = 0;
    int currentCount;
    
    boolean available[MAXN];
    
    for(i=0; i<MAXN; i++){
        available[i] = TRUE;
    }
    
    while(totalCount < vertexCount){
        v = 0;
        while(!available[v]) v++;
        
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
        fprintf(stderr, "%d ", currentCount);
        totalCount += currentCount;
    }
    fprintf(stderr, "\n");
}

void handle2Factor(int factor[]){
    twoFactorCount++;
    if(printSizes){
        print2FactorSizes(factor);
    } else {
        print2Factor(factor);
    }
}

void findAll2FactorsImpl(boolean available[], boolean positionAdjacencyList[][MAXN], int factor[]){
    int v, i;
    
    v = 0;
    while(v<vertexCount && !available[v]) v++;
    
    if(v==vertexCount){
        handle2Factor(factor);
    } else {
        available[v] = FALSE;
        for(i=0; i<3; i++){
            int n = graph[v][i];
            if(available[n]){
                factor[v] = i;
                factor[n] = positionAdjacencyList[n][v];
                available[n] = FALSE;
                findAll2FactorsImpl(available, positionAdjacencyList, factor);
                available[n] = TRUE;
            }
        }
        available[v] = TRUE;
    }
}

void findAll2Factors(){
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
        findAll2FactorsImpl(available, positionAdjacencyList, factor);
        available[neighbour] = TRUE;
    }
    
    fprintf(stderr, "Found %d 2-factor%s.\n\n", twoFactorCount,
            twoFactorCount == 1 ? "" : "s");
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s prints all the 2-factors of the given cubic graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -s, --sizes\n");
    fprintf(stderr, "       Print the sizes of the components instead of the components.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    int graphsRead = 0;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"sizes", no_argument, NULL, 's'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hs", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                printSizes = TRUE;
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
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;
        
        fprintf(stderr, "Graph %d:\n", graphsRead);
        findAll2Factors();
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

