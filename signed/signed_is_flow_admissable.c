/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format and computes whether
 * they are flow-admissable.
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_is_flow_admissable -O4  signed_is_flow_admissable.c \
 *           shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

#define MAX_EDGE_COUNT 64
//higher numbers would be possible by using arrays as sets

unsigned long long int fullSets[MAX_EDGE_COUNT+1] = 
{
    0ULL,
    (1ULL <<  1) - 1, (1ULL <<  2) - 1, (1ULL <<  3) - 1, (1ULL <<  4) - 1,
    (1ULL <<  5) - 1, (1ULL <<  6) - 1, (1ULL <<  7) - 1, (1ULL <<  8) - 1,
    (1ULL <<  9) - 1, (1ULL << 10) - 1, (1ULL << 11) - 1, (1ULL << 12) - 1,
    (1ULL << 13) - 1, (1ULL << 14) - 1, (1ULL << 15) - 1, (1ULL << 16) - 1,
    (1ULL << 17) - 1, (1ULL << 18) - 1, (1ULL << 19) - 1, (1ULL << 20) - 1,
    (1ULL << 21) - 1, (1ULL << 22) - 1, (1ULL << 23) - 1, (1ULL << 24) - 1,
    (1ULL << 25) - 1, (1ULL << 26) - 1, (1ULL << 27) - 1, (1ULL << 28) - 1,
    (1ULL << 29) - 1, (1ULL << 30) - 1, (1ULL << 31) - 1, (1ULL << 32) - 1,
    (1ULL << 33) - 1, (1ULL << 34) - 1, (1ULL << 35) - 1, (1ULL << 36) - 1,
    (1ULL << 37) - 1, (1ULL << 38) - 1, (1ULL << 39) - 1, (1ULL << 40) - 1,
    (1ULL << 41) - 1, (1ULL << 42) - 1, (1ULL << 43) - 1, (1ULL << 44) - 1,
    (1ULL << 45) - 1, (1ULL << 46) - 1, (1ULL << 47) - 1, (1ULL << 48) - 1,
    (1ULL << 49) - 1, (1ULL << 50) - 1, (1ULL << 51) - 1, (1ULL << 52) - 1,
    (1ULL << 53) - 1, (1ULL << 54) - 1, (1ULL << 55) - 1, (1ULL << 56) - 1,
    (1ULL << 57) - 1, (1ULL << 58) - 1, (1ULL << 59) - 1, (1ULL << 60) - 1,
    (1ULL << 61) - 1, (1ULL << 62) - 1, (1ULL << 63) - 1, 18446744073709551615ULL
};

int edgeCounter = 0;

void handleSimpleCycle(unsigned long long int *possibleEquivalentOneSets,
        unsigned long long int *edgesInCycle, int *negativeEdgesInCycle){
    if((*negativeEdgesInCycle)%2==0){
        //cycle is balanced
        //remove all edges in cycle from set of possible equivalent signatures
        *possibleEquivalentOneSets &= ~(*edgesInCycle);
    }
}

void checkSimpleCycles_impl(GRAPH graph, ADJACENCY adj, int firstVertex, int secondVertex,
        int currentVertex, unsigned long long int *possibleEquivalentOneSets,
        unsigned long long int *verticesInCycle, unsigned long long int *edgesInCycle, int *negativeEdgesInCycle){
    int i;
    for(i=0; i<adj[currentVertex]; i++){
        EDGE *e = graph[currentVertex][i];
        int neighbour = (e->smallest == currentVertex) ? e->largest : e->smallest;
        if((1ULL << e->index) & (*edgesInCycle)){
            //edge already in cycle
            continue;
        } else if((neighbour != firstVertex) && ((1ULL << neighbour) & (*verticesInCycle))){
            //vertex already in cycle (and not first vertex)
            continue;
        } else if(neighbour < firstVertex){
            //cycle not in canonical form
            continue;
        } else if(neighbour == firstVertex){
            //we have returned to the first vertex
            if(currentVertex < secondVertex){
                //cycle not in canonical form
                continue;
            }
            *edgesInCycle |= (1ULL<<e->index); //add edge
            if(e->isNegative) (*negativeEdgesInCycle)++;
            handleSimpleCycle(possibleEquivalentOneSets, edgesInCycle, negativeEdgesInCycle);
            if(e->isNegative) (*negativeEdgesInCycle)--;
            *edgesInCycle ^= (1ULL<<e->index); //remove edge (we know that it is in the set, so we can just toggle it)
        } else {
            //we continue the cycle
            *verticesInCycle |= (1ULL<<neighbour); //add vertex
            *edgesInCycle |= (1ULL<<e->index); //add edge
            if(e->isNegative) (*negativeEdgesInCycle)++;
            checkSimpleCycles_impl(graph, adj, firstVertex, secondVertex, neighbour,
                    possibleEquivalentOneSets, verticesInCycle, edgesInCycle, negativeEdgesInCycle);
            if(e->isNegative) (*negativeEdgesInCycle)--;
            *edgesInCycle ^= (1ULL<<e->index); //remove edge (we know that it is in the set, so we can just toggle it)
            *verticesInCycle ^= (1ULL<<neighbour); //remove vertex (we know that it is in the set, so we can just toggle it)
        }
        if(!(*possibleEquivalentOneSets)){
            //there are no possible equivalent one sets left
            return;
        }
    }
}

/* Checks the simple cycles and returns TRUE if the graph is flow-admissable.
 */
boolean isFlowAdmissable(GRAPH graph, ADJACENCY adj, int order){
    int v,i,negativeEdgeCount;
    for(i=1; i<=order; i++){
        if(adj[i]==1){
            //if there are vertices of degree 1 it is not flow-admissable
            return FALSE;
        }
    }
    for(i=0; i<edgeCounter; i++){
        edges[i].index = i;
        if(edges[i].isNegative) negativeEdgeCount++;
    }
    if(negativeEdgeCount==0){
        return TRUE;
    } else if(negativeEdgeCount==1){
        return FALSE;
    }
    unsigned long long int possibleEquivalentOneSets = fullSets[edgeCounter];
    for(v = 1; v < order; v++){ //intentionally skip v==order!
        for(i=0; i<adj[v]; i++){
            EDGE *e = graph[v][i];
            int neighbour = e->largest;
            if(neighbour == v){
                //cycle not in canonical form: we have seen this cycle already
                continue;
            } else {
                //start a cycle
                unsigned long long int verticesInCycle = (1ULL<<v) & (1ULL<<neighbour); //add vertex
                unsigned long long int edgesInCycle = (1ULL<<(e->index)); //add edge
                int negativeEdgesInCycle = (e->isNegative) ? 1 : 0;
                checkSimpleCycles_impl(graph, adj, v, neighbour, neighbour,
                        &possibleEquivalentOneSets, &verticesInCycle, &edgesInCycle, &negativeEdgesInCycle);
                if(!possibleEquivalentOneSets){
                    //each edge is contained in a balanced cycle
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s determines whether signed graphs in signed_code\n", name);
    fprintf(stderr, "format are flow-admissable.\n\n");
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

    int graphCount = 0;
    int graphsFiltered = 0;

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
        if(edgeCounter > MAX_EDGE_COUNT){
            fprintf(stderr, "Current version only supports graphs with up to %d edges.\n", MAX_EDGE_COUNT);
            fprintf(stderr, "Graph %d has %d edges. Exiting!\n", graphCount, edgeCounter);
            exit(EXIT_FAILURE);
        }
        
        boolean value = isFlowAdmissable(graph, adj, order);
        if(doFiltering){
            if(invert && !value){
                graphsFiltered++;
                writeSignedCode(graph, adj, order, stdout);
            } else if(!invert && value){
                graphsFiltered++;
                writeSignedCode(graph, adj, order, stdout);
            }
        } else {
            if(value){
                fprintf(stdout, "Graph %d is flow-admissable.\n", graphCount);
            } else {
                fprintf(stdout, "Graph %d is not flow-admissable.\n", graphCount);
            }
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    if(doFiltering){
        fprintf(stderr, "Filtered %d graph%s that %s%s flow-admissable.\n", 
                graphsFiltered, graphsFiltered==1 ? "" : "s",
                graphsFiltered==1 ? "is" : "are",
                invert ? " not" : "");
    }

    return (EXIT_SUCCESS);
}

