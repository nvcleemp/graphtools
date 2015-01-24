Graph Tools
===========

This repository contains a suite of utility programs to process graphs.

Compilation instructions
------------------------

Compiling the programs in this project should be straightforward. If it is not please contact me.

* Clone the project
* Download [nauty](http://cs.anu.edu.au/~bdm/nauty/) and place it in a directory called `nauty` in the root folder of the repository and in the folders `planar` and `signed`. Also copy the file `nauty.h` into the folders `conversion` and `multicode`.
* Open a terminal, change into the root of the repository and compile the programs using the command `make`.

Repository layout
-----------------

This repository is organised as follows:

* **conversion**: several programs to convert different file formats to each other.
* **cubic**: programs to work with cubic (simple) graphs
* **embedders**: programs to embed graphs
* **invariants**: programs to compute several invariants (usually for graphs in multicode format)
* **multicode**: programs to work with graphs in multicode format
* **planar**: programs to work with plane graphs in planarcode format
* **signed**: programs to work with signed graphs in signedcode format
* **visualise**: programs to construct images of graphs

Naming conventions for programs
-------------------------------

When the programs are built, all programs are put together in a single directory. There are some naming conventions to make it easier to find the correct program:

* **name starts with _multi\__**: this program operates on graphs in multicode format
* **name ends with _\_pl_**: this program operates on plane graphs in planarcode format
* **name starts with _signed\__**: this program operates on signed graphs in signedcode format
* **name starts with _cubic\__**: this program operates on cubic graphs in multicode format

File formats
------------

### The multicode format

Any filename is allowed, but the convention is to use the extension `.mc`, `.multicode` or `.code`.
The file starts with a header this header is one of `>>multi_code<<`, `>>multi_code le<<` or `>>multi_code be<<`.
The first two headers mean that little endian is used, the third that big endian is being used (see later).
If the graph contains less than 255 vertices, then the first entry is the order of the graph.
Then to each vertex *x* there is a list of neighbours of *x* with higher numbers than *x*.
The vertices are numbered from 1 to *n* (where *n* is the order of the graph).
Each list is closed by a zero.
It is possible that some lists are empty.
The last list is always empty (no neighbours of n with a higher number than *n*),
so the last "list" is not followed by a zero.

If the graph contains 255 or more vertices, then the first entry is a zero.
After that the same format as with the smaller graphs is used,
but now each entry consists of two bytes instead of one byte.
These two-byte-numbers are in little endian or big endian depending on the header of the file.

In both cases, after the last entry the following graph follows immediately.

### The planarcode format

Any filename is allowed, but the convention is to use the extension `.pc`, `.plc`, or `.planarcode`.
The file starts with a header this header is one of `>>planar_code<<`, `>>planar_code le<<` or `>>planar_code be<<`.
The first two headers mean that little endian is used, the third that big endian is being used (see later).
If the graph contains less than 255 vertices, then the first entry is the order of the graph.
Then to each vertex *x* there is a list of neighbours of *x* in their cyclic order around *x*.
There is no convention about clockwise or counterclockwise, but of course it should be the same for any vertex.
The vertices are numbered from 1 to *n* (where *n* is the order of the graph).
Each list is closed by a zero.

If the graph contains 255 or more vertices, then the first entry is a zero.
After that the same format as with the smaller graphs is used,
but now each entry consists of two bytes instead of one byte.
These two-byte-numbers are in little endian or big endian depending on the header of the file.

In both cases, after the last entry the following graph follows immediately.

### Old-style files

For each of the previous file formats there also exists an old-style format. These old-style formats are exactly the same except that they do not include the header. Note that without the header it becomes very difficult to know which code a file contains if the file does not have a 'well-chosen' name. At the moment most programs in this repository do not support the old-style formats.
