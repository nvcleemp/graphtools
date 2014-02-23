/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "signed_output.h"

void writeSignedCodeChar(GRAPH graph, ADJACENCY adj, int order, FILE *f){
    int i, j;
    
    //write the number of vertices
    fputc(order, f);
    
    for(i=1; i<order; i++){
        for(j=0; j<adj[i]; j++){
            if(i==graph[i][j]->smallest){
                fputc(graph[i][j]->largest, f);
                fputc(graph[i][j]->isNegative ? NEGATIVE : POSITIVE, f);
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

void writeSignedCodeShort(GRAPH graph, ADJACENCY adj, int order, FILE *f){
    int i, j;
    
    //write the number of vertices
    fputc(0, f);
    writeShort(order, f);
    
    for(i=1; i<order; i++){
        for(j=0; j<adj[i]; j++){
            if(i==graph[i][j]->smallest){
                writeShort(graph[i][j]->largest, f);
                writeShort(graph[i][j]->isNegative ? NEGATIVE : POSITIVE, f);
            }
        }
        writeShort(0, f);
    }
}

void writeSignedCode(GRAPH graph, ADJACENCY adj, int order, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>signed_code<<");
    }
    
    if (order <= 252) {
        writeSignedCodeChar(graph, adj, order, f);
    } else if (order <= 252*256) {
        writeSignedCodeShort(graph, adj, order, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}
