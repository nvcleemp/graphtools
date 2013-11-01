/* 
 * File:   multi_connectivity.h
 * Author: nvcleemp
 *
 * Created on November 1, 2013, 9:47 AM
 */

#ifndef MULTI_CONNECTIVITY_H
#define	MULTI_CONNECTIVITY_H

boolean findPath(GRAPH graph, ADJACENCY adj, int currentVertex, int target, int *paths, int order, boolean* currentPath);

int findMaxFlowInSTNetwork(GRAPH graph, ADJACENCY adj, int source, int target, int maxValue);

int findEdgeConnectivity(GRAPH graph, ADJACENCY adj);

#endif	/* MULTI_CONNECTIVITY_H */

