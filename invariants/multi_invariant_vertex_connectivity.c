/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 * 
 * Based on the invariant computer for vertex connectivity from Grinvin.
 */

/*
 * Computes the vertex connectivity of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_vertex_connectivity -O4 -DINVARIANT=vertex_connectivity \
 *     -DINVARIANTNAME="vertex connectivity" multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_vertex_connectivity.c
 */

#include "../multicode/shared/multicode_base.h"

int directedGraph[2*(MAXN) + 1][2*(MAXN) + 1];

boolean currentPath[2*(MAXN)+1];

boolean findPath_impl(GRAPH graph, ADJACENCY adj, int currentVertex, int target) {
    int j;
    int order = graph[0][0];
    
    currentPath[currentVertex]=TRUE;
    if(currentVertex==target){
        return TRUE;
    }

    if(currentVertex <= order){ //in-vertex

        for(j = 0; j < adj[currentVertex]; j++){
            int nextVertex = graph[currentVertex][j] + order;
            if(directedGraph[currentVertex][nextVertex]>0 && !currentPath[nextVertex]){
                directedGraph[currentVertex][nextVertex]--;
                directedGraph[nextVertex][currentVertex]++;
                if(findPath_impl(graph, adj, nextVertex, target)){
                    return TRUE;
                } else {
                    directedGraph[currentVertex][nextVertex]++;
                    directedGraph[nextVertex][currentVertex]--;
                }
            }
        }           
        int nextVertex = currentVertex + order;
        if(directedGraph[currentVertex][nextVertex]>0 && !currentPath[nextVertex]){
            directedGraph[currentVertex][nextVertex]--;
            directedGraph[nextVertex][currentVertex]++;
            if(findPath_impl(graph, adj, nextVertex, target)){
                return TRUE;
            } else {
                directedGraph[currentVertex][nextVertex]++;
                directedGraph[nextVertex][currentVertex]--;
            }
        }     

    } else { //out-vertex

        for(j = 0; j < adj[currentVertex - order]; j++){
            int nextVertex = graph[currentVertex - order][j];
            if(directedGraph[currentVertex][nextVertex]>0 && !currentPath[nextVertex]){
                directedGraph[currentVertex][nextVertex]--;
                directedGraph[nextVertex][currentVertex]++;
                if(findPath_impl(graph, adj, nextVertex, target)){
                    return TRUE;
                } else {
                    directedGraph[currentVertex][nextVertex]++;
                    directedGraph[nextVertex][currentVertex]--;
                }
            }
        }           
        int nextVertex = currentVertex - order;
        if(directedGraph[currentVertex][nextVertex]>0 && !currentPath[nextVertex]){
            directedGraph[currentVertex][nextVertex]--;
            directedGraph[nextVertex][currentVertex]++;
            if(findPath_impl(graph, adj, nextVertex, target)){
                return TRUE;
            } else {
                directedGraph[currentVertex][nextVertex]++;
                directedGraph[nextVertex][currentVertex]--;
            }
        }     

    }

    currentPath[currentVertex]=FALSE;
    return FALSE;
}

//try to find a path from the current vertex to the target
boolean findPath(GRAPH graph, ADJACENCY adj, int currentVertex, int target) {
    int i;
    for(i = 1; i <= 2*(graph[0][0]); i++){
        currentPath[i] = FALSE;
    }
    return findPath_impl(graph, adj, currentVertex, target);
}


//returns the minimum of the maxflow of the st-network and satisfied
int findMaxFlowInSTNetwork(GRAPH graph, ADJACENCY adj, int source, int target, int satisfied){
    int i, j, order = graph[0][0];
    
    //construct directed graph
    for(i = 1; i <= 2*order; i++){
        for(j = 1; j <= 2*order; j++){
            directedGraph[i][j] = 0;
        }
    }
    for(i=1; i<=order; i++){
        for(j = 0; j < adj[i]; j++){
            directedGraph[i + order][graph[i][j]]++;
        }
        directedGraph[i][i + order] = 1;
        
    }
    int pathCount = 0;
    while(findPath(graph, adj, source + order, target) && pathCount < satisfied){
        pathCount++;
    }
    return pathCount;
}

int vertex_connectivity(GRAPH graph, ADJACENCY adj){
    if(graph[0][0] < 2) return 0;
    
    //find minimum degree (i.e., upperbound for vertex connectivity)
    int minDeg = graph[0][0];
    int i, j;
    for(i = 1; i <= graph[0][0]; i++){
        if(adj[i] < minDeg){
            minDeg = adj[i];
        }
    }
    
    int minimumCutSize = minDeg;
    for(i = 1; i <= minimumCutSize + 1; i++){
        for(j = i; j <= graph[0][0]; j++){
            minimumCutSize = findMaxFlowInSTNetwork(graph, adj, i, j, minimumCutSize);
        }
    }
    return minimumCutSize;
}