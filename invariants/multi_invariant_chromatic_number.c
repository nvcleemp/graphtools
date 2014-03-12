/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Computes the chromatic number of a graph in multicode format
 * 
 * Compile like this:
 *     
 *     cc -o multi_invariant_chromatic_number -O4 -DINVARIANT=chromaticNumber \
 *     -DINVARIANTNAME="chromatic number" multi_int_invariant.c \
 *     ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c \
 *     ../multicode/shared/multicode_output.c \
 *     multi_invariant_chromatic_number.c
 */

#include "../multicode/shared/multicode_base.h"

int colourGraph(GRAPH graph, ADJACENCY adj, int* graphPartition, int currentVertex, int maximumColours){
    int i, j, partitionCount;
    //first see how many partitions we have
    partitionCount = 0;
    for(i = 1; i < currentVertex; i++){
        if(graphPartition[i]==i){
            partitionCount++;
        }
    }
    
    if(partitionCount >= maximumColours){
        //we won't find anything better with this partition
        return maximumColours;
    }
    if(currentVertex > graph[0][0]){
        //all vertices have been coloured, and partitionCount is less than maximumColours
        return partitionCount;
    }
    
    //colour the current vertex
    
    //first try all existing colour classes
    for(i = 1; i < currentVertex; i++){
        if(graphPartition[i]==i){
            boolean canGoInThisPartition = TRUE;
            j = 0;
            while(canGoInThisPartition && j < adj[currentVertex]){
                //the vertex can go in this partition if all of its neighbours
                //are either not in a colour class or in a different colour class
                canGoInThisPartition = (graph[currentVertex][j] > currentVertex) ||
                        (graphPartition[graph[currentVertex][j]] != i);
                j++;
            }
            if(canGoInThisPartition){
                graphPartition[currentVertex] = i;
                maximumColours = colourGraph(graph, adj, graphPartition, currentVertex + 1, maximumColours);
            }
        }
    }
    
    //finally try to put the vertex in a new colour class
    graphPartition[currentVertex] = currentVertex;
    maximumColours = colourGraph(graph, adj, graphPartition, currentVertex + 1, maximumColours);
    
    return maximumColours;
}

int chromaticNumber(GRAPH graph, ADJACENCY adj){
    int i, minDeg, maxDeg, n;
    
    n = graph[0][0];
    
    if(n<=1){
        return 0;
    }
    
    minDeg = maxDeg = adj[1];
    
    for(i=2; i<=n; i++){
        if(adj[i] < minDeg){
            minDeg = adj[i];
        } else if(adj[i] > maxDeg){
            maxDeg = adj[i];
        }
    }
    
    if((maxDeg == n - 1) && (minDeg == n - 1)){
        return n; //complete graph
    } else if((maxDeg == 2) && (minDeg == 2) && (n % 2)){
        return 3; //odd cycle or union of cycles and at least one is odd
    } else if(maxDeg == 0){
        return 1; //no edges
    }
    
    int graphPartition[MAXN + 1];
    
    //initially all vertices are in the same partition
    for(i = 1; i <= MAXN; i++){
        graphPartition[i] = 1;
    }
    
    return colourGraph(graph, adj, graphPartition, 2, maxDeg + 1);
}