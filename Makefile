all: build/statsPlanar build/countPlanar build/filterPlanar

clean:
	rm -rf build

build/statsPlanar: statsPlanar.c
	mkdir -p build
	cc -o build/statsPlanar -O4 statsPlanar.c

build/countPlanar: countPlanar.c
	mkdir -p build
	cc -o build/countPlanar -O4 countPlanar.c

build/filterPlanar: filterPlanar.c
	mkdir -p build
	cc -o build/filterPlanar -O4 filterPlanar.c