/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2016 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#define _GNU_SOURCE //needed for asprintf
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "unionfind.h"

#include "shared/planar_base.h"
#include "shared/planar_input.h"
#include "shared/planar_automorphismgroup.h"

UNIONFIND *get_orbits(PLANE_GRAPH *pg, PG_AUTOMORPHISM_GROUP *aut_group){
    UNIONFIND *orbits = unionfind_prepareNew(pg->nv);
    
    int i, j;
    
    for(i = 0; i < aut_group->size; i++){
        for(j = 0; j < pg->nv; j++){
            unionfind_union(orbits, j, aut_group->automorphisms[i][j]);
        }
    }
    
    return orbits;
}

char *name_platonic(PLANE_GRAPH *pg){
    char *name;
    switch (pg->nv) {
        case 4:
            if(asprintf(&name, "%s", "tetrahedron") == -1){
                name=NULL;
            }
            break;
        case 6:
            if(asprintf(&name, "%s", "octahedron") == -1){
                name=NULL;
            }
            break;
        case 8:
            if(asprintf(&name, "%s", "cube") == -1){
                name=NULL;
            }
            break;
        case 12:
            if(asprintf(&name, "%s", "icosahedron") == -1){
                name=NULL;
            }
            break;
        case 20:
            if(asprintf(&name, "%s", "dodecahedron") == -1){
                name=NULL;
            }
            break;
        default:
            if(asprintf(&name, "%s", "ERROR") == -1){
                name=NULL;
            }
            break;
    }
    return name;
}

char *name_archimedean_2(PLANE_GRAPH *pg, UNIONFIND *face_orbits){
    //name the archimedean solids with 2 face orbits
    int i, face_size1, face_size2, minFace, maxFace;
    char *name;
    
    i = 0;
    while(i < pg->nf && face_orbits->parent[i]!=i) i++;
    face_size1 = pg->faceSize[i];
    i++;
    while(i < pg->nf && face_orbits->parent[i]!=i) i++;
    face_size2 = pg->faceSize[i];
    
    if(face_size1 < face_size2){
        minFace = face_size1;
        maxFace = face_size2;
    } else {
        minFace = face_size2;
        maxFace = face_size1;
    }
    
    if (minFace==3){
        if(maxFace==4){
            if(asprintf(&name, "%s", "cuboctahedron") == -1){
                name = NULL;
            }
        } else if(maxFace==5){
            if(asprintf(&name, "%s", "icosidodecahedron") == -1){
                name = NULL;
            }
        } else if(maxFace==6){
            if(asprintf(&name, "%s", "truncated tetrahedron") == -1){
                name = NULL;
            }
        } else if(maxFace==8){
            if(asprintf(&name, "%s", "truncated cube") == -1){
                name = NULL;
            }
        } else if(maxFace==10){
            if(asprintf(&name, "%s", "truncated dodecahedron") == -1){
                name = NULL;
            }
        } else {
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        }
    } else if(minFace==4 && maxFace==6){
        if(asprintf(&name, "%s", "truncated octahedron") == -1){
            name = NULL;
        }
    } else if(minFace==5 && maxFace==6){
        if(asprintf(&name, "%s", "truncated icosahedron") == -1){
            name = NULL;
        }
    } else {
        if(asprintf(&name, "%s", "ERROR") == -1){
            name = NULL;
        }
    }
    return name;
}

char *name_archimedean_3(PLANE_GRAPH *pg, UNIONFIND *face_orbits){
    //name the archimedean solids with 3 face orbits
    int i, faceSize1, faceSize2, faceSize3, minFace, middleFace, maxFace;
    char *name;
    
    i = 0;
    while(i < pg->nf && face_orbits->parent[i]!=i) i++;
    faceSize1 = pg->faceSize[i];
    i++;
    while(i < pg->nf && face_orbits->parent[i]!=i) i++;
    faceSize2 = pg->faceSize[i];
    i++;
    while(i < pg->nf && face_orbits->parent[i]!=i) i++;
    faceSize3 = pg->faceSize[i];
    
    if(faceSize1 < faceSize2){
        if(faceSize3 < faceSize1){
            minFace = faceSize3;
            middleFace = faceSize1;
            maxFace = faceSize2;
        } else if (faceSize3 < faceSize2){
            minFace = faceSize1;
            middleFace = faceSize3;
            maxFace = faceSize2;
        } else {
            minFace = faceSize1;
            middleFace = faceSize2;
            maxFace = faceSize3;
        }
    } else {
        if(faceSize3 < faceSize2){
            minFace = faceSize3;
            middleFace = faceSize2;
            maxFace = faceSize1;
        } else if (faceSize3 < faceSize1){
            minFace = faceSize2;
            middleFace = faceSize3;
            maxFace = faceSize1;
        } else {
            minFace = faceSize2;
            middleFace = faceSize1;
            maxFace = faceSize3;
        }
    }
    
    if (minFace==3){
        if(middleFace==3 && maxFace==4){
            if(asprintf(&name, "%s", "snub cube") == -1){
                name = NULL;
            }
        } else if(middleFace==3 && maxFace==5){
            if(asprintf(&name, "%s", "snub dodecahedron") == -1){
                name = NULL;
            }
        } else if(middleFace==4 && maxFace==4){
            if(asprintf(&name, "%s", "rhombicuboctahedron") == -1){
                name = NULL;
            }
        } else if(middleFace==4 && maxFace==5){
            if(asprintf(&name, "%s", "rhombicosidodecahedron") == -1){
                name = NULL;
            }
        } else {
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        }
    } else if(minFace==4 && middleFace==6){
        if(maxFace==8){
            if(asprintf(&name, "%s", "truncated cuboctahedron") == -1){
                name = NULL;
            }
        } else if(maxFace==10){
            if(asprintf(&name, "%s", "truncated icosidodecahedron") == -1){
                name = NULL;
            }
        } else {
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        }
    } else {
        if(asprintf(&name, "%s", "ERROR") == -1){
            name = NULL;
        }
    }
    return name;
}

char *name_prism_or_antiprism(PLANE_GRAPH *pg, UNIONFIND *faceOrbits){
    int i, j;
    char *name;
    
    i = 0;
    while(i < pg->nf && (faceOrbits->parent[i]!=i || faceOrbits->treeSize[i]!=2)) i++;
    j = 0;
    while(j < pg->nf && (faceOrbits->parent[j]!=j || faceOrbits->treeSize[j]==2)) j++;
    
    if(pg->faceSize[j]==4){
        //prism
        if(i == pg->nf){
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        } else if(asprintf(&name, "%d-gonal prism", pg->faceSize[i]) == -1){
            name = NULL;
        }
    } else if(pg->faceSize[j]==3){
        //antiprism
        if(i == pg->nf){
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        } else if(asprintf(&name, "%d-gonal antiprism", pg->faceSize[i]) == -1){
            name = NULL;
        }
    } else {
        if(asprintf(&name, "%s", "ERROR") == -1){
            name = NULL;
        }
    }
    
    return name;
}

char *name_uniform_polyhedron(PLANE_GRAPH *pg){
    int i;
    char *name;
    
    if(pg->degree[0]==2){
        if(asprintf(&name, "cycle on %d vertices", pg->nv) == -1){
            name = NULL;
        }
        return name;
    }
    
    PLANE_GRAPH *dual = getDualGraph(pg);
    PG_AUTOMORPHISM_GROUP *autGroupDual = determineAutomorphisms(dual);
    
    UNIONFIND *orbitsDual = get_orbits(dual, autGroupDual);
    
    if(orbitsDual->setCount == 1){
        unionfind_free(orbitsDual);
        freeAutomorphismGroup(autGroupDual);
        free(dual);
        return name_platonic(pg);
    } else if(orbitsDual->setCount == 2){
        int minSetSize = dual->nv;
        for(i = 0; i < dual->nv; i++){
            if(orbitsDual->parent[i] == i && orbitsDual->treeSize[i] < minSetSize){
                minSetSize = orbitsDual->treeSize[i];
            }
        }
        if(minSetSize == 2){
            return name_prism_or_antiprism(pg, orbitsDual);
        } else {
            return name_archimedean_2(pg, orbitsDual);
        }
    } else {
        return name_archimedean_3(pg, orbitsDual);
    }
    if(asprintf(&name, "%s", "uniform polyhedron") == -1){
        name = NULL;
    }
    return name;
}

char *name_pyramid(PLANE_GRAPH *pg){
    char *name;
    if(asprintf(&name, "%d-gonal pyramid", pg->nv-1) == -1){
        name = NULL;
    }
    
    return name;
}

char *name_two_orbit_polyhedra_with_poles(PLANE_GRAPH *pg){
    char *name;
    PLANE_GRAPH *dual = getDualGraph(pg);
    PG_AUTOMORPHISM_GROUP *autGroupDual = determineAutomorphisms(dual);
    
    UNIONFIND *orbitsDual = get_orbits(dual, autGroupDual);

    if(orbitsDual->setCount == 1){
        freePlaneGraph(dual);
        freeAutomorphismGroup(autGroupDual);
        unionfind_free(orbitsDual);
        if(pg->faceSize[0]==3){
            //bypyramid
            if(asprintf(&name, "%d-gonal bipyrimad", pg->nv - 2) == -1){
                name = NULL;
            }
        } else if(pg->faceSize[0]==4){
            //trapezohedron
            if(asprintf(&name, "%d-gonal trapezohedron", (pg->nv - 2)/2) == -1){
                name = NULL;
            }
        } else {
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        }
    } else if(orbitsDual->setCount == 2){
        int i, faceSize1, faceSize2;
    
        i = 0;
        while(i < pg->nf && orbitsDual->parent[i]!=i) i++;
        faceSize1 = pg->faceSize[i];
        i++;
        while(i < pg->nf && orbitsDual->parent[i]!=i) i++;
        faceSize2 = pg->faceSize[i];
        
        freePlaneGraph(dual);
        freeAutomorphismGroup(autGroupDual);
        unionfind_free(orbitsDual);
        if(faceSize1==3 && faceSize2==3){
            //gyroelongated bipyramid
            if(asprintf(&name, "%d-gonal gyroelongated bipyrimad", (pg->nv - 2)/2) == -1){
                name = NULL;
            }
        } else if((faceSize1==3 && faceSize2==4) || (faceSize1==4 && faceSize2==3)){
            //elongated bipyramid
            if(asprintf(&name, "%d-gonal elongated bipyramid", (pg->nv - 2)/2) == -1){
                name = NULL;
            }
        } else if(pg->nv==10 && ((faceSize1==3 && faceSize2==5) ||
                                    (faceSize1==5 && faceSize2==3))){
            if(asprintf(&name, "%s", "double-pinched cube") == -1){
                name = NULL;
            }
        } else {
            if(asprintf(&name, "%s", "ERROR") == -1){
                name = NULL;
            }
        }
    } else {
        if(asprintf(&name, "%s", "ERROR") == -1){
            name = NULL;
        }
    }
    return name;
}

char *name_two_orbit_polyhedra(PLANE_GRAPH *pg, UNIONFIND *vertexOrbits){
    int i;
    int minOrbitSize = pg->nv;
    for(i = 0; i < pg->nv; i++){
        if(vertexOrbits->parent[i] == i && vertexOrbits->treeSize[i] < minOrbitSize){
            minOrbitSize = vertexOrbits->treeSize[i];
        }
    }
    //TODO: replace by just getting all orbit sizes
    
    if(minOrbitSize == 1){
        return name_pyramid(pg);
    } else if(minOrbitSize == 2){
        return name_two_orbit_polyhedra_with_poles(pg);
    }
}

char *name_graph(PLANE_GRAPH *pg){
    PG_AUTOMORPHISM_GROUP *aut_group = determineAutomorphisms(pg);
    
    UNIONFIND *orbits = get_orbits(pg, aut_group);
    
    if(orbits->setCount==1){
        unionfind_free(orbits);
        freeAutomorphismGroup(aut_group);
        return name_uniform_polyhedron(pg);
    } else if(orbits->setCount==2){
        freeAutomorphismGroup(aut_group);
        char *name = name_two_orbit_polyhedra(pg, orbits);
        unionfind_free(orbits);
        return name;
    }
    
    char *name = NULL;
    
    if(asprintf(&name, "Plane graph on %d %s", pg->nv, 
            pg->nv == 1 ? "vertex" : "vertices") == -1){
        
    }
    
    return name;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "The program %s gives names for graphs in planarcode format.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "Valid options\n=============\n");
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

    int graph_count = 0;
    
    DEFAULT_PG_INPUT_OPTIONS(options);
    options.computeDual = TRUE;
    PLANE_GRAPH *pg;
    while ((pg = readAndDecodePlanarCode(stdin, &options))) {
        graph_count++;
        char *graph_name = name_graph(pg);
        if(graph_name==NULL){
            fprintf(stdout, "UNKNOWN\n");
            fprintf(stderr, "Unable to get name for graph %d.\n", graph_count);
        } else {
            fprintf(stdout, "%s\n", graph_name);
            free(graph_name);
        }
        freePlaneGraph(pg);
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graph_count, graph_count==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

