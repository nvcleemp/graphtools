/* Usage: cat codes | multiread 
 * or:    cat codes | multiread graphNumber
 */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"

int maxvalence;

void writeGraph_impl(GRAPH g, int columns, char *numberFormat, char *headerSeparator, char* emptyCell) {
    int x, y, lowerBound, upperBound;
    fprintf(stdout, "\n\n ");

    fprintf(stdout, numberFormat, g[0][0]);

    for (x = 1; (x <= g[0][0])&&(x <= columns); x++) {
        fprintf(stdout, numberFormat, x);
    }
    fprintf(stdout, "|\n");

    fprintf(stdout, " ");

    for (x = 0; (x <= g[0][0])&&(x <= columns); x++) {
        fprintf(stdout, "|%s", headerSeparator);
    }
    fprintf(stdout, "|\n");

    for (x = 0; x < maxvalence; x++) {
        fprintf(stdout, " |%s", emptyCell);
        for (y = 1; (y <= g[0][0])&&(y <= columns); y++) {
            if (g[y][x] == EMPTY) {
                fprintf(stdout, "|%s", emptyCell);
            } else {
                fprintf(stdout, numberFormat, g[y][x]);
            }
        }
        fprintf(stdout, "|\n");
    }

    lowerBound = columns + 1;
    upperBound = 2*columns;

    while (g[0][0] >= lowerBound) {
        fprintf(stdout, "\n");

        fprintf(stdout, "  %s", emptyCell);

        for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
            fprintf(stdout, numberFormat, x);
        }
        fprintf(stdout, "|\n");

        fprintf(stdout, "  %s", emptyCell);

        for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
            fprintf(stdout, "|%s", headerSeparator);
        }
        fprintf(stdout, "|\n");

        for (y = 0; y < maxvalence; y++) {
            fprintf(stdout, "  %s", emptyCell);
            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                if (g[x][y] == EMPTY) {
                    fprintf(stdout, "|%s", emptyCell);
                } else {
                    fprintf(stdout, numberFormat, g[x][y]);
                }
            }
            fprintf(stdout, "|\n");
        }
        lowerBound += columns;
        upperBound += columns;
    }
}

void writeGraph2Digits(GRAPH g) {
    writeGraph_impl(g, 24, "|%2d", "==", "  ");
}

void writeGraph3Digits(GRAPH g) {
    writeGraph_impl(g, 18, "|%3d", "===", "   ");
}

void writeGraph4Digits(GRAPH g) {
    writeGraph_impl(g, 14, "|%4d", "====", "    ");
}

void writeGraph(GRAPH g) {
    if (g[0][0] < 100) {
        writeGraph2Digits(g);
    } else if (g[0][0] < 1000) {
        writeGraph3Digits(g);
    } else /*if (g[0][0] < 10000)*/ {
        writeGraph4Digits(g);
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

    while (readMultiCode(code, &codeLength, stdin)){

        graphCount++;
        if ((!filterGraph) || (filterGraph == graphCount)) {
            decodeMultiCode(code, codeLength, graph, adj);
            for (i = 1, maxvalence = 0; i <= graph[0][0]; i++) {
                if (adj[i] > maxvalence) {
                    maxvalence = adj[i];
                }
            }
            printf("\n\n\n Graph Nr: %d \n\n", graphCount);
            writeGraph(graph);
        }
    }



    return (0);

}


