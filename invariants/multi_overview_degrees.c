/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/*
 * Gives an overview of the degrees that appear in the graphs
 * that are read from standard in.
 * 
 * Compile like this:
 *     
 *     cc -o multi_overview_degrees -O4 ../multicode/shared/multicode_base.c\
 *     ../multicode/shared/multicode_input.c multi_overview_degrees.c
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

FREQUENCY_TABLE *overviewTable = NULL;

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

void buildOverviewTable(GRAPH graph, ADJACENCY adj){
    int v;
    int order = graph[0][0];
    
    overviewTable = newFrequencyTableElement(adj[1]);
    
    for(v = 2; v <= order; v++){
        addToFrequencyTable(overviewTable, adj[v]);
        if(interrupted) break;
    }
}

void printOverview(FREQUENCY_TABLE *ft){
    if(ft == NULL){
        return;
    }
    
    printOverview(ft->smaller);
    fprintf(stderr, "%d %s with degree %d.\n",
            ft->frequency, ft->frequency == 1 ? "vertex" : "vertices", ft->key);
    printOverview(ft->larger);
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s gives an overview of the degrees in graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -t n\n");
    fprintf(stderr, "    --timeout n\n");
    fprintf(stderr, "       Stop building the overview after n seconds and use the data found up to\n");
    fprintf(stderr, "       that moment.\n");
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
        
        buildOverviewTable(graph, adj);
        
        printOverview(overviewTable);
        
        freeFrequencyTable(overviewTable);
        overviewTable = NULL;
    }

    return (EXIT_SUCCESS);
}

