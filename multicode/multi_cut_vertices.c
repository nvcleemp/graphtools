/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * prints a list of all cut vertices.
 * 
 * Compile with:
 *     
 *     cc -o multi_cut_vertices -O4  multi_cut_vertices.c shared/multicode_base.c \
 *     shared/multicode_input.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"

boolean humanReadable = TRUE;

void findCutVertices_impl(GRAPH graph, ADJACENCY adj, int vertex, boolean visited[],
        int dfsLabels[], int *dfsCounter, int parent[], int lowestReachable[], boolean cutVertices[]){
    int i;
    
    visited[vertex] = TRUE;
    lowestReachable[vertex] = dfsLabels[vertex] = (*dfsCounter)++;
    
    int children = 0;
    for(i = 0; i < adj[vertex]; i++){
        int neighbour = graph[vertex][i];
        if(visited[neighbour]){
            if(parent[vertex] != neighbour){
                //we have found a back edge
                lowestReachable[vertex] = 
                        lowestReachable[vertex] < dfsLabels[neighbour] ?
                            lowestReachable[vertex] :
                            dfsLabels[neighbour];
            }
        } else {
            //this is a forward edge
            children++;
            parent[neighbour] = vertex;
            findCutVertices_impl(graph, adj, neighbour, visited, dfsLabels, 
                    dfsCounter, parent, lowestReachable, cutVertices);
            if(parent[vertex] && lowestReachable[neighbour] >= dfsLabels[vertex]){
                cutVertices[vertex] = TRUE;
            }
            lowestReachable[vertex] = 
                        lowestReachable[vertex] < lowestReachable[neighbour] ?
                            lowestReachable[vertex] :
                            lowestReachable[neighbour];
        }
    }
    if(!parent[vertex] && children > 1){
        //a root is a cut vertex if it has multiple children
        cutVertices[vertex] = TRUE;
    }
}

void findCutVertices(GRAPH graph, ADJACENCY adj){
    int i;
    boolean visited[MAXN+1];
    boolean cutVertices[MAXN+1];
    int dfsLabels[MAXN+1];
    int lowestReachable[MAXN+1];
    int parent[MAXN+1];
    
    for(i = 1; i <= MAXN; i++){
        visited[i] = FALSE;
        cutVertices[i] = FALSE;
    }
    
    int counter = 1;
    
    for(i = 1; i <= graph[0][0]; i++){
        if(!visited[i]){
            parent[i] = 0; //i.e., i is a root
            findCutVertices_impl(graph, adj, i, visited, dfsLabels, &counter,
                    parent, lowestReachable, cutVertices);
        }
    }
    
    if(humanReadable){
        fprintf(stderr, "cut vertices: ");
        for(i = 1; i <= graph[0][0]; i++){
            if(cutVertices[i]){
                fprintf(stderr, "%d ", i);
            }
        }
        fprintf(stderr, "\n");
    } else {
        for(i = 1; i <= graph[0][0]; i++){
            if(cutVertices[i]){
                fprintf(stdout, "%d ", i);
            }
        }
        fprintf(stdout, "0\n");
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s prints out the cut vertices of the given graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -m, --machine\n");
    fprintf(stderr, "       Output the results in a machine-friendly format. All the cut vertices\n");
    fprintf(stderr, "       are output on one line separated by a space and closed by a zero.\n");
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
        {"machine", no_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hm", long_options, &option_index)) != -1) {
        switch (c) {
            case 'm':
                humanReadable = FALSE;
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
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        findCutVertices(graph, adj);
    }

    return (EXIT_SUCCESS);
}

