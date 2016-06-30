/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "multicode_output.h"

void writeMultiCodeChar(GRAPH *graph, FILE *f){
    int i, j;
    
    int vertexCount = graph->n;
    
    //write the number of vertices
    fputc(vertexCount, f);
    
    for(i=0; i<vertexCount-1; i++){
        for(j=0; j<graph->degrees[i]; j++){
            if(i<NEIGHBOUR(graph, i, j)){
                fputc(NEIGHBOUR(graph, i, j) + 1, f);
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

void writeMultiCodeShort(GRAPH *graph, FILE *f){
    int i, j;
    
    int vertexCount = graph->n;
    
    //write the number of vertices
    fputc(0, f);
    writeShort(vertexCount, f);
    
    for(i=1; i<vertexCount; i++){
        for(j=0; j<graph->degrees[i]; j++){
            if(i<NEIGHBOUR(graph, i, j)){
                writeShort(NEIGHBOUR(graph, i, j) + 1, f);
            }
        }
        writeShort(0, f);
    }
}

void writeMultiCode(GRAPH *graph, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>multi_code<<");
    }
    
    if (graph->n <= 252) {
        writeMultiCodeChar(graph, f);
    } else if (graph->n <= 252*256) {
        writeMultiCodeShort(graph, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}
