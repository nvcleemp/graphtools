/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "cubic_output.h"

void writeCubicMultiCodeChar(GRAPH graph, int vertexCount, FILE *f){
    int i, j;
    
    //write the number of vertices
    fputc(vertexCount, f);
    
    for(i=0; i<vertexCount-1; i++){
        for(j=0; j<3; j++){
            if(i<graph[i][j]){
                fputc(graph[i][j]+1, f);
            }
        }
        fputc(0, f);
    }
}

void writeShort(unsigned short value, FILE *f){
    if (fwrite(&value, sizeof (unsigned short), 1, f) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }
}

void writeCubicMultiCodeShort(GRAPH graph, int vertexCount, FILE *f){
    int i, j;
    
    //write the number of vertices
    fputc(0, f);
    writeShort(vertexCount, f);
    
    for(i=0; i<vertexCount-1; i++){
        for(j=0; j<3; j++){
            if(i<graph[i][j]){
                writeShort(graph[i][j]+1, f);
            }
        }
        writeShort(0, f);
    }
}

void writeCubicMultiCode(GRAPH graph, int vertexCount, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>multi_code<<");
    }
    
    if (vertexCount <= 255) {
        writeCubicMultiCodeChar(graph, vertexCount, f);
    } else if (vertexCount <= 255*256) {
        writeCubicMultiCodeShort(graph, vertexCount, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}
