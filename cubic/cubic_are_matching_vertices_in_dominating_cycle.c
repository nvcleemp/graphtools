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
 *     cc -o cubic_are_matching_vertices_in_dominating_cycle -O4 \
 *        cubic_are_matching_vertices_in_dominating_cycle.c \
 *        shared/cubic_base.c shared/cubic_input.c shared/cubic_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>

#include "shared/cubic_base.h"
#include "shared/cubic_input.h"
#include "shared/cubic_output.h"

#define BIT(i) (((unsigned long long int)1)<<(i-1))

int vertexCount;
GRAPH graph;

int edgeCount;
int edges[3 * MAXN / 2][2];
int adjacencyMatrix[MAXN][MAXN];
unsigned long long int edgeSets[3 * MAXN / 2];

unsigned long long int* matchings;
unsigned long long int* matchedVertices;
int* matchingsFirstEdge;//stores one edge for each matching

int targetMatchingSize = -1;

int matchingCount = 0;

int firstVertexNeighbour1;
int firstVertexNeighbour2;

boolean printVertices = FALSE;

/* Check the current cycle. Returns TRUE if the cycle is dominating.
 */
boolean handleCycle(unsigned long long int currentCycleEdges,
        unsigned long long int currentCycleVertices, int cycleLength) {
    int i;
    //check that cycle is dominating
    for(i = 0; i < edgeCount; i++){
        if(!(edgeSets[i] & currentCycleVertices)){
            return FALSE;
        }
    }
    
    //remove all matchings contained in this cycle
    int removedMatchings = 0;
    i = 0;
    while(i < matchingCount){
        if((matchedVertices[i] & currentCycleVertices) == matchedVertices[i]){
            matchings[i] = matchings[matchingCount-1];
            matchedVertices[i] = matchedVertices[matchingCount-1];
            matchingsFirstEdge[i] = matchingsFirstEdge[matchingCount-1];
            matchingCount--;
            removedMatchings++;
        } else {
            i++;
        }
    }
    
    //normalize return value (instead of returning removedMatchings)
    if(removedMatchings)
        return TRUE;
    else
        return FALSE;
}

/* Try to extend the current partial cycle to a dominating cycle.
 * Returns TRUE if this is possible, and FALSE otherwise.
 */
boolean extendCycle(int newVertex, int newEdge, int firstVertex,
        unsigned long long int currentCycleEdges,
        unsigned long long int currentCycleVertices,
        int cycleLength) {
    int i;

    if (newVertex == firstVertex) {
        return handleCycle(currentCycleEdges | BIT(newEdge), currentCycleVertices, cycleLength);
    }
    if (BIT(newVertex) & currentCycleVertices) {
        //new vertex is already in cycle and is not the first vertex 
        return FALSE;
    }

    currentCycleVertices |= BIT(newVertex);
    currentCycleEdges |= BIT(newEdge);

    //prevent us of continuing with a cycle that can't be closed
    if (newVertex == firstVertexNeighbour1 && (BIT(firstVertexNeighbour2) & currentCycleVertices)) {
        if (extendCycle(firstVertex, adjacencyMatrix[newVertex][firstVertex],
                firstVertex, currentCycleEdges, currentCycleVertices, cycleLength + 1)) {
            return TRUE;
        }
    } else if (newVertex == firstVertexNeighbour2 && (BIT(firstVertexNeighbour1) & currentCycleVertices)) {
        if (extendCycle(firstVertex, adjacencyMatrix[newVertex][firstVertex],
                firstVertex, currentCycleEdges, currentCycleVertices, cycleLength + 1)) {
            return TRUE;
        }
    } else {
        for (i = 0; i < 3; i++) {
            if (extendCycle(graph[newVertex][i],
                    adjacencyMatrix[newVertex][graph[newVertex][i]], firstVertex,
                    currentCycleEdges, currentCycleVertices, cycleLength + 1)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

boolean findCycleThroughMatchingVertices(int matching){
    int nextVertex;
    int firstVertex = edges[matchingsFirstEdge[matching]][0];
    
    
    firstVertexNeighbour1 = graph[firstVertex][1];
    firstVertexNeighbour2 = graph[firstVertex][2];
    nextVertex = graph[firstVertex][0];
    if(extendCycle(nextVertex, adjacencyMatrix[firstVertex][nextVertex], firstVertex, 0L, BIT(firstVertex), 1)){
        return TRUE;
    }
    firstVertexNeighbour1 = graph[firstVertex][0];
    firstVertexNeighbour2 = graph[firstVertex][2];
    nextVertex = graph[firstVertex][1];
    if(extendCycle(nextVertex, adjacencyMatrix[firstVertex][nextVertex], firstVertex, 0L, BIT(firstVertex), 1)){
        return TRUE;
    }
    firstVertexNeighbour1 = graph[firstVertex][0];
    firstVertexNeighbour2 = graph[firstVertex][1];
    nextVertex = graph[firstVertex][2];
    if(extendCycle(nextVertex, adjacencyMatrix[firstVertex][nextVertex], firstVertex, 0L, BIT(firstVertex), 1)){
        return TRUE;
    }
    
    return FALSE;
}

void findCycles(){
    boolean foundCycle = findCycleThroughMatchingVertices(matchingCount-1);
    
    while(foundCycle && matchingCount > 0){
        foundCycle = findCycleThroughMatchingVertices(matchingCount-1);
    }
}

/* Check the current matching. Outputs the graph if this matching is not
 * contained in a dominating cycle.
 */
void handleMatching(unsigned long long int currentlyMatchedVertices,
        unsigned long long int currentMatching) {
    int i;
    matchings[matchingCount] = currentMatching;
    matchedVertices[matchingCount] = currentlyMatchedVertices;
    i = 0;
    while(i < edgeCount && !(currentMatching & BIT(i))) i++;
    matchingsFirstEdge[matchingCount] = i;
    matchingCount++;
}

/* Tries to extend the current matching.
 */
void extendMatching(int nextVertex, unsigned long long int currentlyMatchedVertices,
        unsigned long long int currentMatching, int currentSize) {
    int i, j;

    if (currentSize == targetMatchingSize) {
        handleMatching(currentlyMatchedVertices, currentMatching);
        return;
    }

    for (i = nextVertex; i < vertexCount; i++) {
        if (!(currentlyMatchedVertices & BIT(i))) {
            for (j = 0; j < 3; j++) {
                int neighbour = graph[i][j];
                if (i < neighbour) {
                    if (!(currentlyMatchedVertices & BIT(neighbour))) {
                        int newEdge = adjacencyMatrix[i][neighbour];
                        extendMatching(i + 1,
                                currentlyMatchedVertices | BIT(i) | BIT(neighbour),
                                currentMatching | BIT(newEdge),
                                currentSize + 1);
                    }
                }
            }
        }
    }
}

void constructMatchings() {
    extendMatching(0, 0L, 0L, 0);
}

int getMaximumNumberOfMatchings() {
    int count, i;
    count = edgeCount - targetMatchingSize + 1;
    for (i = 1; i < targetMatchingSize; i++) {
        count *= edgeCount - targetMatchingSize + i + 1;
        count /= i + 1;
    }
    return count;
}

void preprocessGraph() {
    int i, j;
    //number the edges in graph
    //construct adjacency matrix: each entry contains the number of the incident edge
    memset(adjacencyMatrix, 0, sizeof (adjacencyMatrix[0][0]) * MAXN * MAXN);
    edgeCount = 0;
    for (i = 0; i < vertexCount; i++) {
        for (j = 0; j < 3; j++) {
            if (i < graph[i][j]) {
                //fprintf(stderr, "%d: %d - %d\n", edgeCount, i, graph[i][j]);
                edges[edgeCount][0] = i;
                edges[edgeCount][1] = graph[i][j];
                edgeSets[edgeCount] = BIT(i) | BIT(graph[i][j]);
                adjacencyMatrix[i][graph[i][j]] = edgeCount;
                adjacencyMatrix[graph[i][j]][i] = edgeCount;
                edgeCount++;
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
    fprintf(stderr, "    -p, --print\n");
    fprintf(stderr, "       Print the vertices in the first matching that can't be extended to a\n       dominating cycle.\n");
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

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"print", no_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hp", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case 'p':
                printVertices = TRUE;
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
        
    int graphsFiltered = 0;
    int graphsRead = 0;

    unsigned short code[MAXCODELENGTH];
    int length;
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;

        preprocessGraph();
        int count = getMaximumNumberOfMatchings();
        matchings = malloc(sizeof (unsigned long long int)*count);
        matchedVertices = malloc(sizeof (unsigned long long int)*count);
        matchingsFirstEdge = malloc(sizeof (int)*count);
        matchingCount = 0;
        constructMatchings();

        findCycles();
        
        if(matchingCount > 0){
            if(printVertices){
                for(i=0; i<vertexCount; i++){
                    if(BIT(i) & matchedVertices[matchingCount-1]){
                        fprintf(stderr, "%d ", i);
                    }
                }
                fprintf(stderr, "\n");
            }
            //graph contains matching that is not contained in dominating cycle
            writeCubicMultiCode(graph, vertexCount, stdout);
            graphsFiltered++;
        }

        free(matchings);
        free(matchedVertices);
        free(matchingsFirstEdge);
    }

    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead == 1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered == 1 ? "" : "s");

    return (EXIT_SUCCESS);
}

