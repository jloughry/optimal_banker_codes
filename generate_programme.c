#include "generate_programme.h"

char * binary (int n, int num_bits);
void blank_line (void);
int count_1_bits (char * binary_string);
int * generate_cardinality_sequence (int n);
void test_count_1_bits (void);
int allowable (int from_row, int from_col, int to_row, int to_col, int * cardinality, int n);
int odd (int n);
int even (int n);
void count_cardinalities (int n);
void verify_hand_generated_cardinality_sequences (void);

typedef struct node node;
struct node {
    int n;
    node ** nodes;
};

// These will only be needed until I get a proper generator written, but
// they might be useful later as test cases for the generator. They have
// index numbers built in for the paranoid validator.

int hand_generated_cardinality_sequence_order_1[][3] = {
    { 0, 1, 2 },
    { 0, 1, -1 },
};

int hand_generated_cardinality_sequence_order_2[][5] = {
    { 0, 1, 2, 3, 4 },
    { 0, 1, 2, 1, -1 },
};

int hand_generated_cardinality_sequence_order_3[][9] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8 },
    { 0, 1, 2, 1, 2, 1, 2, 3, -1 },
};

int hand_generated_cardinality_sequence_order_4[][17] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 3,  2,  3,  2,  3,  4,  3, -1 },
};

int hand_generated_cardinality_sequence_order_5[][33] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 1,  2,  3,  2,  3,  2,  3,  2,  3,  2,  3,
        2,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  5, -1 },
};

int hand_generated_cardinality_sequence_order_6[][65] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64 },
    { 0, 1, 2, 1, 2, 1, 2, 1, 2, 1,  2,  1,  2,  3,  2,  3,  2,  3,  2,  3,
         2,  3,  2,  3,  2,  3,  2,  3,  2,  3,  2,  3,  4,  3,  4,  3,  4,
         3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  3,  4,  5,
         4,  5,  4,  5,  4,  5,  4,  5,  6,  5, -1 },
};

int main (int argc, char ** argv) {
    int n = 0;
    int row = 0;
    int col = 0;
    int * cardinality = NULL;
    node * root = NULL;

    switch (argc) {
        case 1:
            n = 6;  // for gdb
            break;
        case 2:
            n = atoi (argv[1]);
            break;
        default:
            fprintf (stderr, "Usage: %s n\n", argv[0]);
            exit (EXIT_FAILURE);
    }

    verify_hand_generated_cardinality_sequences ();

    assert (n > 0);

    cardinality = generate_cardinality_sequence (n);

    root = malloc (sizeof (struct node));
    assert (root);
    root->n = 0;
    root->nodes = NULL;

    // Write the header of the DOT source file to stdout.

    printf ("/*\n");
    printf ("    dot -T pdf order-%d_graph_generated.dot -o order-%d_graph_generated.pdf\n",
        n, n);
    printf ("*/\n");
    blank_line ();
    printf ("digraph order%d {\n", n);
    blank_line ();
    printf ("    node [shape=plaintext]\n");
    blank_line ();

    // Draw row markers down the left side of the graph.

    for (row = 0; row < (1 << n); row ++) {
        printf ("    level_%d [label=\"%d (%d)\"]\n",
            row, row, cardinality[row]);
    }

    blank_line ();
    printf ("    /* Connect the left side row markers invisibly so they stay lined up. */\n");

    blank_line ();
    printf ("    edge [style=invis]\n");
    blank_line ();

    printf ("    level_0");
    for (row = 0; row < (1 << n); row ++) {
        printf (" -> level_%d", row);

        // break long lines

        if ((row % 5) == 4) {
            blank_line ();
            printf ("        ");
        }
    }

    // The purpose of `ordering=out' is to keep DOT from re-ordering nodes
    // on the page.

    blank_line ();
    blank_line ();
    printf ("    graph [ordering=out]\n");
    printf ("    node [shape=rect]\n");
    blank_line ();

    printf ("    /* set of all possible states */\n");
    blank_line ();

    for (row = 0; row < (1 << n); row ++) {
        printf ("    {\n");
        printf ("        rank=same; level_%d\n", row);
        blank_line ();

        for (col = 0; col < (1 << n); col ++) {
            char * p = NULL;

            p = binary (col, n);

            printf ("        level_%d_%s [label=\"%s\"", row, p, p);

            // The all-zeroes state is always gone through; colour it red.

            if ( (row == 0) && (count_1_bits (p) == 0) ) {
                printf (",color=red,fontcolor=red");
            }

            // The all-ones state is always gone through; colour it red.

            if ( odd (n) && (row == (1 << n) - 1) && (count_1_bits (p) == n) ) {
                printf (",color=red,fontcolor=red");
            }
            else if ( even (n) && (row == (1 << n) - 2) && (count_1_bits (p) == n) ) {
                printf (",color=red,fontcolor=red");
            }

            // Grey out unreachable states that have the wrong cardinality.

            if (count_1_bits (p) != cardinality[row]) {
                printf (",color=grey,fontcolor=grey");
            }

            printf ("]\n");

            free (p);
        }
        printf ("    }\n");
        blank_line ();
    }

    printf ("    edge [style=invis]\n");

    blank_line ();
    printf ("    /* Connect the states invisibly so they stay lined up vertically. */\n");

    for (col = 0; col < (1 << n); col ++) {
        blank_line ();

        for (row = 0; row < ((1 << n) - 1); row ++) {
            char * p = NULL;

            p = binary (col, n);
            printf ("    level_%d_%s -> level_%d_%s\n", row, p, row + 1, p);
            free (p);
        }
    }

    blank_line ();
    printf ("    /* Connect the states invisibly so they stay lined up horizontally. */\n");

    for (row = 0; row < (1 << n); row ++) {
        char * p = NULL;

        blank_line ();
        p = binary (0, n);
        printf ("    level_%d_%s", row, p);
        free (p);

        for (col = 1; col < (1 << n); col ++) {
            char * p = NULL;

            p = binary (col, n);
            printf (" -> level_%d_%s", row, p);
            free (p);

            // break long lines

            if ((col % 5) == 4) {
                printf ("\n    ");
            }
        }
        blank_line ();
    }

    // Now generate the graph of allowable transitions.

    blank_line ();
    printf ("    /* These are the allowable transitions. */\n");
    blank_line ();
    printf ("    edge [style=solid,color=black]\n");
    blank_line ();

    for (row = 0; row < ( (1 << n) - 1); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            int row_plus_one_col = 0;

            for (row_plus_one_col = 0; row_plus_one_col < (1 << n); row_plus_one_col ++) {
                if (allowable (row, col, row + 1, row_plus_one_col, cardinality, n)) {

                    assert (count_1_bits (binary (col,n)) == cardinality[row]);
                    assert (count_1_bits (binary (row_plus_one_col, n)) == cardinality[row + 1]);

                    printf ("    level_%d_%s -> level_%d_%s\n",
                        row, binary(col, n), row + 1, binary (row_plus_one_col, n));
                }
            }
        }
    }

    // End of DOT source file.

    printf ("}\n");
    blank_line ();

    // Free memory if necessary.

    if (n > 6) {
        free (cardinality);
    }

    free (root);

    return EXIT_SUCCESS;
}

// Return a string containing the binary representation of $n$ in $b$ bits.
//
// The caller is responsible for freeing the string.

char * binary (int n, int num_bits) {
    int i = 0;
    char * s = NULL;

    // Make sure the result fits in the specified number of bits.

    assert ( log (n) <= (double) num_bits );

    s = malloc (num_bits + 1);
    if (!s) {
        fprintf (stderr, "malloc() failed\n");
        exit (EXIT_FAILURE);
    }
    s[0] = '\0';

    for (i = (1 << (num_bits - 1)); i > 0; i >>= 1) {
        strcat (s, ((n & i) == i) ? "1" : "0");
    }

    return s;
}

// Put a blank line in the output.

void blank_line (void) {
    printf ("\n");
}

// Count the `1' bits in a string representation of a binary number.

int count_1_bits (char * binary_string) {
    char * p = NULL;
    int count = 0;

    p = binary_string;

    assert (p != NULL);

    while (*p) {
        if ('1' == *p) {
            ++ count;
        }
        ++ p;
    }
    return count;
}

// Test cases for the count_1_bits() function.

void test_count_1_bits (void) {
    assert (count_1_bits ("0") == 0);
    assert (count_1_bits ("1") == 1);
    assert (count_1_bits ("10101") == 3);
    assert (count_1_bits ("abc") == 0);
    assert (count_1_bits ("") == 0);
}

// Generate the list of cardinalities we need.
//
// If n > 5 then the caller is responsible for freeing the array.

int * generate_cardinality_sequence (int n) {
    int i = 0;
    int * cardinality = NULL;

    assert (n > 0);

    switch (n) {
        case 1:
            cardinality = hand_generated_cardinality_sequence_order_1[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 2:
            cardinality = hand_generated_cardinality_sequence_order_2[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 3:
            cardinality = hand_generated_cardinality_sequence_order_3[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 4:
            cardinality = hand_generated_cardinality_sequence_order_4[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 5:
            cardinality = hand_generated_cardinality_sequence_order_5[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 6:
            cardinality = hand_generated_cardinality_sequence_order_6[1];
            assert (-1 == cardinality[1 << n]);
            break;
        default:
            cardinality = (int *) malloc (n);
            assert (cardinality != NULL);
            for (i = 0; i < n; i ++) {
                cardinality[i] = -1;
            }
            break;
    }
    return cardinality;
}

// Is the indicated transition an allowable transition?

int allowable (int from_row, int from_col, int to_row, int to_col, int * cardinality, int n) {
    if ( (count_1_bits (binary (from_col, n)) == cardinality[from_row])
        && (count_1_bits (binary (to_col, n)) == cardinality[to_row]) ) {
            return 1;
    }
    return 0;
}

// Is $n$ odd?

int odd (int n) {
    return (n % 2);
}

// Is $n$ even?

int even (int n) {
    return !odd(n);
}

// This is a scaffolding function to count cardinalities.

void count_cardinalities (int n) {
    int i = 0;
    char * p = NULL;

    assert (n > 0);

    for (i = 0; i < (1 << n); i ++) {
        p = binary (i, n);
        fprintf (stderr, "%d\n", count_1_bits (p));
        free (p);
    }
}

// Verify that each hand-made cardinality sequence is the right length.

void verify_hand_generated_cardinality_sequences (void) {
    int i = 0;

    for (i = 0; i < (1 << 1); i ++) {
        assert (hand_generated_cardinality_sequence_order_1[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_1[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_1[1][i] <= 1);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_1[1][i]);

    for (i = 0; i < (1 << 2); i ++) {
        assert (hand_generated_cardinality_sequence_order_2[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_2[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_2[1][i] <= 2);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_2[1][i]);

    for (i = 0; i < (1 << 3); i ++) {
        assert (hand_generated_cardinality_sequence_order_3[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_3[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_3[1][i] <= 3);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_3[1][i]);

    for (i = 0; i < (1 << 4); i ++) {
        assert (hand_generated_cardinality_sequence_order_4[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_4[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_4[1][i] <= 4);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_4[1][i]);

    for (i = 0; i < (1 << 5); i ++) {
        assert (hand_generated_cardinality_sequence_order_5[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_5[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_5[1][i] <= 5);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_5[1][i]);

    for (i = 0; i < (1 << 6); i ++) {
        assert (hand_generated_cardinality_sequence_order_6[0][i] == i);
        assert (hand_generated_cardinality_sequence_order_6[1][i] >= 0);
        assert (hand_generated_cardinality_sequence_order_6[1][i] <= 6);
    }
    assert (-1 == hand_generated_cardinality_sequence_order_6[1][i]);

    return;
}

