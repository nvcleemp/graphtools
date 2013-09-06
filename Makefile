
SOURCES = stats_pl.c count_pl.c filter_pl.c split_pl.c \
          Makefile COPYRIGHT.txt LICENSE.txt README.md

all: build/stats_pl build/count_pl build/filter_pl \
	build/split_pl build/nauty_pl

clean:
	rm -rf build
	rm -rf dist

build/stats_pl: stats_pl.c
	mkdir -p build
	cc -o build/stats_pl -O4 stats_pl.c

build/count_pl: count_pl.c
	mkdir -p build
	cc -o build/count_pl -O4 count_pl.c

build/filter_pl: filter_pl.c
	mkdir -p build
	cc -o build/filter_pl -O4 filter_pl.c

build/split_pl: split_pl.c
	mkdir -p build
	cc -o build/split_pl -O4 split_pl.c

build/nauty_pl: nauty_pl.c nauty/nauty.c nauty/nautil.c nauty/naugraph.c nauty/schreier.c nauty/naurng.c
	mkdir -p build
	cc -o build/nauty_pl -O4 nauty_pl.c nauty/nauty.c nauty/nautil.c nauty/naugraph.c nauty/schreier.c \
	nauty/naurng.c

sources: dist/graphtools-sources.zip dist/graphtools-sources.tar.gz

dist/graphtools-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/graphtools-sources $(SOURCES)

dist/graphtools-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/graphtools-sources.tar.gz $(SOURCES)