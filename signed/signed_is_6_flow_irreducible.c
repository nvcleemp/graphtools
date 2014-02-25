/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format and computes whether
 * they are a 6-flow irreducible.
 * 
 * 
 * Compile with:
 *     
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int edgeCounter = 0;

int graphCount = 0;
int graphsFiltered = 0;

boolean containsReducibleMultiEdge(GRAPH graph, ADJACENCY adj, int order){
    int i, j, v, w;
    int multiplicity, negativeEdgeCount;
    
    for(v = 1; v <= order; v++){
        unsigned long long int checkedNeighbours = 0ULL;
        for(i = 0; i < adj[v]; i++){
            if(graph[v][i]->smallest==v &&
                    !(checkedNeighbours & (1<<(graph[v][i]->largest-1)))){
                checkedNeighbours |= 1<<(graph[v][i]->largest-1);
                w = graph[v][i]->largest;
                multiplicity = negativeEdgeCount = 0;
                for(j = 0; j < adj[v]; j++){
                    if(graph[v][j]->largest==w){
                        multiplicity++;
                        if(graph[v][j]->isNegative){
                            negativeEdgeCount++;
                        }
                    }
                }
                if(multiplicity==1){
                    //simple edges are never reducible
                    continue;
                }
                if(negativeEdgeCount > 1){
                    //if multiple edges are negative, then they can be reduced to a single negative edge
                    return TRUE;
                }
                if(negativeEdgeCount==0){
                    //a positive multi-edge is always reducible
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

boolean containsReducibleTriangle(GRAPH graph, ADJACENCY adj, int order){
    int i, j, v, w;
    unsigned long long int neighbourhood[MAXN];
    unsigned long long int negativeNeighbourhood[MAXN];
    
    for(v = 1; v <= order; v++){
        neighbourhood[v-1] = 0ULL;
        negativeNeighbourhood[v-1] = 0ULL;
        for(i = 0; i < adj[v]; i++){
            if(graph[v][i]->smallest==v){
                neighbourhood[v-1] |= (1 << (graph[v][i]->largest-1));
            } else {
                neighbourhood[v-1] |= (1 << (graph[v][i]->smallest-1));
            }
            if(graph[v][i]->isNegative){
                if(graph[v][i]->smallest==v){
                    negativeNeighbourhood[v-1] |= (1 << (graph[v][i]->largest-1));
                } else {
                    negativeNeighbourhood[v-1] |= (1 << (graph[v][i]->smallest-1));
                }
            }
        }
    }
    for(v = 1; v <= order; v++){
        for(i = 0; i < adj[v]; i++){
            if(graph[v][i]->smallest==v &&
                    !(negativeNeighbourhood[v-1] & (1<<(graph[v][i]->largest-1)))){
                w = graph[v][i]->largest;
                for(j = 0; j < adj[w]; j++){
                    if(graph[w][j]->smallest==w &&
                            !(negativeNeighbourhood[w-1] & (1<<(graph[w][j]->largest-1))) &&
                            !(negativeNeighbourhood[v-1] & (1<<(graph[w][j]->largest-1))) &&
                            (neighbourhood[v-1] & (1<<(graph[w][j]->largest-1)))){
                        //we found a positive triangle: v,w,graph[w][j]->largest
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

boolean is6FlowIrreducible(GRAPH graph, ADJACENCY adj, int order){
    int i;
    int minDegree = MAXN;
    for(i=1; i<=order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
        }
    }
    if(minDegree==1){
        fprintf(stderr, "The input graph was not flow-admissable. Exiting!\n");
        exit(EXIT_FAILURE);
    }
    if(minDegree==2){
        //degree 2 vertices are always reducible
        return FALSE;
    }
    if(containsReducibleMultiEdge(graph, adj, order)){
        return FALSE;
    }
    if(containsReducibleTriangle(graph, adj, order)){
        return FALSE;
    }
    
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s determines whether signed graphs in signed_code\n", name);
    fprintf(stderr, "format are a 6-flow irreducible.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] k\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs and give a larger value for MAXN.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       Filter graphs that have a k-flow.\n");
    fprintf(stderr, "    -i, --invert\n");
    fprintf(stderr, "       Invert the filter.\n");
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
    
    boolean doFiltering = FALSE;
    boolean invert = FALSE;
    boolean asMulticode = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"invert", no_argument, NULL, 'i'},
        {"filter", no_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hfi", long_options, &option_index)) != -1) {
        switch (c) {
            case 'i':
                invert = TRUE;
                break;
            case 'f':
                doFiltering = TRUE;
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
    while (readSignedCode(code, &length, stdin)) {
        int order;
        decodeSignedCode(code, length, graph, adj, &order);
        graphCount++;
        
        boolean value = is6FlowIrreducible(graph, adj, order);
        if(doFiltering){
            if(invert && !value){
                graphsFiltered++;
                if(asMulticode){
                    writeAsMultiCode(graph, adj, order, stdout);
                } else {
                    writeSignedCode(graph, adj, order, stdout);
                }
            } else if(!invert && value){
                graphsFiltered++;
                if(asMulticode){
                    writeAsMultiCode(graph, adj, order, stdout);
                } else {
                    writeSignedCode(graph, adj, order, stdout);
                }
            }
        } else {
            if(value){
                fprintf(stdout, "Graph %d is 6-flow-irreducible.\n", graphCount);
            } else {
                fprintf(stdout, "Graph %d is not 6-flow-irreducible.\n", graphCount);
            }
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    if(doFiltering){
        fprintf(stderr, "Filtered %d graph%s that %s %s 6-flow-irreducible.\n", 
                graphsFiltered, graphsFiltered==1 ? "" : "s",
                graphsFiltered==1 ? "is" : "are",
                invert ? "not" : "");
    }

    return (EXIT_SUCCESS);
}

