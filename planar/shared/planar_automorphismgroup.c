/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "planar_automorphismgroup.h"

#define ABORT_IF_NULL(pointer) if(pointer==NULL){exit(-1);}

//////////////////////////////////////////////////////////////////////////////

typedef struct __pg_aut_comp_data PG_AUT_COMP_DATA;

struct __pg_aut_comp_data {
    int *certificate;
    int *canonicalLabelling;
    int *reverseCanonicalLabelling;
    PG_EDGE **canonicalFirstEdge;
    int *alternateLabelling;
    PG_EDGE **alternateFirstEdge;
    
    PG_EDGE **orientationPreservingStartingEdges;
    PG_EDGE **orientationReversingStartingEdges;
    int startingEdgesCount; //the number of starting edges is always the same for both orientations
    
    int *queue;
    
    boolean hasChiralGroup;
};

PG_AUT_COMP_DATA *getAutomorphismComputationWorkspace(PLANE_GRAPH *pg){
    int e, n;
    
    e = pg->ne;
    n = pg->nv;
    
    PG_AUT_COMP_DATA *workspace = malloc(sizeof(PG_AUT_COMP_DATA));
    ABORT_IF_NULL(workspace);
    
    workspace->certificate = malloc(sizeof(int)*(e+n));
    ABORT_IF_NULL(workspace->certificate);
    workspace->canonicalLabelling = malloc(sizeof(int)*n);
    ABORT_IF_NULL(workspace->canonicalLabelling);
    workspace->reverseCanonicalLabelling = malloc(sizeof(int)*n);
    ABORT_IF_NULL(workspace->reverseCanonicalLabelling);
    workspace->canonicalFirstEdge = malloc(sizeof(PG_EDGE*)*n);
    ABORT_IF_NULL(workspace->canonicalFirstEdge);
    workspace->alternateLabelling = malloc(sizeof(int)*n);
    ABORT_IF_NULL(workspace->alternateLabelling);
    workspace->alternateFirstEdge = malloc(sizeof(PG_EDGE*)*n);
    ABORT_IF_NULL(workspace->alternateFirstEdge);
    
    workspace->orientationPreservingStartingEdges = malloc(sizeof(PG_EDGE*)*e);
    ABORT_IF_NULL(workspace->orientationPreservingStartingEdges);
    workspace->orientationReversingStartingEdges = malloc(sizeof(PG_EDGE*)*e);
    ABORT_IF_NULL(workspace->orientationReversingStartingEdges);
    
    workspace->queue = malloc(sizeof(int)*n);
    ABORT_IF_NULL(workspace->queue);
    
    return workspace;
}

PG_AUTOMORPHISM_GROUP *allocateAutomorphismGroup(PLANE_GRAPH *pg){
    int i;
    
    PG_AUTOMORPHISM_GROUP *aut = (PG_AUTOMORPHISM_GROUP *)malloc(sizeof(PG_AUTOMORPHISM_GROUP));
    
    aut->automorphisms = (int **)malloc(sizeof(int *)*(pg->ne)*2);
    
    for(i = 0; i < (pg->ne)*2; i++){
        aut->automorphisms[i] = (int *)malloc(sizeof(int)*(pg->nv));
    }
    
    aut->allocatedSize = (pg->ne)*2;
    aut->graph = pg;
    
    aut->size = 0;
    aut->orientationPreservingCount = 0;
    aut->orientationReversingCount = 0;
    
    return aut;
}

void trimAutomorphismGroup(PG_AUTOMORPHISM_GROUP *aut){
    int i;
    
    for(i = aut->size; i < aut->allocatedSize; i++){
        free(aut->automorphisms[i]);
    }
    
    aut->automorphisms = (int **)realloc(aut->automorphisms, sizeof(int *)*(aut->size));
    aut->allocatedSize = aut->size;
}

void freeAutomorphismGroup(PG_AUTOMORPHISM_GROUP *aut){
    int i;
    
    for(i = 0; i < aut->allocatedSize; i++){
        free(aut->automorphisms[i]);
    }
    
    free(aut->automorphisms);
    free(aut);
}

void findStartingEdges(PLANE_GRAPH *pg, PG_AUT_COMP_DATA *workspace){
    int i, startingDegree, startingFaceSize, minimumFrequency;
    int degreeFrequency[pg->nv];
    int faceSizeFrequency[pg->nv];
    PG_EDGE *start, *edge;
    
    for(i = 0; i < pg->nv; i++){
        degreeFrequency[i] = faceSizeFrequency[i] = 0;
    }
    
    workspace->startingEdgesCount = 0;
    
    //build the degree frequency table
    for(i = 0; i < pg->nv; i++){
        degreeFrequency[pg->degree[i]]++;
    }
    
    //find the smallest degree with the lowest frequency
    minimumFrequency = pg->nv + 1;
    for(i = 0; i < pg->nv; i++){
        if(degreeFrequency[i] && degreeFrequency[i] < minimumFrequency){
            startingDegree = i;
            minimumFrequency = degreeFrequency[i];
        }
    }
    
    //build the frequency table of face sizes incident to a vertex with startingDegree
    for(i = 0; i < pg->nv; i++){
        if(pg->degree[i] == startingDegree){
            start = edge = pg->firstedge[i];
            
            do {
                faceSizeFrequency[pg->faceSize[edge->rightface]]++;
                edge = edge->next;
            } while (start != edge);
        }
    }
    
    //find the smallest face size incident with a vertex of startingDegree and with smallest frequency
    minimumFrequency = pg->ne + 1;
    for(i = 0; i < pg->nv; i++){
        if(faceSizeFrequency[i] && faceSizeFrequency[i] < minimumFrequency){
            startingFaceSize = i;
            minimumFrequency = faceSizeFrequency[i];
        }
    }
    
    //store all starting edges
    for(i = 0; i < pg->nv; i++){
        if(pg->degree[i] == startingDegree){
            start = edge = pg->firstedge[i];
            
            do {
                if(pg->faceSize[edge->rightface] == startingFaceSize){
                    workspace->orientationPreservingStartingEdges[workspace->startingEdgesCount] = edge;
                    workspace->orientationReversingStartingEdges[workspace->startingEdgesCount] = edge->next;
                    workspace->startingEdgesCount++;
                }
                edge = edge->next;
            } while (start != edge);
        }
    }
}

void constructCertificate(PLANE_GRAPH *pg, PG_EDGE *eStart, PG_AUT_COMP_DATA *workspace){
    int i;
    for(i=0; i<pg->nv; i++){
        workspace->canonicalLabelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int position = 0;
    workspace->queue[0] = eStart->start;
    workspace->canonicalFirstEdge[eStart->start] = eStart;
    workspace->canonicalLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = workspace->queue[tail++];
        e = elast = workspace->canonicalFirstEdge[currentVertex];
        do {
            if(workspace->canonicalLabelling[e->end]==INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->canonicalLabelling[e->end] = vertexCounter++;
                workspace->canonicalFirstEdge[e->end] = e->inverse;
            }
            workspace->certificate[position++] = workspace->canonicalLabelling[e->end];
            e = e->next;
        } while (e!=elast);
        workspace->certificate[position++] = INT_MAX;
    }
    for(i = 0; i < pg->nv; i++){
        workspace->reverseCanonicalLabelling[workspace->canonicalLabelling[i]] = i;
    }
}

void constructCertificateOrientationReversed(PLANE_GRAPH *pg, PG_EDGE *eStart, PG_AUT_COMP_DATA *workspace){
    int i;
    for(i=0; i<pg->nv; i++){
        workspace->canonicalLabelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int position = 0;
    workspace->queue[0] = eStart->start;
    workspace->canonicalFirstEdge[eStart->start] = eStart;
    workspace->canonicalLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = workspace->queue[tail++];
        e = elast = workspace->canonicalFirstEdge[currentVertex];
        do {
            if(workspace->canonicalLabelling[e->end]==INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->canonicalLabelling[e->end] = vertexCounter++;
                workspace->canonicalFirstEdge[e->end] = e->inverse;
            }
            workspace->certificate[position++] = workspace->canonicalLabelling[e->end];
            e = e->prev;
        } while (e!=elast);
        workspace->certificate[position++] = INT_MAX;
    }
    for(i = 0; i < pg->nv; i++){
       workspace->reverseCanonicalLabelling[workspace->canonicalLabelling[i]] = i;
    }
}

/* returns 1 if this edge leads to a better certificate
 * returns 0 if this edge leads to the same certificate
 * returns -1 if this edge leads to a worse certificate
 */
int hasBetterCertificateOrientationPreserving(PLANE_GRAPH *pg, PG_EDGE *eStart, PG_AUTOMORPHISM_GROUP *autGroup, PG_AUT_COMP_DATA *workspace){
    int i, j;
    for(i=0; i<pg->nv; i++){
        workspace->alternateLabelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int currentPos = 0;
    workspace->queue[0] = eStart->start;
    workspace->alternateFirstEdge[eStart->start] = eStart;
    workspace->alternateLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = workspace->queue[tail++];
        e = elast = workspace->alternateFirstEdge[currentVertex];
        do {
            if(workspace->alternateLabelling[e->end]==INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->alternateLabelling[e->end] = vertexCounter++;
                workspace->alternateFirstEdge[e->end] = e->inverse;
            }
            if(workspace->alternateLabelling[e->end] < workspace->certificate[currentPos]){
                constructCertificate(pg, eStart, workspace);
                autGroup->size = 1;
                autGroup->orientationPreservingCount = 1;
                return 1;
            } else if(workspace->alternateLabelling[e->end] > workspace->certificate[currentPos]){
                return -1;
            }
            currentPos++;
            e = e->next;
        } while (e!=elast);
        //INT_MAX will always be at least the value of certificate[currentPos]
        if(INT_MAX > workspace->certificate[currentPos]){
            return -1;
        }
        currentPos++;
    }
    for(j = 0; j < pg->nv; j++){
        autGroup->automorphisms[autGroup->size][j] 
                = workspace->reverseCanonicalLabelling[workspace->alternateLabelling[j]];
    }
    autGroup->size++;
    autGroup->orientationPreservingCount++;
    return 0;
}

int hasBetterCertificateOrientationReversing(PLANE_GRAPH *pg, PG_EDGE *eStart, PG_AUTOMORPHISM_GROUP *autGroup, PG_AUT_COMP_DATA *workspace){
    int i, j;
    for(i=0; i<pg->nv; i++){
        workspace->alternateLabelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int currentPos = 0;
    workspace->queue[0] = eStart->start;
    workspace->alternateFirstEdge[eStart->start] = eStart;
    workspace->alternateLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = workspace->queue[tail++];
        e = elast = workspace->alternateFirstEdge[currentVertex];
        do {
            if(workspace->alternateLabelling[e->end]==INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->alternateLabelling[e->end] = vertexCounter++;
                workspace->alternateFirstEdge[e->end] = e->inverse;
            }
            if(workspace->alternateLabelling[e->end] < workspace->certificate[currentPos]){
                constructCertificateOrientationReversed(pg, eStart, workspace);
                workspace->hasChiralGroup = TRUE;
                autGroup->size = 1;
                autGroup->orientationPreservingCount = 1;
                autGroup->orientationReversingCount = 0;
                return 1;
            } else if(workspace->alternateLabelling[e->end] > workspace->certificate[currentPos]){
                return -1;
            }
            currentPos++;
            e = e->prev;
        } while (e!=elast);
        //INT_MAX will always be at least the value of certificate[currentPos]
        if(INT_MAX > workspace->certificate[currentPos]){
            return -1;
        }
        currentPos++;
    }
    for(j = 0; j < pg->nv; j++){
        autGroup->automorphisms[autGroup->size][j] 
                = workspace->reverseCanonicalLabelling[workspace->alternateLabelling[j]];
    }
    if(workspace->hasChiralGroup){
        autGroup->orientationPreservingCount++;
    } else {
        autGroup->orientationReversingCount++;
    }
    autGroup->size++;
    return 0;
}

PG_AUTOMORPHISM_GROUP *determineAutomorphisms(PLANE_GRAPH *pg){
    int i;
    
    PG_AUTOMORPHISM_GROUP *aut = allocateAutomorphismGroup(pg);
    
    //identity
    for(i = 0; i < pg->nv; i++){
        aut->automorphisms[0][i] = i;
    }
    
    aut->size = 1;
    aut->orientationPreservingCount = 1;
    aut->orientationReversingCount = 0;
    
    PG_AUT_COMP_DATA *workspace = getAutomorphismComputationWorkspace(pg);
    workspace->hasChiralGroup = FALSE;
    
    //find starting edges
    findStartingEdges(pg, workspace);
    
    //construct initial certificate
    constructCertificate(pg, workspace->orientationPreservingStartingEdges[0], workspace);
    
    //look for better automorphism
    for(i = 1; i < workspace->startingEdgesCount; i++){
        int result = hasBetterCertificateOrientationPreserving(pg,
                                workspace->orientationPreservingStartingEdges[i],
                                aut, workspace);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
    for(i = 0; i < workspace->startingEdgesCount; i++){
        int result = hasBetterCertificateOrientationReversing(pg,
                                workspace->orientationReversingStartingEdges[i],
                                aut, workspace);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
    
    return aut;
}