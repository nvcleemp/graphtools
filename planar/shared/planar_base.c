/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planar_base.h"
#include <stdio.h>
#include <stdlib.h>

PLANE_GRAPH *newPlaneGraph(int maxn, int maxe) {
    if(maxn <= 0){
        fprintf(stderr, "maxn should be a positive integer -- exiting!\n");
        return NULL;
    }
    
    if(maxe <= 0 || maxe > (6*maxn-12)){
        maxe = 6*maxn-12;
    }
    
    PLANE_GRAPH *pg = (PLANE_GRAPH *)malloc(sizeof(PLANE_GRAPH));
    
    if(pg == NULL){
        fprintf(stderr, "Insufficient memory for plane_graph -- exiting!\n");
        return NULL;
    }
    
    pg->maxn = maxn;
    pg->maxe = maxe;
    
    pg->nv = pg->ne = pg->nf = 0;
    
    pg->edges = (PG_EDGE *)malloc(sizeof(PG_EDGE)*maxe);
    
    if(pg->edges == NULL){
        fprintf(stderr, "Insufficient memory for edges -- exiting!\n");
        free(pg);
        return NULL;
    }
    
    pg->firstedge = (PG_EDGE **)malloc(sizeof(PG_EDGE *)*maxn);
    
    if(pg->firstedge == NULL){
        fprintf(stderr, "Insufficient memory for firstedges -- exiting!\n");
        free(pg->edges);
        free(pg);
        return NULL;
    }
    
    pg->degree = (int *)malloc(sizeof(int)*maxn);
    
    if(pg->degree == NULL){
        fprintf(stderr, "Insufficient memory for degrees -- exiting!\n");
        free(pg->degree);
        free(pg->edges);
        free(pg);
        return NULL;
    }
    
    //by default the dual is not yet initialised
    pg->maxf = 0;
    pg->facestart = NULL;
    pg->faceSize = NULL;
    pg->dualComputed = FALSE;
    pg->markvalue = 30000;
    
    return pg;
}

void freePlaneGraph(PLANE_GRAPH *pg){
    if(pg->facestart != NULL){
        free(pg->facestart);
    }
    
    if(pg->faceSize != NULL){
        free(pg->faceSize);
    }
    
    free(pg->degree);
    free(pg->firstedge);
    free(pg->edges);
    free(pg);
}

PG_EDGE *findEdge(PLANE_GRAPH *pg, int from, int to) {
    PG_EDGE *e, *elast;

    e = elast = pg->firstedge[from];
    do {
        if (e->end == to) {
            return e;
        }
        e = e->next;
    } while (e != elast);
    fprintf(stderr, "error while looking for edge from %d to %d.\n", from, to);
    exit(EXIT_FAILURE);
}

/* Store in the rightface field of each edge the number of the face on
   the right hand side of that edge.  Faces are numbered 0,1,....  Also
   store in facestart[i] an example of an edge in the clockwise orientation
   of the face boundary, and the size of the face in facesize[i], for each i.
   Returns the number of faces. */
void makeDual(PLANE_GRAPH *pg) {
    register int i, sz;
    register PG_EDGE *e, *ex, *ef, *efx;

    RESETMARKS(pg);
    
    //first allocate the memory to store faces if this has not yet been done
    int maxf = 2*pg->maxn - 4;
    if(pg->maxf < maxf){
        if(pg->facestart!=NULL){
            free(pg->facestart);
        }
        if(pg->faceSize!=NULL){
            free(pg->faceSize);
        }
        pg->maxf = maxf;
        
        pg->facestart = (PG_EDGE **)malloc(sizeof(PG_EDGE *)*maxf);
        
        if(pg->facestart == NULL){
            pg->maxf = 0;
            return;
        }
        
        pg->faceSize = (int *)malloc(sizeof(int)*maxf);
        
        if(pg->faceSize == NULL){
            pg->maxf = 0;
            free(pg->facestart);
            return;
        }
    }

    int nf = 0;
    for (i = 0; i < pg->nv; ++i) {

        e = ex = pg->firstedge[i];
        do {
            if (!ISMARKEDLO(pg, e)) {
                pg->facestart[nf] = ef = efx = e;
                sz = 0;
                do {
                    ef->rightface = nf;
                    MARKLO(pg, ef);
                    ef = ef->inverse->prev;
                    ++sz;
                } while (ef != efx);
                pg->faceSize[nf] = sz;
                ++nf;
            }
            e = e->next;
        } while (e != ex);
    }
    pg->nf = nf;
    pg->dualComputed = TRUE;
}

void clearAllEdgeLabels(PLANE_GRAPH *pg){
    int i;
    
    for(i = 0; i < pg->maxe; i++){
        pg->edges[i].label = NULL;
    }
}
