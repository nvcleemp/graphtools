/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "multicode_input.h"
#include<string.h>

GRAPH *decode_multi_code(unsigned short* code, GRAPH_INPUT_OPTIONS *options) {
    int i, j, current_vertex, vertex_count, maxn, maxval;
    
    vertex_count = code[0];
    
    if(options->maxn > 0){
        maxn = options->maxn;
    } else if(options->maxn_factor > 0){
        maxn = vertex_count*options->maxn_factor;
    } else {
        maxn = vertex_count + options->maxn_offset;
    }
    
    //determine max degree
    int degrees[vertex_count];
    for(i = 0; i < vertex_count; i++){
        degrees[i] = 0;
    }
    maxval = j = 0;
    i = 1;
    current_vertex = 0;
    while(current_vertex < vertex_count - 1) {
        if (code[i] == 0) {
            if(degrees[current_vertex] > maxval)
                maxval = degrees[current_vertex];
            current_vertex++;
        } else {
            degrees[current_vertex]++;
            degrees[code[i]-1]++;
        }
        i++;
    }
    //check last vertex
    if(degrees[current_vertex] > maxval)
        maxval = degrees[current_vertex];
    
    if(options->maxval > 0){
        maxval = options->maxval;
    } else if(options->maxval_factor > 0){
        maxval *= options->maxval_factor;
    } else {
        maxval += options->maxval_offset;
    }
    
    GRAPH *graph = new_graph(maxn, maxval);
    prepare_graph(graph, vertex_count);

    //go through code and add edges
    i = 1;
    current_vertex = 0;
    while(current_vertex < vertex_count - 1) {
        if (code[i] == 0) {
            current_vertex++;
        } else {
            add_edge(graph, current_vertex, code[i] - 1);
        }
        i++;
    }
    
    return graph;
}

unsigned short *read_multi_code(FILE *file, GRAPH_INPUT_OPTIONS *options) {
    static boolean first = TRUE;
    unsigned char c;
    char testheader[20];
    int buffer_size = 0, zero_counter = 0;
    
    int read_count;
    
    int code_length = options->initial_code_length;
    unsigned short* code = malloc(code_length*sizeof(unsigned short));
    if(code == NULL){
        fprintf(stderr, "Insufficient memory to store code for this graph.\n");
        return NULL;
    }

    if (first) {
        first = FALSE;
        
        if(options->contains_header){
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
    if(options->remove_internal_headers){
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            //nothing left in file
            return (0);
        }

        if (c == '>') {
            // could be a header, or maybe just a 62 (which is also possible for unsigned char
            code[0] = c;
            buffer_size = 1;
            zero_counter = 0;
            code[1] = (unsigned short) getc(file);
            if (code[1] == 0) zero_counter++;
            code[2] = (unsigned short) getc(file);
            if (code[2] == 0) zero_counter++;
            buffer_size = 3;
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
                buffer_size = 1;
                zero_counter = 0;
            }
        } else {
            //no header present
            buffer_size = 1;
            zero_counter = 0;
        }
    }

    //start reading the graph
    if (c != 0) {
        code[0] = c;
        while (zero_counter < code[0]-1) {
            if(buffer_size==code_length){
                code_length = (2*code_length < (code[0]*(code[0] - 1)/2)) ? 2*code_length : code[0]*(code[0] - 1)/2;
                // code[0]*(code[0] - 1)/2 is the maximum code length for this number of vertices 
                unsigned short* new_code = realloc(code, code_length*sizeof(unsigned short));
                if(new_code == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = new_code;
                }
            }
            code[buffer_size] = (unsigned short) getc(file);
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    } else {
        read_count = fread(code, sizeof (unsigned short), 1, file);
        if(!read_count){
            fprintf(stderr, "Unexpected EOF.\n");
            free(code);
            return NULL;
        }
        buffer_size = 1;
        zero_counter = 0;
        while (zero_counter < code[0]-1) {
            if(buffer_size==code_length){
                code_length = (2*code_length < (code[0]*(code[0] - 1)/2)) ? 2*code_length : (code[0]*(code[0] - 1)/2);
                // (code[0]*(code[0] - 1)/2) is the maximum code length for this number of vertices 
                unsigned short* new_code = realloc(code, code_length*sizeof(unsigned short));
                if(new_code == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = new_code;
                }
            }
            read_count = fread(code + buffer_size, sizeof (unsigned short), 1, file);
            if(!read_count){
                fprintf(stderr, "Unexpected EOF.\n");
                free(code);
                return NULL;
            }
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    }

    return code;
}

GRAPH *read_and_decode_multi_code(FILE *f, GRAPH_INPUT_OPTIONS *options){
    unsigned short *code = read_multi_code(f, options);
    if(code == NULL){
        return NULL;
    }
    GRAPH *graph = decode_multi_code(code, options);
    free(code);
    return graph;
}
