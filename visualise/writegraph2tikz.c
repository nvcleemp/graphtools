/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <getopt.h>

typedef int boolean;
#define TRUE 1
#define FALSE 0

#define MAXN 1000
#define MAXVAL (MAXN-1)

#define EMPTY USHRT_MAX

typedef unsigned short GRAPH[MAXN + 1][MAXVAL + 1];
typedef unsigned short ADJACENCY[MAXN + 1];
typedef double COORDINATES[MAXN + 1][2];

double offsetX = 1.0;
double offsetY = 1.0;

double width = 5.0;
double height = 5.0;

double scale;

boolean standalone = FALSE;

char *nodeStyle = "circle, fill";
char *edgeStyle = "draw, thick";

void addEdge(GRAPH graph, ADJACENCY adj, int v, int w) {
    graph[v][adj[v]] = w;
    graph[w][adj[w]] = v;
    adj[v]++;
    adj[w]++;
}

#define MAX_LINE_LENGTH 3000

void readWritegraph2d(FILE *fp, GRAPH graph, ADJACENCY adj, COORDINATES coord) {
    // based on the version in embed.c
    int i, j;
    
    char line[MAX_LINE_LENGTH]; /* input buffer			*/
    char *remain_line;          /* remaining line		*/
    int read_char;              /* number of read characters	*/
    int lineno;                 /* number of current line	*/

    int vert;                   /* current vertex number	*/
    double x, y;                /* position of current vertex	*/
    int neighbour;              /* vertex number of neighbor	*/

    int nv;                     /* number of vertices so far	*/

    /* --- initialize --- */
    
    //mark all vertices as having degree 0
    for (i = 1; i <= MAXN; i++) {
        adj[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = EMPTY;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = 0;
    }

    nv = 0;
    lineno = 0;

    x = y = 0.0;

    /* --- read the input one line at a time --- */

    while (fgets(line, MAX_LINE_LENGTH, fp) == line) {
        /* --- check if a complete line was read --- */

        if (line[strlen(line) - 1] != '\n') {
            fprintf(stderr, "Error while reading input -- exiting!\n");
            exit(EXIT_FAILURE);
        }

        /* --- keep track of the line number --- */

        lineno++;

        /* --- first line is special --- */

        if (lineno == 1) {
            /* --- handle the format tag if present --- */

            sscanf(line, " %n", &read_char);
            if (line[read_char] == '>') {
                if (strncmp(line + read_char, ">>writegraph2d", 14) != 0) {
                    fprintf(stderr, "Incorrect header -- exiting!\n");
                    exit(EXIT_FAILURE);
                }
                continue;
            }
        }

        /* --- read the vertex number --- */

        if (sscanf(line, "%d %n", &vert, &read_char) != 1)
            continue;
        remain_line = &line[read_char];

        /* --- number 0 indicates end of current graph --- */

        if (vert == 0){
            break;
        }

        /* --- complain if not the expected number --- */

        if (vert != ++nv) {
            fprintf(stderr, "Error while reading next vertex: expected %d, got %d -- exiting!\n", nv, vert);
            exit(EXIT_FAILURE);
        }

        /* --- read the first two coordinates for this vertex --- */

        if (sscanf(remain_line, "%lf %lf %n", &x, &y, &read_char) < 2) {
            fprintf(stderr, "Error while coordinates for vertex %d -- exiting!\n", nv);
            exit(EXIT_FAILURE);
        }
        remain_line += read_char;

        if (nv > MAXN) {
            fprintf(stderr, "Recompile with a larger value for MAXN to support graphs of this size -- exiting!\n");
        }

        /* --- store coordinate values --- */

        coord[nv][0] = x;
        coord[nv][1] = y;

        /* --- read neighbor list and create edges --- */

        while (sscanf(remain_line, "%d%n", &neighbour, &read_char) == 1) {
            remain_line += read_char;

            if (nv < neighbour) {
                addEdge(graph, adj, nv, neighbour);
            }
        }
    }
    
    graph[0][0] = nv;
}

void shiftVertices(int order, COORDINATES coord){
    int i;
    double xMin, yMin;
    
    xMin = coord[1][0];
    yMin = coord[1][1];
    
    for(i = 2; i <= order; i++){
        if (coord[i][0] < xMin){
            xMin = coord[i][0];
        }
        if (coord[i][1] < yMin){
            yMin = coord[i][1];
        }
    }
    
    double xShift = -xMin;
    double yShift = -yMin;
    
    for(i = 1; i <= order; i++){
        coord[i][0] += xShift;
        coord[i][1] += yShift;
    }
    
}

double getCoordinateWidth(int order, COORDINATES coord){
    int i;
    double xMin, xMax;
    
    xMin = xMax = coord[1][0];
    
    for(i = 2; i <= order; i++){
        if (coord[i][0] < xMin){
            xMin = coord[i][0];
        } else if(coord[i][0] > xMax){
            xMax = coord[i][0];
        }
    }
    
    return xMax - xMin;
}

double getCoordinateHeight(int order, COORDINATES coord){
    int i;
    double yMin, yMax;
    
    yMin = yMax = coord[1][1];
    
    for(i = 2; i <= order; i++){
        if (coord[i][1] < yMin){
            yMin = coord[i][1];
        } else if(coord[i][1] > yMax){
            yMax = coord[i][1];
        }
    }
    
    return yMax - yMin;
}

void scaleVertices(int order, COORDINATES coord){
    double xScale = width / getCoordinateWidth(order, coord);
    double yScale = height / getCoordinateHeight(order, coord);
    
    int i;
    
    for(i = 1; i <= order; i++){
        coord[i][0] *= xScale;
        coord[i][1] *= yScale;
    }
}

void transformVertices(GRAPH graph, COORDINATES coord){
    //transform coordinates
    shiftVertices(graph[0][0], coord);
    scaleVertices(graph[0][0], coord);
}

void drawVertexAt(int nr, double x, double y, FILE *f){
    fprintf(f, "    \\node (v%d) at (%f,%f) {};\n", nr, x, y);
}
void drawEdgeFrom(int v0, int v1, FILE *f){
    fprintf(f, "    \\path (v%d) edge (v%d);\n", v0, v1);
}

void drawGraph(GRAPH graph, ADJACENCY adj, COORDINATES coord, FILE *f){
    int i,j;
    
    int order = graph[0][0];
    
    for(i=1; i<=order; i++){
        drawVertexAt(i, coord[i][0], coord[i][1], f);
    }
    
    for(i=1; i<=order; i++){
        for(j=0; j<adj[i]; j++){
            int nv = graph[i][j];
            if(i < nv){
                drawEdgeFrom(i, nv, f);
            }
        }
    }
}

void graph2tikz(GRAPH graph, ADJACENCY adj, COORDINATES coord, FILE *f){
    transformVertices(graph, coord);
    
    if(standalone){
        fprintf(f, "\\documentclass[tikz]{standalone}\n");
        fprintf(f, "\\begin{document}\n");
    }
    
    fprintf(f, "\\begin{tikzpicture}[every node/.style={%s},every edge/.style={%s}]\n",
            nodeStyle, edgeStyle);
    
    drawGraph(graph, adj, coord, f);
    
    fprintf(f, "\\end{tikzpicture}\n");
    
    if(standalone){
        fprintf(f, "\\end{document}\n");
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s generates a TikZ picture of the input graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -W, --width #\n");
    fprintf(stderr, "       Specify the width of the image. Defaults to 5.0.\n");
    fprintf(stderr, "    -H, --height #\n");
    fprintf(stderr, "       Specify the height of the image. Defaults to 5.0.\n");
    fprintf(stderr, "    -n, --node #\n");
    fprintf(stderr, "       Specify the style for the nodes. Defaults to \"circle, fill\".\n");
    fprintf(stderr, "    -e, --edge #\n");
    fprintf(stderr, "       Specify the style for the edges. Defaults to \"draw, thick\".\n");
    fprintf(stderr, "    -s, --standalone\n");
    fprintf(stderr, "       Generate a TeX-file that can be build, instead of a TikZ-picture\n");
    fprintf(stderr, "       to include into another file.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char** argv) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"width", required_argument, NULL, 'W'},
        {"height", required_argument, NULL, 'H'},
        {"node", required_argument, NULL, 'n'},
        {"edge", required_argument, NULL, 'e'},
        {"standalone", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hW:H:sn:e:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'W':
                width = atof(optarg);
                break;
            case 'H':
                height = atof(optarg);
                break;
            case 'n':
                nodeStyle = optarg;
                break;
            case 'e':
                edgeStyle = optarg;
                break;
            case 's':
                standalone = TRUE;
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
    
    GRAPH graph;
    ADJACENCY adj;
    COORDINATES coord;
    
    readWritegraph2d(stdin, graph, adj, coord);
    
    graph2tikz(graph, adj, coord, stdout);

    return 0;
}
