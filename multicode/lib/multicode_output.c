/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "multicode_output.h"

void write_multi_code_char(GRAPH *graph, FILE *f){
    int i, j;
    
    int vertex_count = graph->n;
    
    //write the number of vertices
    fputc(vertex_count, f);
    
    for(i=0; i<vertex_count-1; i++){
        for(j=0; j<graph->degrees[i]; j++){
            if(i<NEIGHBOUR(graph, i, j)){
                fputc(NEIGHBOUR(graph, i, j) + 1, f);
            }
        }
        fputc(0, f);
    }
}

void write_short(unsigned short value, FILE *f){
    if (fwrite(&value, sizeof (unsigned short), 1, f) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }
}

void write_multi_code_short(GRAPH *graph, FILE *f){
    int i, j;
    
    int vertex_count = graph->n;
    
    //write the number of vertices
    fputc(0, f);
    write_short(vertex_count, f);
    
    for(i=1; i<vertex_count; i++){
        for(j=0; j<graph->degrees[i]; j++){
            if(i<NEIGHBOUR(graph, i, j)){
                write_short(NEIGHBOUR(graph, i, j) + 1, f);
            }
        }
        write_short(0, f);
    }
}

void write_multi_code(GRAPH *graph, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>multi_code<<");
    }
    
    if (graph->n <= 252) {
        write_multi_code_char(graph, f);
    } else if (graph->n <= 252*256) {
        write_multi_code_short(graph, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}
