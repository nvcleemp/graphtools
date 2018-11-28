/* 
 * File:   multi_connectivity.h
 * Author: nvcleemp
 *
 * Created on November 1, 2013, 9:47 AM
 */

#ifndef MULTICODE_CONNECTIVITY_H
#define	MULTICODE_CONNECTIVITY_H

boolean find_path(GRAPH *graph, int current_vertex, int target, int *paths, int order, boolean* current_path, int *capacity);

int find_max_flow_in_st_network(GRAPH *graph, int source, int target, int max_value);

int find_edge_connectivity(GRAPH *graph);

#endif	/* MULTICODE_CONNECTIVITY_H */

