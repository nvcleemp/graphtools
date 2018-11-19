/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "multicode_input.h"
#include<string.h>

GRAPH *decodeMultiCode(unsigned short* code, GRAPH_INPUT_OPTIONS *options) {
    int i, j, currentVertex, vertexCount, maxn, maxval;
    
    vertexCount = code[0];
    
    if(options->maxn > 0){
        maxn = options->maxn;
    } else if(options->maxnFactor > 0){
        maxn = vertexCount*options->maxnFactor;
    } else {
        maxn = vertexCount + options->maxnOffset;
    }
    
    //determine max degree
    int degrees[vertexCount];
    for(i = 0; i < vertexCount; i++){
        degrees[i] = 0;
    }
    maxval = j = 0;
    i = 1;
    currentVertex = 0;
    while(currentVertex < vertexCount - 1) {
        if (code[i] == 0) {
            if(degrees[currentVertex] > maxval)
                maxval = degrees[currentVertex];
            currentVertex++;
        } else {
            degrees[currentVertex]++;
            degrees[code[i]-1]++;
        }
        i++;
    }
    //check last vertex
    if(degrees[currentVertex] > maxval)
        maxval = degrees[currentVertex];
    
    if(options->maxval > 0){
        maxval = options->maxval;
    } else if(options->maxvalFactor > 0){
        maxval *= options->maxvalFactor;
    } else {
        maxval += options->maxvalOffset;
    }
    
    GRAPH *graph = newGraph(maxn, maxval);
    prepareGraph(graph, vertexCount);

    //go through code and add edges
    i = 1;
    currentVertex = 0;
    while(currentVertex < vertexCount - 1) {
        if (code[i] == 0) {
            currentVertex++;
        } else {
            addEdge(graph, currentVertex, code[i] - 1);
        }
        i++;
    }
    
    return graph;
}

unsigned short *readMultiCode(FILE *file, GRAPH_INPUT_OPTIONS *options) {
    static boolean first = TRUE;
    unsigned char c;
    char testheader[20];
    int bufferSize = 0, zeroCounter = 0;
    
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
            if (fread(&testheader, sizeof (unsigned char), 12, file) != 12) {
                fprintf(stderr, "can't read header: file too small.\n");
                free(code);
                return NULL;
            }
            testheader[12] = 0;
            if (strcmp(testheader, ">>multi_code") != 0) {
                fprintf(stderr, "No multicode header detected.\n");
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
        while (zeroCounter < code[0]-1) {
            if(bufferSize==codeLength){
                codeLength = (2*codeLength < (code[0]*(code[0] - 1)/2)) ? 2*codeLength : code[0]*(code[0] - 1)/2;
                // code[0]*(code[0] - 1)/2 is the maximum code length for this number of vertices 
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
        while (zeroCounter < code[0]-1) {
            if(bufferSize==codeLength){
                codeLength = (2*codeLength < (code[0]*(code[0] - 1)/2)) ? 2*codeLength : (code[0]*(code[0] - 1)/2);
                // (code[0]*(code[0] - 1)/2) is the maximum code length for this number of vertices 
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

GRAPH *readAndDecodeMultiCode(FILE *f, GRAPH_INPUT_OPTIONS *options){
    unsigned short *code = readMultiCode(f, options);
    if(code == NULL){
        return NULL;
    }
    GRAPH *graph = decodeMultiCode(code, options);
    free(code);
    return graph;
}
