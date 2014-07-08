/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * writes the components of those graphs to standard out in multicode format.
 * 
 * Compile with:
 *     
 *     cc -o multi_component -O4  multi_component.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

//some macros for the stack in the next method
#define PUSH(stack, value) stack[top++] = (value)
#define POP(stack) stack[--top]
#define STACKISEMPTY top==0
#define STACKISNOTEMPTY top>0

void detectComponent(GRAPH graph, ADJACENCY adj, int vertex, boolean visited[]){
    int i, j, top, currentVertex;
    boolean currentComponent[MAXN+1];
    int stack[MAXN];
    int newLabels[MAXN+1];
    
    for(i = 1; i <= MAXN; i++){
        currentComponent[i] = FALSE;
    }
    
    top = 0;
    
    PUSH(stack, vertex);
    visited[vertex] = currentComponent[vertex] = TRUE;
    
    while(STACKISNOTEMPTY){
        currentVertex = POP(stack);
        for(i = 0; i < adj[currentVertex]; i++){
            int neighbour = graph[currentVertex][i];
            if(!visited[neighbour]){
                PUSH(stack, neighbour);
                visited[neighbour] = currentComponent[neighbour] = TRUE;
            }
        }
    }
    
    currentVertex = 0;
    
    for(i = 1; i <= graph[0][0]; i++){
        if(currentComponent[i]){
            currentVertex++;
            newLabels[i] = currentVertex;
        }
    }
    
    GRAPH componentGraph;
    ADJACENCY componentAdj;
    
    prepareGraph(componentGraph, componentAdj, currentVertex);
    
    for(i = 1; i <= graph[0][0]; i++){
        if(currentComponent[i]){
            for(j = 0; j < adj[i]; j++){
                if(graph[i][j] > i){
                    addEdge(componentGraph, componentAdj, 
                            newLabels[i], newLabels[graph[i][j]]);
                }
            }
        }
    }
    
    writeMultiCode(componentGraph, componentAdj, stdout);
}

void writeComponents(GRAPH graph, ADJACENCY adj){
    int i;
    boolean visited[MAXN+1];
    
    for(i = 1; i <= MAXN; i++){
        visited[i] = FALSE;
    }
    
    for(i = 1; i <= graph[0][0]; i++){
        if(!visited[i]){
            detectComponent(graph, adj, i, visited);
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s writes the components of graphs in multicode format.\n\n", name);
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
        
        writeComponents(graph, adj);
    }

    return (EXIT_SUCCESS);
}

