all: statsPlanar

clean:

statsPlanar: statsPlanar.c
	cc -o statsPlanar -O4 statsPlanar.c
