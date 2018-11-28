/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program reads planar graphs from standard in and
 * filters them based on the degrees.   
 * 
 * 
 * Compile with:
 *     
 *     cc -o name -O4 filter_degree_pl.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>


#ifndef MAXN
#define MAXN 64            /* the maximum number of vertices */
#endif
#define MAXE (6*MAXN-12)    /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)      /* the maximum number of faces */
#define MAXVAL (MAXN-1)  /* the maximum degree of a vertex */
#define MAXCODELENGTH (MAXN+MAXE+3)

#define INFI (MAXN + 1)

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE  1

typedef int boolean;

typedef struct e /* The data type used for edges */ {
    int start; /* vertex where the edge starts */
    int end; /* vertex where the edge ends */
    int rightface; /* face on the right side of the edge
                          note: only valid if make_dual() called */
    struct e *prev; /* previous edge in clockwise direction */
    struct e *next; /* next edge in clockwise direction */
    struct e *inverse; /* the edge that is inverse to this one */
    int mark, index; /* two ints for temporary use;
                          Only access mark via the MARK macros. */
} EDGE;

EDGE *firstedge[MAXN]; /* pointer to arbitrary edge out of vertex i. */
int degree[MAXN];

EDGE *facestart[MAXF]; /* pointer to arbitrary edge of face i. */
int face_sizes[MAXF]; /* pointer to arbitrary edge of face i. */

EDGE edges[MAXE];

static int markvalue = 30000;
#define RESETMARKS {int mki; if ((markvalue += 2) > 30000) \
       { markvalue = 2; for (mki=0;mki<MAXE;++mki) edges[mki].mark=0;}}
#define MARK(e) (e)->mark = markvalue
#define MARKLO(e) (e)->mark = markvalue
#define MARKHI(e) (e)->mark = markvalue+1
#define UNMARK(e) (e)->mark = markvalue-1
#define ISMARKED(e) ((e)->mark >= markvalue)
#define ISMARKEDLO(e) ((e)->mark == markvalue)
#define ISMARKEDHI(e) ((e)->mark > markvalue)

int nv;
int ne;

int degree_table[MAXN];
int degree_table_lower_bound[MAXN];
int degree_table_upper_bound[MAXN];


//////////////////////////////////////////////////////////////////////////////

//=============== Writing planarcode of graph ===========================

void write_planar_code_char(){
    int i;
    EDGE *e, *elast;
    
    //write the number of vertices
    fputc(nv, stdout);
    
    for(i=0; i<nv; i++){
        e = elast = firstedge[i];
        do {
            fputc(e->end + 1, stdout);
            e = e->next;
        } while (e != elast);
        fputc(0, stdout);
    }
}

void write_short(unsigned short value){
    if (fwrite(&value, sizeof (unsigned short), 1, stdout) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }
}

void write_planar_code_short(){
    int i;
    EDGE *e, *elast;
    
    //write the number of vertices
    fputc(0, stdout);
    write_short(nv);
    
    
    for(i=0; i<nv; i++){
        e = elast = firstedge[i];
        do {
            write_short(e->end + 1);
            e = e->next;
        } while (e != elast);
        write_short(0);
    }
}

void write_planar_code(){
    static int first = TRUE;
    
    if(first){
        first = FALSE;
        
        fprintf(stdout, ">>planar_code<<");
    }
    
    if (nv + 1 <= 255) {
        write_planar_code_char();
    } else if (nv + 1 <= 65535) {
        write_planar_code_short();
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
    
}

//=============== Checking for property ===========================

void build_degrees_table(){
    int i;
    
    for(i = 0; i < MAXN; i++){
        degree_table[i] = 0;
    }
    for(i = 0; i < nv; i++){
        degree_table[degree[i]]++;
    }
}

boolean has_correct_degrees(){
    int i;
    
    build_degrees_table();
    
    for(i = 0; i < MAXN; i++){
        if(degree_table[i] > degree_table_upper_bound[i] ||
                degree_table[i] < degree_table_lower_bound[i]){
            return FALSE;
        }
    }
    return TRUE;
}

//=============== Reading and decoding planarcode ===========================

EDGE *find_edge(int from, int to) {
    EDGE *e, *elast;

    e = elast = firstedge[from];
    do {
        if (e->end == to) {
            return e;
        }
        e = e->next;
    } while (e != elast);
    fprintf(stderr, "error while looking for edge from %d to %d.\n", from, to);
    exit(0);
}

void decode_planar_code(unsigned short* code) {
    /* complexity of method to determine inverse isn't that good, but will have to satisfy for now
     */
    int i, j, code_position;
    int edge_counter = 0;
    EDGE *inverse;

    nv = code[0];
    code_position = 1;

    for (i = 0; i < nv; i++) {
        degree[i] = 0;
        firstedge[i] = edges + edge_counter;
        edges[edge_counter].start = i;
        edges[edge_counter].end = code[code_position] - 1;
        edges[edge_counter].next = edges + edge_counter + 1;
        if (code[code_position] - 1 < i) {
            inverse = find_edge(code[code_position] - 1, i);
            edges[edge_counter].inverse = inverse;
            inverse->inverse = edges + edge_counter;
        } else {
            edges[edge_counter].inverse = NULL;
        }
        edge_counter++;
        code_position++;
        for (j = 1; code[code_position]; j++, code_position++) {
            if (j == MAXVAL) {
                fprintf(stderr, "MAXVAL too small: %d\n", MAXVAL);
                exit(0);
            }
            edges[edge_counter].start = i;
            edges[edge_counter].end = code[code_position] - 1;
            edges[edge_counter].prev = edges + edge_counter - 1;
            edges[edge_counter].next = edges + edge_counter + 1;
            if (code[code_position] - 1 < i) {
                inverse = find_edge(code[code_position] - 1, i);
                edges[edge_counter].inverse = inverse;
                inverse->inverse = edges + edge_counter;
            } else {
                edges[edge_counter].inverse = NULL;
            }
            edge_counter++;
        }
        firstedge[i]->prev = edges + edge_counter - 1;
        edges[edge_counter - 1].next = firstedge[i];
        degree[i] = j;

        code_position++; /* read the closing 0 */
    }

    ne = edge_counter;
}

/**
 * 
 * @param code
 * @param length
 * @param file
 * @return returns 1 if a code was read and 0 otherwise. Exits in case of error.
 */
int read_planar_code(unsigned short code[], int *length, FILE *file) {
    static int first = 1;
    unsigned char c;
    char testheader[20];
    int buffer_size, zero_counter;
    
    int read_count;


    if (first) {
        first = 0;

        if (fread(&testheader, sizeof (unsigned char), 13, file) != 13) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[13] = 0;
        if (strcmp(testheader, ">>planar_code") == 0) {

        } else {
            fprintf(stderr, "No planarcode header detected -- exiting!\n");
            exit(1);
        }
        //read reminder of header (either empty or le/be specification)
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            return FALSE;
        }
        while (c!='<'){
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                return FALSE;
            }
        }
        //read one more character
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            return FALSE;
        }
    }

    /* possibly removing interior headers -- only done for planarcode */
    if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
        //nothing left in file
        return (0);
    }

    if (c == '>') {
        // could be a header, or maybe just a 62 (which is also possible for unsigned char
        code[0] = c;
        buffer_size = 1;
        zero_counter = 0;
        code[1] = (unsigned short) getc(file);
        if (code[1] == 0) zero_counter++;
        code[2] = (unsigned short) getc(file);
        if (code[2] == 0) zero_counter++;
        buffer_size = 3;
        // 3 characters were read and stored in buffer
        if ((code[1] == '>') && (code[2] == 'p')) /*we are sure that we're dealing with a header*/ {
            while ((c = getc(file)) != '<');
            /* read 2 more characters: */
            c = getc(file);
            if (c != '<') {
                fprintf(stderr, "Problems with header -- single '<'\n");
                exit(1);
            }
            if (!fread(&c, sizeof (unsigned char), 1, file)) {
                //nothing left in file
                return (0);
            }
            buffer_size = 1;
            zero_counter = 0;
        }
    } else {
        //no header present
        buffer_size = 1;
        zero_counter = 0;
    }

    if (c != 0) /* unsigned chars would be sufficient */ {
        code[0] = c;
        if (code[0] > MAXN) {
            fprintf(stderr, "Constant N too small %d > %d \n", code[0], MAXN);
            exit(1);
        }
        while (zero_counter < code[0]) {
            code[buffer_size] = (unsigned short) getc(file);
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    } else {
        read_count = fread(code, sizeof (unsigned short), 1, file);
        if(!read_count){
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
        if (code[0] > MAXN) {
            fprintf(stderr, "Constant N too small %d > %d \n", code[0], MAXN);
            exit(1);
        }
        buffer_size = 1;
        zero_counter = 0;
        while (zero_counter < code[0]) {
            read_count = fread(code + buffer_size, sizeof (unsigned short), 1, file);
            if(!read_count){
                fprintf(stderr, "Unexpected EOF.\n");
                exit(1);
            }
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    }

    *length = buffer_size;
    return (1);


}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s filters planar graphs based on degrees.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -e d:c, --exact d:c\n");
    fprintf(stderr, "       Graphs that do not have exactly c vertices of degree d are rejected.\n");
    fprintf(stderr, "    -l d:c, --lower d:c\n");
    fprintf(stderr, "       Graphs that have less than c vertices of degree d are rejected.\n");
    fprintf(stderr, "    -u d:c, --upper d:c\n");
    fprintf(stderr, "       Graphs that have more than c vertices of degree d are rejected.\n");
    fprintf(stderr, "    -U c, --all-upper c\n");
    fprintf(stderr, "       Graphs that have more than c vertices of any degree are rejected.\n");
    fprintf(stderr, "    -a d, --any d\n");
    fprintf(stderr, "       Reset the bounds for number of vertices of degree d.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {
    
    //prepare bounds tables
    int i;
    for(i = 0; i < MAXN; i++){
        degree_table_lower_bound[i] = 0;
        degree_table_upper_bound[i] = MAXN;
    }

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"exact", required_argument, NULL, 'e'},
        {"lower", required_argument, NULL, 'l'},
        {"upper", required_argument, NULL, 'u'},
        {"all-upper", required_argument, NULL, 'U'},
        {"any", required_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;
    int deg, deg_bound;

    while ((c = getopt_long(argc, argv, "he:l:u:U:a:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'e':
                if(sscanf(optarg, "%d:%d", &deg, &deg_bound)!=2){
                    fprintf(stderr, "Error while parsing arguments -- exiting!\n");
                    fprintf(stderr, "Incorrect format: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                if(deg < 0 || deg >= MAXN){
                    fprintf(stderr, "Incorrect value for degree -- exiting!\n");
                    fprintf(stderr, "Incorrect value: %d\n", deg);
                    exit(EXIT_FAILURE);
                }
                degree_table_lower_bound[deg] = deg_bound;
                degree_table_upper_bound[deg] = deg_bound;
                break;
            case 'l':
                if(sscanf(optarg, "%d:%d", &deg, &deg_bound)!=2){
                    fprintf(stderr, "Error while parsing arguments -- exiting!\n");
                    fprintf(stderr, "Incorrect format: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                if(deg < 0 || deg >= MAXN){
                    fprintf(stderr, "Incorrect value for degree -- exiting!\n");
                    fprintf(stderr, "Incorrect value: %d\n", deg);
                    exit(EXIT_FAILURE);
                }
                degree_table_lower_bound[deg] = deg_bound;
                break;
            case 'u':
                if(sscanf(optarg, "%d:%d", &deg, &deg_bound)!=2){
                    fprintf(stderr, "Error while parsing arguments -- exiting!\n");
                    fprintf(stderr, "Incorrect format: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                if(deg < 0 || deg >= MAXN){
                    fprintf(stderr, "Incorrect value for degree -- exiting!\n");
                    fprintf(stderr, "Incorrect value: %d\n", deg);
                    exit(EXIT_FAILURE);
                }
                degree_table_upper_bound[deg] = deg_bound;
                break;
            case 'U':
                deg_bound = atoi(optarg);
                for(i = 0; i < MAXN; i++){
                    degree_table_upper_bound[i] = deg_bound;
                }
                break;
            case 'a':
                deg = atoi(optarg);
                if(deg < 0 || deg >= MAXN){
                    fprintf(stderr, "Incorrect value for degree -- exiting!\n");
                    fprintf(stderr, "Incorrect value: %d\n", deg);
                    exit(EXIT_FAILURE);
                }
                degree_table_lower_bound[deg] = 0;
                degree_table_upper_bound[deg] = MAXN;
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

    /*=========== read planar graphs ===========*/

    int graph_count = 0;
    int filtered_count = 0;

    unsigned short code[MAXCODELENGTH];
    int length;
    while (read_planar_code(code, &length, stdin)) {
        decode_planar_code(code);
        if(has_correct_degrees()){
            write_planar_code();
            filtered_count++;
        }
        graph_count++;
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graph_count, 
                graph_count==1 ? "" : "s");
    fprintf(stderr, "Written %d graph%s.\n", filtered_count, 
                filtered_count==1 ? "" : "s");
}
