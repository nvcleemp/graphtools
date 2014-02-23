/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in multi_code format from standard in,
 * and writes it in signed_code format (with all edges positive) to stdout.
 * 
 * 
 * Compile with:
 *     
 *     cc -o multicode2signedcode -O4  multicode2signedcode.c\
 *          shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#include "../signed/shared/signed_base.h"
#include "../signed/shared/signed_input.h"
#include "../signed/shared/signed_output.h"

int edgeCounter = 0;

int main(int argc, char *argv[]) {
    GRAPH graph;
    ADJACENCY adj;
    int graphCount, written;
    int codeLength;
    unsigned short code[MAXCODELENGTH];

    int filterGraph = 0;

    if (argc >= 2) {
        sscanf(argv[1], "%d", &filterGraph);
    }

    graphCount = written = 0;

    while (readMultiCode(code, &codeLength, stdin)){

        graphCount++;
        if ((!filterGraph) || (filterGraph == graphCount)) {
            int order;
            decodeMultiCode(code, codeLength, graph, adj, &order);
            writeSignedCode(graph, adj, order, stdout);
            written++;
        }
    }

    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    fprintf(stderr, "Written %d graph%s.\n", written, written==1 ? "" : "s");

    return (0);

}


