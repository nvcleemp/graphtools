/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */


#include "multicode_base.h"

/* This method adds the edge (v,w) to graph. This assumes that adj contains
 * the current degree of the vertices v and w. This degrees are then updated.
 */
void addEdge(GRAPH graph, ADJACENCY adj, int v, int w) {
    graph[v][adj[v]] = w;
    graph[w][adj[w]] = v;
    adj[v]++;
    adj[w]++;
}

boolean _removeEdge(GRAPH graph, ADJACENCY adj, int v, int w){
    int vi = 0, wi = 0;
    
    //find first position of w in the adjacency list of v
    while(vi < adj[v] && graph[v][vi]!=w){
        vi++;
    }
    if(vi == adj[v]){
        return FALSE;
    }
    
    //find first position of w in the adjacency list of v
    while(wi < adj[w] && graph[w][wi]!=v){
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
    
    //mark all vertices as having degree 0
    for (i = 1; i <= vertexCount; i++) {
        adj[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = EMPTY;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = 0;
    }
    
    graph[0][0] = vertexCount;
}

boolean areAdjacent(GRAPH graph, ADJACENCY adj, int v, int w){
    int i;
    
    for(i = 0; i < adj[v]; i++){
        if(graph[v][i]==w) return TRUE;
    }
    
    return FALSE;
}
