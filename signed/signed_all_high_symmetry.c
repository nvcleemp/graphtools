/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multi_code format from standard in,
 * and writes 'all' signatures to standard out in signed_code format.
 * At least all signatures up to equivalence will be written, but several
 * signatures up to equivalence will be written several times. This version
 * computes the orbits of edge sets. 
 * 
 * This version does NOT support multigraphs!!!
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_all_high_symmetry -O4  signed_all_high_symmetry.c \
 *           shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include <getopt.h>

#define MAXN 100

#define MAX_EDGE_COUNT 28

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"
#include "nauty/nausparse.h"

int graphsWritten = 0;

int edgeCounter = 0;

FILE *outFile;

int negativeEdgesAtVertex[MAXN+1];

boolean noOne = FALSE;

//28 @ 32 bits roughly uses 1 gigabyte of memory
#define MAX_EDGE_SETS_COUNT 1 << 28

unsigned int edgeSets[MAX_EDGE_SETS_COUNT];

int edgeSetCounter;

/* Nauty worksize */
#define WORKSIZE 50 * MAXM

/** Nauty variables */
int lab[MAXN], ptn[MAXN], orbits[MAXN];
statsblk stats;
setword workspace[WORKSIZE];

sparsegraph sg; /* Sparse graph datastructure for nauty */
DEFAULTOPTIONS_SPARSEGRAPH(nautyOptions);

permutation automorphismGroupGenerators[MAXN][MAXN];
int numberOfGenerators;

void printEdgeSet(unsigned int edgeSet){
    int i;
    for(i = 0; i < edgeCounter; i++){
        if((1<<i)&edgeSet){
            fprintf(stderr, "%d-%d ", edges[i].smallest, edges[i].largest);
        }
    }
    fprintf(stderr, "\n");
}

void storeGenerator(int count, permutation perm[], nvector orbits[],
        int numorbits, int stabvertex, int n) {
    memcpy(automorphismGroupGenerators + numberOfGenerators, perm, sizeof(permutation) * n);

    numberOfGenerators++;
}

/* This method translates the internal data structure to nauty's sparse graph
 * data structure, so the graph can be passed to nauty.
 */
void translateCurrentGraphToNautySparseGraph(GRAPH graph, ADJACENCY adj, int order){
    sg.nv = order;
    sg.nde = edgeCounter;

    int i, j;
    for(i = 1; i <= order;i++) {
        sg.d[i-1] = adj[i];
        for(j = 0; j < adj[i]; j++) {
            if(graph[i][j]->largest==i){
                sg.e[(i-1) * MAXVAL + j] = graph[i][j]->smallest - 1;
            } else {
                sg.e[(i-1) * MAXVAL + j] = graph[i][j]->largest - 1;
            }
        }
    }
}

void initNautyOptions(int order) {
    nautyOptions.getcanon = FALSE;
    nautyOptions.userautomproc = storeGenerator;

    /* Init the nauty datastructures */
    SG_INIT(sg);
    SG_ALLOC(sg, MAXN, MAXVAL * MAXN, "Failed to allocate memory to store graph");

    //sg.v only has to be set once
    int i;
    for(i = 0; i < MAXN; i++) {
        sg.v[i] = i * MAXVAL;
    }
}

int getImageOfEdgeSet(unsigned int edgeSet, permutation *automorphism){
    int i, j;
    unsigned int image = 0;

    for(i = 0; i<edgeCounter; i++){
        if(edgeSet & (1<<i)){
            int v = edges[i].smallest;
            int w = edges[i].largest;
            int vImage = automorphism[v-1]+1;
            int wImage = automorphism[w-1]+1;
            if(vImage > wImage){
                int temp = vImage;
                vImage = wImage;
                wImage = temp;
            }
            j = 0;
            while(j < edgeCounter && (edges[j].smallest!=vImage || edges[j].largest!=wImage)) j++;
            if(j==edgeCounter){
                fprintf(stderr, "Error while finding image of edge -- exiting!\n");
                exit(EXIT_FAILURE);
            }
            image |= 1 << j;
        }
    }

    return image;
}

//=================== union-find ==============================

int findRootOfElement(int *forest, int element) {
    //find with path-compression
    if(element!=forest[element]){
        forest[element]=findRootOfElement(forest, forest[element]);
    }
    return forest[element];
}

//implementation of union-find algorithm for sets of vertices
void unionElements(int *forest, int *treeSizes, int *numberOfComponents, int element1, int element2){
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

void storeSignature(){
    int i;
    edgeSets[edgeSetCounter] = 0;
    for(i = 0; i < edgeCounter; i++){
        if(edges[i].isNegative){
            edgeSets[edgeSetCounter] |= (1 << i);
        }
    }
    edgeSetCounter++;
}

void loadSignature(int signatureNumber){
    int i;
    for(i = 0; i < edgeCounter; i++){
        edges[i].isNegative = (edgeSets[signatureNumber] & (1 << i));
    }
}

void assignSigns_impl(int currentEdge, int negativeEdgeCount, ADJACENCY adj){
    if(currentEdge == edgeCounter){
        if(noOne && negativeEdgeCount==1){
            return;
        }
        storeSignature();
    } else {
        edges[currentEdge].isNegative = FALSE;
        assignSigns_impl(currentEdge+1, negativeEdgeCount, adj);
        edges[currentEdge].isNegative = TRUE;
        int v = edges[currentEdge].largest;
        int w = edges[currentEdge].smallest;
        negativeEdgesAtVertex[v]++;
        negativeEdgesAtVertex[w]++;
        if((negativeEdgesAtVertex[v]*2<=adj[v]) &&
                (negativeEdgesAtVertex[w]*2<=adj[w])){
            assignSigns_impl(currentEdge+1, negativeEdgeCount + 1, adj);
        }
        negativeEdgesAtVertex[v]--;
        negativeEdgesAtVertex[w]--;
    }
}

void assignSigns(GRAPH graph1, ADJACENCY adj, int order){
    //compute sets
    int i,j,k;
    edgeSetCounter = 0;
    for(i=1; i < order; i++){
        negativeEdgesAtVertex[i] = 0;
    }
    assignSigns_impl(0, 0, adj);
    
    //translate to nauty graph
    initNautyOptions(order);
    translateCurrentGraphToNautySparseGraph(graph1, adj, order);
    
    //calculate automorphism group generators
    numberOfGenerators = 0;
    nauty((graph*) &sg, lab, ptn, NULL, orbits, &nautyOptions, &stats, workspace, WORKSIZE, MAXM, order, NULL);    
    
    //perform union-find
    int forest[edgeSetCounter];
    int treeSizes[edgeSetCounter];
    for(i = 0; i < edgeSetCounter; i++){
        forest[i] = i;
        treeSizes[i] = 1;
    }
    
    int numberOfOrbits = edgeSetCounter;
    for(i=0; i<numberOfGenerators; i++){
        for(j=0; j<edgeSetCounter; j++){
            unsigned int edgeSetImage = getImageOfEdgeSet(edgeSets[j], automorphismGroupGenerators[i]);
            k = 0;
            while((k<edgeSetCounter) && (edgeSetImage != edgeSets[k])) k++;
            if(k==edgeSetCounter){
                fprintf(stderr, "Error while finding image of edge set -- exiting!\n");
                exit(EXIT_FAILURE);
            }
            unionElements(forest, treeSizes, &numberOfOrbits, j, k);
        }
    }
    
    //write signed graphs
    for(i=0; i<edgeSetCounter; i++){
        if(forest[i]==i){
            loadSignature(i);
            writeSignedCode(graph1, adj, order, outFile);
            graphsWritten++;
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s assigns signs to graphs in multicode format in 'all'\n", name);
    fprintf(stderr, "possible ways.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    --no-one\n");
    fprintf(stderr, "       Exclude assignments that only contain 1 negative edge.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {
    GRAPH graph;
    ADJACENCY adj;
    int graphCount;
    int codeLength;
    unsigned short code[MAXCODELENGTH];
    
    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"no-one", no_argument, NULL, 0},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                //handle long option with no alternative
                switch(option_index) {
                    case 0:
                        noOne = TRUE;
                        break;
                    default:
                        fprintf(stderr, "Illegal option index %d.\n", option_index);
                        usage(name);
                        return EXIT_FAILURE;
                }
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
    
    outFile = stdout;

    graphCount = 0;

    while (readMultiCode(code, &codeLength, stdin)){
        graphCount++;
        int order;
        decodeMultiCode(code, codeLength, graph, adj, &order);
        assignSigns(graph, adj, order);
    }

    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    fprintf(stderr, "Written %d signed graph%s.\n", graphsWritten, graphsWritten==1 ? "" : "s");

    return EXIT_SUCCESS;

}


