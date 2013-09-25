/* Aufruf: cat Codes | multilies ------- oder
           cat Codes | multilies welchergraph */

#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<limits.h>

#define EMPTY USHRT_MAX

#define MAXN 4000
#define MAXVALENCE 100

typedef unsigned short ENTRYTYPE;
typedef ENTRYTYPE GRAPH[MAXN + 1][MAXVALENCE + 1];
typedef ENTRYTYPE ADJACENCY[MAXN + 1];

int maxvalence, codeLength;
ENTRYTYPE vertexCount;

void writeGraph(GRAPH g) {
    int x, y, lowerBound, upperBound;
    fprintf(stdout, "\n\n ");


    if (g[0][0] < 100) {
        fprintf(stdout, "|%2d", g[0][0]);

        for (x = 1; (x <= g[0][0])&&(x <= 24); x++) {
            fprintf(stdout, "|%2d", x);
        }
        fprintf(stdout, "|\n");

        fprintf(stdout, " ");

        for (x = 0; (x <= g[0][0])&&(x <= 24); x++) {
            fprintf(stdout, "|==");
        }
        fprintf(stdout, "|\n");

        for (x = 0; x < maxvalence; x++) {
            fprintf(stdout, " |  ");
            for (y = 1; (y <= g[0][0])&&(y <= 24); y++) {
                if (g[y][x] == EMPTY) {
                    fprintf(stdout, "|  ");
                } else {
                    fprintf(stdout, "|%2d", g[y][x]);
                }
            }
            fprintf(stdout, "|\n");
        }

        lowerBound = 25;
        upperBound = 48;

        while (g[0][0] >= lowerBound) {
            fprintf(stdout, "\n");

            fprintf(stdout, "    ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|%2d", x);
            }
            fprintf(stdout, "|\n");

            fprintf(stdout, "    ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|==");
            }
            fprintf(stdout, "|\n");

            for (y = 0; y < maxvalence; y++) {
                fprintf(stdout, "    ");
                for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                    if (g[x][y] == EMPTY) {
                        fprintf(stdout, "|  ");
                    } else {
                        fprintf(stdout, "|%2d", g[x][y]);
                    }
                }
                fprintf(stdout, "|\n");
            }
            lowerBound += 24;
            upperBound += 24;
        }
    } else if (g[0][0] < 1000) {
        fprintf(stdout, "|%3d", g[0][0]);

        for (x = 1; (x <= g[0][0])&&(x <= 24); x++) {
            fprintf(stdout, "|%3d", x);
        }
        fprintf(stdout, "|\n");

        fprintf(stdout, " ");

        for (x = 0; (x <= g[0][0])&&(x <= 24); x++) {
            fprintf(stdout, "|===");
        }
        fprintf(stdout, "|\n");

        for (x = 0; x < maxvalence; x++) {
            fprintf(stdout, " |   ");
            for (y = 1; (y <= g[0][0])&&(y <= 24); y++) {
                if (g[y][x] == EMPTY) {
                    fprintf(stdout, "|   ");
                } else {
                    fprintf(stdout, "|%3d", g[y][x]);
                }
            }
            fprintf(stdout, "|\n");
        }

        lowerBound = 25;
        upperBound = 48;

        while (g[0][0] >= lowerBound) {
            fprintf(stdout, "\n");

            fprintf(stdout, "     ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|%3d", x);
            }
            fprintf(stdout, "|\n");

            fprintf(stdout, "     ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|===");
            }
            fprintf(stdout, "|\n");

            for (y = 0; y < maxvalence; y++) {
                fprintf(stdout, "     ");
                for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++){
                    if (g[x][y] == EMPTY) {
                        fprintf(stdout, "|   ");
                    } else {
                        fprintf(stdout, "|%3d", g[x][y]);
                    }
                }
                fprintf(stdout, "|\n");
            }
            lowerBound += 24;
            upperBound += 24;
        }
    } else /*if (g[0][0] < 10000)*/ {
        fprintf(stdout, "|%4d", g[0][0]);

        for (x = 1; (x <= g[0][0])&&(x <= 19); x++) {
            fprintf(stdout, "|%4d", x);
        }
        fprintf(stdout, "|\n");

        fprintf(stdout, " ");

        for (x = 0; (x <= g[0][0])&&(x <= 19); x++) {
            fprintf(stdout, "|====");
        }
        fprintf(stdout, "|\n");

        for (x = 0; x < maxvalence; x++) {
            fprintf(stdout, " |    ");
            for (y = 1; (y <= g[0][0])&&(y <= 19); y++) {
                if (g[y][x] == EMPTY) {
                    fprintf(stdout, "|    ");
                } else {
                    fprintf(stdout, "|%4d", g[y][x]);
                }
            }
            fprintf(stdout, "|\n");
        }

        lowerBound = 20;
        upperBound = 38;

        while (g[0][0] >= lowerBound) {
            fprintf(stdout, "\n");

            fprintf(stdout, "      ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|%4d", x);
            }
            fprintf(stdout, "|\n");

            fprintf(stdout, "      ");

            for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++) {
                fprintf(stdout, "|====");
            }
            fprintf(stdout, "|\n");

            for (y = 0; y < maxvalence; y++) {
                fprintf(stdout, "      ");
                for (x = lowerBound; (x <= g[0][0])&&(x <= upperBound); x++)
                    if (g[x][y] == EMPTY) {
                        fprintf(stdout, "|    ");
                    } else {
                        fprintf(stdout, "|%4d", g[x][y]);
                    }
                fprintf(stdout, "|\n");
            }
            lowerBound += 19;
            upperBound += 19;
        }
    }

}





/* Fuegt die Kante (v,w) in den Graphen graph ein. Dabei wird aber davon */
/* ausgegangen, dass in adj die wirklich aktuellen werte fuer die */
/* Adjazenzen stehen. Die adjazenzen werden dann aktualisiert. */

void addEdge(GRAPH graph, ADJACENCY adj, int v, int w) {
    graph[v][adj[v]] = w;
    graph[w][adj[w]] = v;
    adj[v]++;
    adj[w]++;
}

void decode(ENTRYTYPE *code, GRAPH graph, ADJACENCY adj, int codelength) {
    int i, j;
    ENTRYTYPE vertexCount;

    graph[0][0] = vertexCount = code[0];

    for (i = 1; i <= vertexCount; i++) {
        adj[i] = 0;
        for (j = 0; j <= MAXVALENCE; j++) {
            graph[i][j] = EMPTY;
        }
    }
    for (j = 1; j <= MAXVALENCE; j++) {
        graph[0][j] = 0;
    }

    j = 1;

    for (i = 1; i < codelength; i++) {
        if (code[i] == 0) {
            j++;
        } else {
            addEdge(graph, adj, j, (int) code[i]);
        }
        if ((adj[code[i]] > MAXVALENCE) || (adj[j] > MAXVALENCE)) {
            fprintf(stderr, "MAXVALENCE too small (%d)!\n", MAXVALENCE);
            exit(0);
        }
    }
}

main(int argc, char *argv[]) {
    GRAPH graph;
    ADJACENCY adj;
    int graphCount, i, zeroCount;
    ENTRYTYPE code[MAXN * MAXVALENCE + MAXN];
    unsigned char dummy;

    int filterGraph = 0;

    if (argc >= 2) {
        sscanf(argv[1], "%d", &filterGraph);
    }

    graphCount = 0;


    for (; fread(&dummy, sizeof (unsigned char), 1, stdin);) {
        if (dummy != 0) {
            vertexCount = code[0] = dummy;
            zeroCount = 0;
            codeLength = 1;
            while (zeroCount < vertexCount - 1) {
                code[codeLength] = getc(stdin);
                if (code[codeLength] == 0) {
                    zeroCount++;
                }
                codeLength++;
            }
        } else {
            fread(code, sizeof (ENTRYTYPE), 1, stdin);
            vertexCount = code[0];
            zeroCount = 0;
            codeLength = 1;
            while (zeroCount < vertexCount - 1) {
                fread(code + codeLength, sizeof (ENTRYTYPE), 1, stdin);
                if (code[codeLength] == 0) {
                    zeroCount++;
                }
                codeLength++;
            }
        }


        graphCount++;
        if ((!filterGraph) || (filterGraph == graphCount)) {
            decode(code, graph, adj, codeLength);
            for (i = 1, maxvalence = 0; i <= vertexCount; i++) {
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


