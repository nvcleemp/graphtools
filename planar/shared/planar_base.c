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

PLANE_GRAPH *getDualGraph(PLANE_GRAPH *pg){
    int i;
    
    if(!pg->dualComputed){
        makeDual(pg);
    }
    
    PLANE_GRAPH *dual = newPlaneGraph(pg->nf, pg->ne);
    
    if(dual==NULL){
        fprintf(stderr, "Insufficient memory to create dual.\n");
        return NULL;
    }
    
    for(i = 0; i < pg->ne; i++){
        pg->edges[i].index = i;
    }
    
    dual->nv = pg->nf;
    dual->ne = pg->ne;
    dual->nf = pg->nv;
    
    for(i = 0; i < pg->ne; i++){
        dual->edges[i].start = pg->edges[i].rightface;
        dual->edges[i].end = pg->edges[i].inverse->rightface;
        dual->edges[i].rightface = pg->edges[i].end;
        
        dual->edges[i].inverse = dual->edges + pg->edges[i].inverse->index;
        dual->edges[i].next = dual->edges + pg->edges[i].inverse->prev->index;
        dual->edges[i].prev = dual->edges + pg->edges[i].next->inverse->index;
    }
    
    for(i = 0; i < pg->nf; i++){
        dual->degree[i] = pg->faceSize[i];
        dual->firstedge[i] = dual->edges + pg->facestart[i]->index;
    }
    
    dual->maxf = pg->nv;
    
    dual->facestart = (PG_EDGE **)malloc(sizeof(PG_EDGE *)*(dual->maxf));
        
    if(dual->facestart == NULL){
        dual->maxf = 0;
        return dual;
    }

    dual->faceSize = (int *)malloc(sizeof(int)*(dual->maxf));

    if(dual->faceSize == NULL){
        dual->maxf = 0;
        free(dual->facestart);
        return dual;
    }
    
    for(i = 0; i < pg->nv; i++){
        dual->faceSize[i] = pg->degree[i];
        dual->facestart[i] = dual->edges + pg->firstedge[i]->inverse->index;
    }
    
    dual->dualComputed = TRUE;
    
    return dual;
}