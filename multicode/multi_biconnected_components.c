/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * outputs the different biconnected components of those graphs.
 * 
 * Compile with:
 *     
 *     cc -o multi_biconnected_components -O4  multi_biconnected_components.c 
 *     shared/multicode_base.c shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

typedef int EDGE[2];

EDGE edgeStack[MAXN*MAXN];
int top;

//some macros for the stack
#define PUSH_EDGE(u,v) edgeStack[top][0] = u; edgeStack[top][1] = v; top++
#define POP_EDGE(u, v) top--; u = edgeStack[top][0]; v = edgeStack[top][1]
#define STACKISEMPTY top==0
#define STACKISNOTEMPTY top>0

void handleBiconnectedComponent(GRAPH graph, ADJACENCY adj, int uEnd, int vEnd){
    int i, u, v;
    GRAPH biconnectedGraph;
    ADJACENCY biconnectedAdj;
    int newLabel[MAXN+1];
    
    for(i = 0; i < MAXN; i++){
        newLabel[i] = 0;
    }
    
    //we will use i to relabel the vertices
    i = 0;
    
    //just prepare the graph for a large enough number of vertices
    prepareGraph(biconnectedGraph, biconnectedAdj, graph[0][0]);
    
    do {
        POP_EDGE(u,v);
        if(!newLabel[u]){
            i++;
            newLabel[u] = i;
        }
        if(!newLabel[v]){
            i++;
            newLabel[v] = i;
        }
        //no extra check needed since each edge is only once in the stack
        addEdge(biconnectedGraph, biconnectedAdj, newLabel[u], newLabel[v]);
    } while(uEnd != u || vEnd != v);
    
    //set the connect number of vertices
    biconnectedGraph[0][0] = i;
    
    writeMultiCode(biconnectedGraph, biconnectedAdj, stdout);
}

void findBiconnectedComponents_impl(GRAPH graph, ADJACENCY adj, int vertex, boolean visited[],
        int dfsLabels[], int *dfsCounter, int parent[], int lowestReachable[]){
    int i;
    
    visited[vertex] = TRUE;
    lowestReachable[vertex] = dfsLabels[vertex] = (*dfsCounter)++;
    
    int children = 0;
    for(i = 0; i < adj[vertex]; i++){
        int neighbour = graph[vertex][i];
        if(visited[neighbour]){
            if(parent[vertex] != neighbour && 
                    (dfsLabels[neighbour] < dfsLabels[vertex])){
                //we have found a back edge
                PUSH_EDGE(vertex, neighbour);
                lowestReachable[vertex] = 
                        lowestReachable[vertex] < dfsLabels[neighbour] ?
                            lowestReachable[vertex] :
                            dfsLabels[neighbour];
            }
        } else {
            //this is a forward edge
            PUSH_EDGE(vertex, neighbour);
            children++;
            parent[neighbour] = vertex;
            findBiconnectedComponents_impl(graph, adj, neighbour, visited, dfsLabels, 
                    dfsCounter, parent, lowestReachable);
            if(lowestReachable[neighbour] >= dfsLabels[vertex]){
                handleBiconnectedComponent(graph, adj, vertex, neighbour);
            }
            lowestReachable[vertex] = 
                        lowestReachable[vertex] < lowestReachable[neighbour] ?
                            lowestReachable[vertex] :
                            lowestReachable[neighbour];
        }
    }
}

void findBiconnectedComponents(GRAPH graph, ADJACENCY adj){
    int i;
    boolean visited[MAXN+1];
    int dfsLabels[MAXN+1];
    int lowestReachable[MAXN+1];
    int parent[MAXN+1];
    
    for(i = 1; i <= MAXN; i++){
        visited[i] = FALSE;
    }
    
    int counter = 1;
    
    for(i = 1; i <= graph[0][0]; i++){
        if(!visited[i]){
            parent[i] = 0; //i.e., i is a root
            findBiconnectedComponents_impl(graph, adj, i, visited, dfsLabels, &counter,
                    parent, lowestReachable);
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s outputs the different biconnected components\n", name);
    fprintf(stderr, "of the input graphs.\n\n");
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
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        findBiconnectedComponents(graph, adj);
    }

    return (EXIT_SUCCESS);
}

