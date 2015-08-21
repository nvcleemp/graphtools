/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes whether a graph in multicode format is 2-leaf-stable, i.e., it is
 * traceable but not hamiltonian, and the removal of any vertex leaves a graph 
 * that is traceable but not hamiltonian.
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_is_2_leaf_stable -O4 -DINVARIANT=is2LeafStable \
 *     multi_boolean_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c
 */

#include "../multicode/shared/multicode_base.h"
#include <stdio.h>

boolean currentPath[MAXN+1];

/**
  * 
  */
boolean continuePathIncludingCheckForHamiltonianCycle(GRAPH graph, ADJACENCY adj, int last, int remaining, int startVertex, boolean *foundHamiltonianCycle) {
    int i;
    
    if(remaining==0){
        //we found a hamiltonian path, but we first check whether there is also a hamiltonian cycle
        for(i = 0; i < adj[last]; i++){
            if(graph[last][i] == startVertex){
                *foundHamiltonianCycle = TRUE;
            }
        }
        return TRUE;
    }
    
    boolean traceable = FALSE;
    for(i = 0; i < adj[last]; i++){
        if(!currentPath[graph[last][i]]){
            currentPath[graph[last][i]]=TRUE;
            if(continuePathIncludingCheckForHamiltonianCycle(graph, adj, graph[last][i], remaining - 1, startVertex, foundHamiltonianCycle)){
                if(*foundHamiltonianCycle){
                    return TRUE;
                } else {
                    traceable = TRUE;
                }
            }
            currentPath[graph[last][i]]=FALSE;
        }
    }
    
    return traceable;
}

/**
  * 
  */
boolean continuePath(GRAPH graph, ADJACENCY adj, int last, int remaining) {
    int i;
    
    if(remaining==0){
        return TRUE;
    }
    
    for(i = 0; i < adj[last]; i++){
        if(!currentPath[graph[last][i]]){
            currentPath[graph[last][i]]=TRUE;
            if(continuePath(graph, adj, graph[last][i], remaining - 1)){
                return TRUE;
            }
            currentPath[graph[last][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean isTraceableFromVertexIncludingCheckForHamiltonianCycle_vertexDeleted(GRAPH graph, ADJACENCY adj, int startVertex, boolean *foundHamiltonianCycle, int deletedVertex){
    int i;
    
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    currentPath[deletedVertex] = TRUE;
    boolean traceable = FALSE;
    
    currentPath[startVertex]=TRUE;
    for(i = 0; i < adj[startVertex]; i++){
        if(graph[startVertex][i]!=deletedVertex){
            currentPath[graph[startVertex][i]]=TRUE;
            //search for path containing the edge (startVertex, graph[startVertex][i])
            if(continuePathIncludingCheckForHamiltonianCycle(graph, adj, graph[startVertex][i], graph[0][0] - 3, startVertex, foundHamiltonianCycle)){
                if(*foundHamiltonianCycle){
                    return TRUE;
                } else {
                    traceable = TRUE;
                }
            }
            currentPath[graph[startVertex][i]]=FALSE;
        }
    }
    currentPath[startVertex]=FALSE;
    
    return traceable;
}


boolean isTraceableFromVertex_vertexDeleted(GRAPH graph, ADJACENCY adj, int startVertex, int deletedVertex){
    int i;
    
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    currentPath[deletedVertex] = TRUE;
    
    currentPath[startVertex]=TRUE;
    for(i = 0; i < adj[startVertex]; i++){
        if(graph[startVertex][i]!=deletedVertex){
            currentPath[graph[startVertex][i]]=TRUE;
            //search for path containing the edge (startVertex, graph[startVertex][i])
            if(continuePath(graph, adj, graph[startVertex][i], graph[0][0] - 3)){
                return TRUE;
            }
            currentPath[graph[startVertex][i]]=FALSE;
        }
    }
    currentPath[startVertex]=FALSE;
    
    return FALSE;
}

boolean removingVertexLeavesTraceableNotHamiltonian(GRAPH graph, ADJACENCY adj, int vertexToRemove){
    int i;
    int order = graph[0][0];
    
    int degreeModification[MAXN + 1];
    for(i = 1; i <= MAXN; i++){
        degreeModification[i] = 0;
    }
    for(i = 0; i < adj[vertexToRemove]; i++){
        degreeModification[graph[vertexToRemove][i]]++;
    }
    
    int minDegree;
    int minDegreeVertex;
    int minDegreeCount;
    
    //check conditions for Dirac's theorem
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(i != vertexToRemove){
            if(adj[i] - degreeModification[i] < minDegree){
                minDegree = adj[i] - degreeModification[i];
                minDegreeVertex = i;
                minDegreeCount = 1;
            } else if(adj[i] - degreeModification[i] == minDegree){
                minDegreeCount++;
            }
        }
    }
    
    if(2*minDegree >= order - 1){
        //this graph is hamiltonian
        return FALSE;
    } else if(minDegree == 1 && minDegreeCount > 2){
        //this graph cannot be traceable
        return FALSE;
    }
    
    if(minDegree == 1){
        //graph is certainly not hamiltonian and hamiltonian path will start
        //from vertex of degree 1
        return isTraceableFromVertex_vertexDeleted(graph, adj, minDegreeVertex, vertexToRemove);
    } else {
        //graph might be hamiltonian
        boolean foundHamiltonianCycle = FALSE;
        int start = vertexToRemove == 1 ? 2 : 1;
        boolean isTraceable = isTraceableFromVertexIncludingCheckForHamiltonianCycle_vertexDeleted(graph, adj, start, &foundHamiltonianCycle, vertexToRemove);
        if(foundHamiltonianCycle){
            return FALSE;
        }
        if(!isTraceable){
            start++;
            if(start==vertexToRemove){
                start++;
            }
            /* we don't need to check the last vertex, since a hamiltonian path
             * starting at that vertex would already have been found from the other
             * end.
             */
            while(start < order && !isTraceableFromVertex_vertexDeleted(graph, adj, start, vertexToRemove)){
                start++;
                if(start==vertexToRemove){
                    start++;
                }
            }
            if(start >= order){
                //graph is not traceable
                return FALSE;
            }
        }
    }
    
    
    return TRUE;
}


boolean isTraceableFromVertexIncludingCheckForHamiltonianCycle(GRAPH graph, ADJACENCY adj, int startVertex, boolean *foundHamiltonianCycle){
    int i;
    
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    boolean traceable = FALSE;
    
    currentPath[startVertex]=TRUE;
    for(i = 0; i < adj[startVertex]; i++){
        currentPath[graph[startVertex][i]]=TRUE;
        //search for path containing the edge (startVertex, graph[startVertex][i])
        if(continuePathIncludingCheckForHamiltonianCycle(graph, adj, graph[startVertex][i], graph[0][0] - 2, startVertex, foundHamiltonianCycle)){
            if(*foundHamiltonianCycle){
                return TRUE;
            } else {
                traceable = TRUE;
            }
        }
        currentPath[graph[startVertex][i]]=FALSE;
    }
    currentPath[startVertex]=FALSE;
    
    return traceable;
}

boolean isTraceableFromVertex(GRAPH graph, ADJACENCY adj, int startVertex){
    int i;
    
    for(i=0; i<=MAXN; i++){
        currentPath[i] = FALSE;
    }
    
    currentPath[startVertex]=TRUE;
    for(i = 0; i < adj[startVertex]; i++){
        currentPath[graph[startVertex][i]]=TRUE;
        //search for path containing the edge (startVertex, graph[startVertex][i])
        if(continuePath(graph, adj, graph[startVertex][i], graph[0][0] - 2)){
            return TRUE;
        }
        currentPath[graph[startVertex][i]]=FALSE;
    }
    currentPath[startVertex]=FALSE;
    
    return FALSE;
}

boolean is2LeafStable(GRAPH graph, ADJACENCY adj){
    int i, v;
    int order = graph[0][0];
    int minDegree;
    int minDegreeVertex;
    
    if(order<3){
        return FALSE;
    }
    
    //check conditions for Dirac's theorem
    minDegree = order;
    for(i = 1; i <= order; i++){
        if(adj[i] < minDegree){
            minDegree = adj[i];
            minDegreeVertex = i;
        }
    }
    
    if(2*minDegree >= order){
        //this graph is hamiltonian
        return FALSE;
    } else if(minDegree == 1){
        //removing the neighbour of this vertex leaves a disconnected graph
        //this graph will not be traceable
        return FALSE;
    }
    
    //check traceable but not hamiltonian
    boolean foundHamiltonianCycle = FALSE;
    boolean isTraceable = isTraceableFromVertexIncludingCheckForHamiltonianCycle(graph, adj, 1, &foundHamiltonianCycle);
    if(foundHamiltonianCycle){
        return FALSE;
    }
    if(!isTraceable){
        int start = 2;
        /* we don't need to check the last vertex, since a hamiltonian path
         * starting at that vertex would already have been found from the other
         * end.
         */
        while(start < order && !isTraceableFromVertex(graph, adj, start)){
            start++;
        }
        if(start == order){
            //graph is not traceable
            return FALSE;
        }
    }
    
    //if we got here then the graph is traceable but not hamiltonian
    
    //try removing each vertex once and check that the resulting graph is 
    //traceable but not hamiltonian
    for(v = 1; v <= order; v++){
        if(!removingVertexLeavesTraceableNotHamiltonian(graph, adj, v)){
            return FALSE;
        }
    }
    
    return TRUE;
}