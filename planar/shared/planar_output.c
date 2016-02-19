/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planar_output.h"
#include <stdlib.h>

void writePlanarCodeChar(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    //write the number of vertices
    fputc(pg->nv, f);
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            fputc(e->end + 1, f);
            e = e->next;
        } while (e != elast);
        fputc(0, f);
    }
}

void writePlanarCodeShort(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    unsigned short temp;
    
    //write the number of vertices
    fputc(0, f);
    temp = pg->nv;
    if (fwrite(&temp, sizeof (unsigned short), 1, stdout) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            temp = e->end + 1;
            if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
                fprintf(stderr, "fwrite() failed -- exiting!\n");
                exit(EXIT_FAILURE);
            }
            e = e->next;
        } while (e != elast);
        temp = 0;
        if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
            fprintf(stderr, "fwrite() failed -- exiting!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void writePlanarCode(PLANE_GRAPH *pg, FILE *f){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(f, ">>planar_code<<");
    }
    
    if (pg->nv + 1 <= 255) {
        writePlanarCodeChar(pg, f);
    } else if (pg->nv + 1 <= 65535) {
        writePlanarCodeShort(pg, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
}


void writeEdgeCodeSmall(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    //write the length of the body
    fputc(pg->ne + pg->nv - 1, f);
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            fputc(e->index, f);
            e = e->next;
        } while (e != elast);
        if(i < pg->nv - 1){
            fputc(255, f);
        }
    }
}

void writeBigEndianMultibyte(FILE *f, int number, int bytecount){
    if(bytecount==1){
        fprintf(f, "%c", number);
    } else {
        int i;
        unsigned int mask = 256 << ((bytecount - 2)*8);
        fprintf(f, "%c", number/mask);
        for(i = 0; i < bytecount - 1; i++){
            number %= mask;
            mask >>= 8;
            fprintf(f, "%c", number/mask);
        }
    }    
}

void writeEdgeCodeLarge(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
    exit(-1);
    
    int k, l, s;
    
    //determine the maximum number of bytes needed to store each of the edges
    l = 1;
    while(pg->ne > (1 << l)){
        l++;
    }
    if(l%8){
        l = l/8 + 1;
    } else {
        l /= 8;
    }
    //check that the largest number does not start with the byte 255.
    if (pg->ne/(256 << ((l-1)*8)) == 255){
        l++;
    }
    if(l>15){
        fprintf(stderr, "This graph can not be stored in edge code format -- exiting!\n");
    }
    
    //s is the number of bytes needed to store the body
    s = (pg->ne + pg->nv - 1)*l;
    
    //k is the number of bytes needed to encode s
    k = 1;
    while(s > (1 << k)){
        k++;
    }
    if(k%8){
        k = k/8 + 1;
    } else {
        k /= 8;
    }
    
    //we start the code with a zero
    fputc(0, f);
    
    //write the byte encoding k and l
    fputc((k<<4)+l, f);
    
    //write the length of the body (s)
    writeBigEndianMultibyte(f, s, k);
    
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            writeBigEndianMultibyte(f, e->index, l);
            e = e->next;
        } while (e != elast);
        if(i <pg-> nv - 1){
            fputc(255, f);
        }
    }
}

void writeEdgeCode(PLANE_GRAPH *pg, FILE *f){
    static int first = TRUE;
    int i, counter=0;
    PG_EDGE *e, *elast;
    
    if(first){
        first = FALSE;
        
        fprintf(f, ">>edge_code<<");
    }
    
    //label the edges
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            e->index = -1;
            e = e->next;
        } while (e != elast);
    }
    for(i=0; i<pg->nv; i++){
        e = elast = pg->firstedge[i];
        do {
            if(e->index == -1){
                e->index = counter;
                e->inverse->index = counter;
                counter++;
            }
            e = e->next;
        } while (e != elast);
    }
    
    if (pg->ne + pg->nv - 1 <= 255) {
        writeEdgeCodeSmall(pg, f);
    } else {
        writeEdgeCodeLarge(pg, f);
    }
    
}