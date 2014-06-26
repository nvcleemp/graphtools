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
#include "shared/cubic_output.h"

int vertexCount;
GRAPH graph;
int twoFactorCount;
int hamiltonianTwoFactorCount;
int disconnectedTwoFactorCount;

int isInMatching[MAXN][MAXN];

boolean isHamiltonianTwoFactor(int factor[]){
    int i, v;
    int currentCount;
    
    boolean available[MAXN];
    
    for(i=0; i<MAXN; i++){
        available[i] = TRUE;
    }
    
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
    return currentCount == vertexCount;
}

void handle2Factor(int factor[]){
    twoFactorCount++;
    if(isHamiltonianTwoFactor(factor)){
        hamiltonianTwoFactorCount++;
    } else {
        disconnectedTwoFactorCount++;
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
            if(available[n] && !isInMatching[v][n]){
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
    hamiltonianTwoFactorCount = 0;
    disconnectedTwoFactorCount = 0;
    
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
    
    if(hamiltonianTwoFactorCount && !disconnectedTwoFactorCount){
//        fprintf(stderr, "OK\n");
        writeCubicMultiCode(graph, vertexCount, stdout);
    } /*else {
        fprintf(stderr, "Too bad\n");
    }*/
}

int count = 0;

void handleMatching(){
    count++;
    findAll2Factors();
}

boolean degree3[MAXN];

void createMatching_impl(){
    int currentVertex = 6;
    while(currentVertex < vertexCount && degree3[currentVertex]){
        currentVertex++;
    }
    
    if(currentVertex == vertexCount){
        handleMatching();
        return;
    }
    
    degree3[currentVertex] = TRUE;
    
    //make all connections for currentVertex
    int i;
    
    for(i = currentVertex + 1; i < vertexCount; i++){
        if(!degree3[i]){
            graph[currentVertex][2] = i;
            graph[i][2] = currentVertex;
            degree3[i] = TRUE;
            isInMatching[currentVertex][i] = isInMatching[i][currentVertex] = TRUE;
            createMatching_impl();
            isInMatching[currentVertex][i] = isInMatching[i][currentVertex] = FALSE;
            degree3[i] = FALSE;
        }
    }
    
    degree3[currentVertex] = FALSE;
}

void createMatching(){
    int i, j;
    
    for(i = 0; i < MAXN; i++){
        for(j = 0; j < MAXN; j++){
            isInMatching[i][j] = FALSE;
        }
    }
    
    for(i = 0; i < vertexCount; i++){
        degree3[i] = (i < 6);
    }
    
    createMatching_impl();
}

void subdivideEdge(int from, int to){
    int i;
    int posFrom, posTo;
    
    i = 0;
    while(graph[from][i]!=to){
        i++;
    }
    posFrom = i;
    
    i = 0;
    while(graph[to][i]!=from){
        i++;
    }
    posTo = i;
    
    graph[from][posFrom] = vertexCount;
    graph[to][posTo] = vertexCount+1;
    graph[vertexCount][0] = from;
    graph[vertexCount][1] = vertexCount+1;
    graph[vertexCount+1][0] = to;
    graph[vertexCount+1][1] = vertexCount;
    
    vertexCount+=2;
}

void revertLastSubdivision(){
    int from = graph[vertexCount-2][0];
    int to = graph[vertexCount-1][0];
    
    int i;
    
    i = 0;
    while(graph[from][i] != vertexCount - 2){
        i++;
    }
    int posFrom = i;
    
    i = 0;
    while(graph[to][i] != vertexCount - 1){
        i++;
    }
    int posTo = i;
    
    graph[from][posFrom] = to;
    graph[to][posTo] = from;
    
    vertexCount -= 2;
}

void subdivideK33impl(int times){
    if(times == 0){
        createMatching();
        return;
    }
    int i, j;
    
    for(i = 0; i < 3; i++){
        for(j = 0; j < 3; j++){
            subdivideEdge(i, graph[i][j]);
            subdivideK33impl(times - 1);
            revertLastSubdivision();
        }
    }
}

void initialSubdivision(){
    subdivideEdge(0, 3);
    subdivideEdge(1, 4);
    subdivideEdge(2, 5);
}

void buildK33(){
    int i, j;
    vertexCount = 6;
    
    for(i = 0; i < 3; i++){
        for(j = 0; j < 3; j++){
            graph[i][j] = j + 3;
            graph[i+3][j] = j;
        }
    }
}

void subdivideK33_first(int times){
    subdivideEdge(0, 6);
    subdivideK33impl(times-1);
    revertLastSubdivision();
    subdivideEdge(0, 4);
    subdivideK33impl(times-1);
    revertLastSubdivision();
}

void subdivideK33(int times){
    buildK33();
    initialSubdivision();
    if(times==0){
        createMatching();
    } else {
        subdivideK33_first(times);
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s prints all the 2-factors of the given cubic\ngraphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -s, --sizes\n");
    fprintf(stderr, "       Print the sizes of the components instead of the components.\n");
    fprintf(stderr, "    -p, --parities\n");
    fprintf(stderr, "       Print the parities of the sizes of the components instead of the\n");
    fprintf(stderr, "       components.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {

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
    
    int times = atoi(argv[argc-1]);
    
    subdivideK33(times);

    return (EXIT_SUCCESS);
}

