/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2018 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * 
 */

#include <string.h>

#include "multicode_base.h"
#include "multicode_connectivity.h"
    
//try to find a path from the current vertex to the target
boolean find_path(GRAPH *graph, int current_vertex, int target, int *paths, int order, boolean* current_path, int *capacity) {
    int i;
    if(current_vertex==target)
        return TRUE;
    current_path[current_vertex]=TRUE;
    for(i=0; i < graph->degrees[current_vertex]; i++){
        int next_vertex = NEIGHBOUR(graph, current_vertex, i);
        if(paths[current_vertex*order + next_vertex]-paths[next_vertex*order + current_vertex]<=capacity[current_vertex*order + next_vertex]-1 && !current_path[next_vertex]){
            paths[current_vertex*order + next_vertex]++;
            if(find_path(graph, next_vertex, target, paths, order, current_path, capacity))
                return TRUE;
            else
                paths[current_vertex*order + next_vertex]--;
        }
    }
    current_path[current_vertex]=FALSE;
    return FALSE;
}

//returns the minimum of the maxflow of the st-network and max_value
int find_max_flow_in_st_network(GRAPH *graph, int source, int target, int max_value){
    int i, j;

    int order = graph->n;
    int paths[order*order];
    int capacity[order*order];
    for(i=0; i<order*order; i++){
        paths[i] = 0;
        capacity[i] = 0;
    }
    for(i = 0; i < order; i++){
        for(j = 0; j < graph->degrees[i]; j++){
            capacity[i*order + NEIGHBOUR(graph, i, j)]++;
        }
    }
    
    int path_count = 0;
    boolean current_path[order];
    for(i=0; i<order; i++){
        current_path[i] = FALSE;
    }
    while(find_path(graph, source, target, paths, order, current_path, capacity) && path_count < max_value){
        path_count++;
        for(i=0; i<order; i++){
            current_path[i] = FALSE;
        }
    }
    return path_count;
}

int find_edge_connectivity(GRAPH *graph){
    int i;
    int min_degree, vertex_min_degree, minimum_cut_size;
    
    min_degree = graph->degrees[0];
    vertex_min_degree = 1;
    
    for (i = 1; i < graph->n; i++){
        if(graph->degrees[i]<min_degree){
            min_degree = graph->degrees[i];
            vertex_min_degree = i;
        }
    }
    
    minimum_cut_size = min_degree;
    for (i = 0; i < graph->n; i++){
        if(i!=vertex_min_degree){
            minimum_cut_size = find_max_flow_in_st_network(graph, vertex_min_degree, i, minimum_cut_size);
        }
        //TODO: add optimalisations
    }
    
    return minimum_cut_size;
}