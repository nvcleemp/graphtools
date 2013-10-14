/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads plane graphs in planar_code format from standard in and
 * writes a tutte embedding to standard out in writegraph2d format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o tutte -O4 tutte.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <math.h>


#ifndef MAXN
#define MAXN 64            /* the maximum number of vertices */
#endif
#define MAXE (6*MAXN-12)    /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)      /* the maximum number of faces */
#define MAXVAL (MAXN-1)  /* the maximum degree of a vertex */
#define MAXCODELENGTH (MAXN+MAXE+3)

#define INFI (MAXN + 1)

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE  1

typedef struct e /* The data type used for edges */ {
    int start; /* vertex where the edge starts */
    int end; /* vertex where the edge ends */
    int rightface; /* face on the right side of the edge
                          note: only valid if make_dual() called */
    struct e *prev; /* previous edge in clockwise direction */
    struct e *next; /* next edge in clockwise direction */
    struct e *inverse; /* the edge that is inverse to this one */
    int mark, index; /* two ints for temporary use;
                          Only access mark via the MARK macros. */

    int left_facesize; /* size of the face in prev-direction of the edge.
        		  Only used for -p option. */
    double x;
    double y;
} EDGE;

typedef double COORDINATES[MAXN][2];

typedef int boolean;

EDGE *firstedge[MAXN]; /* pointer to arbitrary edge out of vertex i. */
int degree[MAXN];

EDGE *facestart[MAXF]; /* pointer to arbitrary edge of face i. */
int faceSize[MAXF]; /* pointer to arbitrary edge of face i. */

EDGE edges[MAXE];

COORDINATES coord;
COORDINATES coord2; //used as a temporary variable

int fixed[MAXN];

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

int iterations = 1000;
int outerface = -1;
int outerfaceEdgeFrom = -1;
int outerfaceEdgeTo = -1;
boolean converge = FALSE;
boolean onlyOne = FALSE;
boolean verbose = FALSE;
int iterationCount;
double precision = 1e-15;

int nv;
int ne;
int nf;


//////////////////////////////////////////////////////////////////////////////

void writeWritegraph2d(FILE *f){
    static int first = TRUE;
    int i;
    EDGE *e, *elast;
    
    if(first){
        first = FALSE;
        
        fprintf(f, ">>writegraph2d<<\n");
    }
    
    for(i = 0; i < nv; i++){
        //current vertex
        fprintf(f, "%3d ", i+1);
        
        //coordinates
        fprintf(f, "%.4f %.4f ", coord[i][0], coord[i][1]);
        
        //neighbours
        e = elast = firstedge[i];
        do {
            fprintf(f, "%3d ", e->end+1);
            e = e->next;
        } while (e != elast);
        
        //next line
        fprintf(f, "\n");
    }
    //end of graph
    fprintf(f, "0\n");
}

//////////////////////////////////////////////////////////////////////////////


int findFaceOnRightSide(int from, int to){
    EDGE *e, *elast;
    
    e = elast = firstedge[from];
    
    if(e->end == to){
        return e->rightface;
    } else {
        do {
            e = e->next;
        } while (e != elast && e->end != to);
        if(e->end == to){
            return e->rightface;
        } else {
            fprintf(stderr, "The vertices %d and %d are not adjacent -- exiting!\n", from, to);
            exit(EXIT_FAILURE);
        }
    }
}

int determineBestOuterface(){
    //just look for the largest face
    int i, maxFace, maxFaceSize = 0;
    
    for(i = 0; i < nf; i++){
        if(faceSize[i] > maxFaceSize){
            maxFaceSize = faceSize[i];
            maxFace = i;
        }
    }
    
    return maxFace;
}

void embedFace(int face){
    EDGE *e, *elast;
    int size = faceSize[face];
    int i = 0;
    int vertex;
    
    double radius = 1.0;
    //make sure that bottom edge of outer face is horizontal
    double startingAngle = (.5*(2.0-size)/size)*M_PI;
    
    e = elast = facestart[face];
    do {
        vertex = e->start;
        coord[vertex][0] = radius*cos(2*i*M_PI/size + startingAngle);
        coord[vertex][1] = radius*sin(2*i*M_PI/size + startingAngle);
        fixed[vertex] = TRUE;
        e = e->inverse->prev;
        i++;
    } while (e != elast);
}

void doTutteEmbeddingIterations(){
    int i, j;
    EDGE *e, *elast;
    double x, y;
    
    boolean noChanges = FALSE;
    
    for(i = 0; (i < iterations) && (!converge || !noChanges); i++){
        noChanges = TRUE;
        for(j = 0; j < nv; j++){
            if(!fixed[j]){
                x = y = 0.0;
                e = elast = firstedge[j];
                do {
                    x += coord[e->end][0];
                    y += coord[e->end][1];
                    e = e->next;
                } while (e != elast);
                coord2[j][0] = x/degree[j];
                coord2[j][1] = y/degree[j];
            }
        }
        for(j = 0; j < nv; j++){
            if(!fixed[j]){
                double distance = hypot(coord[j][0]-coord2[j][0], coord[j][1]-coord2[j][1]);
                if(distance > precision){
                    coord[j][0] = coord2[j][0];
                    coord[j][1] = coord2[j][1];
                    noChanges = FALSE;
                }
            }
        }
    }
    iterationCount = i;
}

void embedGraph(){
    int i;
    if(outerfaceEdgeFrom!=-1 && outerfaceEdgeTo!=-1){
        outerface = findFaceOnRightSide(outerfaceEdgeFrom, outerfaceEdgeTo);
    } else {
        outerface = determineBestOuterface();
    }
    
    //embed and fix the vertices of the outerface
    for(i=0; i<MAXN; i++){
        fixed[i] = FALSE;
    }
    embedFace(outerface);
    
    //place remaining vertices at the origin
    for(i=0; i<nv; i++){
        if(!fixed[i]){
            coord[i][0] = coord[i][1] = 0.0;
        }
    }
    
    //iteratively embed remaining vertices
    doTutteEmbeddingIterations();
    
    //write embedded graph
    writeWritegraph2d(stdout);
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
 * @param length
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

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s generates a tutte embedding for planar graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -i, --iterations #\n");
    fprintf(stderr, "       Specify the number of iterations. Defaults to 1000.\n");
    fprintf(stderr, "    -f, --outerface #,#\n");
    fprintf(stderr, "       Specify the outer face. Given is a directed edge and the outer face\n");
    fprintf(stderr, "       is the face on the right side of this directed edge.\n");
    fprintf(stderr, "    -c, --converge\n");
    fprintf(stderr, "       Stop the iterations when the positions have converged. The program still\n");
    fprintf(stderr, "       stops when the maximum number of iterations has been reached.\n");
    fprintf(stderr, "    -p, --precision #\n");
    fprintf(stderr, "       Specify the precision used for convergence detection. This defaults to\n");
    fprintf(stderr, "       15, which means a vertex is said to have converged when the distance\n");
    fprintf(stderr, "       between the new coordinates and the old coordinates is less then 1e-15.\n");
    fprintf(stderr, "    -m, --multiple\n");
    fprintf(stderr, "       Embed all graphs from the input. This is the default behaviour except\n");
    fprintf(stderr, "       when an outer face is specified.\n");
    fprintf(stderr, "    -v, --verbose\n");
    fprintf(stderr, "       Be more verbose.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"iterations", required_argument, NULL, 'i'},
        {"outerface", required_argument, NULL, 'f'},
        {"converge", no_argument, NULL, 'c'},
        {"precision", required_argument, NULL, 'p'},
        {"multiple", no_argument, NULL, 'm'},
        {"verbose", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hi:f:cp:mv", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'i':
                iterations = atoi(optarg);
                break;
            case 'f':
                sscanf(optarg, "%d,%d", &outerfaceEdgeFrom, &outerfaceEdgeTo);
                onlyOne = TRUE;
                break;
            case 'c':
                converge = TRUE;
                break;
            case 'p':
                precision = pow(10, -atoi(optarg));
                break;
            case 'm':
                onlyOne = FALSE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }

    /*=========== read planar graphs ===========*/
    
    int numberOfGraphs = 0;

    unsigned short code[MAXCODELENGTH];
    int length;
    if(readPlanarCode(code, &length, stdin)){
        decodePlanarCode(code);
        embedGraph();
        numberOfGraphs++;
        if(verbose){
            fprintf(stderr, "Graph embedded using %d iteration%s.\n",
                    iterationCount, iterationCount==1? "" : "s");
        }
    }
    if(!onlyOne){
        while (readPlanarCode(code, &length, stdin)) {
            decodePlanarCode(code);
            embedGraph();
            numberOfGraphs++;
            if(verbose){
                fprintf(stderr, "Graph embedded using %d iteration%s.\n",
                        iterationCount, iterationCount==1? "" : "s");
            }
        }
    }
    fprintf(stderr, "Output embedding of %d graph%s.\n", numberOfGraphs, 
                numberOfGraphs==1 ? "" : "s");
}
