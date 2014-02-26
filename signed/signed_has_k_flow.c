/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format and computes whether
 * they have a k-flow.
 * 
 * 
 * Compile with:
 *     
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int k;

int edgeCounter = 0;

int graphCount = 0;
int graphsFiltered = 0;

int unassignedEdgesAtVertex[MAXN+1];
int flowAtVertex[MAXN+1];
int flowValue[MAXE];

FILE *flowFile = NULL;
boolean showFlow = FALSE;

//============================= Printing flows ==============================

void writeFlow_impl(GRAPH g, int order, int columns, char *numberFormat, char *headerSeparator, char* emptyCell, int maxDegree) {
    int x, y, lowerBound, upperBound;
    fprintf(flowFile, "\n\n ");

    fprintf(flowFile, numberFormat, order);
    fprintf(flowFile, "     ");

    for (x = 1; (x <= order)&&(x <= columns); x++) {
        fprintf(flowFile, numberFormat, x);
        fprintf(flowFile, "     ");
    }
    fprintf(flowFile, "|\n");

    fprintf(flowFile, " ");

    for (x = 0; (x <= order)&&(x <= columns); x++) {
        fprintf(flowFile, "|%s", headerSeparator);
    }
    fprintf(flowFile, "|\n");

    for (x = 0; x < maxDegree; x++) {
        fprintf(flowFile, " |%s", emptyCell);
        for (y = 1; (y <= order)&&(y <= columns); y++) {
            if (g[y][x] == NULL) {
                fprintf(flowFile, "|%s", emptyCell);
            } else {
                fprintf(flowFile, numberFormat, g[y][x]->largest == y ? g[y][x]->smallest : g[y][x]->largest);
                fprintf(flowFile, g[y][x]->isNegative ? "-" : "+");
                fprintf(flowFile, "(% 2d)", flowValue[g[y][x]->index]);
            }
        }
        fprintf(flowFile, "|\n");
    }

    lowerBound = columns + 1;
    upperBound = 2*columns;

    while (order >= lowerBound) {
        fprintf(flowFile, "\n");

        fprintf(flowFile, "  %s", emptyCell);

        for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
            fprintf(flowFile, numberFormat, x);
            fprintf(flowFile, "     ");
        }
        fprintf(flowFile, "|\n");

        fprintf(flowFile, "  %s", emptyCell);

        for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
            fprintf(flowFile, "|%s", headerSeparator);
        }
        fprintf(flowFile, "|\n");

        for (y = 0; y < maxDegree; y++) {
            fprintf(flowFile, "  %s", emptyCell);
            for (x = lowerBound; (x <= order)&&(x <= upperBound); x++) {
                if (g[x][y] == NULL) {
                    fprintf(flowFile, "|%s", emptyCell);
                } else {
                    fprintf(flowFile, numberFormat, g[x][y]->largest == x ? g[x][y]->smallest : g[x][y]->largest);
                    fprintf(flowFile, g[x][y]->isNegative ? "-" : "+");
                    fprintf(flowFile, "(% 2d)", flowValue[g[x][y]->index]);
                }
            }
            fprintf(flowFile, "|\n");
        }
        lowerBound += columns;
        upperBound += columns;
    }
}

void writeFlow2Digits(GRAPH g, int order, int maxDegree) {
    writeFlow_impl(g, order, 8, "|%2d", "=======", "       ", maxDegree);
}

void writeFlow3Digits(GRAPH g, int order, int maxDegree) {
    writeFlow_impl(g, order, 7, "|%3d", "========", "        ", maxDegree);
}

void writeFlow4Digits(GRAPH g, int order, int maxDegree) {
    writeFlow_impl(g, order, 6, "|%4d", "=========", "         ", maxDegree);
}

void writeFlow(GRAPH g, ADJACENCY adj, int order) {
    int maxDegree = 0;
    int i;
    for(i = 1; i <= order; i++){
        if(adj[i]>maxDegree){
            maxDegree = adj[i];
        }
    }
    if (order < 100) {
        writeFlow2Digits(g, order, maxDegree);
    } else if (order < 1000) {
        writeFlow3Digits(g, order, maxDegree);
    } else /*if (order < 10000)*/ {
        writeFlow4Digits(g, order, maxDegree);
    }

}

boolean findKflow(GRAPH graph, ADJACENCY adj, int order, int unassignedEdges){
    int i, j;
    
    if(unassignedEdges==0){
        return TRUE;
    }
    //first see if there is a vertex incident to only one unassigned edge
    i = 1;
    int minUnassignedEdges = MAXN;
    int minUnassignedEdgesVertex;
    while(i <= order && unassignedEdgesAtVertex[i]!=1){
        if(unassignedEdgesAtVertex[i]>0 && unassignedEdgesAtVertex[i] < minUnassignedEdges){
            minUnassignedEdges = unassignedEdgesAtVertex[i];
            minUnassignedEdgesVertex = i;
        }
        i++;
    }
    if(i<=order){
        //assign that remaining edge
        if(flowAtVertex[i]==0 || flowAtVertex[i] <= -k || flowAtVertex[i] >= k){
            //does not lead to a valid k-flow
            return FALSE;
        }
        //find the edge
        j = 0;
        while(j < adj[i] && flowValue[graph[i][j]->index]!=0){
            j++;
        }
        if(j == adj[i]){
            fprintf(stderr, "Program reached inconsistent state -- exiting!\n");
            exit(EXIT_FAILURE);
        }
        EDGE *edge = graph[i][j];
        
        //determine the value
        int value;
        if(edge->isNegative){
            //negative edges are always seen as 'sources'
            value = -flowAtVertex[i];
        } else if(i==edge->smallest){
            //orientation is from small to large
            value = flowAtVertex[i];
        } else {
            value = -flowAtVertex[i];
        }
        
        //set the value
        flowValue[edge->index] = value;
        unassignedEdgesAtVertex[edge->smallest]--;
        unassignedEdgesAtVertex[edge->largest]--;
        if(edge->isNegative){
            flowAtVertex[edge->smallest]+=value;
            flowAtVertex[edge->largest]+=value;
        } else {
            flowAtVertex[edge->smallest]-=value;
            flowAtVertex[edge->largest]+=value;
        }
        
        //check legality at other vertex
        int neighbour;
        if(i==edge->smallest){
            neighbour = edge->largest;
        } else {
            neighbour = edge->smallest;
        }
        if(unassignedEdgesAtVertex[neighbour]>0 || flowAtVertex[neighbour]==0){
            //recurse
            if(findKflow(graph, adj, order, unassignedEdges-1)){
                return TRUE;
            }
        }
        //reset values
        flowValue[edge->index] = 0;
        unassignedEdgesAtVertex[edge->smallest]++;
        unassignedEdgesAtVertex[edge->largest]++;
        if(edge->isNegative){
            flowAtVertex[edge->smallest]-=value;
            flowAtVertex[edge->largest]-=value;
        } else {
            flowAtVertex[edge->smallest]+=value;
            flowAtVertex[edge->largest]-=value;
        }
        
        return FALSE;
    } else {
        //choose an edge incident to a vertex which is incident to the least number of unassigned edges
        
        i = 0;
        while(i < adj[minUnassignedEdgesVertex] &&
                flowValue[graph[minUnassignedEdgesVertex][i]->index]!=0){
           i++; 
        }
        if(i==adj[minUnassignedEdgesVertex]){
            fprintf(stderr, "Program reached inconsistent state -- exiting!\n");
            exit(EXIT_FAILURE);
        }
        EDGE *edge = graph[minUnassignedEdgesVertex][i];
        
        for(i = -k + 1; i < 0; i++){
            int value = i;
            //set the value
            flowValue[edge->index] = value;
            unassignedEdgesAtVertex[edge->smallest]--;
            unassignedEdgesAtVertex[edge->largest]--;
            if(edge->isNegative){
                flowAtVertex[edge->smallest]+=value;
                flowAtVertex[edge->largest]+=value;
            } else {
                flowAtVertex[edge->smallest]-=value;
                flowAtVertex[edge->largest]+=value;
            }

            //check legality
            if((unassignedEdgesAtVertex[edge->largest]>0 || flowAtVertex[edge->largest]==0)
                    && (unassignedEdgesAtVertex[edge->smallest]>0 || flowAtVertex[edge->smallest]==0)){
                //recurse
                if(findKflow(graph, adj, order, unassignedEdges-1)){
                    return TRUE;
                }
            }
            //reset values
            flowValue[edge->index] = 0;
            unassignedEdgesAtVertex[edge->smallest]++;
            unassignedEdgesAtVertex[edge->largest]++;
            if(edge->isNegative){
                flowAtVertex[edge->smallest]-=value;
                flowAtVertex[edge->largest]-=value;
            } else {
                flowAtVertex[edge->smallest]+=value;
                flowAtVertex[edge->largest]-=value;
            }
        }
        for(i = 1; i < k; i++){
            int value = i;
            //set the value
            flowValue[edge->index] = value;
            unassignedEdgesAtVertex[edge->smallest]--;
            unassignedEdgesAtVertex[edge->largest]--;
            if(edge->isNegative){
                flowAtVertex[edge->smallest]+=value;
                flowAtVertex[edge->largest]+=value;
            } else {
                flowAtVertex[edge->smallest]-=value;
                flowAtVertex[edge->largest]+=value;
            }

            //check legality
            if((unassignedEdgesAtVertex[edge->largest]>0 || flowAtVertex[edge->largest]==0)
                    && (unassignedEdgesAtVertex[edge->smallest]>0 || flowAtVertex[edge->smallest]==0)){
                //recurse
                if(findKflow(graph, adj, order, unassignedEdges-1)){
                    return TRUE;
                }
            }
            //reset values
            flowValue[edge->index] = 0;
            unassignedEdgesAtVertex[edge->smallest]++;
            unassignedEdgesAtVertex[edge->largest]++;
            if(edge->isNegative){
                flowAtVertex[edge->smallest]-=value;
                flowAtVertex[edge->largest]-=value;
            } else {
                flowAtVertex[edge->smallest]+=value;
                flowAtVertex[edge->largest]-=value;
            }
        }
    }
    return FALSE;
}

boolean hasKflow(GRAPH graph, ADJACENCY adj, int order){
    int i;
    for(i=1; i<=order; i++){
        unassignedEdgesAtVertex[i] = adj[i];
        flowAtVertex[i] = 0;
    }
    for(i=0; i<edgeCounter; i++){
        flowValue[i] = 0;
        edges[i].index = i;
    }
    return findKflow(graph, adj, order, edgeCounter);
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s determines whether signed graphs in signed_code\n", name);
    fprintf(stderr, "format have a k-flow.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] k\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs and give a larger value for MAXN.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       Filter graphs that have a k-flow.\n");
    fprintf(stderr, "    -i, --invert\n");
    fprintf(stderr, "       Invert the filter.\n");
    fprintf(stderr, "    -m, --multicode\n");
    fprintf(stderr, "       Export the graphs in multi_code format (signs are not exported).\n");
    fprintf(stderr, "    -s, --show\n");
    fprintf(stderr, "       Shows the k-flow if there is one. This feature is disabled if -f is used.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] k\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    GRAPH graph;
    ADJACENCY adj;
    
    boolean doFiltering = FALSE;
    boolean invert = FALSE;
    boolean asMulticode = FALSE;
    
    flowFile = stderr;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"invert", no_argument, NULL, 'i'},
        {"filter", no_argument, NULL, 'f'},
        {"multicode", no_argument, NULL, 'm'},
        {"show", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hfims", long_options, &option_index)) != -1) {
        switch (c) {
            case 'i':
                invert = TRUE;
                break;
            case 'f':
                doFiltering = TRUE;
                break;
            case 'm':
                asMulticode = TRUE;
                break;
            case 's':
                showFlow = TRUE;
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
    
    if(argc - optind != 1){
        usage(name);
        return EXIT_FAILURE;
    }
    k = atoi(argv[optind]);
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readSignedCode(code, &length, stdin)) {
        int order;
        decodeSignedCode(code, length, graph, adj, &order);
        graphCount++;
        
        boolean value = hasKflow(graph, adj, order);
        if(doFiltering){
            if(invert && !value){
                graphsFiltered++;
                if(asMulticode){
                    writeAsMultiCode(graph, adj, order, stdout);
                } else {
                    writeSignedCode(graph, adj, order, stdout);
                }
            } else if(!invert && value){
                graphsFiltered++;
                if(asMulticode){
                    writeAsMultiCode(graph, adj, order, stdout);
                } else {
                    writeSignedCode(graph, adj, order, stdout);
                }
            }
        } else {
            if(value){
                fprintf(stderr, "Graph %d has a %d-flow.\n", graphCount, k);
                if(showFlow){
                    writeFlow(graph, adj, order);
                }
            } else {
                fprintf(stderr, "Graph %d does not have a %d-flow.\n", graphCount, k);
            }
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphCount, graphCount==1 ? "" : "s");
    if(doFiltering){
        fprintf(stderr, "Filtered %d graph%s that %s a %d-flow.\n", 
                graphsFiltered, graphsFiltered==1 ? "" : "s",
                graphsFiltered==1 ?
                    (invert ? "does not have" : "has") :
                    (invert ? "do not have" : "have"),
                k);
    }

    return (EXIT_SUCCESS);
}

