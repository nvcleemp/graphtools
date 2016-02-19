/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANAR_BASE_H
#define	PLANAR_BASE_H

#define FALSE 0
#define TRUE  1

typedef int boolean;
typedef struct __pg_edge PG_EDGE;
typedef struct __plane_graph PLANE_GRAPH;

/* The data type used for edges */ 
struct __pg_edge {
    //the start and end vertex of this edge
    int start;
    int end;
    
    //the face on the right side of the edge
    //NOTE: only valid if make_dual() called
    int rightface;
    
    PG_EDGE *prev; /* previous edge in clockwise direction */
    PG_EDGE *next; /* next edge in clockwise direction */
    PG_EDGE *inverse; /* the edge that is inverse to this one */
    
     /* two ints for temporary use;
      * Only access mark via the MARK macros.
      */
    int mark, index;
    
    //a label can be used to store any additional information you want to
    //associate with an edge
    void *label;
};

struct __plane_graph {
    int nv;
    int ne;
    int nf;
    
    int maxn;
    int maxe;
    int maxf;
    
    //an array containing all edges of this graph
    PG_EDGE *edges;
    
    //an array containing for each vertex a pointer to an edge leaving that vertex
    PG_EDGE **firstedge;
    
    //an array containing the degree of each vertex
    int *degree;
    
    //an array containing for each face a pointer to an edge in the clockwise 
    //boundary of that face
    PG_EDGE **facestart;
    
    //an array containing the size of each face
    int *faceSize;
    
    //TRUE if the dual of this graph has been computed
    boolean dualComputed;
};

static int markvalue = 30000;
#define RESETMARKS(pg) {int mki; if ((markvalue += 2) > 30000) \
       { markvalue = 2; for (mki=0;mki<(pg)->maxe;++mki) (pg)->edges[mki].mark=0;}}
#define MARK(e) (e)->mark = markvalue
#define MARKLO(e) (e)->mark = markvalue
#define MARKHI(e) (e)->mark = markvalue+1
#define UNMARK(e) (e)->mark = markvalue-1
#define ISMARKED(e) ((e)->mark >= markvalue)
#define ISMARKEDLO(e) ((e)->mark == markvalue)
#define ISMARKEDHI(e) ((e)->mark > markvalue)

/**
 * Creates a new planar graph data structure that can hold plane graphs with
 * up to maxn vertices and maxe oriented edges. If maxe is zero, then the 
 * theoretical maximum for maxn is computed and used.
 * 
 * This function might return a NULL pointer if insufficient memory was
 * available or the number of vertices is illegal.
 */
PLANE_GRAPH *newPlaneGraph(int maxn, int maxe);

void freePlaneGraph(PLANE_GRAPH *pg);

void makeDual(PLANE_GRAPH *pg);

PG_EDGE *findEdge(PLANE_GRAPH *pg, int from, int to);

void clearAllEdgeLabels(PLANE_GRAPH *pg);

#endif	/* PLANAR_BASE_H */

