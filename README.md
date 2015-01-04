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
