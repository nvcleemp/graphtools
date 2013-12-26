/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput. Based on readscd.c by M. Meringer.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in genreg's shortcode format from standard in,
 * and writes the graph to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o genreg2multicode -O4  genreg2multicode.c shared/multicode_base.c \
 *     shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_output.h"

int n, k;

int dekomp(FILE *file, char* code) {
    char readbits, samebits;

    samebits = getc(file);
    if (feof(file))
        return (0);

    readbits = n * k / 2 - samebits;

    fread(code + samebits, sizeof (char), readbits, file);
    if (feof(file))
        return (-1);

    return (1);
}

void codetograph(char *code, GRAPH graph, ADJACENCY adj) {
    char i, v = 1, w;
    
    prepareGraph(graph, adj, n);

    for (i = n * k / 2; i > 0; i--) {
        w = *code;
        while (adj[v] == k) {
            v++;
        }
        addEdge(graph, adj, v, w);
        code++;
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s converts graphs from genreg's shortcode to multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n k\n\n", name);
    fprintf(stderr, "Here n is the number of vertices in the graphs, and k is the degree.\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] n k\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;

    int erg;
    char *code;

    GRAPH graph;
    ADJACENCY adj;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
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

    if (argc - optind != 2) {
        fprintf(stderr, "Too few parameters.\n");
        usage(name);
        return EXIT_FAILURE;
    }

    n = atoi(argv[optind]);
    k = atoi(argv[optind + 1]);

    code = (char*) calloc(n * k / 2, sizeof (char));

    while ((erg = dekomp(stdin, code)) != 0) {
        codetograph(code, graph, adj);
        writeMultiCode(graph, adj, stdout);
    }

    if (erg == -1) {
        fprintf(stderr, "Error while reading graph.\n");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

