# Mondriaan

Mondriaan is an LLVM-based compiler for the Piet programming language. It is currently in development. Please do not use in production.

## Algorithm

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
    a) If the vertex is a terminal vertex:
        1. Close the open sequence collection.
        2. Create a new function with the closed sequence collection.
        3. Return the new function name.
    b) If the operation is a pointer:
        1. Close the open sequence collection
        2. If the closed sequence including the pointer exists as a function:
            a. YES: translate as a function call. Return existing function name.
            b. NO: create a new function and translate the operation block as its block. Add 4 PHI calls (?) for each new possible DP and CC combination. Recursively execute the algorithm starting from the pointer vertex, with each new pair; each returned function is then called from the relevant PHI call (?). Return new function name.
    c) If the operation is a switch:
    d) If the operation is not a pointer/switch, add the operation to the
        open sequence collection.