/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include<stdio.h>
#include "multicode_base.h"


GRAPH *newGraph(int maxn, int maxval){
    if(maxn <= 0){
        fprintf(stderr, "maxn should be a positive integer -- exiting!\n");
        return NULL;
    }
    
    if(maxval <= 0){
        fprintf(stderr, "maxval should be a positive integer -- exiting!\n");
        return NULL;
    }
    
    GRAPH *graph = (GRAPH *)malloc(sizeof(GRAPH));
    
    if(graph == NULL){
        fprintf(stderr, "Insufficient memory for graph -- exiting!\n");
        return NULL;
    }
    
    graph->maxn = maxn;
    graph->maxval = maxval;
    
    graph->n = 0;
    
    graph->graph = (unsigned short *)malloc(sizeof(unsigned short)*maxn*maxval);
    
    if(graph->graph == NULL){
        fprintf(stderr, "Insufficient memory for graph -- exiting!\n");
        free(graph);
        return NULL;
    }
    
    graph->degrees = (unsigned short *)malloc(sizeof(unsigned short)*maxn);
    
    if(graph->degrees == NULL){
        fprintf(stderr, "Insufficient memory for degrees -- exiting!\n");
        free(graph->graph);
        free(graph);
        return NULL;
    }
    
    return graph;
}

void freeGraph(GRAPH *graph){
    free(graph->degrees);
    free(graph->graph);
    free(graph);
}

/* This method adds the edge (v,w) to graph.
 */
void addEdge(GRAPH *graph, int v, int w) {
    NEIGHBOUR(graph, v, graph->degrees[v]) = w;
    NEIGHBOUR(graph, w, graph->degrees[w]) = v;
    graph->degrees[v]++;
    graph->degrees[w]++;
    if(graph->degrees[v] > graph->maxval || graph->degrees[v] > graph->maxval){
        fprintf(stderr, "Degree too large for graph -- exiting!\n");
        exit(-1);
    }
}

boolean _removeEdge(GRAPH *graph, int v, int w){
    int vi = 0, wi = 0;
    
    //find first position of w in the adjacency list of v
    while(vi < graph->degrees[v] && NEIGHBOUR(graph, v, vi)!=w){
        vi++;
    }
    if(vi == graph->degrees[v]){
        return FALSE;
    }
    
    //find first position of w in the adjacency list of v
    while(wi < graph->degrees[w] && NEIGHBOUR(graph, w, wi)!=v){
        wi++;
    }
    if(wi == graph->degrees[w]){
        //shouldn't happen!
        return FALSE;
    }
    
    //the edge exists
    graph->degrees[v]--;
    graph->degrees[w]--;
    NEIGHBOUR(graph, v, vi) = NEIGHBOUR(graph, v, graph->degrees[v]);
    NEIGHBOUR(graph, w, wi) = NEIGHBOUR(graph, w, graph->degrees[w]);
    
    return TRUE;
}

/* This method removes an edge (v,w) from graph. This assumes that adj contains
 * the current degree of the vertices v and w. This degrees are then updated.
 * If all is TRUE, then all edges between v and w are removed. For simple graphs
 * the value of all is ignored.
 */
void removeEdge(GRAPH *graph, int v, int w, boolean all){
    boolean edgeRemoved = _removeEdge(graph, v, w);
    while(all && edgeRemoved){
        edgeRemoved = _removeEdge(graph, v, w);
    }
}

void prepareGraph(GRAPH *graph, int vertexCount) {
    int i, j;
    
    if(vertexCount > graph->maxn){
        fprintf(stderr, "Illegal size for graph (%d) -- exiting!\n", vertexCount);
        exit(-1);
    }
    
    //mark all vertices as having degree 0
    for (i = 0; i < vertexCount; i++) {
        graph->degrees[i] = 0;
        for (j = 0; j < graph->maxval; j++) {
            NEIGHBOUR(graph, i, j) = EMPTY;
        }
    }
    
    graph->n = vertexCount;
}

boolean areAdjacent(GRAPH *graph, int v, int w){
    int i;
    
    for(i = 0; i < graph->degrees[v]; i++){
        if(NEIGHBOUR(graph, v, i) == w) return TRUE;
    }
    
    return FALSE;
}
