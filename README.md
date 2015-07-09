optimal_banker_codes
====================

A programme to do an exhaustive search (depth-first, with early
disqualification) for optimal banker sequences of order *n*.

TODO:
-----

- Need a function to count the 1s in a string representation of a binary
number.

- Need a generator for the cardinality sequence, *e.g.*,

````
0 (0)
1 (1)
2 (2)
3 (1)
4 (2)
5 (1)
6 (2)
7 (1)
8 (2)
9 (3)
A (2)
B (3)
C (2)
D (3)
E (4)
F (3)
````

- Build the tree in memory as we generate the DOT file for graphics.

- Depth-first search the tree from the root `000...0`, keeping a (sorted?)
list of all nodes seen so far, disqualifying any branch as soon as it hits
a repeated node.

- Conjecture: there exists at least one optimal banker sequence for all
*n*.

- Colour unreachable nodes grey; colour verified nodes red.

