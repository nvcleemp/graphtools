
SOURCES = statsPlanar.c countPlanar.c filterPlanar.c \
          Makefile COPYRIGHT.txt LICENSE.txt README.md

all: build/statsPlanar build/countPlanar build/filterPlanar

clean:
	rm -rf build
	rm -rf dist

build/statsPlanar: statsPlanar.c
	mkdir -p build
	cc -o build/statsPlanar -O4 statsPlanar.c

build/countPlanar: countPlanar.c
	mkdir -p build
	cc -o build/countPlanar -O4 countPlanar.c

build/filterPlanar: filterPlanar.c
	mkdir -p build
	cc -o build/filterPlanar -O4 filterPlanar.c

sources: dist/graphtools-sources.zip dist/graphtools-sources.tar.gz

dist/graphtools-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/graphtools-sources $(SOURCES)

dist/graphtools-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/graphtools-sources.tar.gz $(SOURCES)