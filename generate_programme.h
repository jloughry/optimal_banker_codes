#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This data structure is used for generating a digraph in memory.
// There will be at most $n$ child nodes, but we don't know $n$ at compile
// time.

typedef struct aluminium_Christmas_tree aluminium_Christmas_tree;

#define MAX_n 7 // larger than 7, the programme segfaults.
#define MAX_POINTERS (3 * MAX_n)

struct aluminium_Christmas_tree {
    int level;
    int value;
    int in_use;
    int visited;
    int num_children;
    int num_children_predicted;
    aluminium_Christmas_tree * next[3 * MAX_n]; // FIXME: monitor for overflow
};

/*
I call this data structure a `great big shiny aluminium Christmas tree'
(ref: A Charlie Brown Christmas, by Charles Schultz).

Once the value of $n$ is known, the shape of the data structure is known
and fixed (modulo a bit of odd/even weirdness in the last row). It can
be indexed into directly like a (slightly demented) multi-dimensional
array, but it's also isomorphic to a tree, and can be traversed like one
using a depth-first search with early disqualification. It's space
efficient, automatically sharing identical nodes (I don't think this
will cause a problem).

Example (fourth order):

level value pointer_list
0,0 -> 1,1; 1,2; 1,4; 1,8

1,1 -> 2,3; 2,5; 2,6; 2,9; 2,10; 2,12
1,2 -> 2,3; 2,5; 2,6; 2,9; 2,10; 2,12
1,4 -> 2,3; 2,5; 2,6; 2,9; 2,10; 2,12
1,8 -> 2,3; 2,5; 2,6; 2,9; 2,10; 2,12

level_0_0000 -> level_1_0001
level_0_0000 -> level_1_0010
level_0_0000 -> level_1_0100
level_0_0000 -> level_1_1000

level_1_0001 -> level_2_0011
level_1_0001 -> level_2_0101
level_1_0001 -> level_2_0110
level_1_0001 -> level_2_1001
level_1_0001 -> level_2_1010
level_1_0001 -> level_2_1100
*/

// Function prototypes:

char * binary (int n, int num_bits);
void blank_line (void);
int count_bits (char * binary_string, char bit_value);
int count_0_bits (char * binary_string);
int count_1_bits (char * binary_string);
int * generate_cardinality_sequence (int n);
void test_count_1_bits (void);
int allowable (int from_row, int from_col, int to_row, int to_col, int * cardinality, int n);
int odd (int n);
int even (int n);
void count_cardinalities (int n);
void verify_one_cardinality_sequence_data (int * index, int * sequence, int order);
void verify_all_hand_made_cardinality_sequence_data (void);
void verify_cardinality_sequence (int * sequence_data, int n);
void display_digraph_node (aluminium_Christmas_tree * p, int n);
void depth_first_search (aluminium_Christmas_tree * p, int n);

// These will only be needed until I get a proper generator written, but
// they might be useful later as test cases for the generator. They have
// index numbers built in for the paranoid validator.

int hand_generated_cardinality_sequence_data_first_order[][3] = {
    { 0, 1, 2 },
    { 0, 1, -1 },
};

int hand_generated_cardinality_sequence_data_second_order[][5] = {
    { 0, 1, 2, 3, 4 },
    { 0, 1, 2, 1, -1 },
};

int hand_generated_cardinality_sequence_data_third_order[][9] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8 },
    { 0, 1, 2, 1, 2, 1, 2, 3, -1 },
};

int hand_generated_cardinality_sequence_data_fourth_order[][17] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 3,  2,  3,  2,  3,  4,  3, -1 },
};

int hand_generated_cardinality_sequence_data_fifth_order[][33] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 1,  2,  3,  2,  3,  2,  3,  2,  3,  2,  3,
        2,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  5, -1 },
};

int hand_generated_cardinality_sequence_data_sixth_order[][65] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 1,  2,  1,  2,  3,  2,  3,  2,  3,  2,  3,
         2,  3,  2,  3,  2,  3,  2,  3,  2,  3,  2,  3,  4,  3,  4,  3,  4,
         3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  5,
         4,  5,  4,  5,  4,  5,  4,  5,  6,  5, -1 },
};

