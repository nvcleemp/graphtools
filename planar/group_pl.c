/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads planar graphs from standard in and
 * determines the symmetry group for each graph.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o group_pl -O4 group_pl.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#define UNKNOWN 0
#define Cn__    1
#define Cnh__   2
#define Cnv__   3
#define S2n__   4
#define Dn__    5
#define Dnh__   6
#define Dnd__   7
#define T__     8
#define Td__    9
#define Th__   10
#define O__    11
#define Oh__   12
#define I__    13
#define Ih__   14


#ifndef MAXN
#define MAXN 1000            /* the maximum number of vertices */
#endif
#define MAXE (6*MAXN-12)    /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)      /* the maximum number of faces */
#define MAXVAL (MAXN-1)  /* the maximum degree of a vertex */
#define MAXCODELENGTH (MAXN+MAXE+3)

#define INFI (MAXN + 1)

typedef int boolean;

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE  1

typedef struct e /* The data type used for edges */ {
    int start;       /* vertex where the edge starts */
    int end;         /* vertex where the edge ends */
    int rightface;   /* face on the right side of the edge
                        note: only valid if make_dual() called */
    struct e *prev;  /* previous edge in clockwise direction */
    struct e *next;  /* next edge in clockwise direction */
    struct e *inverse; /* the edge that is inverse to this one */
    int mark, index; /* two ints for temporary use;
                          Only access mark via the MARK macros. */
} EDGE;

EDGE *firstedge[MAXN]; /* pointer to arbitrary edge out of vertex i. */
int degree[MAXN];

EDGE *facestart[MAXF]; /* pointer to arbitrary edge of face i. */
int faceSize[MAXF]; /* pointer to arbitrary edge of face i. */

EDGE edges[MAXE];

static int markvalue = 30000;
#define RESETMARKS {int mki; if ((markvalue += 2) > 30000) \
       { markvalue = 2; for (mki=0;mki<MAXE;++mki) edges[mki].mark=0;}}
#define MARK(e) (e)->mark = markvalue
#define MARKLO(e) (e)->mark = markvalue
#define MARKHI(e) (e)->mark = markvalue+1
#define UNMARK(e) (e)->mark = markvalue-1
#define ISMARKED(e) ((e)->mark >= markvalue)
#define ISMARKEDLO(e) ((e)->mark == markvalue)
#define ISMARKEDHI(e) ((e)->mark > markvalue)

int nv;
int ne;
int nf;

int automorphisms[2*MAXE][MAXN]; //there are at most 2e automorphisms (e = #arcs)
int automorphismsCount;
int orientationPreservingAutomorphismsCount;
int orientationReversingAutomorphismsCount;

int numberOfGraphs = 0;
int reportsWritten = 0;

boolean groupHasParameter[15] = {FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
                                 FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

//////////////////////////////////////////////////////////////////////////////

typedef struct group_list_element {
    int groupId;
    int groupParameter;
    boolean anyParameterAllowed;
    
    int frequency;
    
    struct group_list_element *smaller;
    struct group_list_element *greater;
} GLE;

typedef GLE GROUPLIST;

int filterEnabled = FALSE;
GROUPLIST *filterList;

/* Return c<0 if the first group is smaller
 * Return 0 if the groups are equal
 * Return c>0 if the first group is greater 
 */
int compareGroups(int groupId1, int groupParameter1, boolean anyParameterAllowed1,
        int groupId2, int groupParameter2, boolean anyParameterAllowed2) {
    if(groupId1 == groupId2) {
        if(!groupHasParameter[groupId1] ||
                (anyParameterAllowed1 && anyParameterAllowed2)){
            return 0;
        } else if(anyParameterAllowed1 && !anyParameterAllowed2){
            return -1;
        } else if(!anyParameterAllowed1 && anyParameterAllowed2){
            return 1;
        } else {
            return groupParameter1 - groupParameter2;
        }
    } else {
        return groupId1 - groupId2;
    }
}

/* Return c<0 if the group is greater than the category
 * Return 0 if the group is included in the category
 * Return c>0 if the group is smaller than the category 
 */
int groupIncludedInCategory(int groupIdCategory, int groupParameterCategory, 
        boolean anyParameterAllowedCategory, int groupId, int groupParameter) {
    if(groupIdCategory == groupId) {
        if(!groupHasParameter[groupIdCategory] || anyParameterAllowedCategory){
            return 0;
        } else {
            return groupParameterCategory - groupParameter;
        }
    } else {
        return groupIdCategory - groupId;
    }
}

GLE *newGroupListElement(int groupId, int groupParameter, boolean anyParameterAllowed){
    GLE *gle = malloc(sizeof(GLE));
    
    if(gle==NULL){
        fprintf(stderr, "Cannot get enough memory to store group list -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
    gle->groupId = groupId;
    gle->groupParameter = groupParameter;
    gle->anyParameterAllowed = anyParameterAllowed;
    
    gle->frequency = 1;
    
    gle->smaller = gle->greater = NULL;
    
    return gle;
}

GROUPLIST *addToGroupList(GROUPLIST *list, int groupId, int groupParameter, boolean anyParameterAllowed){
    if(list == NULL){
        return newGroupListElement(groupId, groupParameter, anyParameterAllowed);
    } else {
        GLE *el = list;
        
        int comparison = compareGroups(el->groupId, el->groupParameter, el->anyParameterAllowed,
                groupId, groupParameter, anyParameterAllowed);
        
        if (comparison < 0) {
            if (el->greater == NULL) {
                el->greater = newGroupListElement(groupId, groupParameter, anyParameterAllowed);
            } else {
                addToGroupList(el->greater, groupId, groupParameter, anyParameterAllowed);
            }
        } else if (comparison > 0) {
            if (el->smaller == NULL) {
                el->smaller = newGroupListElement(groupId, groupParameter, anyParameterAllowed);
            } else {
                addToGroupList(el->smaller, groupId, groupParameter, anyParameterAllowed);
            }
        } else {
            el->frequency++;
        }
        
        return list;
    }
}

boolean containsGroup(GROUPLIST *list, int groupId, int groupParameter, boolean anyParameterAllowed){
    GLE* el = list;
    
    while (el != NULL) {
        int comparison = compareGroups(el->groupId, el->groupParameter, el->anyParameterAllowed,
                groupId, groupParameter, anyParameterAllowed);
        
        if (comparison == 0) {
            return TRUE;
        } else if (comparison < 0) {
            el = el->greater;
        } else {
            el = el->smaller;
        }
    }
    
    //reached a NULL without finding the group
    return FALSE;
}

boolean groupIncludedInList(GROUPLIST *list, int groupId, int groupParameter){
    GLE* el = list;
    
    while (el != NULL) {
        int comparison = groupIncludedInCategory(el->groupId, el->groupParameter,
                el->anyParameterAllowed, groupId, groupParameter);
        
        if (comparison == 0) {
            return TRUE;
        } else if (comparison < 0) {
            el = el->greater;
        } else {
            el = el->smaller;
        }
    }
    
    //reached a NULL without finding the group
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

int lengthHelper(unsigned x) {
    if(x < 10) return 1;
    if(x < 100) return 2;
    if(x < 1000) return 3;
    if(x < 10000) return 4;
    if(x < 100000) return 5;
    if(x < 1000000) return 6;
    if(x < 10000000) return 7;
    if(x < 100000000) return 8;
    if(x < 1000000000) return 9;
    return 10;
}

int printingLengthOfInteger(int x) {
    return x<0 ? lengthHelper(-x)+1 : lengthHelper(x);
}

void printGroupName(FILE *f, int groupId, int groupParameter, boolean anyParameterAllowed, int minimumLength){
    int currentLength;
    if(groupId==UNKNOWN){
        fprintf(f, "UNKNOWN");
        currentLength = 7;
    } else if(groupId==Cn__){
        if(anyParameterAllowed){
            fprintf(f, "C*");
            currentLength = 2;
        } else {
            fprintf(f, "C%d", groupParameter);
            currentLength = 1 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==Cnh__){
        if(anyParameterAllowed){
            fprintf(f, "C*h");
            currentLength = 3;
        } else {
            fprintf(f, "C%dh", groupParameter);
            currentLength = 2 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==Cnv__){
        if(anyParameterAllowed){
            fprintf(f, "C*v");
            currentLength = 3;
        } else {
            fprintf(f, "C%dv", groupParameter);
            currentLength = 2 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==S2n__){
        if(anyParameterAllowed){
            fprintf(f, "S*");
            currentLength = 2;
        } else {
            fprintf(f, "S%d", 2*groupParameter);
            currentLength = 1 + printingLengthOfInteger(2*groupParameter);
        }
    } else if(groupId==Dn__){
        if(anyParameterAllowed){
            fprintf(f, "D*");
            currentLength = 2;
        } else {
            fprintf(f, "D%d", groupParameter);
            currentLength = 1 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==Dnh__){
        if(anyParameterAllowed){
            fprintf(f, "D*h");
            currentLength = 3;
        } else {
            fprintf(f, "D%dh", groupParameter);
            currentLength = 2 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==Dnd__){
        if(anyParameterAllowed){
            fprintf(f, "D*d");
            currentLength = 3;
        } else {
            fprintf(f, "D%dd", groupParameter);
            currentLength = 2 + printingLengthOfInteger(groupParameter);
        }
    } else if(groupId==T__){
        fprintf(f, "T");
        currentLength = 1;
    } else if(groupId==Td__){
        fprintf(f, "Td");
        currentLength = 2;
    } else if(groupId==Th__){
        fprintf(f, "Th");
        currentLength = 2;
    } else if(groupId==O__){
        fprintf(f, "O");
        currentLength = 1;
    } else if(groupId==Oh__){
        fprintf(f, "Oh");
        currentLength = 2;
    } else if(groupId==I__){
        fprintf(f, "I");
        currentLength = 1;
    } else if(groupId==Ih__){
        fprintf(f, "Ih");
        currentLength = 2;
    } else {
        fprintf(stderr, "Illegal group id: %d -- exiting!\n", groupId);
        exit(EXIT_FAILURE);
    }
    int i;
    for(i = currentLength; i < minimumLength; i++) fprintf(f, " ");
}

void printItem(FILE *f, GROUPLIST *listElement, int minimumGroupNameLength){
    printGroupName(f, listElement->groupId, listElement->groupParameter, 
            listElement->anyParameterAllowed, minimumGroupNameLength);
    fprintf(f, "\n");
}

void printItemFrequency(FILE *f, GROUPLIST *listElement, int minimumGroupNameLength){
    printGroupName(f, listElement->groupId, listElement->groupParameter, 
            listElement->anyParameterAllowed, minimumGroupNameLength);
    fprintf(f, " %*d\n", minimumGroupNameLength, listElement->frequency);
}

void printGroupList(FILE *f, GROUPLIST *list, int minimumGroupNameLength, 
        void (*itemPrinter)(FILE *file, GROUPLIST *item, int l)){
    if(list!=NULL){
        printGroupList(f, list->smaller, minimumGroupNameLength, itemPrinter);
        itemPrinter(f, list, minimumGroupNameLength);
        printGroupList(f, list->greater, minimumGroupNameLength, itemPrinter);
    }
}

//////////////////////////////////////////////////////////////////////////////

int certificate[MAXE+MAXN];
int canonicalLabelling[MAXN];
int reverseCanonicalLabelling[MAXN];
EDGE *canonicalFirstedge[MAXN];
int alternateLabelling[MAXN];
EDGE *alternateFirstedge[MAXN];
int queue[MAXN];
boolean hasChiralGroup;

EDGE *orientationPreservingStartingEdges[MAXE];
EDGE *orientationReversingStartingEdges[MAXE];
int startingEdgesCount; //the number of starting edges is always the same for both orientations

void findStartingEdges(){
    int i, startingDegree, startingFaceSize, minimumFrequency;
    int degreeFrequency[MAXN] = {0};
    int faceSizeFrequency[MAXN] = {0};
    EDGE *start, *edge;
    
    startingEdgesCount = 0;
    
    //build the degree frequency table
    for(i = 0; i < nv; i++){
        degreeFrequency[degree[i]]++;
    }
    
    //find the smallest degree with the lowest frequency
    minimumFrequency = MAXN;
    for(i = 0; i < MAXN; i++){
        if(degreeFrequency[i] && degreeFrequency[i] < minimumFrequency){
            startingDegree = i;
            minimumFrequency = degreeFrequency[i];
        }
    }
    
    //build the frequency table of face sizes incident to a vertex with startingDegree
    for(i = 0; i < nv; i++){
        if(degree[i] == startingDegree){
            start = edge = firstedge[i];
            
            do {
                faceSizeFrequency[faceSize[edge->rightface]]++;
                edge = edge->next;
            } while (start != edge);
        }
    }
    
    //find the smallest face size incident with a vertex of startingDegree and with smallest frequency
    minimumFrequency = MAXE;
    for(i = 0; i < MAXN; i++){
        if(faceSizeFrequency[i] && faceSizeFrequency[i] < minimumFrequency){
            startingFaceSize = i;
            minimumFrequency = faceSizeFrequency[i];
        }
    }
    
    //store all starting edges
    for(i = 0; i < nv; i++){
        if(degree[i] == startingDegree){
            start = edge = firstedge[i];
            
            do {
                if(faceSize[edge->rightface] == startingFaceSize){
                    orientationPreservingStartingEdges[startingEdgesCount] = edge;
                    orientationReversingStartingEdges[startingEdgesCount] = edge->next;
                    startingEdgesCount++;
                }
                edge = edge->next;
            } while (start != edge);
        }
    }
}

void constructCertificate(EDGE *eStart){
    int i;
    for(i=0; i<MAXN; i++){
        canonicalLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int position = 0;
    queue[0] = eStart->start;
    canonicalFirstedge[eStart->start] = eStart;
    canonicalLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = queue[tail++];
        e = elast = canonicalFirstedge[currentVertex];
        do {
            if(canonicalLabelling[e->end]==MAXN){
                queue[head++] = e->end;
                canonicalLabelling[e->end] = vertexCounter++;
                canonicalFirstedge[e->end] = e->inverse;
            }
            certificate[position++] = canonicalLabelling[e->end];
            e = e->next;
        } while (e!=elast);
        certificate[position++] = MAXN;
    }
    for(i = 0; i < nv; i++){
        reverseCanonicalLabelling[canonicalLabelling[i]] = i;
    }
}

void constructCertificateOrientationReversed(EDGE *eStart){
    int i;
    for(i=0; i<MAXN; i++){
        canonicalLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int position = 0;
    queue[0] = eStart->start;
    canonicalFirstedge[eStart->start] = eStart;
    canonicalLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = queue[tail++];
        e = elast = canonicalFirstedge[currentVertex];
        do {
            if(canonicalLabelling[e->end]==MAXN){
                queue[head++] = e->end;
                canonicalLabelling[e->end] = vertexCounter++;
                canonicalFirstedge[e->end] = e->inverse;
            }
            certificate[position++] = canonicalLabelling[e->end];
            e = e->prev;
        } while (e!=elast);
        certificate[position++] = MAXN;
    }
    for(i = 0; i < nv; i++){
       reverseCanonicalLabelling[canonicalLabelling[i]] = i;
    }
}

/* returns 1 if this edge leads to a better certificate
 * returns 0 if this edge leads to the same certificate
 * returns -1 if this edge leads to a worse certificate
 */
int hasBetterCertificateOrientationPreserving(EDGE *eStart){
    int i, j;
    for(i=0; i<MAXN; i++){
        alternateLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int currentPos = 0;
    queue[0] = eStart->start;
    alternateFirstedge[eStart->start] = eStart;
    alternateLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = queue[tail++];
        e = elast = alternateFirstedge[currentVertex];
        do {
            if(alternateLabelling[e->end]==MAXN){
                queue[head++] = e->end;
                alternateLabelling[e->end] = vertexCounter++;
                alternateFirstedge[e->end] = e->inverse;
            }
            if(alternateLabelling[e->end] < certificate[currentPos]){
                constructCertificate(eStart);
                automorphismsCount = 1;
                orientationPreservingAutomorphismsCount = 1;
                return 1;
            } else if(alternateLabelling[e->end] > certificate[currentPos]){
                return -1;
            }
            currentPos++;
            e = e->next;
        } while (e!=elast);
        //MAXN will always be at least the value of certificate[currentPos]
        if(MAXN > certificate[currentPos]){
            return -1;
        }
        currentPos++;
    }
    for(j = 0; j < nv; j++){
        automorphisms[automorphismsCount][j] 
                = reverseCanonicalLabelling[alternateLabelling[j]];
    }
    automorphismsCount++;
    orientationPreservingAutomorphismsCount++;
    return 0;
}

int hasBetterCertificateOrientationReversing(EDGE *eStart){
    int i, j;
    for(i=0; i<MAXN; i++){
        alternateLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int currentPos = 0;
    queue[0] = eStart->start;
    alternateFirstedge[eStart->start] = eStart;
    alternateLabelling[eStart->start] = 0;
    while(head>tail){
        int currentVertex = queue[tail++];
        e = elast = alternateFirstedge[currentVertex];
        do {
            if(alternateLabelling[e->end]==MAXN){
                queue[head++] = e->end;
                alternateLabelling[e->end] = vertexCounter++;
                alternateFirstedge[e->end] = e->inverse;
            }
            if(alternateLabelling[e->end] < certificate[currentPos]){
                constructCertificateOrientationReversed(eStart);
                hasChiralGroup = TRUE;
                automorphismsCount = 1;
                orientationPreservingAutomorphismsCount = 1;
                orientationReversingAutomorphismsCount = 0;
                return 1;
            } else if(alternateLabelling[e->end] > certificate[currentPos]){
                return -1;
            }
            currentPos++;
            e = e->prev;
        } while (e!=elast);
        //MAXN will always be at least the value of certificate[currentPos]
        if(MAXN > certificate[currentPos]){
            return -1;
        }
        currentPos++;
    }
    for(j = 0; j < nv; j++){
        automorphisms[automorphismsCount][j] 
                = reverseCanonicalLabelling[alternateLabelling[j]];
    }
    if(hasChiralGroup){
        orientationPreservingAutomorphismsCount++;
    } else {
        orientationReversingAutomorphismsCount++;
    }
    automorphismsCount++;
    return 0;
}

void determineAutomorphisms(){
    int i;
    
    hasChiralGroup = FALSE;
    
    //identity
    for(i = 0; i < nv; i++){
        automorphisms[0][i] = i;
    }
    
    automorphismsCount = 1;
    orientationPreservingAutomorphismsCount = 1;
    orientationReversingAutomorphismsCount = 0;
    
    //find starting edges
    findStartingEdges();
    
    //construct initial certificate
    constructCertificate(orientationPreservingStartingEdges[0]);
    
    //look for better automorphism
    for(i = 1; i < startingEdgesCount; i++){
        int result = hasBetterCertificateOrientationPreserving(
                                orientationPreservingStartingEdges[i]);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
    for(i = 0; i < startingEdgesCount; i++){
        int result = hasBetterCertificateOrientationReversing(
                                orientationReversingStartingEdges[i]);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
}

boolean hasOrientationPreservingSymmetryWithGivenAction(int v, int w, int vImage, int wImage){
    int i = 0;
    while(i < orientationPreservingAutomorphismsCount){
        if(automorphisms[i][v] == vImage && automorphisms[i][w] == wImage){
            return TRUE;
        }
        i++;
    }
    
    return FALSE;
}

int identifyRotationalSymmetryThroughVertex(int v){
    EDGE *edge;
    int deg, neighbour, i;
    
    deg = degree[v];
    edge = firstedge[v];
    i = 0;
    neighbour = edge->end;
    
    while(i < deg/2){
        i++;
        edge = edge->next;
        if(hasOrientationPreservingSymmetryWithGivenAction(v, neighbour, v, edge->end)){
            return deg/i;
        }
    }
    
    return 1;
}

int identifyRotationalSymmetryThroughFace(int f){
    EDGE *edge;
    int deg, i, v, w;
    
    deg = faceSize[f];
    edge = facestart[f];
    i = 0;
    v = edge->start;
    w = edge->end;
    
    while(i < deg/2){
        i++;
        edge = edge->next->inverse;
        if(hasOrientationPreservingSymmetryWithGivenAction(v, w, edge->start, edge->end)){
            return deg/i;
        }
    }
    
    return 1;
}

boolean hasRotationalSymmetryThroughEdge(EDGE *e){
    return hasOrientationPreservingSymmetryWithGivenAction(e->start, e->end, e->end, e->start);
}

/* Returns TRUE if the graph contains a mirror symmetry fixating the given vertex.
 * This method assumes that the automorphism group is already determined.
 */
boolean hasOrientationReversingSymmetryStabilisingGivenVertex(int v){
    int i;
    
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        if(automorphisms[i][v] == v){
            return TRUE;
        }
    }
    
    return FALSE;
}

/* Returns TRUE if the graph contains a mirror symmetry fixating the given face.
 * This method assumes that the automorphism group is already determined.
 */
boolean hasOrientationReversingSymmetryStabilisingGivenFace(int f){
    int i, from, to;
    EDGE *e;
    
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        e = facestart[f];
        from = automorphisms[i][e->start];
        to = automorphisms[i][e->end];
        e = firstedge[from];
        while(e->end != to) {
            e = e->next;
        }
        if(e->inverse->rightface == f){
            return TRUE;
        }
    }
    
    return FALSE;
}

/* Returns TRUE if the graph contains a mirror symmetry fixating the given edge.
 * This method assumes that the automorphism group is already determined.
 */
boolean hasOrientationReversingSymmetryStabilisingGivenEdge(int e){
    int i, from, to;
    EDGE *edge = edges+e;
    
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        from = automorphisms[i][edge->start];
        to = automorphisms[i][edge->end];
        if((edge->start == from && edge->end == to) ||
                (edge->start == to && edge->end == from)){
            return TRUE;
        }
    }
    
    return FALSE;
}

boolean hasOrientationReversingSymmetryWithFixPoint(){
    int i, j;
    
    //first check if any vertices are fixed
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        for(j = 0; j < nv; j++){
            if(automorphisms[i][j] == j){
                return TRUE;
            }
        }
    }
    
    //next we check if any edge is fixed as a set
    //we don't need to check that a directed edge is fixed
    //because in that case also the vertices are fixed
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        for(j = 0; j < ne; j++){
            if(j < edges[j].inverse->index){
                int start = edges[j].start;
                int end = edges[j].end;
                if(automorphisms[i][start] == end &&
                        automorphisms[i][end] == start){
                    return TRUE;
                }
            }
        }
    }
    
    //we don't need to check if there is a fixpoint in a face
    //because in that case at least either a vertex or an edge
    //is fixed
    return FALSE;
}

boolean isSymmetryWithFixPoint(int i){
    int j;
        
    //first check if any vertices are fixed
    for(j = 0; j < nv; j++){
        if(automorphisms[i][j] == j){
            return TRUE;
        }
    }
    
    //next we check if any edge is fixed as a set
    //we don't need to check that a directed edge is fixed
    //because in that case also the vertices are fixed
    for(j = 0; j < ne; j++){
        if(j < edges[j].inverse->index){
            int start = edges[j].start;
            int end = edges[j].end;
            if(automorphisms[i][start] == end &&
                    automorphisms[i][end] == start){
                return TRUE;
            }
        }
    }
    
    //we don't need to check if there is a fixpoint in a face
    //because in that case at least either a vertex or an edge
    //is fixed
    return FALSE;
}

int countOrientationReversingSymmetriesWithFixPoints(){
    int i, count;
    
    count = 0;
    
    //first check if any vertices are fixed
    for(i = orientationPreservingAutomorphismsCount; i < automorphismsCount; i++){
        if(isSymmetryWithFixPoint(i)){
            count++;
        }
    }
    
    return count;
}

void determineAutomorphismGroupInfiniteFamilies(int *groupId,
        int *groupParameter, int rotDegree, int center, boolean centerIsVertex,
        boolean centerIsEdge){
    if(orientationReversingAutomorphismsCount==0){
        if(automorphismsCount==rotDegree){
            *groupId = Cn__;
        } else if(automorphismsCount==2*rotDegree){
            *groupId = Dn__;
        } else {
            fprintf(stderr, "Illegal order for chiral axial symmetry group containing a %d-fold rotation: %d -- exiting!\n", rotDegree, automorphismsCount);
            exit(EXIT_FAILURE);
        }
        *groupParameter = rotDegree;
    } else if(automorphismsCount == 4*rotDegree){
        int orientationReversingSymmetriesWithFixPoints = 
                      countOrientationReversingSymmetriesWithFixPoints();
        if(orientationReversingSymmetriesWithFixPoints == rotDegree){
            *groupId = Dnd__;
            *groupParameter = rotDegree;
        } else if(orientationReversingSymmetriesWithFixPoints == rotDegree + 1){
            *groupId = Dnh__;
            *groupParameter = rotDegree;
        } else {
            fprintf(stderr, "Illegal number of orientation reversing automorphisms with fixpoints for chiral axial symmetry group containing a %d-fold rotation that has order %d: %d -- exiting!\n", rotDegree, automorphismsCount, orientationReversingSymmetriesWithFixPoints);
            exit(EXIT_FAILURE);
        }
    } else if(automorphismsCount == 2*rotDegree){
        if(centerIsVertex){
            if(hasOrientationReversingSymmetryStabilisingGivenVertex(center)){
                *groupId = Cnv__;
                *groupParameter = rotDegree;
                return;
            }
        } else if(centerIsEdge){
            if(hasOrientationReversingSymmetryStabilisingGivenEdge(center)){
                *groupId = Cnv__;
                *groupParameter = rotDegree;
                return;
            }
        } else {
            if(hasOrientationReversingSymmetryStabilisingGivenFace(center)){
                *groupId = Cnv__;
                *groupParameter = rotDegree;
                return;
            }
        }
        //if the rotational center is not fixed by any orientation reversing symmetry,
        //we check if there is any fix point
        if(hasOrientationReversingSymmetryWithFixPoint()){
            *groupId = Cnh__;
            *groupParameter = rotDegree;
        } else {
            *groupId = S2n__;
            *groupParameter = rotDegree;
        }
    } else {
        fprintf(stderr, "Illegal order for achiral axial symmetry group containing a %d-fold rotation: %d -- exiting!\n", rotDegree, automorphismsCount);
        exit(EXIT_FAILURE);
    }
}

void determineAutomorphismGroupOther(int *groupId, int *groupParameter){
    //most of these groups can be identified based on the size of the group
    //or the size of the group combined with the chirality
    //the only exceptions are Td and Th. These have the same size and the same
    //chirality.
    if(automorphismsCount==120){
        *groupId = Ih__;
    } else if(automorphismsCount==60){
        *groupId = I__;
    } else if(automorphismsCount==48){
        *groupId = Oh__;
    } else if(automorphismsCount==24){
        if(orientationReversingAutomorphismsCount==0){
            *groupId = O__;
        } else {
            int orientationReversingSymmetriesWithFixPoints = 
                      countOrientationReversingSymmetriesWithFixPoints();
            if(orientationReversingSymmetriesWithFixPoints == 6){
                *groupId = Td__;
            } else if(orientationReversingSymmetriesWithFixPoints == 3){
                *groupId = Th__;
            } else {
                fprintf(stderr, "Illegal number of orientation reversing automorphisms with fixpoints for achiral non-axial symmetry group with order 24: %d -- exiting!\n", orientationReversingSymmetriesWithFixPoints);
                exit(EXIT_FAILURE);
            }
        }
    } else if(automorphismsCount==12){
        *groupId = T__;
    } else {
        fprintf(stderr, "Illegal order for a non-axial symmetry group -- exiting!\n");
        exit(EXIT_FAILURE);
    }
}

void determineAutomorphismGroup(int *groupId, int *groupParameter){
    int i, j;
    int maxRotation, maxRotationCenter;
    boolean maxRotationCenterIsVertex, maxRotationCenterIsEdge;
    maxRotation = -1;
    
    //we start by determining all automorphisms
    determineAutomorphisms();
    
    //if the group is chiral and has an odd order or an order less than 4
    //then it is a cyclic group. (since D1 is equal to C2)
    if(orientationReversingAutomorphismsCount == 0 && 
            (automorphismsCount < 4 || automorphismsCount%2 == 1)){
        *groupId = Cn__;
        *groupParameter = automorphismsCount;
        return;
    } else if(orientationReversingAutomorphismsCount == 1 && automorphismsCount == 2){
        if(hasOrientationReversingSymmetryWithFixPoint()){
            *groupId = Cnh__;
            *groupParameter = 1;
        } else {
            *groupId = S2n__;
            *groupParameter = 1;
        }
        return;
    }
    
    //next we will count the number of rotational axis
    //each axis gets counted twice
    
    //first we set the counts to 0
    int foldCount[6];
    for(i = 0; i < 6; i++){
        foldCount[i] = 0;
    }
    
    //first we look for rotational axis through a vertex
    for(i = 0; i < nv; i++){
        int rotDegree = identifyRotationalSymmetryThroughVertex(i);
        
        //if the order of the rotation is larger than 5, then we known that it 
        //has to be one of the axial symmetry groups
        if(rotDegree>5){
            determineAutomorphismGroupInfiniteFamilies(
                    groupId, groupParameter, rotDegree, i, TRUE, FALSE);
            return;
        }
        //otherwise we just count the axis
        foldCount[rotDegree]++;
        
        //store the maxRotationCenter
        if(rotDegree > maxRotation){
            maxRotation = rotDegree;
            maxRotationCenter = i;
            maxRotationCenterIsEdge = FALSE;
            maxRotationCenterIsVertex = TRUE;
        }
        
        //if there is more than one 3-fold rotational axis, then it can't be
        //one of the axial symmetry groups
        if(foldCount[3] > 2){
            determineAutomorphismGroupOther(groupId, groupParameter);
            return;
        }
        
        //if there is more than one type of n-fold rotational axis with n at
        //least 3, then it can't be one of the axial symmetry groups
        int rotTypeCount = 0;
        for(j = 3; j < 6; j++){
            if(foldCount[j]){
                rotTypeCount++;
            }
        }
        if(rotTypeCount>1){
            determineAutomorphismGroupOther(groupId, groupParameter);
            return;
        }
    }
    
    for(i = 0; i < nf; i++){
        int rotDegree = identifyRotationalSymmetryThroughFace(i);
        
        //if the order of the rotation is larger than 5, then we known that it 
        //has to be one of the axial symmetry groups
        if(rotDegree>5){
            determineAutomorphismGroupInfiniteFamilies(
                    groupId, groupParameter, rotDegree, i, FALSE, FALSE);
            return;
        }
        foldCount[rotDegree]++;
        
        //store the maxRotationCenter
        if(rotDegree > maxRotation){
            maxRotation = rotDegree;
            maxRotationCenter = i;
            maxRotationCenterIsEdge = FALSE;
            maxRotationCenterIsVertex = FALSE;
        }
        
        //if there is more than one 3-fold rotational axis, then it can't be
        //one of the axial symmetry groups
        if(foldCount[3] > 2){
            determineAutomorphismGroupOther(groupId, groupParameter);
            return;
        }
        
        //if there is more than one type of n-fold rotational axis with n at
        //least 3, then it can't be one of the axial symmetry groups
        int rotTypeCount = 0;
        for(j = 3; j < 6; j++){
            if(foldCount[j]){
                rotTypeCount++;
            }
        }
        if(rotTypeCount>1){
            determineAutomorphismGroupOther(groupId, groupParameter);
            return;
        }
    }
    
    if(foldCount[3]<=2){
        //first we also check all edges, so that we also have all 2-fold rotations
        for(i = 0; i < ne; i++){
            if(i < edges[i].inverse->index 
                    && hasRotationalSymmetryThroughEdge(edges+i)){
                foldCount[2]++;
                
                //store the maxRotationCenter
                if(2 > maxRotation){
                    maxRotation = 2;
                    maxRotationCenter = i;
                    maxRotationCenterIsEdge = TRUE;
                    maxRotationCenterIsVertex = FALSE;
                }
            }
        }
        determineAutomorphismGroupInfiniteFamilies(
                groupId, groupParameter, maxRotation, maxRotationCenter,
                maxRotationCenterIsVertex, maxRotationCenterIsEdge);
    } else {
        //shouldn't happen
        determineAutomorphismGroupOther(groupId, groupParameter);
    }
}

//////////////////////////////////////////////////////////////////////////////

//=============== Writing planarcode of graph ===========================

void writePlanarCodeChar(){
    int i;
    EDGE *e, *elast;
    
    //write the number of vertices
    fputc(nv, stdout);
    
    for(i=0; i<nv; i++){
        e = elast = firstedge[i];
        do {
            fputc(e->end + 1, stdout);
            e = e->next;
        } while (e != elast);
        fputc(0, stdout);
    }
}

void writeShort(unsigned short value){
    if (fwrite(&value, sizeof (unsigned short), 1, stdout) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }
}

void writePlanarCodeShort(){
    int i;
    EDGE *e, *elast;
    
    //write the number of vertices
    fputc(0, stdout);
    writeShort(nv);
    
    
    for(i=0; i<nv; i++){
        e = elast = firstedge[i];
        do {
            writeShort(e->end + 1);
            e = e->next;
        } while (e != elast);
        writeShort(0);
    }
}

void writePlanarCode(){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>planar_code<<");
    }
    
    if (nv + 1 <= 255) {
        writePlanarCodeChar();
    } else if (nv + 1 <= 65535) {
        writePlanarCodeShort();
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
    
}

//=============== Reading and decoding planarcode ===========================

EDGE *findEdge(int from, int to) {
    EDGE *e, *elast;

    e = elast = firstedge[from];
    do {
        if (e->end == to) {
            return e;
        }
        e = e->next;
    } while (e != elast);
    fprintf(stderr, "error while looking for edge from %d to %d.\n", from, to);
    exit(0);
}

/* Store in the rightface field of each edge the number of the face on
   the right hand side of that edge.  Faces are numbered 0,1,....  Also
   store in facestart[i] an example of an edge in the clockwise orientation
   of the face boundary, and the size of the face in facesize[i], for each i.
   Returns the number of faces. */
void makeDual() {
    register int i, sz;
    register EDGE *e, *ex, *ef, *efx;

    RESETMARKS;

    nf = 0;
    for (i = 0; i < nv; ++i) {

        e = ex = firstedge[i];
        do {
            if (!ISMARKEDLO(e)) {
                facestart[nf] = ef = efx = e;
                sz = 0;
                do {
                    ef->rightface = nf;
                    MARKLO(ef);
                    ef = ef->inverse->prev;
                    ++sz;
                } while (ef != efx);
                faceSize[nf] = sz;
                ++nf;
            }
            e = e->next;
        } while (e != ex);
    }
}

void decodePlanarCode(unsigned short* code) {
    /* complexity of method to determine inverse isn't that good, but will have to satisfy for now
     */
    int i, j, codePosition;
    int edgeCounter = 0;
    EDGE *inverse;

    nv = code[0];
    codePosition = 1;

    for (i = 0; i < nv; i++) {
        degree[i] = 0;
        firstedge[i] = edges + edgeCounter;
        edges[edgeCounter].start = i;
        edges[edgeCounter].end = code[codePosition] - 1;
        edges[edgeCounter].next = edges + edgeCounter + 1;
        if (code[codePosition] - 1 < i) {
            inverse = findEdge(code[codePosition] - 1, i);
            edges[edgeCounter].inverse = inverse;
            inverse->inverse = edges + edgeCounter;
        } else {
            edges[edgeCounter].inverse = NULL;
        }
        edges[edgeCounter].index = edgeCounter;
        edgeCounter++;
        codePosition++;
        for (j = 1; code[codePosition]; j++, codePosition++) {
            if (j == MAXVAL) {
                fprintf(stderr, "MAXVAL too small: %d\n", MAXVAL);
                exit(0);
            }
            edges[edgeCounter].start = i;
            edges[edgeCounter].end = code[codePosition] - 1;
            edges[edgeCounter].prev = edges + edgeCounter - 1;
            edges[edgeCounter].next = edges + edgeCounter + 1;
            if (code[codePosition] - 1 < i) {
                inverse = findEdge(code[codePosition] - 1, i);
                edges[edgeCounter].inverse = inverse;
                inverse->inverse = edges + edgeCounter;
            } else {
                edges[edgeCounter].inverse = NULL;
            }
            edges[edgeCounter].index = edgeCounter;
            edgeCounter++;
        }
        firstedge[i]->prev = edges + edgeCounter - 1;
        edges[edgeCounter - 1].next = firstedge[i];
        degree[i] = j;

        codePosition++; /* read the closing 0 */
    }

    ne = edgeCounter;

    makeDual();

    // nv - ne/2 + nf = 2
}

/**
 * 
 * @param code
 * @param laenge
 * @param file
 * @return returns 1 if a code was read and 0 otherwise. Exits in case of error.
 */
int readPlanarCode(unsigned short code[], int *length, FILE *file) {
    static int first = 1;
    unsigned char c;
    char testheader[20];
    int bufferSize, zeroCounter;
    
    int readCount;


    if (first) {
        first = 0;

        if (fread(&testheader, sizeof (unsigned char), 13, file) != 13) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[13] = 0;
        if (strcmp(testheader, ">>planar_code") == 0) {

        } else {
            fprintf(stderr, "No planarcode header detected -- exiting!\n");
            exit(1);
        }
        //read reminder of header (either empty or le/be specification)
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            return FALSE;
        }
        while (c!='<'){
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                return FALSE;
            }
        }
        //read one more character
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            return FALSE;
        }
    }

    /* possibly removing interior headers -- only done for planarcode */
    if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
        //nothing left in file
        return (0);
    }

    if (c == '>') {
        // could be a header, or maybe just a 62 (which is also possible for unsigned char
        code[0] = c;
        bufferSize = 1;
        zeroCounter = 0;
        code[1] = (unsigned short) getc(file);
        if (code[1] == 0) zeroCounter++;
        code[2] = (unsigned short) getc(file);
        if (code[2] == 0) zeroCounter++;
        bufferSize = 3;
        // 3 characters were read and stored in buffer
        if ((code[1] == '>') && (code[2] == 'p')) /*we are sure that we're dealing with a header*/ {
            while ((c = getc(file)) != '<');
            /* read 2 more characters: */
            c = getc(file);
            if (c != '<') {
                fprintf(stderr, "Problems with header -- single '<'\n");
                exit(1);
            }
            if (!fread(&c, sizeof (unsigned char), 1, file)) {
                //nothing left in file
                return (0);
            }
            bufferSize = 1;
            zeroCounter = 0;
        }
    } else {
        //no header present
        bufferSize = 1;
        zeroCounter = 0;
    }

    if (c != 0) /* unsigned chars would be sufficient */ {
        code[0] = c;
        if (code[0] > MAXN) {
            fprintf(stderr, "Constant N too small %d > %d \n", code[0], MAXN);
            exit(1);
        }
        while (zeroCounter < code[0]) {
            code[bufferSize] = (unsigned short) getc(file);
            if (code[bufferSize] == 0) zeroCounter++;
            bufferSize++;
        }
    } else {
        readCount = fread(code, sizeof (unsigned short), 1, file);
        if(!readCount){
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
        if (code[0] > MAXN) {
            fprintf(stderr, "Constant N too small %d > %d \n", code[0], MAXN);
            exit(1);
        }
        bufferSize = 1;
        zeroCounter = 0;
        while (zeroCounter < code[0]) {
            readCount = fread(code + bufferSize, sizeof (unsigned short), 1, file);
            if(!readCount){
                fprintf(stderr, "Unexpected EOF.\n");
                exit(1);
            }
            if (code[bufferSize] == 0) zeroCounter++;
            bufferSize++;
        }
    }

    *length = bufferSize;
    return (1);


}

//================== PARSE GROUP NAME ================

int parseGroupParameter(char* input, int* groupParameter, boolean *anyParameterAllowed){
    if (input[0] == '*'){
        *anyParameterAllowed = TRUE;
        return 1;
    } else if (isdigit(input[0])){
        int length = 1;
        while(isdigit(input[length])) length++;
        *groupParameter = atoi(input);
        *anyParameterAllowed = FALSE;
        return length;
    } else {
        fprintf(stderr, "Illegal group parameter: %s -- exiting!\n", input);
        exit(EXIT_FAILURE);
    }
}

void parseGroup(char* input, int* groupId, int* groupParameter, boolean* anyParameterAllowed){
    if(strcmp(input, "Cs")==0){
        //Cs is equal to C1h
        *groupId = Cnh__;
        *groupParameter = 1;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced Cs by C1h\n");
    } else if(strcmp(input, "C1v")==0){
        //C1v is equal to C1h
        *groupId = Cnh__;
        *groupParameter = 1;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced C1v by C1h\n");
    } else if(strcmp(input, "Ci")==0){
        //Ci is equal to S2
        *groupId = S2n__;
        *groupParameter = 1;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced Ci by S2\n");
    } else if(strcmp(input, "D1")==0){
        //D1 is equal to C2
        *groupId = Cn__;
        *groupParameter = 2;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced D1 by C2\n");
    } else if(strcmp(input, "D1h")==0){
        //D1h is equal to C2v
        *groupId = Cnv__;
        *groupParameter = 2;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced D1h by C2v\n");
    } else if(strcmp(input, "D1d")==0){
        //D1d is equal to C2h
        *groupId = Cnh__;
        *groupParameter = 2;
        *anyParameterAllowed = FALSE;
        fprintf(stderr, "Replaced D1d by C2h\n");
    } else if (input[0] == 'T'){
        if (input[1] == '\0') {
            *groupId = T__;
        } else if (input[2] != '\0') {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        } else if (input[1] == 'd') {
            *groupId = Td__;
        } else if (input[1] == 'h') {
            *groupId = Th__;
        } else {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        }
    } else if (input[0] == 'O') {
        if (input[1] == '\0') {
            *groupId = O__;
        } else if (input[2] != '\0') {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        } else if (input[1] == 'h') {
            *groupId = Oh__;
        } else {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        }
    } else if (input[0] == 'I') {
        if (input[1] == '\0') {
            *groupId = I__;
        } else if (input[2] != '\0') {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        } else if (input[1] == 'h') {
            *groupId = Ih__;
        } else {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        }
    } else if (input[0] == 'C') {
        int length = parseGroupParameter(input + 1, groupParameter, anyParameterAllowed);
        if (!(*anyParameterAllowed) && (*groupParameter <= 0)) {
            fprintf(stderr, "Illegal group parameter: %d -- exiting!\n", *groupParameter);
            exit(EXIT_FAILURE);
        } else if (input[length + 1] == '\0') {
            *groupId = Cn__;
        } else if (input[length + 1] == 'h' && input[length + 2] == '\0') {
            *groupId = Cnh__;
        } else if (input[length + 1] == 'v' && input[length + 2] == '\0') {
            *groupId = Cnv__;
        } else {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        }
    } else if (input[0] == 'S') {
       int length = parseGroupParameter(input + 1, groupParameter, anyParameterAllowed);
       if (input[length + 1] != '\0') {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
       } else if (!(*anyParameterAllowed) && 
               (*groupParameter <= 0 || (*groupParameter)%2 == 1)) {
            fprintf(stderr, "Illegal group parameter: %d -- exiting!\n", *groupParameter);
            exit(EXIT_FAILURE);
       } else {
           *groupId = S2n__;
           (*groupParameter) /= 2;
       }
    } else if (input[0] == 'D') {
        int length = parseGroupParameter(input + 1, groupParameter, anyParameterAllowed);
        if (!(*anyParameterAllowed) && (*groupParameter <= 0)) {
            fprintf(stderr, "Illegal group parameter: %d -- exiting!\n", *groupParameter);
            exit(EXIT_FAILURE);
        } else if (input[length + 1] == '\0') {
            *groupId = Dn__;
        } else if (input[length + 1] == 'h' && input[length + 2] == '\0') {
            *groupId = Dnh__;
        } else if (input[length + 1] == 'd' && input[length + 2] == '\0') {
            *groupId = Dnd__;
        } else if (input[length + 1] == 'v' && input[length + 2] == '\0') {
            *groupId = Dnd__;
            fprintf(stderr, "Replaced D%dv by D%dd\n", *groupParameter, *groupParameter);
        } else {
            fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Illegal group name: %s -- exiting!\n", input);
        exit(EXIT_FAILURE);
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s determines the symmetry type of plane graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       If this option is given then several group names can be given as argument\n");
    fprintf(stderr, "       and the program will filter out the graphs which have one of the specified\n");
    fprintf(stderr, "       groups as symmetry group.\n");
    fprintf(stderr, "       Valid group names are:\n");
    fprintf(stderr, "       Cn       where n is either a positive integer or *\n");
    fprintf(stderr, "       Cnh      where n is either a positive integer or *\n");
    fprintf(stderr, "       Cnv      where n is either a positive integer or *\n");
    fprintf(stderr, "       Sn       where n is either a positive even integer or *\n");
    fprintf(stderr, "       Dn       where n is either a positive integer or *\n");
    fprintf(stderr, "       Dnh      where n is either a positive integer or *\n");
    fprintf(stderr, "       Dnd      where n is either a positive integer or *\n");
    fprintf(stderr, "       T, Th, Td, O, Oh, I, Ih\n");
    fprintf(stderr, "    -i, --invert\n");
    fprintf(stderr, "       Inverts the filter, i.e., filter graphs not having one of the specified\n");
    fprintf(stderr, "       groups.\n");
    fprintf(stderr, "    -s, --summary\n");
    fprintf(stderr, "       At the end give a summary of the encountered groups.\n");
    fprintf(stderr, "    -q, --quiet\n");
    fprintf(stderr, "       Do not show group info for individual graphs.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {
    
    boolean giveSummary = FALSE;
    GROUPLIST *summary = NULL;
    boolean singleInfo = TRUE;
    boolean inverted = FALSE;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"filter", no_argument, NULL, 'f'},
        {"summary", no_argument, NULL, 's'},
        {"quiet", no_argument, NULL, 'q'},
        {"invert", no_argument, NULL, 'i'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hfsqi", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case 'f':
                filterEnabled = TRUE;
                break;
            case 's':
                giveSummary = TRUE;
                break;
            case 'q':
                singleInfo = FALSE;
                break;
            case 'i':
                inverted = TRUE;
                break;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    
    if(filterEnabled && argc - optind == 0){
        usage(name);
        return EXIT_FAILURE;
    } else if(filterEnabled){
        //parse the group names that need to be filtered
        int groupId = UNKNOWN, groupParameter = 0;
        boolean anyParameterAllowed = FALSE;
        
        int i;
        for(i = optind; i < argc; i++){
            parseGroup(argv[i], &groupId, &groupParameter, &anyParameterAllowed);
            
            filterList = addToGroupList(filterList, groupId, groupParameter, 
                    anyParameterAllowed);
        }
        
        fprintf(stderr, "Filtering out graphs that %shave one of the following groups:\n", inverted ? "do not " : "");
        printGroupList(stderr, filterList, 0, printItem);
    }

    /*=========== read planar graphs ===========*/

    unsigned short code[MAXCODELENGTH];
    int length;
    while (readPlanarCode(code, &length, stdin)) {
        decodePlanarCode(code);
        numberOfGraphs++;
        int groupId = UNKNOWN;
        int groupParameter = 0;
        determineAutomorphismGroup(&groupId, &groupParameter);
        if(filterEnabled){
            if(inverted){
                if(!groupIncludedInList(filterList, groupId, groupParameter)){
                    writePlanarCode();
                }
            } else {
                if(groupIncludedInList(filterList, groupId, groupParameter)){
                    writePlanarCode();
                }
            }
        } else if(singleInfo){
            fprintf(stderr, "Graph %d has group ", numberOfGraphs);
            printGroupName(stderr, groupId, groupParameter, FALSE, 0);
            fprintf(stderr, "\n");
        }
        if(giveSummary){
            summary = addToGroupList(summary, groupId, groupParameter, FALSE);
        }
    }

    if(giveSummary){
        printGroupList(stderr, summary, 7, printItemFrequency);
    }
}
