/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "pngtoolkit.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

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

double scale = 95.0;
int margin = 5;

colour_t backgroundColour = {255, 255, 255, 0};
colour_t lineColour = {0, 0, 0, 255};

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

double getScale(int order, int width, int height, int margin, COORDINATES coord){
    int targetWidth = width - 2*margin;
    int targetHeight = height - 2*margin;
    
    double xScale = targetWidth / getCoordinateWidth(order, coord);
    double yScale = targetHeight / getCoordinateHeight(order, coord);
    
    return xScale < yScale ? xScale : yScale;
}

void drawVertexAt(double x, double y, bitmap_t *image){
    point_t v;
    
    v.x = (int)(x*scale) + margin;
    v.y = (int)(y*scale) + margin;
    
    fillCircle(v, 4, &lineColour, image);
    drawCircle(v, 4, &backgroundColour, image);
}
void drawEdgeAt(double x0, double y0, double x1, double y1, bitmap_t *image){
    point_t p0, p1;
    
    p0.x = (int)(x0*scale) + margin;
    p0.y = (int)(y0*scale) + margin;
    p1.x = (int)(x1*scale) + margin;
    p1.y = (int)(y1*scale) + margin;
    
    drawLine(p0, p1, 2, &lineColour, image);
}

void drawGraph(GRAPH graph, ADJACENCY adj, COORDINATES coord, bitmap_t *image){
    int i,j;
    
    int order = graph[0][0];
    
    for(i=1; i<=order; i++){
        for(j=0; j<adj[i]; j++){
            int nv = graph[i][j];
            if(i < nv){
                drawEdgeAt(coord[i][0], coord[i][1], coord[nv][0], coord[nv][1], image);
            }
        }
    }
    
    for(i=1; i<=order; i++){
        drawVertexAt(coord[i][0], coord[i][1], image);
    }
}

void graph2png(GRAPH graph, ADJACENCY adj, COORDINATES coord, char *fileName){
    bitmap_t image;

    image.width = 200;
    image.height = 200;

    image.pixels = calloc(sizeof (pixel_t), image.width * image.height);
    
    //transform coordinates
    shiftVertices(graph[0][0], coord);
    scale = getScale(graph[0][0], image.width, image.height, 5, coord);

    colourBackground(&backgroundColour, &image);
    
    drawGraph(graph, adj, coord, &image);
    
    savePng(&image, fileName);
}

int main(int argc, char** argv) {
    
    char *fileName = "image.png";
    
    if(argc > 1){
        fileName = argv[1];
    }

    GRAPH graph;
    ADJACENCY adj;
    COORDINATES coord;
    
    readWritegraph2d(stdin, graph, adj, coord);
    
    graph2png(graph, adj, coord, fileName);

    return 0;
}
