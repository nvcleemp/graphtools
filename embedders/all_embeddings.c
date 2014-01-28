/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2014 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs from standard in and
 * writes all combinatorial embeddings to standard out. 
 * This program does not check for isomorphic copies.  
 * 
 * 
 * Compile with:
 *     
 *     cc -o all_embeddings -O4 all_embeddings.c ../multicode/shared/multicode_base.c\
 *       ../multicode/shared/multicode_input.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "../multicode/shared/multicode_base.h"
#include "../multicode/shared/multicode_input.h"

int readGraphs = 0;
int writtenGraphs = 0;

void writeAsPlanarCodeChar(GRAPH graph, ADJACENCY adj, FILE *f){
    int i, j;
    
    int vertexCount = graph[0][0];
    
    //write the number of vertices
    fputc(vertexCount, f);
    
    for(i=1; i<=vertexCount; i++){
        for(j=0; j<adj[i]; j++){
            fputc(graph[i][j], f);
        }
        fputc(0, f);
    }
}

static void writeShort(unsigned short value, FILE *f){
    if (fwrite(&value, sizeof (unsigned short), 1, f) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }
}

void writeAsPlanarCodeShort(GRAPH graph, ADJACENCY adj, FILE *f){
    int i, j;
    
    int vertexCount = graph[0][0];
    
    //write the number of vertices
    fputc(0, f);
    writeShort(graph[0][0], f);
    
    for(i=1; i<=vertexCount; i++){
        for(j=0; j<adj[i]; j++){
            writeShort(graph[i][j], f);
        }
        writeShort(0, f);
    }
}

void writeAsPlanarCode(GRAPH graph, ADJACENCY adj, FILE *f){
    static boolean first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>planar_code<<");
    }
    
    if (graph[0][0] <= 252) {
        writeAsPlanarCodeChar(graph, adj, f);
    } else if (graph[0][0] <= 252*256) {
        writeAsPlanarCodeShort(graph, adj, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}

void createAllEmbeddings_impl(int v, GRAPH graph, ADJACENCY adj){
    //first check whether the embedding is completed
    if(v == graph[0][0]+1){
        writeAsPlanarCode(graph, adj, stdout);
        writtenGraphs++;
        return;
    }
    
    //if v has degree 1 or degree 2, continue to next vertex
    if(adj[v]==1 || adj[v]==2){
        createAllEmbeddings_impl(v + 1, graph, adj);
        return;
    }
    
    //if all or all but one neighbour of v have degree 1, then just continue to next vertex
    int degree1Neighbours = 0;
    int i;
    for(i = 0; i < adj[v]; i++){
        if(adj[graph[v][i]]==1){
            degree1Neighbours++;
        }
    }
    if(degree1Neighbours > adj[v]-1){
        createAllEmbeddings_impl(v + 1, graph, adj);
        return;
    }
    
    //otherwise try all permutations
    int perms[adj[v]];
    int original[adj[v]];
    for(i = 0; i < adj[v]; i++){
        perms[i] = i;
        original[i] = graph[v][i];
    }
    
    int k, l;
    while(TRUE){
        //find k
        k = adj[v]-2;
        while(k>0 && perms[k] > perms[k+1]){
            k--;
        }
        if(k==0){
            break;
        }
        
        //find l
        l = adj[v] - 1;
        while(perms[k] > perms[l]){
            l--;
        }
        //loop will stop because perms[k] < perms[k+1]
        
        //swap perms[k] and perms[l]
        int temp = perms[k];
        perms[k] = perms[l];
        perms[l] = temp;
        
        //reverse perms[k+1] to perms[degree[v]-1]
        int steps = (adj[v] - k)/2; //TODO
        for(i = 1; i <= steps; i++){
            temp = perms[k+i];
            perms[k+i] = perms[adj[v]-i];
            perms[adj[v]-i] = temp;
        }
        
        //apply permutation
        for(i = 0; i < adj[v]; i++){
            graph[v][i] = original[perms[i]];
        }
        
        //embed other vertices
        createAllEmbeddings_impl(v + 1, graph, adj);
    }
}

inline void createAllEmbeddings(GRAPH graph, ADJACENCY adj){
    createAllEmbeddings_impl(1, graph, adj);
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s generates all combinatorial embeddings of the input graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

    GRAPH graph;
    ADJACENCY adj;
    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
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

    /*=========== read planar graphs ===========*/

    unsigned short code[MAXCODELENGTH];
    int length;
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        readGraphs++;
        createAllEmbeddings(graph, adj);
    }
    
    fprintf(stderr, "Read %d graph%s.\n", readGraphs, readGraphs==1 ? "" : "s");
}
