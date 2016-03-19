/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multicode format from standard in,
 * identifies the specified vertices and writes the new graph to 
 * standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_identify -O4  multi_identify.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

int doFind(int *parents, int element);

void doPathCompress(int *parents, int minimumElement, int maximumElement){
    int i;
    for(i=minimumElement; i<= maximumElement; i++){
        doFind(parents, i);
    }
}

//find
int doFind(int *parents, int element){
     if(parents[element] != element){
        parents[element] = doFind(parents, parents[element]);
     }
     return parents[element];
}

//union
void makeUnion(int *parents, int *depth, int el1, int el2){
    
     int parent1 = doFind(parents, el1);
     int parent2 = doFind(parents, el2);
     if (parent1==parent2){
         return;
     } else if(parent2 < parent1){
         int temp = parent2;
         parent2 = parent1;
         parent1 = temp;
     }

     if(depth[parent1] < depth[parent2]){
         parents[parent1] = parent2;
     } else if (depth[parent1] > depth[parent2]){
         parents[parent2] = parent1;
     } else {
         parents[parent2] = parent1;
         depth[parent1] += 1;
     }
}

//The complexity of this function is horrible, but we just need it for a single graph,
//so maybe close your eyes while reading this function.
void makeIdentifications(GRAPH graph, ADJACENCY adj, 
        GRAPH resultGraph, ADJACENCY resultAdj, int newOrder, 
        int *old2New, int *new2Old, int *combinedVertices){
    int i, j;
    
    //remove the edges between vertices that will be identified
    for(i = 1; i < graph[0][0]; i++){
        for(j = i + 1; j<=graph[0][0]; j++){
            if(combinedVertices[i]==combinedVertices[j]){
                removeEdge(graph, adj, i, j, TRUE);
            }
        }
    }
    
    prepareGraph(resultGraph, resultAdj, newOrder);
    
    for(i=1; i<=graph[0][0]; i++){
        for(j=0; j<adj[i]; j++){
            int u = old2New[combinedVertices[i]];
            int v = old2New[combinedVertices[graph[i][j]]];
            if(u < v && !areAdjacent(resultGraph, resultAdj, u, v)){
                addEdge(resultGraph, resultAdj, u, v);
            }
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s reads a graph in multicode format and identifies\n", name);
    fprintf(stderr, "the specified vertices.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] u1,v1 u2,v2 u3,v3 ...\n\n", name);
    fprintf(stderr, "This identifies vertex u1 with vertex v1, vertex u2 with vertex v2,...\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] u1,v1 u2,v2 u3,v3 ...\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH graph;
    ADJACENCY adj;
    
    GRAPH resultGraph;
    ADJACENCY resultAdj;

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
    
    int identificationCount = argc - optind;
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
    } else {
        fprintf(stderr, "Error! Could not read graph.\n");
        return (EXIT_FAILURE);
    }
    
    //set up union-find data structure
    int combinedVertices[MAXN+1];
    int depth[MAXN+1];
    for (i = 1; i <= graph[0][0]; i++){
        combinedVertices[i] = i;
        depth[i] = 0;
    }
    
    
    //read identifications
    for (i = 0; i < identificationCount; i++){
        int u;
        int v;
        if(sscanf(argv[optind + i], "%d,%d", &u, &v)!=2){
            fprintf(stderr, "Error while reading vertices to be identified.\n");
            usage(name);
            return EXIT_FAILURE;
        }
        makeUnion(combinedVertices, depth, u, v);
    }
    
    doPathCompress(combinedVertices, 1, graph[0][0]);

    //construct translation table between old and new vertex labels
    int old2New[MAXN+1];
    int new2Old[MAXN+1];
    
    for(i = 1; i <= graph[0][0]; i++){
        old2New[i] = 0;
        new2Old[i] = 0;
    }
    
    int newCounter = 0;
    for(i = 1; i <= graph[0][0]; i++){
        int parent = combinedVertices[i];
        if(!old2New[parent]){
            newCounter++;
            new2Old[newCounter] = parent;
            old2New[parent] = newCounter;
        }
    }
    
    //construct the new graph
    makeIdentifications(graph, adj, resultGraph, resultAdj, newCounter, old2New, new2Old, combinedVertices);

    //write the new graph
    writeMultiCode(resultGraph, resultAdj, stdout);

    return (EXIT_SUCCESS);
}

