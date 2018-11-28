/* 
 * File:   planar_automorphismgroup.h
 * Author: nvcleemp
 *
 * Created on February 23, 2016, 3:26 PM
 */

#ifndef PLANAR_AUTOMORPHISMGROUP_H
#define	PLANAR_AUTOMORPHISMGROUP_H

#include "planar_base.h"

typedef struct __pg_automorphism_group PG_AUTOMORPHISM_GROUP;

struct __pg_automorphism_group {
    int **automorphisms;
    int size;
    int orientationPreservingCount;
    int orientationReversingCount;
    
    PLANE_GRAPH *graph;
    
    int allocatedSize;
};

PG_AUTOMORPHISM_GROUP *determineAutomorphisms(PLANE_GRAPH *pg);

void freeAutomorphismGroup(PG_AUTOMORPHISM_GROUP *aut);


#endif	/* PLANAR_AUTOMORPHISMGROUP_H */

