all: statsPlanar countPlanar

clean:

statsPlanar: statsPlanar.c
	cc -o statsPlanar -O4 statsPlanar.c

countPlanar: countPlanar.c
	cc -o countPlanar -O4 countPlanar.c