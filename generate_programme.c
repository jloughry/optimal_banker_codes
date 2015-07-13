#include "generate_programme.h"

int main (int argc, char ** argv) {
    int n = 0;
    int row = 0;
    int col = 0;
    int * cardinality = NULL;
    tree_node * root = NULL;
    int i = 0;

    switch (argc) {
        case 2:
            n = atoi (argv[1]);
            break;
        default:
            fprintf (stderr, "Usage: %s n\n", argv[0]);
            exit (EXIT_FAILURE);
    }

    assert (n > 0);
    assert (n < MAX_N);

    verify_all_hand_made_cardinality_sequence_data ();

    cardinality = generate_cardinality_sequence (n);

    verify_cardinality_sequence (cardinality, n);

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

        if ((row % 4) == 3) {
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
        char * p = NULL;

        blank_line ();
        p = binary (col, n);
        printf ("    level_%d_%s -> level_%d_%s; ", 0, p, 1, p);
        free (p);

        for (row = 1; row < ((1 << n) - 1); row ++) {
            p = binary (col, n);
            printf ("level_%d_%s -> level_%d_%s", row, p, row + 1, p);
            free (p);

            if ( row < ((1 << n) - 2)) {
                printf (";");
            }

            // break long lines

            if ((row % 2) == 1) {
                printf ("\n    ");
            }
            else if ( row < ((1 << n) - 2)) {
                printf (" ");
            }
        }
        blank_line ();
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
            p = binary (col, n);
            printf (" -> level_%d_%s", row, p);
            free (p);

            // break long lines

            if ((col % 4) == 3) {
                printf ("\n    ");
            }
        }
    }

    blank_line ();

    // Now generate the graph of allowable transitions (but only if we have
    // good cardinality data).

    assert (NULL == root);
    root = malloc (sizeof (tree_node));
    assert (root);
    root->level = 0;
    root->value = 0;
    for (i = 0; i < MAX_N; i ++) {
        root->next[i] = NULL;
    }
    display_tree_node (root, n);

    if (cardinality[0] >= 0) {
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
                            row, binary (col, n), row + 1, binary (row_plus_one_col, n));
                    }
                }
            }
            blank_line ();
        }
    }
    else {
        fprintf (stderr,
            "We have no cardinality data; not attempting to generate the graph of allowed transitions.\n");
    }

    // End of DOT source file.

    printf ("    /* end of .dot file */\n");
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

    s = malloc (sizeof (char) * (num_bits + 1));
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
// If n > 6 then the caller is responsible for freeing the array.

int * generate_cardinality_sequence (int n) {
    int i = 0;
    int * cardinality = NULL;
    int length = 0;

    assert (n > 0);

    switch (n) {
        case 1:
            cardinality = hand_generated_cardinality_sequence_data_first_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 2:
            cardinality = hand_generated_cardinality_sequence_data_second_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 3:
            cardinality = hand_generated_cardinality_sequence_data_third_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 4:
            cardinality = hand_generated_cardinality_sequence_data_fourth_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 5:
            cardinality = hand_generated_cardinality_sequence_data_fifth_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        case 6:
            cardinality = hand_generated_cardinality_sequence_data_sixth_order[1];
            assert (-1 == cardinality[1 << n]);
            break;
        default:
            length = 1 << n;
            cardinality = malloc (sizeof (int) * length);
            assert (cardinality != NULL);
            for (i = 0; i <= length; i ++) {
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
    return !odd (n);
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

// Verify that hand-made cardinality sequence data are well-formed.

void verify_one_cardinality_sequence_data (int * index, int * sequence, int order) {
    int i = 0;
    int length = 0;

    length = 1 << order;
    for (i = 0; i < length; i ++) {
        assert (index[i] == i);
        assert (sequence[i] >= 0);
        assert (sequence[i] <= order);
    }
    assert (-1 == sequence[length]);

    return;
}

// Sanity check any cardinality sequence.

void verify_cardinality_sequence (int * sequence, int n) {
    int i = 0;
    int length = 0;

    length = 1 << n;
    switch (sequence[0]) {
        case -1:
            for (i = 1; i <= length; i ++) {
                assert (-1 == sequence[i]);
            }
            break;
        default:
            for (i = 0; i < length; i ++) {
                assert ( (sequence[i] >= 0) && (sequence[i] <= n) );
            }
            assert (-1 == sequence[length]);
            break;
    }
}

// Validate the hand-made cardinality sequence data.

void verify_all_hand_made_cardinality_sequence_data (void) {
    // The reason the name is passed in twice is because it's not allowed
    // to pass a multidimensional array directly into a function.

    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_first_order[0],
        hand_generated_cardinality_sequence_data_first_order[1], 1);
    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_second_order[0],
        hand_generated_cardinality_sequence_data_second_order[1], 2);
    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_third_order[0],
        hand_generated_cardinality_sequence_data_third_order[1], 3);
    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_fourth_order[0],
        hand_generated_cardinality_sequence_data_fourth_order[1], 4);
    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_fifth_order[0],
        hand_generated_cardinality_sequence_data_fifth_order[1], 5);
    verify_one_cardinality_sequence_data (hand_generated_cardinality_sequence_data_sixth_order[0],
        hand_generated_cardinality_sequence_data_sixth_order[1], 6);

    return;
}

// Display a single node of the tree.

void display_tree_node (tree_node * p, int n) {
    if (NULL == p) {
        fprintf (stderr, "NULL tree_node.\n");
    }
    else {
        fprintf (stderr, "tree_node %p: level %d, value %d has ", (void *) p, p->level, p->value);
        if (p->next) {
            int i = 0;

            fprintf (stderr, "children");
            for (i = 0; i < n; i++) {
                fprintf (stderr, " %p", p->next[i]);
            }
            fprintf (stderr, ".\n");
        }
        else {
            fprintf (stderr, "no children.\n");
        }
    }
    return;
}

