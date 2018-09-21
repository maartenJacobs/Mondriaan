# Mondriaan

Mondriaan is an LLVM-based compiler for the Piet programming language. It is currently in development. Please do not use in production.

## Usage

```
Usage: Mondriaan [--emit-llvm] [-S] -o output_file source_file
```

## Architecture

In a nutshell: Source file => \[ Parser ] => PietGraph => \[ Translator ] => LLVM IR => \[ LLVM ]

The Mondriaan compiler accepts a Piet source file, parses the file into a directed cyclic 
graph and translates the graph into LLVM IR. The LLVM IR is then passed to the LLVM optimiser,
and backend to generate an executable file.

## Algorithm

The Mondriaan compiler transforms the Piet source file into a directed cyclic graph:

* The vertices of the graph represent the codel blocks of the source file. The vertices 
have an integer size and a color. 1 vertex is marked the initial vertex; this is the topleft
codel block in the source file. Several vertices can be marked as terminal vertices; these
vertices have no edges coming out of them. All non-terminal edges have exactly 8 edges,
representing all possible paths from the vertex to the next.
* The edges of the graph represent the paths between codel blocks and are labeled with a 
Direction Pointer and Codel Chooser. Each edge represents 1 path between 2 vertices. A vertex 
can have a change directive, specifying a new Direction Pointer and Codel Chooser.

Using this graph, the compiler walks the graph starting with the initial graph, an open 
function (initially "main"), a Direction Pointer of Right and a Codel Chooser of Left. 
The compiler then selects the edge of the initial vertex labeled with the initial Direction 
Pointer and Codel Chooser pair. As each new vertex is visited, an LLVM basic block is 
constructed to collect the instructions represented by the path taken by the compiler.

When a terminal vertex is encountered, the current block is closed, the function is
constructed using the block, and the function is called from the currently open function.

When a "pointer" or "switch" instruction is visited, the block is put in a half-open position:
2 to 4 function calls are added, depending on the instruction, but we construct the function
from the block and call it from the currently open function. Then the half-open block becomes
the currently open block. The translation algorithm is then executed for each possible path
from the current codel block, becoming the described function calls.

### Graph construction

### Requirements

Given a graph of vertices and edges where:

* the vertices are codel blocks, and,
* the edges are pairs of DP and CC combinations, with an optional new DP and CC combination.

Each vertex has exactly 8 edges, one for each of the possible combinations of DP and CC.

One vertex is marked as the initial vertex.

### Steps

Keep a map of sequence collections to translated function names.
e.g. A-(DP left, CC right)-B-(DP left, CC left)-D-... => "sequence42"

The algorithm returns a function name.

Start from the initial vertex with DP=right and CC=left.

1. If there is no open sequence collection, start a new, empty sequence collection.
2. Travel from the selected vertex to the next vertex, following the edge labelled with the current DP and CC pair.
3. If the selected edge has a new DP and CC combination, set DP and CC to this new combination.
4. On each new vertex, determine the operation:
    1. If the vertex is a terminal vertex:
        1. Close the open sequence collection.
        2. Create a new function with the closed sequence collection.
        3. Return the new function name.
    2. If the operation is a pointer:
        1. Close the open sequence collection
        2. If the closed sequence including the pointer exists as a function:
            a. YES: translate as a function call. Return existing function name.
            b. NO: create a new function and translate the operation block as its block.
            Add 4 PHI calls (?) for each new possible DP and CC combination. Recursively execute the algorithm
            starting from the pointer vertex, with each new pair; each returned function is then called from the
            relevant PHI call (?). Return new function name.
    3. If the operation is a switch:
        1. Close the open sequence collection
        2. If the closed sequence including the pointer exists as a function:
            a. YES: translate as a function call. Return existing function name.
            b. NO: create a new function and translate the operation block as its block.
            Add 2 PHI calls (?) for each new possible DP and CC combination. Recursively execute the algorithm
            starting from the pointer vertex, with each new pair; each returned function is then called from the
            relevant PHI call (?). Return new function name.
    4. If the operation is not a pointer/switch, add the operation to the
        open sequence collection.

## Credits

Credit for the Piet programming language goes to the creator, David Morgan-Mar. You might want to consider
supporting him on [Patreon](https://www.patreon.com/dmmaus).

Credit for the samples goes to their respective creators:

- PietHello.png originated from http://www.dangermouse.net/esoteric/piet/samples.html
- PointerTest.png was created using `npietedit`
