/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads a graph in signedcode format from standard in,
 * and displays it in a human-readable format on stdout.
 * 
 * 
 * Compile with:
 *     
 *     cc -o signed_show -O4  signed_show.c shared/signed_base.c shared/signed_input.c
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"

int edgeCounter = 0;

int maxvalence;

void writeGraph_impl(GRAPH g, int order, int columns, char *numberFormat, char *headerSeparator, char* emptyCell) {
    int x, y, lowerBound, upperBound;
    fprintf(stdout, "\n\n ");

    fprintf(stdout, numberFormat, order);
    fprintf(stdout, " ");

    for (x = 1; (x <= order)&&(x <= columns); x++) {
        fprintf(stdout, numberFormat, x);
        fprintf(stdout, " ");
    }
    fprintf(stdout, "|\n");

    fprintf(stdout, " ");

    for (x = 0; (x <= order)&&(x <= columns); x++) {
        fprintf(stdout, "|%s", headerSeparator);
    }
    fprintf(stdout, "|\n");

    for (x = 0; x < maxvalence; x++) {
        fprintf(stdout, " |%s", emptyCell);
        for (y = 1; (y <= order)&&(y <= columns); y++) {
            if (g[y][x] == NULL) {
                fprintf(stdout, "|%s", emptyCell);
            } else {
                fprintf(stdout, numberFormat, g[y][x]->largest == y ? g[y][x]->smallest : g[y][x]->largest);
                fprintf(stdout, g[y][x]->isNegative ? "-" : "+");
            }
        }
        fprintf(stdout, "|\n");
    }

    lowerBound = columns + 1;
    upperBound = 2*columns;

    while (order >= lowerBound) {
        fprintf(stdout, "\n");

        fprintf(stdout, "  %s", emptyCell);

        for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
            fprintf(stdout, numberFormat, x);
        }
        fprintf(stdout, "|\n");

        fprintf(stdout, "  %s", emptyCell);

        for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
            fprintf(stdout, "|%s", headerSeparator);
        }
        fprintf(stdout, "|\n");

        for (y = 0; y < maxvalence; y++) {
            fprintf(stdout, "  %s", emptyCell);
            for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
                if (g[x][y] == NULL) {
                    fprintf(stdout, "|%s", emptyCell);
                } else {
                    fprintf(stdout, numberFormat, g[y][x]->largest == y ? g[y][x]->smallest : g[y][x]->largest);
                    fprintf(stdout, g[y][x]->isNegative ? "-" : "+");
                }
            }
            fprintf(stdout, "|\n");
        }
        lowerBound += columns;
        upperBound += columns;
    }
}

void writeGraph2Digits(GRAPH g, int order) {
    writeGraph_impl(g, order, 18, "|%2d", "===", "   ");
}

void writeGraph3Digits(GRAPH g, int order) {
    writeGraph_impl(g, order, 14, "|%3d", "====", "    ");
}

void writeGraph4Digits(GRAPH g, int order) {
    writeGraph_impl(g, order, 10, "|%4d", "=====", "     ");
}

void writeGraph(GRAPH g, int order) {
    if (order < 100) {
        writeGraph2Digits(g, order);
    } else if (order < 1000) {
        writeGraph3Digits(g, order);
    } else /*if (order < 10000)*/ {
        writeGraph4Digits(g, order);
    }

}

int main(int argc, char *argv[]) {
    GRAPH graph;
    ADJACENCY adj;
    int graphCount, i;
    int codeLength;
    unsigned short code[MAXCODELENGTH];

    int filterGraph = 0;

    if (argc >= 2) {
        sscanf(argv[1], "%d", &filterGraph);
    }

    graphCount = 0;

    while (readSignedCode(code, &codeLength, stdin)){

        graphCount++;
        if ((!filterGraph) || (filterGraph == graphCount)) {
            int order;
            decodeSignedCode(code, codeLength, graph, adj, &order);
            for (i = 1, maxvalence = 0; i <= order; i++) {
                if (adj[i] > maxvalence) {
                    maxvalence = adj[i];
                }
            }
            printf("\n\n\n Graph Nr: %d \n\n", graphCount);
            writeGraph(graph, order);
        }
    }



    return (0);

}


