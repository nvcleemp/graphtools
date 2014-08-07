/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads simple graphs from standard in and
 * computes the edge orbits.
 * 
 * 
 * Compile with:
 * 
 *     cc -o multi_edge_orbits -O4 multi_edge_orbits.c \
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

typedef int VERTEXPAIR[2];

/* Nauty worksize */
#define WORKSIZE 50 * MAXM

/** Nauty variables */
int lab[MAXN], ptn[MAXN], orbits[MAXN];
static DEFAULTOPTIONS_GRAPH(options);
statsblk stats;
setword workspace[WORKSIZE];

graph ng[MAXN*MAXM]; /* nauty graph datastructure */

int generators[MAXN][MAXN];
int generatorCount;

int n;
int m;

int graphCount = 0;

// debugging methods

void printGenerators(int vertexCount){
    int i, j;
    fprintf(stderr, "Generators:\n");
    for(i = 0; i < generatorCount; i++){
        for(j = 0; j < vertexCount; j++){
            fprintf(stderr, "%d ", generators[i][j]);
        }
        fprintf(stderr, "\n");
    }
}

// end debugging methods

/**
 * Method which is called each time nauty finds a generator.
 */
void storeGenerators(int count, int perm[], nvector orbits[], int numorbits, int stabvertex, int n) {
    memcpy(generators + generatorCount, perm, sizeof(int) * n);

    generatorCount++;
}

void initNautyRelatedVariables(){
    options.userautomproc = storeGenerators;
}

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

inline void callNauty(){
    
    //call nauty
    generatorCount = 0;
    nauty((graph*) &ng, lab, ptn, NULL, orbits, &options, &stats, workspace, WORKSIZE, m, n, NULL);
}

int findRootOfElement(int forest[], int element) {
    //find with path-compression
    if(element!=forest[element]){
        forest[element]=findRootOfElement(forest, forest[element]);
    }
    return forest[element];
}

void unionElements(int forest[], int treeSizes[], int *numberOfComponents, int element1, int element2){
    int root1 = findRootOfElement(forest, element1);
    int root2 = findRootOfElement(forest, element2);

    if(root1==root2) return;

    if(treeSizes[root1]<treeSizes[root2]){
        forest[root1]=root2;
        treeSizes[root2]+=treeSizes[root1];
    } else {
        forest[root2]=root1;
        treeSizes[root1]+=treeSizes[root2];
    }
    (*numberOfComponents)--;
}

void determineEdgeOrbits(VERTEXPAIR edges[], int edgesCount, int edgeOrbits[], int edgeOrbitSizes[], int *orbitsCount) {
    int i, j, k, temp;

    //initialization of the variables
    for(i=0; i<edgesCount; i++){
        edgeOrbits[i]=i;
        edgeOrbitSizes[i]=1;
    }
    *orbitsCount=edgesCount;

    if(generatorCount==0){
        //if the automorphism group is trivial
        return;
    }

    int *permutation;
    VERTEXPAIR edge;
    
    for(i = 0; i < generatorCount; i++) {
        permutation = generators[i];

        for(j = 0; j<edgesCount; j++){
            //apply permutation to current edge
            edge[0] = permutation[edges[j][0]];
            edge[1] = permutation[edges[j][1]];

            //canonical form of edge
            if(edge[0]>edge[1]){
                temp = edge[1];
                edge[1] = edge[0];
                edge[0] = temp;
            }

            //search the pair in the list
            for(k = 0; k<edgesCount; k++){
                if(edge[0] == edges[k][0] && edge[1] == edges[k][1]){
                    unionElements(edgeOrbits, edgeOrbitSizes, orbitsCount, j, k);
                    break; //the list of edges doesn't contain any duplicates so we can stop
                }
            }
        }
    }

    //make sure that each element is connected to its root
    for(i = 0; i < edgesCount; i++){
        findRootOfElement(edgeOrbits, i);
    }
}

void findEdgeOrbits(GRAPH graph, ADJACENCY adj){
    VERTEXPAIR edges[MAXN * (MAXN - 1)/2];
    
    n = graph[0][0];
    
    initNautyRelatedVariables();
    translateGraphToNautyDenseGraph(graph, adj);
    
    int i, v, edgeCount = 0, edgeOrbitCount;
    setword *gv;
    
    //call Nauty so we have the automorphism group
    callNauty();
    
    //build a list of all edges
    for(v = 0; v < n; v++){
        gv = GRAPHROW(ng, v, m);
        for (i = -1; (i = nextelement(gv,m,i)) >= 0;){
            if(v < i){
                edges[edgeCount][0] = v;
                edges[edgeCount][1] = i;
                edgeCount++;
            }
        }
    }
    
    //using malloc to dynamically allocate the arrays on the heap
    //otherwise we might run out of stack space for large, dense graphs
    int *edgeOrbits = (int *) malloc(sizeof(int) * edgeCount);
    int *edgeOrbitSizes = (int *) malloc(sizeof(int) * edgeCount);
    
    //partition the edges into orbits
    determineEdgeOrbits(edges, edgeCount, edgeOrbits, edgeOrbitSizes, &edgeOrbitCount);
    
    fprintf(stderr, "Graph %d has %d edge orbit%s.\n", graphCount, edgeOrbitCount,
            edgeOrbitCount == 1 ? "" : "s");
    
    int orbitCount = 0;
    for(i = 0; i < edgeCount; i++){
        if(edgeOrbits[i] == i){
            fprintf(stderr, "Orbit %d (representative %d - %d) contains %d edge%s.\n",
                    ++orbitCount, edges[i][0] + 1, edges[i][1] + 1, edgeOrbitSizes[i],
                    edgeOrbitSizes[i] == 1 ? "" : "s");
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s computes the edge orbits of graphs.\n\n", name);
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
        
        findEdgeOrbits(graph, adj);
    }
    
    return EXIT_SUCCESS;
}
