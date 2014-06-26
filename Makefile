
SOURCES = planar/stats_pl.c planar/count_pl.c planar/select_pl.c\
          planar/split_pl.c planar/dual_pl.c planar/non_iso_pl/non_iso_pl.c\
          planar/non_iso_pl/hashfunction.c planar/non_iso_pl/splay.c\
	  planar/subdivide_vertex.c planar/regular_pl.c planar/bipartite_pl.c\
	  planar/random_relabel_pl.c\
	  planar/adjlist2planarcode.py\
          conversion/gconv.c conversion/gconvman.txt conversion/Makefile\
          multicode/multiread.c multicode/multi_add_edges.c\
          multicode/multi_combine.c multicode/multi_remove_edges.c\
          multicode/multi_non_iso.c multicode/multi_filter_regular.c\
          multicode/multi_filter_snark.c multicode/multi_corona.c\
          multicode/multi_induced_subgraph.c multicode/multi_mycielski.c\
          multicode/multi_identify.c multicode/multi_filter_bipartite.c\
          multicode/shared/multicode_base.c multicode/shared/multicode_base.h\
          multicode/shared/multicode_input.c multicode/shared/multicode_input.h\
          multicode/shared/multicode_output.c multicode/shared/multicode_output.h\
          multicode/connect/connect_general.c multicode/connect/connect_general.h\
          multicode/connect/multi_cyclic_connect.c\
          multicode/connect/multi_complete_connect.c\
          multicode/connect/multi_path_connect.c\
          embedders/embed.c\
          visualise/writegraph2png/ant.xml visualise/writegraph2png/visualise/*\
          visualise/pngtoolkit.c visualise/pngtoolkit.h visualise/writegraph2png.c\
          invariants/multi_int_invariant.c invariants/multi_invariant_order.c\
          cubic/shared/cubic_base.c cubic/shared/cubic_base.h\
          cubic/shared/cubic.c cubic/shared/cubic_input.h\
          cubic/shared/cubic_output.c cubic/shared/cubic_output.h\
          cubic/cubic_is_odd_2_factored.c\
          Makefile COPYRIGHT.txt LICENSE.txt README.md

MULTICODE_SHARED = multicode/shared/multicode_base.c\
                   multicode/shared/multicode_input.c\
                   multicode/shared/multicode_output.c

CUBIC_SHARED = cubic/shared/cubic_base.c cubic/shared/cubic_input.c\
               cubic/shared/cubic_output.c

SIGNED_SHARED = signed/shared/signed_base.c signed/shared/signed_input.c\
                signed/shared/signed_output.c

all: planar conversion multi visualise embedders invariants cubic signed

planar: build/stats_pl build/count_pl build/select_pl \
	build/split_pl build/nauty_pl build/dual_pl \
	build/non_iso_pl build/subdivide_vertex build/regular_pl \
	build/random_relabel_pl build/bipartite_pl build/delete_pl\
	build/multiply_pl build/fill_face_pl build/show_pl\
	build/delete_edges_pl

conversion: build/gconv build/genreg2multicode build/freetree2multicode\
            build/multicode2signedcode build/pregraphcode2multicode

multi: build/multiread build/multi_add_edges build/multi_cyclic_connect \
       build/multi_complete_connect build/multi_path_connect \
       build/multi_combine  build/multi_remove_edges build/multi_corona \
       build/multi_filter_regular build/multi_filter_snark \
       build/multi_induced_subgraph build/multi_identify build/multi_mycielski\
       build/multi_filter_bipartite build/multi_non_iso build/multi_select\
       build/multi_complement

visualise: build/writegraph2png build/writegraph2png.jar build/writegraph2tikz

embedders: build/embed build/tutte build/circular build/all_embeddings build/hamiltonian_embed

invariants: build/multi_invariant_order build/multi_invariant_edge_connectivity \
            build/multi_invariant_girth build/multi_invariant_essential_edge_connectivity\
            build/multi_invariant_hamiltonian_cycles build/multi_invariant_hamiltonian_cycles_edge_incidence\
            build/multi_invariant_hamiltonian_cycles_universal_edges\
            build/multi_invariant_hamiltonian_cycles_uncovered_edges\
            build/multi_invariant_is_hamiltonian build/multi_invariant_chromatic_number\
            build/multi_invariant_maximum_degree build/multi_invariant_vertex_connectivity

cubic: build/cubic_is_odd_2_factored build/cubic_is_matching_in_dominating_cycle\
       build/cubic_extend_matching_to_dominating_cycle build/cubic_is_matching_in_dominating_cycle2\
       build/cubic_is_matching_and_vertices_in_dominating_cycle\
       build/cubic_are_matching_vertices_in_dominating_cycle\
       build/cubic_every_edge_in_5_cycle build/cubic_all_2_factors\
       build/cubic_K33S_M_2_factors

signed: build/signed_show build/signed_all build/signed_all_high_symmetry\
        build/signed_has_k_flow build/signed_is_6_flow_irreducible\
        build/signed_is_flow_admissable build/signed_random_equivalent\
        build/signed_is_flow_admissable_ST build/signed_select\
        build/signed_has_barbell build/signed_underlying\
        build/signed_has_balanced_hamiltonian_cycle

clean:
	rm -rf build
	rm -rf dist

build/stats_pl: planar/stats_pl.c
	mkdir -p build
	cc -o build/stats_pl -O4 planar/stats_pl.c

build/count_pl: planar/count_pl.c
	mkdir -p build
	cc -o build/count_pl -O4 planar/count_pl.c

build/select_pl: planar/select_pl.c
	mkdir -p build
	cc -o build/select_pl -O4 planar/select_pl.c

build/split_pl: planar/split_pl.c
	mkdir -p build
	cc -o build/split_pl -O4 planar/split_pl.c

build/nauty_pl: planar/nauty_pl.c planar/nauty/nauty.c planar/nauty/nautil.c planar/nauty/naugraph.c planar/nauty/schreier.c planar/nauty/naurng.c
	mkdir -p build
	cc -o build/nauty_pl -O4 planar/nauty_pl.c planar/nauty/nauty.c \
	planar/nauty/nautil.c planar/nauty/naugraph.c planar/nauty/schreier.c \
	planar/nauty/naurng.c
	
build/dual_pl: planar/dual_pl.c
	mkdir -p build
	cc -o build/dual_pl -O4 planar/dual_pl.c
	
build/regular_pl: planar/regular_pl.c
	mkdir -p build
	cc -o build/regular_pl -O4 planar/regular_pl.c

build/non_iso_pl: planar/non_iso_pl/non_iso_pl.c planar/non_iso_pl/hashfunction.c planar/non_iso_pl/splay.c
	mkdir -p build
	cc -o build/non_iso_pl -O4 planar/non_iso_pl/non_iso_pl.c
	
build/random_relabel_pl: planar/random_relabel_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/bipartite_pl: planar/bipartite_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/delete_pl: planar/delete_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/delete_edges_pl: planar/delete_edges_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multiply_pl: planar/multiply_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/fill_face_pl: planar/fill_face_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/show_pl: planar/show_pl.c
	mkdir -p build
	cc -o $@ -O4 $^
	
build/subdivide_vertex: planar/subdivide_vertex.c
	mkdir -p build
	cc -o build/subdivide_vertex -O4 planar/subdivide_vertex.c
	
build/multiread: multicode/multiread.c multicode/shared/multicode_base.c multicode/shared/multicode_input.c
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_add_edges: multicode/multi_add_edges.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_remove_edges: multicode/multi_remove_edges.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_combine: multicode/multi_combine.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_corona: multicode/multi_corona.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_complement: multicode/multi_complement.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_induced_subgraph: multicode/multi_induced_subgraph.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_identify: multicode/multi_identify.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_mycielski: multicode/multi_mycielski.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_filter_regular: multicode/multi_filter_regular.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_filter_snark: multicode/multi_filter_snark.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_filter_bipartite: multicode/multi_filter_bipartite.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/multi_select: multicode/multi_select.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_cyclic_connect: multicode/connect/multi_cyclic_connect.c \
	                    multicode/connect/connect_general.c \
	                     $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_complete_connect: multicode/connect/multi_complete_connect.c \
	                    multicode/connect/connect_general.c \
	                     $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_path_connect: multicode/connect/multi_path_connect.c \
	                    multicode/connect/connect_general.c \
	                     $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/multi_non_iso:multicode/multi_non_iso.c nauty/nautil.c nauty/nauty.c nauty/naugraph.c
	mkdir -p build
	cc -O4 -o $@ $^
	
build/writegraph2png: visualise/writegraph2png.c visualise/pngtoolkit.c
	mkdir -p build
	cc -o $@ -O4 $^ -lpng -lm
	
build/writegraph2tikz: visualise/writegraph2tikz.c
	mkdir -p build
	cc -o $@ -O4 $^

build/writegraph2png.jar: visualise/writegraph2png/**/*
	mkdir -p build
	cd visualise/writegraph2png && ant -f ant.xml

build/gconv: conversion/gconv.c
	cd conversion && make

build/genreg2multicode: conversion/genreg2multicode.c multicode/shared/multicode_base.c\
                   multicode/shared/multicode_output.c
	mkdir -p build
	cc -o $@ -O4 $^

build/freetree2multicode: conversion/freetree2multicode.c multicode/shared/multicode_base.c\
                   multicode/shared/multicode_output.c
	mkdir -p build
	cc -o $@ -O4 -DMAXVAL=20 $^

build/multicode2signedcode: conversion/multicode2signedcode.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/pregraphcode2multicode: conversion/pregraphcode2multicode.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^
	
build/embed: embedders/embed.c
	mkdir -p build
	cc -o $@ -O4 $^ -lm

build/tutte: embedders/tutte.c 
	mkdir -p build
	cc -o $@ -O4 $^ -lm

build/circular: embedders/circular.c multicode/shared/multicode_base.c multicode/shared/multicode_input.c
	mkdir -p build
	cc -o $@ -O4 $^ -lm

build/all_embeddings: embedders/all_embeddings.c $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DMAXVAL=20 $^

build/hamiltonian_embed: embedders/hamiltonian_embed.c multicode/shared/multicode_base.c multicode/shared/multicode_input.c
	mkdir -p build
	cc -o $@ -O4 $^ -lm
	
build/multi_invariant_order: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_order.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=order $^

build/multi_invariant_edge_connectivity: invariants/multi_int_invariant.c \
                             invariants/connectivity/multi_connectivity.c \
                             invariants/multi_invariant_edge_connectivity.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -g -DINVARIANT=edge_connectivity $^

build/multi_invariant_essential_edge_connectivity: invariants/multi_int_invariant.c \
                             invariants/connectivity/multi_connectivity.c \
                             invariants/multi_invariant_essential_edge_connectivity.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -g -DINVARIANT=essential_edge_connectivity $^

build/multi_invariant_girth: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_girth.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=girth $^

build/multi_invariant_hamiltonian_cycles: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_hamiltonian_cycles.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=hamiltonianCycles -DINVARIANTNAME="number of hamiltonian cycles" $^
	
build/multi_invariant_hamiltonian_cycles_edge_incidence: invariants/multi_double_invariant.c \
                             invariants/multi_invariant_hamiltonian_cycles.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=hamiltonianCyclesEdgeIncidence $^
	
build/multi_invariant_hamiltonian_cycles_universal_edges: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_hamiltonian_cycles.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=hamiltonianCyclesUniversalEdges -DINVARIANTNAME="number of edges that lie in all hamiltonian cycles" $^
	
build/multi_invariant_hamiltonian_cycles_uncovered_edges: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_hamiltonian_cycles.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=hamiltonianCyclesUncoveredEdges -DINVARIANTNAME="number of edges that do not lie in any hamiltonian cycle" $^
	
build/multi_invariant_is_hamiltonian: invariants/multi_boolean_invariant.c \
                             invariants/multi_invariant_is_hamiltonian.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=isHamiltonian -DINVARIANTNAME="hamiltonian" $^

build/multi_invariant_chromatic_number: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_chromatic_number.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -g -DINVARIANT=chromaticNumber -DINVARIANTNAME="chromatic number" $^

build/multi_invariant_maximum_degree: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_maximum_degree.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -g -DINVARIANT=maximumDegree -DINVARIANTNAME="maximum degree" $^
	
build/multi_invariant_vertex_connectivity: invariants/multi_int_invariant.c \
                             invariants/multi_invariant_vertex_connectivity.c \
                             $(MULTICODE_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DINVARIANT=vertex_connectivity -DINVARIANTNAME="vertex connectivity" $^

build/cubic_is_odd_2_factored: cubic/cubic_is_odd_2_factored.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/cubic_is_matching_in_dominating_cycle: cubic/cubic_is_matching_in_dominating_cycle.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/cubic_is_matching_in_dominating_cycle2: cubic/cubic_is_matching_in_dominating_cycle2.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/cubic_extend_matching_to_dominating_cycle: cubic/cubic_extend_matching_to_dominating_cycle.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/cubic_is_matching_and_vertices_in_dominating_cycle: cubic/cubic_is_matching_and_vertices_in_dominating_cycle.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -g -O4 $^

build/cubic_are_matching_vertices_in_dominating_cycle: cubic/cubic_are_matching_vertices_in_dominating_cycle.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -g -O4 $^

build/cubic_every_edge_in_5_cycle: cubic/cubic_every_edge_in_5_cycle.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -g -O4 $^

build/cubic_all_2_factors: cubic/cubic_all_2_factors.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -g -O4 $^

build/cubic_K33S_M_2_factors: cubic/cubic_K33S_M_2_factors.c $(CUBIC_SHARED)
	mkdir -p build
	cc -o $@ -g -O4 $^

build/signed_show: signed/signed_show.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_all: signed/signed_all.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_all_high_symmetry: signed/signed_all_high_symmetry.c $(SIGNED_SHARED)\
	                        signed/nauty/nauty.c signed/nauty/nautil.c\
	                        signed/nauty/nausparse.c signed/nauty/schreier.c\
	                        signed/nauty/naurng.c
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_has_k_flow: signed/signed_has_k_flow.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_is_6_flow_irreducible: signed/signed_is_6_flow_irreducible.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DMAXN=64 $^

build/signed_is_flow_admissable: signed/signed_is_flow_admissable.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DMAXN=63 $^

build/signed_random_equivalent: signed/signed_random_equivalent.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_is_flow_admissable_ST: signed/signed_is_flow_admissable_ST.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 -DMAXN=63 $^

build/signed_select: signed/signed_select.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_has_barbell: signed/signed_has_barbell.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_underlying: signed/signed_underlying.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

build/signed_has_balanced_hamiltonian_cycle: signed/signed_has_balanced_hamiltonian_cycle.c $(SIGNED_SHARED)
	mkdir -p build
	cc -o $@ -O4 $^

sources: dist/graphtools-sources.zip dist/graphtools-sources.tar.gz

dist/graphtools-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/graphtools-sources $(SOURCES)

dist/graphtools-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/graphtools-sources.tar.gz $(SOURCES)
