/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads cubic graphs in multicode format from standard in and 
 * writes those that have a matching of the specified size and a set of vertices
 * of the specified size which are not contained in a dominating cycle to 
 * standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o cubic_is_matching_and_vertices_in_dominating_cycle -O4\
 *        cubic_is_matching_and_vertices_in_dominating_cycle.c \
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

#define BIT(i) ((1ULL)<<(i))

int vertexCount;
GRAPH graph;

int edgeCount;
int edges[3 * MAXN / 2][2];
int adjacencyMatrix[MAXN][MAXN];
unsigned long long int edgeSets[3 * MAXN / 2];

unsigned long long int* vertexSets;
unsigned long long int* matchings;
unsigned long long int* matchedVertices;
int* matchingsFirstEdge;//stores one edge for each matching

int targetMatchingSize = -1;
int targetVertexSetSize = -1;

int matchingAndVertexSetCount = 0;

int firstVertexNeighbour1;
int firstVertexNeighbour2;

boolean printMatching = FALSE;

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
    while(i < matchingAndVertexSetCount){
        if(((matchings[i] & currentCycleEdges) == matchings[i]) &&
                ((vertexSets[i] & currentCycleVertices) == vertexSets[i])){
            vertexSets[i] = vertexSets[matchingAndVertexSetCount-1];
            matchings[i] = matchings[matchingAndVertexSetCount-1];
            matchedVertices[i] = matchedVertices[matchingAndVertexSetCount-1];
            matchingsFirstEdge[i] = matchingsFirstEdge[matchingAndVertexSetCount-1];
            matchingAndVertexSetCount--;
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

boolean extendCycle(int newVertex, int newEdge, int firstVertex,
        unsigned long long int currentCycleEdges, unsigned long long int currentCycleVertices,
        int cycleLength);
boolean extendCycleAlongMatchingEdge(int newVertex, int newEdge, int firstVertex,
        unsigned long long int currentCycleEdges, unsigned long long int currentCycleVertices,
        int cycleLength);

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
    } else if (BIT(newVertex) & matchedVertices[matchingAndVertexSetCount-1]) {
        for(i = 0; i < edgeCount; i++){
            if ((matchings[matchingAndVertexSetCount-1] & BIT(i))){
                if (edges[i][0] == newVertex){
                    if (extendCycleAlongMatchingEdge(edges[i][1], i, firstVertex,
                            currentCycleEdges, currentCycleVertices, cycleLength + 1)) {
                        return TRUE;
                    }
                } else if (edges[i][1] == newVertex) {
                    if (extendCycleAlongMatchingEdge(edges[i][0], i, firstVertex,
                            currentCycleEdges, currentCycleVertices, cycleLength + 1)) {
                        return TRUE;
                    }
                }
            }
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

/* Try to extend the current partial cycle to a dominating cycle.
 * Returns TRUE if this is possible, and FALSE otherwise.
 */
boolean extendCycleAlongMatchingEdge(
        int newVertex, int newEdge, int firstVertex,
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

boolean findCycleThroughMatching(int matching){    
    int firstVertex = edges[matchingsFirstEdge[matching]][0];
    
    if(graph[firstVertex][0] == edges[matchingsFirstEdge[matching]][1]){
        firstVertexNeighbour1 = graph[firstVertex][1];
        firstVertexNeighbour2 = graph[firstVertex][2];
    } else if(graph[firstVertex][1] == edges[matchingsFirstEdge[matching]][1]){
        firstVertexNeighbour1 = graph[firstVertex][0];
        firstVertexNeighbour2 = graph[firstVertex][2];
    } else {
        firstVertexNeighbour1 = graph[firstVertex][0];
        firstVertexNeighbour2 = graph[firstVertex][1];
    }
    
    return extendCycleAlongMatchingEdge(edges[matchingsFirstEdge[matching]][1],
            matchingsFirstEdge[matching], firstVertex, 0ULL, BIT(firstVertex), 1);
}

void findCycles(){
    boolean foundCycle = findCycleThroughMatching(matchingAndVertexSetCount-1);
    
    while(foundCycle && matchingAndVertexSetCount > 0){
        foundCycle = findCycleThroughMatching(matchingAndVertexSetCount-1);
    }
}

/* Store the matching and the vertex set.
 */
void handleVertexSet(unsigned long long int currentVertexSet,
        unsigned long long int currentlyMatchedVertices,
        unsigned long long int currentMatching) {
    int i;
    vertexSets[matchingAndVertexSetCount] = currentVertexSet;
    matchings[matchingAndVertexSetCount] = currentMatching;
    matchedVertices[matchingAndVertexSetCount] = currentlyMatchedVertices;
    i = 0;
    while(i < edgeCount && !(currentMatching & BIT(i))) i++;
    matchingsFirstEdge[matchingAndVertexSetCount] = i;
    matchingAndVertexSetCount++;
}

/* Tries to extend the current vertex set.
 */
void extendVertexSet(int nextVertex,
        unsigned long long int currentVertexSet,
        int currentSize,
        unsigned long long int currentlyMatchedVertices,
        unsigned long long int currentMatching) {
    int i;

    if (currentSize == targetVertexSetSize) {
        handleVertexSet(currentVertexSet, currentlyMatchedVertices, currentMatching);
        return;
    }

    for (i = nextVertex; i < vertexCount; i++) {
        if (!(currentlyMatchedVertices & BIT(i))) {
            extendVertexSet(i + 1,
                    currentVertexSet | BIT(i),
                    currentSize + 1,
                    currentlyMatchedVertices,
                    currentMatching);
        }
    }
}

/* Handle a completed matching: generate the accompanying vertex sets
 */
void handleMatching(unsigned long long int currentlyMatchedVertices,
        unsigned long long int currentMatching) {
    //start generating all vertex sets that can accompany this matching
    extendVertexSet(0, 0ULL, 0, currentlyMatchedVertices, currentMatching);
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
    extendMatching(0, 0ULL, 0ULL, 0);
    
/*    matchingCount = 1;
    matchedVertices[0] = BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17);
    matchings[0] = BIT(21) | BIT(24) | BIT(26);
    matchingsFirstEdge[0] = 26;
 */
}

int getMaximumNumberOfMatchingsAndVertices() {
    int matchingCount, vertexSetCount, i;
    matchingCount = edgeCount - targetMatchingSize + 1;
    for (i = 1; i < targetMatchingSize; i++) {
        matchingCount *= edgeCount - targetMatchingSize + i + 1;
        matchingCount /= i + 1;
    }
    vertexSetCount = (vertexCount - 2*targetMatchingSize) - targetVertexSetSize + 1;
    for (i = 1; i < targetVertexSetSize; i++) {
        vertexSetCount *= (vertexCount - 2*targetMatchingSize)
                - targetVertexSetSize + i + 1;
        vertexSetCount /= i + 1;
    }
    return matchingCount * vertexSetCount;
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

    if (argc - optind != 2) {
        usage(name);
        return EXIT_FAILURE;
    }

    targetMatchingSize = atoi(argv[optind]);
    targetVertexSetSize = atoi(argv[optind + 1]);
        
    int graphsFiltered = 0;
    int graphsRead = 0;

    unsigned short code[MAXCODELENGTH];
    int length;
    while (readCubicMultiCode(code, &length, stdin)) {
        decodeCubicMultiCode(code, length, graph, &vertexCount);
        graphsRead++;

        preprocessGraph();
        int count = getMaximumNumberOfMatchingsAndVertices();
        vertexSets = malloc(sizeof (unsigned long long int)*count);
        matchings = malloc(sizeof (unsigned long long int)*count);
        matchedVertices = malloc(sizeof (unsigned long long int)*count);
        matchingsFirstEdge = malloc(sizeof (int)*count);
        matchingAndVertexSetCount = 0;
        constructMatchings();
        if(matchingAndVertexSetCount > count){
            fprintf(stderr, "Too many matching and vertex sets -- exiting!\n");
            exit(EXIT_FAILURE);
        }

        if(matchingAndVertexSetCount > 0){
            findCycles();
        }
        
        if(matchingAndVertexSetCount > 0){
            //graph contains matching that is not contained in dominating cycle
            writeCubicMultiCode(graph, vertexCount, stdout);
            graphsFiltered++;
        }
        
        if(printMatching && matchingAndVertexSetCount){
            fprintf(stderr, "Graph %d\nVertices: ", graphsFiltered);
            for(i = 0; i < vertexCount; i++){
                if(vertexSets[matchingAndVertexSetCount-1] & BIT(i)){
                    fprintf(stderr, "%d ", i+1);
                }
            }
            fprintf(stderr, "\nEdges: ");
            for(i = 0; i < edgeCount; i++){
                if(matchings[matchingAndVertexSetCount-1] & BIT(i)){
                    fprintf(stderr, "%d-%d ", edges[i][0] + 1, edges[i][1] + 1);
                }
            }
            fprintf(stderr, "\n");
        }

        free(vertexSets);
        free(matchings);
        free(matchedVertices);
        free(matchingsFirstEdge);
    }

    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead == 1 ? "" : "s");
    fprintf(stderr, "Filtered %d graph%s.\n", graphsFiltered, graphsFiltered == 1 ? "" : "s");

    return (EXIT_SUCCESS);
}

