#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>    // must be included after stdlib.h

#define USAGE   "Usage: %s [-1g] n"

#define CHECKPOINT_FILE "checkpoint.xml"

#define FALSE 0
#define TRUE !FALSE

typedef int boolean;

// This data structure is used for generating a digraph in memory.
// There will be at most $n$ child nodes, but we don't know $n$ at compile
// time.

typedef struct aluminium_Christmas_tree aluminium_Christmas_tree;

#define MAX_n 8 // larger than 8, the programme segfaults.

struct aluminium_Christmas_tree {
    int level;
    int value;
    int in_use;
    int num_children;
    int num_children_predicted;
    boolean visited;
    aluminium_Christmas_tree ** next;
};

struct list_node {
    int num;
    struct list_node * next;
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
void test_count_1_bits (void);
boolean allowable (int from_row, int from_col, int to_row, int to_col, int * cardinality, int n);
boolean odd (int n);
boolean even (int n);
void count_cardinalities (int n);
void verify_one_cardinality_sequence_data (int * index, int * sequence, int order);
void verify_all_hand_made_cardinality_sequence_data (void);
void acid_test_for_cardinality_sequence (int * sequence_data, int n);
void display_digraph_node (aluminium_Christmas_tree * p, int n);
void depth_first_search (aluminium_Christmas_tree * p,
    int * cardinality_sequence, int n, boolean first_solution_only);
void breadth_first_search (aluminium_Christmas_tree * p, int n);
void reset_visited_flags (aluminium_Christmas_tree * p);
void gmp_printfcomma2 (mpz_t n);
void gmp_printfcomma (mpz_t n);
void sanity_check_sequence (int * sequence, int * cardinality, int n);
void emit_sequence (int * sequence, int n);
void display_sequence_helper (int * sequence, int n);
int * generate_cardinality_sequence (int n);
int first_empty_slot (int * a, int length);
void test_generate_cardinality_sequence_function (void);
void test_generate_cardinality_sequence_function_helper (int order);
void checkpoint (int n);
void emit_tabs (FILE * fp, int how_deep);
void open_XML_tag (FILE * fp, char * tag, int nesting);
void close_XML_tag (FILE * fp, char * tag, int nesting);
void write_XML_string_value (FILE * fp, char * tag, char * value, int nesting);
void write_XML_integer_value (FILE * fp, char * tag, int value, int nesting);
void write_XML_long_long_value (FILE * fp, char * tag, long long value, int nesting);
void write_XML_mpz_integer_value (FILE * fp, char * tag, mpz_t value, int nesting);
void usage (char * programme_name);
void write_dot_file (aluminium_Christmas_tree * root, int * cardinality, int n);
void process_command_line_options (int argc, char ** argv,
    boolean * option_1, boolean * option_g, int * n);

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

// The following is used in printf statements when writing the .dot file.

#define TAB "    "

