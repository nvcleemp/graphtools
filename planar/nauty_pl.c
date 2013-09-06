/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*  This program reads plane graphs from stdin and uses nauty to determine the
 *  automorphism groups of the underlying graphs.
 * 
 * 
 * Compile with:
 *     
 *     cc -o nauty_pl -O4 nauty_pl.c nauty/nauty.c nauty/nautil.c nauty/naugraph.c nauty/schreier.c nauty/naurng.c
 * 
 * A recent version of nauty is assumed to be present in a directory called nauty.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


#ifndef MAXN
#define MAXN 64            /* the maximum number of vertices */
#endif

#include "nauty/nauty.h"

int nv;

int numberOfGraphs = 0;

boolean readPlanarCode(FILE *f){
    static int first = 1;
    unsigned char c;
    char testheader[20];
    int zeroCounter;
    graph g[MAXN*MAXM];
        
    int i;
    for(i=0; i<MAXN; i++){
        set *v;
        v = GRAPHROW(g, i, MAXM);
        EMPTYSET(v, MAXM);
    }
    
    if (first) {
        first = 0;

        if (fread(&testheader, sizeof (unsigned char), 13, f) != 13) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[13] = 0;
        if (strcmp(testheader, ">>planar_code") != 0) {
            fprintf(stderr, "No planarcode header detected -- exiting!\n");
            exit(1);
        }
        //read reminder of header (either empty or le/be specification)
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            return FALSE;
        }
        while (c!='<'){
            if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
                return FALSE;
            }
        }
        //read one more character
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            return FALSE;
        }
    }
    

    if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
        //nothing left in file
        return (0);
    }

    if (c != 0) /* unsigned chars would be sufficient */ {
        int order = c;
        nv = order;
        
        zeroCounter = 0;
        while (zeroCounter < order) {
            int neighbour = (unsigned short) getc(f);
            if (neighbour == 0) {
                zeroCounter++;
            } else {
                fprintf(stderr, "%d - %d\n", zeroCounter, neighbour-1);
                set *gv, *gn;
                gv = GRAPHROW(g, zeroCounter, MAXM);
                gn = GRAPHROW(g, neighbour-1, MAXM);
                ADDELEMENT(gv, neighbour-1);
                ADDELEMENT(gn, zeroCounter);
            }
        }
    } else {
        int order = 0;
        int readCount = fread(&order, sizeof (unsigned short), 1, f);
        if(!readCount){
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
        nv = order;
        
        int neighbour = 0;
        zeroCounter = 0;
        while (zeroCounter < order) {
            readCount = fread(&neighbour, sizeof (unsigned short), 1, f);
            if(!readCount){
                fprintf(stderr, "Unexpected EOF.\n");
                exit(1);
            }
            if (neighbour == 0) {
                zeroCounter++;
            } else {
                set *gv, *gn;
                gv = GRAPHROW(g, zeroCounter, MAXM);
                gn = GRAPHROW(g, neighbour-1, MAXM);
                ADDELEMENT(gv, neighbour-1);
                ADDELEMENT(gn, zeroCounter);
            }
        }
    }
    
    int lab[nv], ptn[nv], orbits[nv];
    DEFAULTOPTIONS_GRAPH(options);
    options.writeautoms = TRUE;
    options.writemarkers = TRUE;
    statsblk stats;
    
    densenauty(g, lab, ptn, orbits, &options, &stats, MAXM, nv, NULL);
    
    fprintf(stderr, "Vertices: %d\n", nv);
    fprintf(stderr, "%lf %d\n", stats.grpsize1, stats.grpsize2);
    fprintf(stderr, "Generators: %d\n", stats.numgenerators);

    return (1);
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s reads plane graphs and uses nauty to determine the\n", name);
    fprintf(stderr, "automorphism groups of the underlying graphs.\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }

    /*=========== read planar graphs ===========*/

    while (readPlanarCode(stdin)) {
        numberOfGraphs++;
    }
    
    fprintf(stderr, "Read %d graph%s.\n", numberOfGraphs, numberOfGraphs==1 ? "" : "s");
}

