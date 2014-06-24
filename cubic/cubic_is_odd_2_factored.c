/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes those that are odd 2-factored to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_is_odd_2_factored -O4  cubic_is_odd_2_factored.c \
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

boolean isNonOdd2Factor(int factor[]){
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
        if(currentCount%2==0){
            return TRUE;
        } else {
            totalCount += currentCount;
        }
    }
    return FALSE;
}

boolean hasNonOdd2FactorImpl(boolean available[], boolean positionAdjacencyList[][MAXN], int factor[]){
    int v, i;
    
    v = 0;
    while(v<vertexCount && !available[v]) v++;
    
    if(v==vertexCount){
        return isNonOdd2Factor(factor);
    } else {
        available[v] = FALSE;
        for(i=0; i<3; i++){
            int n = graph[v][i];
            if(available[n]){
                factor[v] = i;
                factor[n] = positionAdjacencyList[n][v];
                available[n] = FALSE;
                if(hasNonOdd2FactorImpl(available, positionAdjacencyList, factor)){
                    return TRUE;
                }
                available[n] = TRUE;
            }
        }
        available[v] = TRUE;
        return FALSE;
    }
}

boolean hasNonOdd2Factor(){
    /*
     * We construct a 2-factor by constructing a 1-factor. The complement is then
     * a 2-factor. In the array factor we store the position in the adjacency list
     * of the edge that is used for the 1-factor. This allows us to handle multi-
     * graphs.
     */
    int i, j;
    
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
        int n = graph[0][i];
        factor[0] = i;
        factor[n] = positionAdjacencyList[n][0];
        available[n] = FALSE;
        if(hasNonOdd2FactorImpl(available, positionAdjacencyList, factor)){
            return TRUE;
        }
        available[n] = TRUE;
    }
    
    return FALSE;
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s .\n\n", name);
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
    
    int graphsRead = 0;
    int graphsFiltered = 0;

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
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;
        
        if(!hasNonOdd2Factor()){
            writeCubicMultiCode(graph, vertexCount, stdout);
            graphsFiltered++;
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

