/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planar_input.h"
#include <stdlib.h>
#include <string.h>

PLANE_GRAPH *decodePlanarCode(unsigned short* code, PG_INPUT_OPTIONS *options) {
    int i, j, codePosition, nv, maxn;
    int edgeCounter = 0;
    PG_EDGE *inverse;

    nv = code[0];
    codePosition = 1;
    
    if(options->maxn <= 0){
        maxn = nv*options->maxnFactor;
    } else {
        maxn = options->maxn;
    }
    
    PLANE_GRAPH *pg = newPlaneGraph(maxn, options->maxe);
    pg->nv = nv;

    for (i = 0; i < nv; i++) {
        pg->degree[i] = 0;
        pg->firstedge[i] = pg->edges + edgeCounter;
        pg->edges[edgeCounter].start = i;
        pg->edges[edgeCounter].end = code[codePosition] - 1;
        pg->edges[edgeCounter].next = pg->edges + edgeCounter + 1;
        if (code[codePosition] - 1 < i) {
            inverse = findEdge(pg, code[codePosition] - 1, i);
            pg->edges[edgeCounter].inverse = inverse;
            inverse->inverse = pg->edges + edgeCounter;
        } else {
            pg->edges[edgeCounter].inverse = NULL;
        }
        edgeCounter++;
        codePosition++;
        for (j = 1; code[codePosition]; j++, codePosition++) {
            pg->edges[edgeCounter].start = i;
            pg->edges[edgeCounter].end = code[codePosition] - 1;
            pg->edges[edgeCounter].prev = pg->edges + edgeCounter - 1;
            pg->edges[edgeCounter].next = pg->edges + edgeCounter + 1;
            if (code[codePosition] - 1 < i) {
                inverse = findEdge(pg, code[codePosition] - 1, i);
                pg->edges[edgeCounter].inverse = inverse;
                inverse->inverse = pg->edges + edgeCounter;
            } else {
                pg->edges[edgeCounter].inverse = NULL;
            }
            edgeCounter++;
        }
        pg->firstedge[i]->prev = pg->edges + edgeCounter - 1;
        pg->edges[edgeCounter - 1].next = pg->firstedge[i];
        pg->degree[i] = j;

        codePosition++; /* read the closing 0 */
    }

    pg->ne = edgeCounter;

    if(options->computeDual){
        makeDual(pg);
    }
    
    return pg;
}

/**
 * 
 * @param code
 * @param length
 * @param file
 * @return returns 1 if a code was read and 0 otherwise. Exits in case of error.
 */
unsigned short *readPlanarCode(FILE *file, PG_INPUT_OPTIONS *options) {
    static boolean first = TRUE;
    unsigned char c;
    char testheader[20];
    int bufferSize, zeroCounter;
    
    int readCount;
    
    int codeLength = options->initialCodeLength;
    unsigned short* code = malloc(codeLength*sizeof(unsigned short));
    if(code == NULL){
        fprintf(stderr, "Insufficient memory to store code for this graph.\n");
        return NULL;
    }

    if (first) {
        first = FALSE;
        
        if(options->containsHeader){
            //we check that there is a header
            if (fread(&testheader, sizeof (unsigned char), 13, file) != 13) {
                fprintf(stderr, "can't read header: file too small.\n");
                free(code);
                return NULL;
            }
            testheader[13] = 0;
            if (strcmp(testheader, ">>planar_code") != 0) {
                fprintf(stderr, "No planarcode header detected.\n");
                free(code);
                return NULL;
            }

            //read reminder of header (either empty or le/be specification)
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                fprintf(stderr, "Invalid formatted header.\n");
                free(code);
                return NULL;
            }
            while (c!='<'){
                if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                    fprintf(stderr, "Invalid formatted header.\n");
                    free(code);
                    return NULL;
                }
            }
            //read one more character (header is closed by <<)
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                fprintf(stderr, "Invalid formatted header.\n");
                free(code);
                return NULL;
            }
        }
    }

    /* possibly removing interior headers */
    if(options->removeInternalHeaders){
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
                    free(code);
                    return NULL;
                }
                if (!fread(&c, sizeof (unsigned char), 1, file)) {
                    //nothing left in file
                    code[0] = 0;
                    return code;
                }
                bufferSize = 1;
                zeroCounter = 0;
            }
        } else {
            //no header present
            bufferSize = 1;
            zeroCounter = 0;
        }
    }

    //start reading the graph
    if (c != 0) {
        code[0] = c;
        while (zeroCounter < code[0]) {
            if(bufferSize==codeLength){
                codeLength = (2*codeLength < (7*code[0] - 9)) ? 2*codeLength : (7*code[0] - 9);
                // 7*code[0]-9 is the maximum code length for this number of vertices 
                unsigned short* newCode = realloc(code, codeLength*sizeof(unsigned short));
                if(newCode == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = newCode;
                }
            }
            code[bufferSize] = (unsigned short) getc(file);
            if (code[bufferSize] == 0) zeroCounter++;
            bufferSize++;
        }
    } else {
        readCount = fread(code, sizeof (unsigned short), 1, file);
        if(!readCount){
            fprintf(stderr, "Unexpected EOF.\n");
            free(code);
            return NULL;
        }
        bufferSize = 1;
        zeroCounter = 0;
        while (zeroCounter < code[0]) {
            if(bufferSize==codeLength){
                codeLength = (2*codeLength < (7*code[0] - 9)) ? 2*codeLength : (7*code[0] - 9);
                // 7*code[0]-9 is the maximum code length for this number of vertices 
                unsigned short* newCode = realloc(code, codeLength*sizeof(unsigned short));
                if(newCode == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = newCode;
                }
            }
            readCount = fread(code + bufferSize, sizeof (unsigned short), 1, file);
            if(!readCount){
                fprintf(stderr, "Unexpected EOF.\n");
                free(code);
                return NULL;
            }
            if (code[bufferSize] == 0) zeroCounter++;
            bufferSize++;
        }
    }

    return code;
}

PLANE_GRAPH *readAndDecodePlanarCode(FILE *f, PG_INPUT_OPTIONS *options){
    unsigned short *code = readPlanarCode(f, options);
    PLANE_GRAPH *pg = decodePlanarCode(code, options);
    free(code);
    return pg;
}
