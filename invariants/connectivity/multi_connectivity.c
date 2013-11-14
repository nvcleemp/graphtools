/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * 
 */

#include <string.h>

#include "../../multicode/shared/multicode_base.h"
    
//try to find a path from the current vertex to the target
boolean findPath(GRAPH graph, ADJACENCY adj, int currentVertex, int target, int *paths, int order, boolean* currentPath, int *capacity) {
    int i;
    if(currentVertex==target)
        return TRUE;
    currentPath[currentVertex]=TRUE;
    for(i=0; i < adj[currentVertex]; i++){
        int nextVertex = graph[currentVertex][i];
        if(paths[currentVertex*(order+1) + nextVertex]-paths[nextVertex*(order+1) + currentVertex]<=capacity[currentVertex*(order+1) + nextVertex]-1 && !currentPath[nextVertex]){
            paths[currentVertex*(order+1) + nextVertex]++;
            if(findPath(graph, adj, nextVertex, target, paths, order, currentPath, capacity))
                return TRUE;
            else
                paths[currentVertex*(order+1) + nextVertex]--;
        }
    }
    currentPath[currentVertex]=FALSE;
    return FALSE;
}

//returns the minimum of the maxflow of the st-network and maxValue
int findMaxFlowInSTNetwork(GRAPH graph, ADJACENCY adj, int source, int target, int maxValue){
    int i, j;

    int order = graph[0][0];
    int paths[(order+1)*(order+1)];
    int capacity[(order+1)*(order+1)];
    for(i=0; i<(order+1)*(order+1); i++){
        paths[i] = 0;
        capacity[i] = 0;
    }
    for(i = 1; i <= order; i++){
        for(j = 0; j < adj[i]; j++){
            capacity[i*(order+1) + graph[i][j]]++;
        }
    }
    
    int pathCount = 0;
    boolean currentPath[order+1];
    for(i=0; i<(order+1); i++){
        currentPath[i] = FALSE;
    }
    while(findPath(graph, adj, source, target, paths, order, currentPath, capacity) && pathCount < maxValue){
        pathCount++;
        for(i=0; i<(order+1); i++){
            currentPath[i] = FALSE;
        }
    }
    return pathCount;
}

int findEdgeConnectivity(GRAPH graph, ADJACENCY adj){
    int i;
    int minDegree, vertexMinDegree, minimumCutSize;
    
    minDegree = adj[1];
    vertexMinDegree = 1;
    
    for (i = 2; i <= graph[0][0]; i++){
        if(adj[i]<minDegree){
            minDegree = adj[i];
            vertexMinDegree = i;
        }
    }
    
    minimumCutSize = minDegree;
    for (i = 1; i <= graph[0][0]; i++){
        if(i!=vertexMinDegree){
            minimumCutSize = findMaxFlowInSTNetwork(graph, adj, vertexMinDegree, i, minimumCutSize);
        }
        //TODO: add optimalisations
    }
    
    return minimumCutSize;
}