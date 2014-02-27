/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */


#include "signed_base.h"
#include<stdio.h>

/* This method adds the edge (v,w) to graph. This assumes that adj contains
 * the current degree of the vertices v and w. This degrees are then updated.
 */
void addEdge(GRAPH graph, ADJACENCY adj, int v, int w, boolean isNegative) {
    if(edgeCounter == MAXE){
        fprintf(stderr, "Maximum number of edges reached -- exiting!");
        exit(EXIT_FAILURE);
    }
    if(v < w){
        edges[edgeCounter].smallest = v;
        edges[edgeCounter].largest = w;
    } else {
        edges[edgeCounter].smallest = w;
        edges[edgeCounter].largest = v;
    }
    edges[edgeCounter].isNegative = isNegative;
    graph[v][adj[v]] = edges + edgeCounter;
    graph[w][adj[w]] = edges + edgeCounter;
    adj[v]++;
    adj[w]++;
    edgeCounter++;
}

boolean _removeEdge(GRAPH graph, ADJACENCY adj, int v, int w){
    int vi = 0, wi = 0;
    
    if(v > w){
        int temp = v;
        v = w;
        w = temp;
    }
    //find first position of w in the adjacency list of v
    while(vi < adj[v] && graph[v][vi]->largest!=w){
        vi++;
    }
    if(vi == adj[v]){
        return FALSE;
    }
    
    //find first position of w in the adjacency list of v
    while(wi < adj[w] && graph[w][wi]!=graph[v][vi]){
        wi++;
    }
    if(wi == adj[w]){
        //shouldn't happen!
        return FALSE;
    }
    
    //the edge exists
    adj[v]--;
    adj[w]--;
    graph[v][vi] = graph[v][adj[v]];
    graph[w][wi] = graph[w][adj[w]];
    
    return TRUE;
}

/* This method removes an edge (v,w) from graph. This assumes that adj contains
 * the current degree of the vertices v and w. This degrees are then updated.
 * If all is TRUE, then all edges between v and w are removed. For simple graphs
 * the value of all is ignored.
 */
void removeEdge(GRAPH graph, ADJACENCY adj, int v, int w, boolean all){
    boolean edgeRemoved = _removeEdge(graph, adj, v, w);
    while(all && edgeRemoved){
        edgeRemoved = _removeEdge(graph, adj, v, w);
    }
}

void prepareGraph(GRAPH graph, ADJACENCY adj, int vertexCount) {
    int i, j;
    
    edgeCounter = 0;
    
    //mark all vertices as having degree 0
    for (i = 1; i <= vertexCount; i++) {
        adj[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = NULL;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = NULL;
    }
}

boolean areAdjacent(GRAPH graph, ADJACENCY adj, int v, int w){
    int i;
    
    if(v > w){
        int temp = v;
        v = w;
        w = temp;
    }
    for(i = 0; i < adj[v]; i++){
        if(graph[v][i]->largest==w) return TRUE;
    }
    
    return FALSE;
}

boolean areAdjacentWithSign(GRAPH graph, ADJACENCY adj, int v, int w, int sign){
    int i;
    
    if(v > w){
        int temp = v;
        v = w;
        w = temp;
    }
    for(i = 0; i < adj[v]; i++){
        if(graph[v][i]->largest==w &&
                graph[v][i]->isNegative==(sign==NEGATIVE)){
            return TRUE;
        }
    }
    
    return FALSE;
}


void switchAtVertex(GRAPH graph, ADJACENCY adj, int v){
    int i;
    for(i = 0; i < adj[v]; i++){
        graph[v][i]->isNegative = !(graph[v][i]->isNegative);
    }
}
