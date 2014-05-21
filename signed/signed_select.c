/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in signed_code format from standard in and
 * writes those that were selected to standard out in signed_code format.
 * 
 * Compile with:
 *     
 *     cc -o signed_select -O4  signed_select.c \
 *     shared/signed_base.c shared/signed_input.c shared/signed_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/signed_base.h"
#include "shared/signed_input.h"
#include "shared/signed_output.h"

int edgeCounter = 0;

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters out the selected graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -m, --modulo r:m\n");
    fprintf(stderr, "       Split the input into m parts and only output part r (0<=r<m).\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] g1 g2\n", name);
    fprintf(stderr, "       %s -m r:m\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    GRAPH graph;
    ADJACENCY adj;
    
    int graphsRead = 0;
    int graphsFiltered = 0;
    
    boolean moduloEnabled = FALSE;
    int moduloRest;
    int moduloMod;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"modulo", required_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hm:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'm':
                moduloEnabled = TRUE;
                if(sscanf(optarg, "%d:%d", &moduloRest, &moduloMod)!=2){
                    fprintf(stderr, "Error while reading modulo -- exiting.\n");
                    usage(name);
                    return EXIT_FAILURE;
                }
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
        
    if (argc - optind == 0 && !moduloEnabled) {
        usage(name);
        return EXIT_FAILURE;
    } else if(argc - optind > 0 && moduloEnabled){
        usage(name);
        return EXIT_FAILURE;
    }
    
    int i;
    int selectedGraphs[argc - optind];
    for (i = 0; i < argc - optind; i++){
        selectedGraphs[i] = atoi(argv[i + optind]);
    }
    

    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readSignedCode(code, &length, stdin)) {
        int order;
        decodeSignedCode(code, length, graph, adj, &order);
        graphsRead++;
        
        if(moduloEnabled){
            if(graphsRead % moduloMod == moduloRest){
                graphsFiltered++;
                writeSignedCode(graph, adj, order, stdout);
            }
        } else if (graphsFiltered < argc - optind && (graphsRead == selectedGraphs[graphsFiltered])) {
            graphsFiltered++;
            writeSignedCode(graph, adj, order, stdout);
        }
    }
    
    fprintf(stderr, "Read %d signed graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d signed graph%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

