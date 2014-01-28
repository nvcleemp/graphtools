/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a tree in freetree's parent array format from standard in,
 * and writes the tree to standard out in multicode format.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o freetree2multicode -O4  freetree2multicode.c shared/multicode_base.c \
 *     shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_output.h"

void translateLine1digit(char *line, int n, GRAPH graph, ADJACENCY adj){
    int i;
    
    prepareGraph(graph, adj, n);
    
    for(i = 1; i < n; i++){
        addEdge(graph, adj, i+1, (int)(line[i] - '0'));
    }
}

void translateLineMultidigit(char *line, int n, GRAPH graph, ADJACENCY adj){
    int i;
    
    prepareGraph(graph, adj, n);
    
    char *part = strtok(line, ",");
    for(i = 1; i < n; i++){
        part = strtok(NULL, ",");
        addEdge(graph, adj, i+1, atoi(part));
    }
}

void translateLine(char *line, int n, GRAPH graph, ADJACENCY adj){
    if(n < 10){
        translateLine1digit(line, n, graph, adj);
    } else {
        translateLineMultidigit(line, n, graph, adj);
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s converts trees from freetree's parent array format to multicode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n\n\n", name);
    fprintf(stderr, "Here n is the number of vertices in the trees.\n");
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
    GRAPH graph;
    ADJACENCY adj;
    
    int n;

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

    if (argc - optind != 1) {
        fprintf(stderr, "Too few parameters.\n");
        usage(name);
        return EXIT_FAILURE;
    }

    n = atoi(argv[optind]);
    
    char *line = NULL;
    size_t size;
    while (getline(&line, &size, stdin) != -1) {
        translateLine(line, n, graph, adj);
        writeMultiCode(graph, adj, stdout);
    }

    return (EXIT_SUCCESS);
}

