/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads planar graphs from standard in and
 * splits them over a given number of files.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o split_pl -O4 split_pl.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>


#ifndef MAXN
#define MAXN 200            /* the maximum number of vertices */
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

typedef int boolean;

int numberOfGraphs = 0;

boolean onePerFile = FALSE;

//=============== Reading and decoding planarcode ===========================

/**
 * 
 * @param code
 * @param length
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

void writePlanarCode(unsigned short code[], int length, FILE *outFile, boolean withHeader){
    int i;
    if(withHeader){
        fprintf(outFile, ">>planar_code<<");
    }
    
    if(code[0]+1 <= 255) {
        for(i=0; i<length; i++){
             fputc(code[i], outFile);
        }
    } else if(code[0]+1 <= 65535) {
        fputc(0, outFile);
        for(i=0; i<length; i++){
             if (fwrite(code+i, sizeof (unsigned short), 1, outFile) != 1) {
                fprintf(stderr, "fwrite() failed -- exiting!\n");
                exit(-1);
            }
        }
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s splits the given graphs over a given number of files.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices.\n", MAXN);
    fprintf(stderr, "Recompile with a larger value for MAXN if you need larger graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, " -h, --help\n");
    fprintf(stderr, "    Print this help and return.\n");
    fprintf(stderr, " -1, --one-per-file\n");
    fprintf(stderr, "    Split this file such that each file contains only one graph.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] n file\n", name);
    fprintf(stderr, "   or: %s [options] -1 file\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"one-per-file", no_argument, NULL, '1'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h1", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '1':
                onePerFile = TRUE;
                break;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    
    if(onePerFile){
        if (argc - optind != 1) {
            usage(name);
            return EXIT_FAILURE;
        }
    } else {
        if (argc - optind != 2) {
            usage(name);
            return EXIT_FAILURE;
        }
    }
    
    if(onePerFile){
        char *fileNameBase = argv[optind];
        char fileName[100];
        

        /*=========== read planar graphs ===========*/

        unsigned short code[MAXCODELENGTH];
        int length;
        while (readPlanarCode(code, &length, stdin)) {
            numberOfGraphs++;
            int n = snprintf(fileName, 100, "%s-%d.code", fileNameBase, numberOfGraphs);
            if (n<0 || n>=100){
                fprintf(stderr, "Filename could not be constructed -- exiting.\n");
                return EXIT_FAILURE;
            }
            FILE *f = fopen(fileName, "w");
            writePlanarCode(code, length, f, TRUE);
            
            fclose(f);
        }
    } else {
        int i;
        
        int fileCount = atoi(argv[optind]);
        char *fileNameBase = argv[optind+1];
        char fileName[100];
        FILE* files[fileCount];
        for (i = 0; i < fileCount; i++){
            int n = snprintf(fileName, 100, "%s-%d.code", fileNameBase, i);
            if (n<0 || n>=100){
                fprintf(stderr, "Filename could not be constructed -- exiting.\n");
                return EXIT_FAILURE;
            }
            FILE *f = fopen(fileName, "w");
            fprintf(f, ">>planar_code<<");
            files[i] = f;
        }

        /*=========== read planar graphs ===========*/

        unsigned short code[MAXCODELENGTH];
        int length;
        while (readPlanarCode(code, &length, stdin)) {
            writePlanarCode(code, length, files[numberOfGraphs%fileCount], FALSE);
            numberOfGraphs++;
        }

        for (i = 0; i < fileCount; i++){
            fclose(files[i]);
        }
    }
}
