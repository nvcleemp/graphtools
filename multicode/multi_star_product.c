/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads two graphs in multicode format from standard in,
 * combines them by performing the star product with the specified 
 * vertices and writes the new graph to standard out in multicode format.
 * The specified vertices should have degree 3.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o multi_star_product -O4  multi_star_product.c shared/multicode_base.c \
 *     shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

boolean giveAllPossibilities = FALSE;

void combineGraphs(GRAPH graph1, ADJACENCY adj1, GRAPH graph2, ADJACENCY adj2,
                   GRAPH combinedGraph, ADJACENCY combinedAdj, int vertex1, int vertex2){
    int i, j;
    
    prepareGraph(combinedGraph, combinedAdj, graph1[0][0] + graph2[0][0] - 2);
    
    for(i=1; i<=graph1[0][0]; i++){
        if(i != vertex1){
            for(j=0; j<adj1[i]; j++){
                if(graph1[i][j] != vertex1 && i < graph1[i][j]){
                    addEdge(combinedGraph, combinedAdj,
                            i < vertex1 ? i : i -1, 
                            graph1[i][j] < vertex1 ? graph1[i][j] : graph1[i][j] - 1);
                }
            }
        }
    }
    
    for(i=1; i<=graph2[0][0]; i++){
        if(i != vertex2){
            for(j=0; j<adj2[i]; j++){
                if(graph2[i][j] != vertex2 && i < graph2[i][j]){
                    addEdge(combinedGraph, combinedAdj,
                            i < vertex2 ? i + graph1[0][0] - 1 : i + graph1[0][0] - 2, 
                            graph2[i][j] < vertex2 ? graph2[i][j] + graph1[0][0] - 1 : graph2[i][j] + graph1[0][0] - 2);
                }
            }
        }
    }
    
    //add connecting edges
    int n11, n12, n13, n21, n22, n23;
    n11 = graph1[vertex1][0] < vertex1 ? graph1[vertex1][0] : graph1[vertex1][0] - 1;
    n12 = graph1[vertex1][1] < vertex1 ? graph1[vertex1][1] : graph1[vertex1][1] - 1;
    n13 = graph1[vertex1][2] < vertex1 ? graph1[vertex1][2] : graph1[vertex1][2] - 1;
    n21 = graph2[vertex2][0] < vertex2 ? 
        graph2[vertex2][0] + graph1[0][0] - 1 :
        graph2[vertex2][0] + graph1[0][0] - 2;
    n22 = graph2[vertex2][1] < vertex2 ? 
        graph2[vertex2][1] + graph1[0][0] - 1 :
        graph2[vertex2][1] + graph1[0][0] - 2;
    n23 = graph2[vertex2][2] < vertex2 ? 
        graph2[vertex2][2] + graph1[0][0] - 1 :
        graph2[vertex2][2] + graph1[0][0] - 2;
    
    addEdge(combinedGraph, combinedAdj, n11, n21);
    addEdge(combinedGraph, combinedAdj, n12, n22);
    addEdge(combinedGraph, combinedAdj, n13, n23);
    
    writeMultiCode(combinedGraph, combinedAdj, stdout);
    
    if(giveAllPossibilities){
        removeEdge(combinedGraph, combinedAdj, n12, n22, FALSE);
        removeEdge(combinedGraph, combinedAdj, n13, n23, FALSE);
        addEdge(combinedGraph, combinedAdj, n12, n23);
        addEdge(combinedGraph, combinedAdj, n13, n22);
        writeMultiCode(combinedGraph, combinedAdj, stdout);
        
        removeEdge(combinedGraph, combinedAdj, n11, n21, FALSE);
        removeEdge(combinedGraph, combinedAdj, n12, n23, FALSE);
        removeEdge(combinedGraph, combinedAdj, n13, n22, FALSE);
        addEdge(combinedGraph, combinedAdj, n11, n22);
        addEdge(combinedGraph, combinedAdj, n12, n21);
        addEdge(combinedGraph, combinedAdj, n13, n23);
        writeMultiCode(combinedGraph, combinedAdj, stdout);
        
        removeEdge(combinedGraph, combinedAdj, n12, n21, FALSE);
        removeEdge(combinedGraph, combinedAdj, n13, n23, FALSE);
        addEdge(combinedGraph, combinedAdj, n12, n23);
        addEdge(combinedGraph, combinedAdj, n13, n21);
        writeMultiCode(combinedGraph, combinedAdj, stdout);
        
        removeEdge(combinedGraph, combinedAdj, n11, n22, FALSE);
        removeEdge(combinedGraph, combinedAdj, n12, n23, FALSE);
        removeEdge(combinedGraph, combinedAdj, n13, n21, FALSE);
        addEdge(combinedGraph, combinedAdj, n11, n23);
        addEdge(combinedGraph, combinedAdj, n12, n21);
        addEdge(combinedGraph, combinedAdj, n13, n22);
        writeMultiCode(combinedGraph, combinedAdj, stdout);
        
        removeEdge(combinedGraph, combinedAdj, n12, n21, FALSE);
        removeEdge(combinedGraph, combinedAdj, n13, n22, FALSE);
        addEdge(combinedGraph, combinedAdj, n12, n22);
        addEdge(combinedGraph, combinedAdj, n13, n21);
        writeMultiCode(combinedGraph, combinedAdj, stdout);
    }
}
    
//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s combines two graphs in multicode format using the star product.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] v w\n\n", name);
    fprintf(stderr, "Vertex v, resp. w, should be a vertex of degree 3 in graph 1, resp. graph 2.\n");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -a, --all\n");
    fprintf(stderr, "       Give all possible ways to perform the star product.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] v w\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    int i;
    
    GRAPH graph1;
    ADJACENCY adj1;
    
    GRAPH graph2;
    ADJACENCY adj2;
    
    GRAPH combinedGraph;
    ADJACENCY combinedAdj;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"all", no_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "ha", long_options, &option_index)) != -1) {
        switch (c) {
            case 'a':
                giveAllPossibilities = TRUE;
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
    
    if(argc - optind != 2){
        usage(name);
        return EXIT_FAILURE;
    }
    
    int vertex1, vertex2;
    if(sscanf(argv[optind + 0], "%d", &vertex1)!=1){
        fprintf(stderr, "Error while reading vertex 1.\n");
        usage(name);
        return EXIT_FAILURE;
    }
    if(sscanf(argv[optind + 1], "%d", &vertex2)!=1){
        fprintf(stderr, "Error while reading vertex 2.\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph1, adj1);
        
    } else {
        fprintf(stderr, "Error! Could not read first graph.\n");
        return (EXIT_FAILURE);
    }
    
    if (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph2, adj2);
        
    } else {
        fprintf(stderr, "Error! Could not read second graph.\n");
        return (EXIT_FAILURE);
    }
    
    if(vertex1 < 0 || graph1[0][0] <= vertex1){
        fprintf(stderr, "Error! First vertex is not part of first graph.\n");
        return (EXIT_FAILURE);
    }
    
    if(vertex2 < 0 || graph2[0][0] <= vertex2){
        fprintf(stderr, "Error! Second vertex is not part of second graph.\n");
        return (EXIT_FAILURE);
    }
    
    if(adj1[vertex1] != 3){
        fprintf(stderr, "Error! First vertex does not have degree 3.\n");
        return (EXIT_FAILURE);
    }
    
    if(adj2[vertex2] != 3){
        fprintf(stderr, "Error! Second vertex does not have degree 3.\n");
        return (EXIT_FAILURE);
    }
    
    combineGraphs(graph1, adj1, graph2, adj2, combinedGraph, combinedAdj, vertex1, vertex2);

    return (EXIT_SUCCESS);
}

