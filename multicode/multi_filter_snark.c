/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads graphs in multicode format from standard in and
 * writes those that are snarks to standard out in multicode format.
 * 
 * Compile with:
 *     
 *     cc -o multi_filter_snarks -O4  multi_filter_snarks.c \
 *     shared/multicode_base.c shared/multicode_input.c shared/multicode_output.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "shared/multicode_base.h"
#include "shared/multicode_input.h"
#include "shared/multicode_output.h"

#define DEBUGASSERT(assertion) if(!(assertion)) {fprintf(stderr, "%s:%u Assertion failed: %s\n", __FILE__, __LINE__, #assertion); fflush(stderr); exit(1);}
//#define DEBUGASSERT(assertion)

boolean onlyCount = FALSE;

boolean verbose = FALSE;

boolean girthAtLeast5 = TRUE;

boolean cyclically4EdgeConnected = TRUE;

//------------------Start connectivity methods--------------------------------

//adapted from snarkhunter


void label_dfs_cut(GRAPH graph, int vertex, int previous_vertex, int number[], int *new_number, int *cut, int *lowpt) {
    int i, neighbour, number_neighbour;
    int locallowpt_vertex, lowpt_neighbour;

    number[vertex] = locallowpt_vertex = *new_number;
    (*new_number)++;
    for(i = 0; i < 3; i++) {
        neighbour = graph[vertex][i];
        if((number_neighbour = number[neighbour])) {
            if((neighbour != previous_vertex) && (number_neighbour < locallowpt_vertex))
                locallowpt_vertex = number_neighbour;
        } else {
            label_dfs_cut(graph, neighbour, vertex, number, new_number, cut, &lowpt_neighbour);
            if(lowpt_neighbour < locallowpt_vertex)
                locallowpt_vertex = lowpt_neighbour;
        }
    }

    if(locallowpt_vertex == number[vertex])
        *cut = 1;
    *lowpt = locallowpt_vertex;

}

/**
 * Checks if the graph without vertices forbidden_vertex1 and forbidden_vertex2 has a bridge.
 * Returns 0 if it has a bridge, else returns 1.
 *
 * If both forbidden_vertex1 >= 0 and forbidden_vertex2 >= 0, forbidden_vertex1
 * will always be < forbidden_vertex2.
 * If forbidden_vertex1 and forbidden_vertex2 are both < 0, no vertices will be removed.
 * If only forbidden_vertex2 < 0 (and forbidden_vertex1 >= 0), only forbidden_vertex1
 * will be removed.
 * If forbidden_vertex1 < 0, forbidden_vertex2 must also be < 0.
 *
 * Remark: because we're dealing with cubic graphs, a graph has a bridge iff
 * it has a cutvertex.
 */
int is_twoconnected(GRAPH graph, int forbidden_vertex1, int forbidden_vertex2) {
    int i, number[graph[0][0]], start;
    int cut, dummy, nextnumber;

    cut = 0;
    for(i = 1; i <= graph[0][0]; i++) number[i] = 0;
    number[forbidden_vertex1] = INT_MAX;
    if(forbidden_vertex1 >= forbidden_vertex2) {
        fprintf(stderr, "Error: forbidden_vertex1 must always be smaller than forbidden_vertex2\n");
        exit(1);
    }
    number[forbidden_vertex2] = INT_MAX;
    if(forbidden_vertex1 == 1) {
        for(start = 1; start <= graph[0][0]; start++)
            if(start != forbidden_vertex2)
                break;
    } else
        start = 1;
    number[start] = 1;
    nextnumber = 2;
    if(graph[start][0] != forbidden_vertex1 && graph[start][0] != forbidden_vertex2)
        label_dfs_cut(graph, graph[start][0], start, number, &nextnumber, &cut, &dummy);
    else if(graph[start][1] != forbidden_vertex1 && graph[start][1] != forbidden_vertex2)
        label_dfs_cut(graph, graph[start][1], start, number, &nextnumber, &cut, &dummy);
    else
        label_dfs_cut(graph, graph[start][2], start, number, &nextnumber, &cut, &dummy);
    if(cut || (number[graph[start][1]] == 0) || (number[graph[start][2]] == 0))
        return 0;
    
    return 1;

}

boolean haveCommonNeighbour(GRAPH graph, int v1, int v2){
    int i, j;
    for(i = 0; i < 3; i++){
        for(j = 0; j< 3; j++){
            if(graph[v1][i]==graph[v2][j]) return TRUE;
        }
    }
    return FALSE;
}

boolean areAdjacent_3regular(GRAPH graph, int v1, int v2){
    int i;
    for(i = 0; i < 3; i++){
        if(graph[v1][i]==v2) return TRUE;
    }
    return FALSE;
}

/**
 * Returns TRUE if current_graph has a nontrivial threecut, else returns FALSE.
 *
 * Algorithm: removes 2 vertices and tests if the resulting graph has a bridge.
 *
 * This algorithm is not very efficient but it's simple.
 */
int has_nontrivial_threecut(GRAPH graph) {

    int i, j;
    for(i = 1; i <= graph[0][0] - 1; i++) {
        for(j = i + 1; j <= graph[0][0]; j++) {
            if(!haveCommonNeighbour(graph, i, j)) { 
                //Don't continue if there is a common neighbour: if common neighbour, there will be a trivial threecut
                //Don't test connectivity if i and j are neighbours, because if it's not twoconnected after
                //the removal of i and j, there will also be other nonadjacent cutvertices which will yield a bridge
                if(!areAdjacent_3regular(graph, i, j)&& !is_twoconnected(graph, i, j)) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//------------------End connectivity methods--------------------------------

//------------------Start girth methods--------------------------------

/**
  * Return the minimum of the given girth and the length of the shortest
  * cycle through the given vertex.
  */
int pseudoGirth(GRAPH graph, int v, int girth) {
    int i;
    
    int n = graph[0][0];
        
    int queue[n];
    int head = 0;
    int tail = 0;

    int levels[n+1];
    for(i=1; i<=n; i++){
        levels[i]=-1;
    }
    queue[head] = v;

    int lim = girth / 2;

    head ++;
    levels[v] = 0;
    while (head > tail) {
        int vertex = queue[tail];
        int d = levels[vertex];
        if (d >= lim)
            return girth; // can never improve current girth
        tail ++;
        for (i=0; i<3; i++) {
            int nb = graph[vertex][i];
            int e = levels[nb];
            if (e < 0) {
                // not yet encountered
                levels[nb] = d+1;
                queue[head] = nb;
                head ++;
            } else if (e == d) {
                // odd cycle
                return 2*d + 1;
            } else if (e > d) {
                // even cycle
                if (girth > 2*e) {
                    girth = 2*e; // == 2*d+2
                }
            } //else {
                // returns towards v, ignore
            //}
        }
    }
    return girth;
}

/**
  * Return the minimum of the given upperBound and the girth.
  */
int girth(GRAPH graph, int upperBound){
    int i;
    int girth = upperBound;
    
    for(i=1; i<=graph[0][0]; i++){
        girth = pseudoGirth(graph, i, girth);
    }
    
    return girth;
}

//------------------End girth methods--------------------------------

//------------------Start colouring methods--------------------------------

/* Store the colouring of the current graph
* This goes to 4 for efficiency reason, only used until 3 at most
*/
int colours[MAXN+1][4];

int coloursAroundVertex[MAXN+1];

int neighbourToIndexMapping[MAXN+1][MAXN+1];

static int markvalue_edges = 3;
unsigned int marks_edges[MAXN+1][4]; //goes to 4 for efficiency reason, only used until 3 at most
#define RESETMARKS_EDGES {int mki, mkj; if ((markvalue_edges += 1) > 3) \
{ markvalue_edges = 1; for(mki=0;mki<MAXN+1;++mki) for(mkj=0;mkj<3;++mkj) marks_edges[mki][mkj]=0;}}
#define MARK_EDGES(v, w) marks_edges[v][w] = markvalue_edges
#define UNMARK_EDGES(v, w) marks_edges[v][w] = markvalue_edges - 1
#define ISMARKED_EDGES(v, w) (marks_edges[v][w] == markvalue_edges)

void initIs3Colourable(GRAPH graph) {
    RESETMARKS_EDGES;
    int i, j;
    for (i = 1; i <= graph[0][0]; i++) {
        coloursAroundVertex[i] = 0;
        for(j = 0; j < 3; j++) {
            neighbourToIndexMapping[i][graph[i][j]] = j;
        }
    }

}

int isConflictingColouring(int currentVertex, int colour) {
    int i;
    for(i = 0; i < 3; i++) {
        if(ISMARKED_EDGES(currentVertex, i) && colours[currentVertex][i] == colour)
            return TRUE;
    }
    return FALSE;

}


inline void determineAvailableColours(int usedColour, int *availableColours) {
    switch(usedColour) {
        case 1:
            availableColours[0] = 2;
            availableColours[1] = 3;
            break;
        case 2:
            availableColours[0] = 1;
            availableColours[1] = 3;
            break;
        case 3:
            availableColours[0] = 1;
            availableColours[1] = 2;
            break;
        default:
            fprintf(stdout, "Invalid previous colour");
            exit(0);
    }
}

void determineUncolouredVertex(int vertex, int *uncolouredVertex, int *missingColour, GRAPH graph) {
    DEBUGASSERT(coloursAroundVertex[vertex] == 2);

    int i;
    int sum_colours = 0;
    for(i = 0; i < 3; i++) {
        if(!ISMARKED_EDGES(vertex, i)) {
            *uncolouredVertex = graph[vertex][i];
        } else {
            sum_colours += colours[vertex][i];
        }
    }
    switch(sum_colours) {
        case 3:
            *missingColour = 3;
            break;
        case 4:
            *missingColour = 2;
            break;
        case 5:
            *missingColour = 1;
            break;
        default:
            fprintf(stderr, "Invalid sum_colours: %d\n", sum_colours);
            fprintf(stderr, "vertex %d\n", vertex);
            exit(0);
    }

}

void unmarkColours(int nonfree_labelled[][2], int nonfree_labelled_size) {
    int i;
    int vertex0, vertex1;
    for(i = 0; i < nonfree_labelled_size; i++) {
        vertex0 = nonfree_labelled[i][0];
        vertex1 = nonfree_labelled[i][1];
        UNMARK_EDGES(vertex0, neighbourToIndexMapping[vertex0][vertex1]);
        UNMARK_EDGES(vertex1, neighbourToIndexMapping[vertex1][vertex0]);
        coloursAroundVertex[vertex0]--;
        coloursAroundVertex[vertex1]--;
    }
}

/*
* Starts from currentvertex and sets colours that are fixed by the current colours.
* The edges that are coloured are stored, so this can be rolled back in case of a conflict.
*/
boolean propagateFixedColours(int currentVertex, int nonfree_labelled[][2], int *nonfree_labelled_size, GRAPH graph) {
    int uncolouredVertex, missingColour;
    //while the colour is fixed for currentVertex
    while(coloursAroundVertex[currentVertex] == 2) {
        //find the colour of the remaining edge
        determineUncolouredVertex(currentVertex, &uncolouredVertex, &missingColour, graph);
        //check that this colour gives no conflicts
        if(!isConflictingColouring(uncolouredVertex, missingColour)) {
            int indexUncolouredVertex = neighbourToIndexMapping[currentVertex][uncolouredVertex];
            int indexCurrentVertex = neighbourToIndexMapping[uncolouredVertex][currentVertex];
            colours[currentVertex][indexUncolouredVertex] = missingColour;
            colours[uncolouredVertex][indexCurrentVertex] = missingColour;

            MARK_EDGES(currentVertex, indexUncolouredVertex);
            MARK_EDGES(uncolouredVertex, indexCurrentVertex);
            coloursAroundVertex[currentVertex] = 3;
            coloursAroundVertex[uncolouredVertex]++;

            nonfree_labelled[*nonfree_labelled_size][0] = currentVertex;
            nonfree_labelled[*nonfree_labelled_size][1] = uncolouredVertex;
            (*nonfree_labelled_size)++;

            currentVertex = uncolouredVertex;
        } else {
            //in case of conflicts: remove colours and return FALSE
            unmarkColours(nonfree_labelled, *nonfree_labelled_size);
            return FALSE;
        }
    }
    return TRUE;

}

/*
* For this method we assume that the graph is a simple graph.
*/
int tryExtendingColouring(int numberOfColouredEdges, int numberOfEdges, GRAPH graph) {
    if(numberOfColouredEdges != numberOfEdges) {
        int currentVertex;

        //look for vertex with uncoloured edges
        //edges around vertex 1 are always coloured
        for(currentVertex = 2; currentVertex <= graph[0][0]; currentVertex++) {
            if(coloursAroundVertex[currentVertex] == 1) {
                break;
            }
            DEBUGASSERT(coloursAroundVertex[currentVertex] != 2)
        }
        DEBUGASSERT(currentVertex <= graph[0][0]);

        int usedColour; //the colour already used at this vertex
        int i;
        for(i = 0; i < 3; i++) {
            if(ISMARKED_EDGES(currentVertex, i)) {
                usedColour = colours[currentVertex][i];
                break;
            }
        }
        DEBUGASSERT(i < 3);

        int availableVertices[2];
        int indexAvailableVertex0 = (i + 1) % 3;
        int indexAvailableVertex1 = (i + 2) % 3;
        availableVertices[0] = graph[currentVertex][indexAvailableVertex0];
        availableVertices[1] = graph[currentVertex][indexAvailableVertex1];

        int availableColours[2];
        determineAvailableColours(usedColour, availableColours);

        DEBUGASSERT((availableColours[0] > 0 && availableColours[0] < 4) && (availableColours[1] > 0 && availableColours[1] < 4));

        int nonfree_labelled[numberOfEdges - numberOfColouredEdges][2];
        int j;
        for(i = 0; i < 2; i++) {
            if(isConflictingColouring(availableVertices[0], availableColours[i]) ||
                    isConflictingColouring(availableVertices[1], availableColours[(i + 1) % 2])) {
                continue;
            }

            int indexCurrentVertex0 = neighbourToIndexMapping[availableVertices[0]][currentVertex];
            int indexCurrentVertex1 = neighbourToIndexMapping[availableVertices[1]][currentVertex];

            colours[availableVertices[0]][indexCurrentVertex0] = availableColours[i];
            colours[availableVertices[1]][indexCurrentVertex1] = availableColours[(i + 1) % 2];

            MARK_EDGES(availableVertices[0], indexCurrentVertex0);
            MARK_EDGES(availableVertices[1], indexCurrentVertex1);

            coloursAroundVertex[availableVertices[0]]++;
            coloursAroundVertex[availableVertices[1]]++;

            int nonfree_labelled_size = 0;
            boolean abort = 0;
            for(j = 0; j < 2;j++) {
                if(!propagateFixedColours(availableVertices[j], nonfree_labelled, &nonfree_labelled_size, graph)) {
                    DEBUGASSERT(nonfree_labelled_size <= numberOfEdges - numberOfColouredEdges)
                    abort = TRUE;
                    break;
                }
                DEBUGASSERT(nonfree_labelled_size <= numberOfEdges - numberOfColouredEdges)
            }
            //fprintf(stderr, "nonfree_labelled_size: %d and abort == %d \n", nonfree_labelled_size, abort);
            if(!abort) {
                colours[currentVertex][indexAvailableVertex0] = availableColours[i];
                colours[currentVertex][indexAvailableVertex1] = availableColours[(i + 1) % 2];
                MARK_EDGES(currentVertex, indexAvailableVertex0);
                MARK_EDGES(currentVertex, indexAvailableVertex1);
                coloursAroundVertex[currentVertex] = 3;

                if(tryExtendingColouring(numberOfColouredEdges + nonfree_labelled_size + 2, numberOfEdges, graph)) {
                    return TRUE;
                } else {
                    unmarkColours(nonfree_labelled, nonfree_labelled_size);
                }
                UNMARK_EDGES(currentVertex, indexAvailableVertex0);
                UNMARK_EDGES(currentVertex, indexAvailableVertex1);
                coloursAroundVertex[currentVertex] = 1;
            }

            UNMARK_EDGES(availableVertices[0], indexCurrentVertex0);
            UNMARK_EDGES(availableVertices[1], indexCurrentVertex1);

            //number_of_colours_snarks[current_vertex] = 1;
            coloursAroundVertex[availableVertices[0]]--;
            coloursAroundVertex[availableVertices[1]]--;
        }
        return FALSE;
    } else {
        return TRUE;
    }
}

/*
* This method only works for simple graphs!
* Stores the colouring in the array
*/
boolean is3ColourableGraph(GRAPH graph) {
    initIs3Colourable(graph);

    int currentVertex = 1;
    int i, neighbour, currentIndex;
    for(i = 0; i < 3; i++) {
        colours[currentVertex][i] = i + 1;
        neighbour = graph[currentVertex][i];
        currentIndex = neighbourToIndexMapping[neighbour][currentVertex];
        colours[neighbour][currentIndex] = i + 1;

        MARK_EDGES(currentVertex, i);
        MARK_EDGES(neighbour, currentIndex);
        coloursAroundVertex[neighbour] = 1;
    }
    coloursAroundVertex[currentVertex] = 3;
    int numberOfEdges = (3*graph[0][0])/2;
    return tryExtendingColouring(3, numberOfEdges, graph);
}

//------------------end colouring methods--------------------------------

boolean is3Regular(int order, ADJACENCY adj){
    int i;
    
    for(i=1; i<=order; i++){
        if(adj[i]!=3){
            return FALSE;
        }
    }
    
    return TRUE;
}

boolean isSnark(GRAPH graph, ADJACENCY adj){
    if(!is3Regular(graph[0][0], adj)){
        if(verbose){
            fprintf(stderr, "Graph is not 3-regular.\n");
        }
        return FALSE;
    }
    
    if(is3ColourableGraph(graph)){
        if(verbose){
            fprintf(stderr, "Graph is 3-edge-colourable.\n");
        }
        return FALSE;
    }
    
    if(girthAtLeast5 || cyclically4EdgeConnected){

        int g = girth(graph, 5); //calculate the minimum of the girth and 5

        if(girthAtLeast5 && g<5){
            if(verbose){
                fprintf(stderr, "Graph has girth %d.\n", g);
            }
            return FALSE;
        }

        if(cyclically4EdgeConnected && graph[0][0] > 4){
            if(g<4){
                if(verbose){
                    fprintf(stderr, "Graph has girth %d.\n", g);
                }
                return FALSE;
            }
            
            if(graph[0][0]<=6){
                if(verbose){
                    fprintf(stderr, "Graph has %d vertices.\n", graph[0][0]);
                }
                return FALSE;
            }
            
            if(has_nontrivial_threecut(graph)){
                if(verbose){
                    fprintf(stderr, "Graph has non-trivial 3-cut.\n");
                }
                return FALSE;
            }
        }
    }
    
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters out graphs that are snarks.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -g, --girth\n");
    fprintf(stderr, "       Do not include girth >= 5 in the definition of snark.\n");
    fprintf(stderr, "    -C, --connectivity\n");
    fprintf(stderr, "       Do not include cyclically-4-edge-connected in the definition of snark.\n");
    fprintf(stderr, "    -v, --verbose\n");
    fprintf(stderr, "       For each graph that is rejected, the reason is printed.\n");
    fprintf(stderr, "    -c, --count\n");
    fprintf(stderr, "       Only count the number of graphs that are snarks.\n");
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
    
    GRAPH graph;
    ADJACENCY adj;
    
    int graphsRead = 0;
    int graphsFiltered = 0;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"connectivity", no_argument, NULL, 'C'},
        {"girth", no_argument, NULL, 'g'},
        {"verbose", no_argument, NULL, 'v'},
        {"count", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hcvCg", long_options, &option_index)) != -1) {
        switch (c) {
            case 'C':
                cyclically4EdgeConnected = FALSE;
                break;
            case 'g':
                girthAtLeast5 = FALSE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 'c':
                onlyCount = TRUE;
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
    
    unsigned short code[MAXCODELENGTH];
    int length;
    while (readMultiCode(code, &length, stdin)) {
        decodeMultiCode(code, length, graph, adj);
        graphsRead++;
        
        if(isSnark(graph, adj)){
            if(!onlyCount){
                writeMultiCode(graph, adj, stdout);
            }
            graphsFiltered++;
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graphsRead, graphsRead==1 ? "" : "s");
    fprintf(stderr, "Filtered %d snark%s.\n", graphsFiltered, graphsFiltered==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

