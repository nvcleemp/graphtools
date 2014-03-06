/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format and computes whether
 * they are flow-admissable. This uses the spanning tree algorithm which
 * has complexity O(en).
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_is_flow_admissable_ST -O4  signed_is_flow_admissable_ST.c \
 *           shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

//MAXN = 63
#define MAX_EDGE_COUNT 64
//higher numbers would be possible by using arrays as sets

int edgeCounter = 0;

//==============================================================
// data structures and functions for the spanning tree
//-----------------------------------------------------

typedef struct STnode {
    int vertex;
    EDGE *edgeToParent;
    struct STnode *nextSibling;
    struct STnode *firstChild;
} SPANNINGTREENODE;

SPANNINGTREENODE spanningTree[MAXN];
int usedSpanningTreeNodes = 0;

SPANNINGTREENODE *getNewSpanningTreeNode(int vertex, EDGE *edgeToParent){
    if(usedSpanningTreeNodes==MAXN){
        fprintf(stderr, "Spanning tree too large -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    spanningTree[usedSpanningTreeNodes].vertex = vertex;
    spanningTree[usedSpanningTreeNodes].edgeToParent = edgeToParent;
    spanningTree[usedSpanningTreeNodes].nextSibling = NULL;
    spanningTree[usedSpanningTreeNodes].firstChild = NULL;
    usedSpanningTreeNodes++;
    return spanningTree + usedSpanningTreeNodes - 1;
}

SPANNINGTREENODE *constructSpanningTree(GRAPH graph, ADJACENCY adj, int order, unsigned long long int *edgesInTree){
    SPANNINGTREENODE *queue[MAXN];
    usedSpanningTreeNodes = 0;

    SPANNINGTREENODE *root = getNewSpanningTreeNode(1, NULL);
    unsigned long long int visitedVertices = (1ULL << 1);
    *edgesInTree = 0ULL;
    
    int currentVertex, i;
    
    //store the position of each edge in the index field
    for(i = 0; i < edgeCounter; i++){
        edges[i].index = i;
    }
    
    int head = 0;
    int tail = 1;
    queue[head] = root;
    
    while(head < tail){
        currentVertex = queue[head]->vertex;
        SPANNINGTREENODE *previousSibling = NULL;
        for(i = 0; i < adj[currentVertex]; i++){
            int neighbour = graph[currentVertex][i]->smallest == currentVertex ? 
                graph[currentVertex][i]->largest :
                graph[currentVertex][i]->smallest;
            if(!((1ULL << neighbour) & visitedVertices)){
                visitedVertices |= (1ULL << neighbour);
                queue[tail] = getNewSpanningTreeNode(neighbour, graph[currentVertex][i]);
                *edgesInTree |= 1ULL << graph[currentVertex][i]->index;
                if(previousSibling!=NULL){
                    previousSibling->nextSibling = queue[tail];
                } else {
                    //this is the first child
                    queue[head]->firstChild = queue[tail];
                }
                previousSibling = queue[tail];
                tail++;
            }
        }
        head++;
    }
    
    return root;
}

//perform a DFS run through the spanning tree and switch the edges to positive
void setSpanningTreePositive(GRAPH graph, ADJACENCY adj, int order, SPANNINGTREENODE *root){
    SPANNINGTREENODE *child = root->firstChild;
    while(child!=NULL){
        if(child->edgeToParent->isNegative){
            switchAtVertex(graph, adj, child->vertex);
        }
        setSpanningTreePositive(graph, adj, order, child);
        child = child->nextSibling;
    }
}

unsigned long long int translateSignatureToBitvector(){
    int i;
    unsigned long long int signature = 0ULL;
    
    for(i = 0; i < edgeCounter; i++){
        if(edges[i].isNegative){
            signature |= (1ULL << i);
        }
    }
    
    return signature;
}

void loadSignature(unsigned long long int signature){
    int i;
    
    for(i = 0; i < edgeCounter; i++){
        edges[i].isNegative = signature & (1ULL << i);
    }
}

/* Checks the simple cycles and returns TRUE if the graph is flow-admissable.
 */
boolean isFlowAdmissable(GRAPH graph, ADJACENCY adj, int order){
    int i,negativeEdgeCount;
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
    if(negativeEdgeCount==1){
        return FALSE;
    }
    
    unsigned long long int edgesInTree = 0ULL;
    SPANNINGTREENODE *spanningTreeRoot = constructSpanningTree(graph, adj, order, &edgesInTree);
    
    setSpanningTreePositive(graph, adj, order, spanningTreeRoot);
    unsigned long long int canonicalForm = translateSignatureToBitvector();
    
    if(canonicalForm && !(canonicalForm & (canonicalForm - 1))){
        //canonical form contains only one edge
        return FALSE;
    }
    
    for(i = 0; i < edgeCounter; i++){
        if(edgesInTree & (1ULL << i)){
            setAllPositive();
            edges[i].isNegative = TRUE;
            setSpanningTreePositive(graph, adj, order, spanningTreeRoot);
            if(canonicalForm == translateSignatureToBitvector()){
                return FALSE;
            }
        }
    }
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s determines whether signed graphs in signed_code\n", name);
    fprintf(stderr, "format are flow-admissable. This version uses the spanning-tree algorithm.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
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
        
        unsigned long long int signature = translateSignatureToBitvector();
        boolean value = isFlowAdmissable(graph, adj, order);
        loadSignature(signature);
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

