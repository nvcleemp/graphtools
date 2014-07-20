/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Gives an overview of the wheels that appear as a subgraph in the graphs
 * that are read from standard in.
 * 
 * Compile like this:
 *     
 *     cc -o multi_wheels_overview -O4 ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c multi_wheels_overview.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_input.h"

typedef struct freqTable {
    int key;
    int frequency;
    
    struct freqTable *smaller;
    struct freqTable *larger;
} FREQUENCY_TABLE;

boolean verticesInCycle[MAXN+1];
boolean neighbourhoods[MAXN+1][MAXN+1];
boolean universalNeighbours[MAXN+1][MAXN+1];

FREQUENCY_TABLE *wheelsTable = NULL;

boolean interrupted = FALSE;

void handleInterruptSignal(int sig){
    if(sig==SIGINT){
        interrupted = TRUE;
        fprintf(stderr, "\nStopping program because of request by user.\n");
    } else {
        fprintf(stderr, "Handler called with wrong signal -- ignoring!\n");
    }
}

void handleAlarmSignal(int sig){
    if(sig==SIGALRM){
        interrupted = TRUE;
        fprintf(stderr, "Stopping program because time-out was reached.\n");
    } else {
        fprintf(stderr, "Handler called with wrong signal -- ignoring!\n");
    }
}

FREQUENCY_TABLE *newFrequencyTableElement(int key){
    FREQUENCY_TABLE *fte = malloc(sizeof(FREQUENCY_TABLE));
    
    fte->key = key;
    fte->frequency = 1;
    
    fte->smaller = fte->larger = NULL;
    
    return fte;
}

void freeFrequencyTable(FREQUENCY_TABLE *ft){
    if(ft == NULL){
        return;
    } else {
        freeFrequencyTable(ft->smaller);
        freeFrequencyTable(ft->larger);
        free(ft);
    }
}

void addToFrequencyTable(FREQUENCY_TABLE *ft, int key){
   if(ft->key == key){
       ft->frequency++;
   } else if(ft->key > key) {
       if(ft->smaller == NULL){
           ft->smaller = newFrequencyTableElement(key);
       } else {
           addToFrequencyTable(ft->smaller, key);
       }
   } else {
       if(ft->larger == NULL){
           ft->larger = newFrequencyTableElement(key);
       } else {
           addToFrequencyTable(ft->larger, key);
       }
   }
}

void handleSimpleCycle(int size, int universalNeighbourCount){
    //if there is a universal neighbour then we have a wheel
    int i;
    for(i = 0; i < universalNeighbourCount; i++){
        if(wheelsTable==NULL){
            wheelsTable = newFrequencyTableElement(size);
        } else {
            addToFrequencyTable(wheelsTable, size);
        }
    }
}

void checkSimpleCycles_impl(
            GRAPH graph, ADJACENCY adj, int firstVertex,
            int secondVertex, int currentVertex, int size,
            int universalNeighbourCount){
    int i, w;
    for(i=0; i<adj[currentVertex]; i++){
        int neighbour = graph[currentVertex][i];
        if((neighbour != firstVertex) && verticesInCycle[neighbour]){
            //vertex already in cycle (and not first vertex)
            continue;
        } else if(neighbour < firstVertex){
            //cycle not in canonical form
            continue;
        } else if(neighbour == firstVertex){
            //we have returned to the first vertex
            if(currentVertex <= secondVertex){
                //cycle not in canonical form
                continue;
            }
            handleSimpleCycle(size, universalNeighbourCount);
        } else {
            //we continue the cycle
            verticesInCycle[neighbour] = TRUE;
            
            //compute the new universal neighbours
            int universalNeighbourCount = 0;
            for(w = 1; w <= graph[0][0]; w++){
                universalNeighbours[size + 1][w] = universalNeighbours[size][w] &&
                        neighbourhoods[neighbour][w];
                if(universalNeighbours[size + 1][w]){
                    universalNeighbourCount++;
                }
            }
                
            if(universalNeighbourCount){
                checkSimpleCycles_impl(graph, adj, firstVertex, secondVertex,
                    neighbour, size+1, universalNeighbourCount);
            }
            verticesInCycle[neighbour] = FALSE;
        }
        if(interrupted) break;
    }
}

void buildWheelsTable(GRAPH graph, ADJACENCY adj){
    int v, w, i;
    int order = graph[0][0];
    
    for(v = 1; v <= order; v++){
        for(w=1; w <= order; w++){
            neighbourhoods[v][w] = FALSE;
        }
        for(i=0; i<adj[v]; i++){
            neighbourhoods[v][graph[v][i]] = TRUE;
        }
    }
    
    for(v = 1; v < order; v++){ //intentionally skip v==order!
        verticesInCycle[v] = TRUE;
        for(i=0; i<adj[v]; i++){
            int neighbour = graph[v][i];
            if(neighbour < v){
                //cycle not in canonical form: we have seen this cycle already
                continue;
            } else {
                //start a cycle
                verticesInCycle[neighbour] = TRUE;
                
                int universalNeighbourCount = 0;
                for(w = 1; w <= order; w++){
                    universalNeighbours[2][w] = neighbourhoods[v][w] &&
                            neighbourhoods[neighbour][w];
                    if(universalNeighbours[2][w]){
                        universalNeighbourCount++;
                    }
                }
                if(universalNeighbourCount){
                    checkSimpleCycles_impl(graph, adj, v, neighbour, neighbour,
                        2, universalNeighbourCount);
                }
                verticesInCycle[neighbour] = FALSE;
            }
            if(interrupted) break;
        }
        verticesInCycle[v] = FALSE;
        if(interrupted) break;
    }
}

void printWheelsOverview(FREQUENCY_TABLE *ft){
    if(ft == NULL){
        return;
    }
    
    printWheelsOverview(ft->smaller);
    if(ft->key==3){
        if(!interrupted && (ft->frequency % 4 != 0)){
            fprintf(stderr, "ERROR -- wrong number of wheels with 3 spokes!\n");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "%d wheel%s with %d spokes.\n",
                (ft->frequency)/4, (ft->frequency)/4 == 1 ? "" : "s", ft->key);
    } else {
        fprintf(stderr, "%d wheel%s with %d spokes.\n",
                ft->frequency, ft->frequency == 1 ? "" : "s", ft->key);
    }
    printWheelsOverview(ft->larger);
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s gives an overview of wheels in graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -t n\n");
    fprintf(stderr, "    --timeout n\n");
    fprintf(stderr, "       Stop looking for wheels after n seconds.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    unsigned long int timeOut = 0;
    
    GRAPH graph;
    ADJACENCY adj;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"timeout", required_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "ht:", long_options, &option_index)) != -1) {
        switch (c) {
            case 't':
                timeOut = strtoul(optarg, NULL, 10);
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
    
    //register handlers for signals 
    signal(SIGALRM, handleAlarmSignal);
    signal(SIGINT, handleInterruptSignal);
    
    if(timeOut) {
        fprintf(stderr, "Sending time-out signal in %lu second%s.\n", 
                            timeOut, timeOut == 1 ? "" : "s");
        alarm(timeOut);
    }
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (!interrupted && readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        
        buildWheelsTable(graph, adj);
        
        printWheelsOverview(wheelsTable);
        
        freeFrequencyTable(wheelsTable);
        wheelsTable = NULL;
    }

    return (EXIT_SUCCESS);
}

