/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multi_code format from standard in,
 * and writes 'all' signatures to standard out in signed_code format.
 * At least all signatures up to equivalence will be written, but several
 * signatures up to equivalence will be written several times. 
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_all -O4  signed_all.c \
 *           shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include <getopt.h>

#define MAXN 100

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int graphsWritten = 0;

int edgeCounter = 0;

FILE *outFile;

int negativeEdgesAtVertex[MAXN+1];

boolean noOne = FALSE;
boolean isNumberOfEdgesFixed = FALSE;
int fixedNumberOfEdges = 0;

void assignSigns_impl(int currentEdge, int negativeEdgeCount, GRAPH graph, ADJACENCY adj, int order){
    if(currentEdge == edgeCounter){
        if(noOne && negativeEdgeCount==1){
            return;
        }
        if(isNumberOfEdgesFixed && negativeEdgeCount!=fixedNumberOfEdges){
            return;
        }
        writeSignedCode(graph, adj, order, outFile);
        graphsWritten++;
    } else {
        edges[currentEdge].isNegative = FALSE;
        assignSigns_impl(currentEdge+1, negativeEdgeCount, graph, adj, order);
        edges[currentEdge].isNegative = TRUE;
        int v = edges[currentEdge].largest;
        int w = edges[currentEdge].smallest;
        negativeEdgesAtVertex[v]++;
        negativeEdgesAtVertex[w]++;
        if((negativeEdgesAtVertex[v]*2<=adj[v]) &&
                (negativeEdgesAtVertex[w]*2<=adj[w])){
            assignSigns_impl(currentEdge+1, negativeEdgeCount + 1, graph, adj, order);
        }
        negativeEdgesAtVertex[v]--;
        negativeEdgesAtVertex[w]--;
    }
}

void assignSigns(GRAPH graph, ADJACENCY adj, int order){
    int i;
    for(i=1; i < order; i++){
        negativeEdgesAtVertex[i] = 0;
    }
    assignSigns_impl(0, 0, graph, adj, order);
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s assigns signs to graphs in multicode format in 'all'\n", name);
    fprintf(stderr, "possible ways. Not all signatures will be written, but for each equivalence\n");
    fprintf(stderr, "class at least one signature will be written.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    --no-one\n");
    fprintf(stderr, "       Exclude assignments that only contain 1 negative edge.\n");
    fprintf(stderr, "    -e n, --edges n\n");
    fprintf(stderr, "       Only give signatures with n negative edges. The use of this switch\n");
    fprintf(stderr, "       disables --no-one.\n");
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
        {"edges", required_argument, NULL, 'e'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "he:", long_options, &option_index)) != -1) {
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
            case 'e':
                isNumberOfEdgesFixed = TRUE;
                fixedNumberOfEdges = atoi(optarg);
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
    
    if(isNumberOfEdgesFixed){
        noOne = FALSE;
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


