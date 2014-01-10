/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes those that have a matching of the specified size which is not contained
 * in a dominating cycle to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_is_matching_in_dominating_cycle -O4  cubic_is_matching_in_dominating_cycle.c \
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

int graphsFiltered = 0;

int targetMatchingSize = -1;

int matchingSize = 0;
int matchingEdges[MAXN][2];
boolean matchedVertices[MAXN];

int matchingCount = 0;

boolean inCurrentCycle[MAXN];
int currentCycle[MAXN];
int cycleLength;

int firstVertexNeighbour1;
int firstVertexNeighbour2;

boolean printMatching = FALSE;

/* Check the current cycle. Returns TRUE if the cycle is dominating.
 */
boolean handleCycle(){
    int i, j;
    //check that each matched vertex is in the cycle.
    //since we use the matching edge as soon as we visit a matched vertex
    //this is sufficient for the cycle to go through the matching
    for(i = 0; i < vertexCount; i++){
        if(matchedVertices[i] && !inCurrentCycle[i]){
            return FALSE;
        }
    }
    
    for(i = 0; i < vertexCount; i++){
        if(!inCurrentCycle[i]){
            for(j = 0; j < 3; j++){
                if(!inCurrentCycle[graph[i][j]]){
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

boolean extendCycle(int newVertex, int firstVertex);
boolean extendCycleAlongMatchingEdge(int newVertex, int firstVertex);

/* Try to extend the current partial cycle to a dominating cycle.
 * Returns TRUE if this is possible, and FALSE otherwise.
 */
boolean extendCycle(int newVertex, int firstVertex){
    int i;
    
    if(newVertex == firstVertex){
        return handleCycle();
    }
    if(inCurrentCycle[newVertex]){
        //new vertex is already in cycle and is not the first vertex 
        return FALSE;
    }
    
    currentCycle[cycleLength] = newVertex;
    inCurrentCycle[newVertex] = TRUE;
    cycleLength++;
    
    //prevent us of continuing with a cycle that can't be closed
    if(newVertex == firstVertexNeighbour1 && inCurrentCycle[firstVertexNeighbour2]){
        if(extendCycle(firstVertex, firstVertex)){
            return TRUE;
        }
    } else if(newVertex == firstVertexNeighbour2 && inCurrentCycle[firstVertexNeighbour1]){
        if(extendCycle(firstVertex, firstVertex)){
            return TRUE;
        }
    } else if(matchedVertices[newVertex]){
        for(i = 0; i < matchingSize; i++){
            if(matchingEdges[i][0] == newVertex){
                if(extendCycleAlongMatchingEdge(matchingEdges[i][1], firstVertex)){
                    return TRUE;
                }
            } else if(matchingEdges[i][1] == newVertex){
                if(extendCycleAlongMatchingEdge(matchingEdges[i][0], firstVertex)){
                    return TRUE;
                }
            }
        }
    } else {
        for(i = 0; i < 3; i++){
            if(extendCycle(graph[newVertex][i], firstVertex)){
                return TRUE;
            }
        }
    }
    
    inCurrentCycle[newVertex] = FALSE;
    cycleLength--;
    return FALSE;
}

/* Try to extend the current partial cycle to a dominating cycle.
 * Returns TRUE if this is possible, and FALSE otherwise.
 */
boolean extendCycleAlongMatchingEdge(int newVertex, int firstVertex){
    int i;
    
    if(newVertex == firstVertex){
        return handleCycle();
    }
    if(inCurrentCycle[newVertex]){
        //new vertex is already in cycle and is not the first vertex 
        return FALSE;
    }
    
    currentCycle[cycleLength] = newVertex;
    inCurrentCycle[newVertex] = TRUE;
    cycleLength++;
    
    //prevent us of continuing with a cycle that can't be closed
    if(newVertex == firstVertexNeighbour1 && inCurrentCycle[firstVertexNeighbour2]){
        if(extendCycle(firstVertex, firstVertex)){
            return TRUE;
        }
    } else if(newVertex == firstVertexNeighbour2 && inCurrentCycle[firstVertexNeighbour1]){
        if(extendCycle(firstVertex, firstVertex)){
            return TRUE;
        }
    } else {
        for(i = 0; i < 3; i++){
            if(extendCycle(graph[newVertex][i], firstVertex)){
                return TRUE;
            }
        }
    }
    
    inCurrentCycle[newVertex] = FALSE;
    cycleLength--;
    return FALSE;
}

/* Check the current matching. Outputs the graph if this matching is not
 * contained in a dominating cycle.
 */
void handleMatching(){
    int i;
    matchingCount++;
    
    for(i = 0; i < MAXN; i++){
        inCurrentCycle[i] = FALSE;
    }
    
    currentCycle[0] = matchingEdges[0][0];
    inCurrentCycle[matchingEdges[0][0]] = TRUE;
    
    if(graph[matchingEdges[0][0]][0] == matchingEdges[0][1]){
        firstVertexNeighbour1 = graph[matchingEdges[0][0]][1];
        firstVertexNeighbour2 = graph[matchingEdges[0][0]][2];
    } else if(graph[matchingEdges[0][0]][1] == matchingEdges[0][1]){
        firstVertexNeighbour1 = graph[matchingEdges[0][0]][0];
        firstVertexNeighbour2 = graph[matchingEdges[0][0]][2];
    } else {
        firstVertexNeighbour1 = graph[matchingEdges[0][0]][0];
        firstVertexNeighbour2 = graph[matchingEdges[0][0]][1];
    }
    
    cycleLength = 1;
    
    if(!extendCycleAlongMatchingEdge(matchingEdges[0][1], matchingEdges[0][0])){

        writeCubicMultiCode(graph, vertexCount, stdout);
        graphsFiltered++;
        if(printMatching){
            fprintf(stderr, "In graph %d:\n", graphsFiltered);
            for(i = 0; i < matchingSize; i++){
                fprintf(stderr, "  %d - %d\n", matchingEdges[i][0], matchingEdges[i][1]);
            }
            fprintf(stderr, "\n");
        }
    }
    
}

/* Tries to extend the current matching.
 */
void extendMatching(int nextVertex){
    int i, j;
    
    if(matchingSize==targetMatchingSize){
        handleMatching();
        return;
    }
    
    for(i = nextVertex; i < vertexCount; i++){
        if(!matchedVertices[i]){
            for(j = 0; j < 3; j++){
                int neighbour = graph[i][j];
                if(i < neighbour){
                    if(!matchedVertices[neighbour]){
                        matchedVertices[i] = TRUE;
                        matchedVertices[neighbour] = TRUE;
                        matchingEdges[matchingSize][0] = i;
                        matchingEdges[matchingSize][1] = neighbour;
                        matchingSize++;
                        extendMatching(i+1);
                        matchingSize--;
                        matchedVertices[i] = FALSE;
                        matchedVertices[neighbour] = FALSE;
                    }
                }
            }
        }
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s checks matchings and dominating cycles.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -m, --matching\n");
    fprintf(stderr, "       Print the matchings that can't be extended to a dominating cycle.\n");
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
    
    int i;
    
    int graphsRead = 0;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"matching", no_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hm", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case 'm':
                printMatching = TRUE;
                break;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
        
    if (argc - optind != 1) {
        usage(name);
        return EXIT_FAILURE;
    }
    
    targetMatchingSize = atoi(argv[optind]);
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;
        
        for(i = 0; i < MAXN; i++){
            matchedVertices[i] = FALSE;
        }
        
        extendMatching(0);        
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

