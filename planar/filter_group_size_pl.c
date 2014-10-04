/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads planar graphs from standard in and
 * filters the ones with the specified group size.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o filter_group_size -O4 filter_group_size.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>


#ifndef MAXN
#define MAXN 1000            /* the maximum number of vertices */
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

int numberOfGraphs = 0;
int filteredGraphs = 0;

int nv;
int ne;
int nf;

int automorphismsCount;
int orientationReversingAutomorphismCount;

//////////////////////////////////////////////////////////////////////////////

int certificate[MAXE+MAXN];
int canonicalLabelling[MAXN];
EDGE *canonicalFirstedge[MAXN];
int alternateCertificate[MAXE+MAXN];
int alternateLabelling[MAXN];
EDGE *alternateFirstedge[MAXN];
int queue[MAXN];

int constructCertificate(EDGE *eStart){
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
    return position;
}

void constructAlternateCertificate(EDGE *eStart){
    int i;
    for(i=0; i<MAXN; i++){
        alternateLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int alternateCertificatePosition = 0;
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
            alternateCertificate[alternateCertificatePosition++] = alternateLabelling[e->end];
            e = e->next;
        } while (e!=elast);
        alternateCertificate[alternateCertificatePosition++] = MAXN;
    }
}

void constructAlternateCertificateOrientationReversing(EDGE *eStart){
    int i;
    for(i=0; i<MAXN; i++){
        alternateLabelling[i] = MAXN;
    }
    EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertexCounter = 1;
    int alternateCertificatePosition = 0;
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
            alternateCertificate[alternateCertificatePosition++] = alternateLabelling[e->end];
            e = e->prev;
        } while (e!=elast);
        alternateCertificate[alternateCertificatePosition++] = MAXN;
    }
}

void calculateAutomorphismGroup(){
    automorphismsCount = 0;
    orientationReversingAutomorphismCount = 0;
    
    //construct certificate
    int pos = 0;
    int i;
    
    pos = constructCertificate(firstedge[0]);
    
    //construct alternate certificates
    EDGE *ebase = firstedge[0];
    
    for(i=0; i<nv; i++){
        if(degree[i]==degree[0]){
            EDGE *e, *elast;

            e = elast = firstedge[i];
            do {
                if(e!=ebase){
                    constructAlternateCertificate(e);
                    if(memcmp(certificate, alternateCertificate, sizeof(int)*pos) == 0) {
                        //count automorphism
                        automorphismsCount++;
                    }
                }
                constructAlternateCertificateOrientationReversing(e);
                if(memcmp(certificate, alternateCertificate, sizeof(int)*pos) == 0) {
                    //count automorphism
                    automorphismsCount++;
                    orientationReversingAutomorphismCount++;
                }
                e = e->next;
            } while (e!=elast);
        }
    }
}

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

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters plane graphs based on the size of the\nautomorphism group.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] s\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] s\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
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

    if (argc - optind != 1) {
        usage(name);
        return EXIT_FAILURE;
    }
    
    int size = 0;
    size = atoi(argv[optind]);
    
    
    /*=========== read planar graphs ===========*/

    unsigned short code[MAXCODELENGTH];
    int length;
    while (readPlanarCode(code, &length, stdin)) {
        decodePlanarCode(code);
        numberOfGraphs++;
        calculateAutomorphismGroup();
        //+1 because the count does not include the identity
        if(automorphismsCount + 1 == size){
            writePlanarCode();
            filteredGraphs++;
        }
    }

    fprintf(stderr, "Read %d graph%s.\n", numberOfGraphs, numberOfGraphs == 1 ? "" : "s");
    fprintf(stderr, "Written %d graph%s.\n", filteredGraphs, filteredGraphs == 1 ? "" : "s");
}
