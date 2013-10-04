/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "multicode_output.h"

void writeMultiCodeChar(GRAPH graph, ADJACENCY adj, FILE *f){
    int i, j;
    
    int vertexCount = graph[0][0];
    
    //write the number of vertices
    fputc(vertexCount, f);
    
    for(i=1; i<vertexCount; i++){
        for(j=0; j<adj[i]; j++){
            if(i<graph[i][j]){
                fputc(graph[i][j], f);
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

void writeMultiCodeShort(GRAPH graph, ADJACENCY adj, FILE *f){
    int i, j;
    
    int vertexCount = graph[0][0];
    
    //write the number of vertices
    fputc(0, f);
    writeShort(graph[0][0], f);
    
    for(i=1; i<vertexCount; i++){
        for(j=0; j<adj[i]; j++){
            if(i<graph[i][j]){
                writeShort(graph[i][j], f);
            }
        }
        writeShort(0, f);
    }
}

void writeMultiCode(GRAPH graph, ADJACENCY adj, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>multi_code<<");
    }
    
    if (graph[0][0] <= 252) {
        writeMultiCodeChar(graph, adj, f);
    } else if (graph[0][0] <= 252*256) {
        writeMultiCodeShort(graph, adj, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}
