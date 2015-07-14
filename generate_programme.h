#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This data structure is used for generating a tree in memory.
// There will be at most $n$ child nodes, but we don't know $n$ at compile
// time.

typedef struct tree_node tree_node;

#define MAX_N 20

struct tree_node {
    int level;
    int value;
    tree_node * next[MAX_N]; // FIXME (make this dynamically allocated)
};

// Function prototypes:

char * binary (int n, int num_bits);
void blank_line (void);
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
void display_tree_node (tree_node * p, int n);
void destroy_tree (tree_node ** root);
void insert_tree_node (tree_node ** root, int n,
    int from_level, int from_value, int to_level, int to_value);

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

