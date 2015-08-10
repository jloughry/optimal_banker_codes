#include "generate_programme.h"

int sequence_accumulator[MAX_n];
long long good_sequences = 0;
long long predicted_number_of_candidate_sequences = 0;

// For some reason I don't understand, if the following variable is defined
// inside main(), the programme segfaults.

int children_per_node_at_level[MAX_n];

int main (int argc, char ** argv) {
    int n = 0;
    int row = 0;
    int col = 0;
    int * cardinality = NULL;
    int * testing_cardinality = NULL;
    aluminium_Christmas_tree big_dumb_array[1 << MAX_n][1 << MAX_n];
    aluminium_Christmas_tree * start = &big_dumb_array[0][0];
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
    assert (n <= MAX_n);
    assert (start);

    // Initialise the big dumb array with sentinel values.

    // Is it possible to malloc a two-dimensional array and index it?

    /*
    big_dumb_array = malloc ((1 << n) * sizeof (aluminium_Christmas_tree)
        * (1 << n) * sizeof (aluminium_Christmas_tree));
    assert (big_dumb_array);
    */

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            big_dumb_array[row][col].level = -1;
            big_dumb_array[row][col].value = -99;
            big_dumb_array[row][col].in_use = 0;
            big_dumb_array[row][col].num_children = 0;
            big_dumb_array[row][col].num_children_predicted = 0;
            big_dumb_array[row][col].next = NULL;
        }
    }

    testing_cardinality = new_cardinality_array (n);
    assert (testing_cardinality);
    fprintf (stderr, "testing_cardinality = ");
    for (i = 0; i < (1 << n) + 1; i ++) {
        fprintf (stderr, "%d ", testing_cardinality[i]);
    }
    fprintf (stderr, "\n");
    free (testing_cardinality);
    testing_cardinality = NULL;

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
            p = NULL;
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
        p = NULL;

        for (row = 1; row < ((1 << n) - 1); row ++) {
            p = binary (col, n);
            printf ("level_%d_%s -> level_%d_%s", row, p, row + 1, p);
            free (p);
            p = NULL;

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
        p = NULL;

        for (col = 1; col < (1 << n); col ++) {
            p = binary (col, n);
            printf (" -> level_%d_%s", row, p);
            free (p);
            p = NULL;

            // break long lines

            if ((col % 4) == 3) {
                printf ("\n    ");
            }
        }
    }

    blank_line ();

    // Now generate the graph of allowable transitions (but only if we have
    // good cardinality data).

    // A great big shiny aluminium Christmas tree!

    if (cardinality[0] >= 0) {
        int number_of_nodes_used = 0;
        double fill_factor_n = 0.0;
        double fill_factor_MAX_n = 0.0;

        printf ("    /* These are the allowable transitions. */\n");
        blank_line ();
        printf ("    edge [style=solid,color=black]\n");
        blank_line ();

        for (row = 0; row < ( (1 << n) - 1); row ++) {
            for (col = 0; col < (1 << n); col ++) {
                int row_plus_one_col = 0;
                int pointer_number = 0;
                int predicted_number_of_children = 0;

                for (row_plus_one_col = 0; row_plus_one_col < (1 << n); row_plus_one_col ++) {

                    if (allowable (row, col, row + 1, row_plus_one_col, cardinality, n)) {

                        assert (count_1_bits (binary (col,n)) == cardinality[row]);
                        assert (count_1_bits (binary (row_plus_one_col, n)) == cardinality[row + 1]);

                        switch (cardinality[row] - cardinality[row + 1]) {
                            case -1:
                                predicted_number_of_children = count_0_bits (binary (col, n));
                                break;
                            case 1:
                                predicted_number_of_children = count_1_bits (binary (col, n));
                                break;
                            default:
                                assert (0); // This should never happen.
                                break;
                        }

                        printf ("    level_%d_%s -> level_%d_%s [label=\"%p -> %p\"]\n",
                            row, binary (col, n), row + 1, binary (row_plus_one_col, n),
                            &big_dumb_array[row][col], &big_dumb_array[row + 1][row_plus_one_col]);

                        assert (pointer_number < predicted_number_of_children);

                        big_dumb_array[row][col].level = row;
                        big_dumb_array[row][col].value = col;
                        big_dumb_array[row][col].in_use = 1;
                        if (NULL == big_dumb_array[row][col].next) {
                            big_dumb_array[row][col].next = malloc (sizeof (aluminium_Christmas_tree *)
                                * predicted_number_of_children + 1);
                            assert (big_dumb_array[row][col].next);
                            big_dumb_array[row][col].next[0] = NULL; // NULL terminate array just in case.
                        }
                        big_dumb_array[row][col].next[pointer_number] =
                            &(big_dumb_array[row + 1][row_plus_one_col]);
                        big_dumb_array[row][col].num_children++;
                        big_dumb_array[row][col].num_children_predicted = predicted_number_of_children;

                        // Why does the following line cause a segfault in error check later?

                        children_per_node_at_level[row] = predicted_number_of_children;

                        // Make sure child nodes exist if they are named.

                        big_dumb_array[row + 1][row_plus_one_col].level = row + 1;
                        big_dumb_array[row + 1][row_plus_one_col].value = row_plus_one_col;
                        big_dumb_array[row + 1][row_plus_one_col].in_use = 1;

                        // However, we don't know anything about *their* children.

#ifdef DEBUG
                        fprintf (stderr,
                            "recorded node %p (%d, %d) -> %p (%d, %d) with %d ",
                            &big_dumb_array[row][col],
                            big_dumb_array[row][col].level,
                            big_dumb_array[row][col].value,
                            &big_dumb_array[row + 1][row_plus_one_col],
                            big_dumb_array[row + 1][row_plus_one_col].level,
                            big_dumb_array[row + 1][row_plus_one_col].value,
                            predicted_number_of_children);
                        switch (predicted_number_of_children) {
                            case 1:
                                fprintf (stderr, "child");
                                break;
                            default:
                                fprintf (stderr, "children");
                                break;
                        }
                        fprintf (stderr, " predicted.\n");
#endif

                        ++ pointer_number;
                    }
                }
            }
            blank_line ();
        }
        if (odd (n)) {
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].level = (1 << n) - 1;
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].value = (1 << n) - 1;
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].in_use = 1;

            fprintf (stderr, "node %p (%d, %d) also created with %d children.\n",
                &big_dumb_array[(1 << n) - 1][(1 << n) - 1],
                big_dumb_array[(1 << n) - 1][(1 << n) - 1].level,
                big_dumb_array[(1 << n) - 1][(1 << n) - 1].value,
                big_dumb_array[(1 << n) - 1][(1 << n) - 1].num_children);
        }

        for (row = 0; row < (1 << n); row ++) {
            for (col = 0; col < (1 << n); col ++) {
                if (big_dumb_array[row][col].in_use) {
                    ++ number_of_nodes_used;
                }
            }
        }

        predicted_number_of_candidate_sequences = 1;
        for (row = 0; row < (1 << n) - 1; row ++) {
            predicted_number_of_candidate_sequences *= children_per_node_at_level[row];
        }

        fill_factor_n = (double) number_of_nodes_used / ( pow(2.0, n) * pow (2.0, n) );
        fill_factor_MAX_n = (double) number_of_nodes_used / ((1 << MAX_n) * (1 << MAX_n));

        fprintf (stderr, "%d nodes used; fill factor = %lf based on n = %d (or %lf based on %d)\n",
            number_of_nodes_used, fill_factor_n, n, fill_factor_MAX_n, MAX_n);

        // Error check.

        for (row = 0; row < (1 << n); row ++) {
            for (col = 0; col < (1 << n); col ++) {
                if (big_dumb_array[row][col].num_children
                    != big_dumb_array[row][col].num_children_predicted) {
                    fprintf (stderr,
                        "misprediction: row %d, col %d (%p); predicted %d children, got %d children\n",
                        row, col, &big_dumb_array[row][col],
                        big_dumb_array[row][col].num_children_predicted,
                        big_dumb_array[row][col].num_children);
                }
            }
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

    fprintf (stderr, "\n");

    // Initialise the sequence accumulator (outside of the recursive fn).

    for (i = 0; i < MAX_n; i ++) {
        sequence_accumulator[i] = -1;
    }

    depth_first_search (start, cardinality, n);

    fprintf (stderr, "\n");
    printfcomma (good_sequences);
    switch (good_sequences) {
        case 1:
            fprintf (stderr, " sequence");
            break;
        default:
            fprintf (stderr, " sequences");
            break;
    }
    fprintf (stderr, " found.\n");

    // Free memory if necessary.

    if (n > 6) {
        free (cardinality);
        cardinality = NULL;
    }

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            if (big_dumb_array[row][col].next) {
                free (big_dumb_array[row][col].next);
                big_dumb_array[row][col].next = NULL;
            }
        }
    }

    return EXIT_SUCCESS;
}

// Return a string containing the binary representation of $n$ in $b$ bits.
//
// The caller is responsible for freeing the string.

char * binary (int n, int num_bits) {
    int i = 0;
    char * s = NULL;

    // Make sure the result fits in the specified number of bits.

    if (n < 0) {
        fprintf (stderr, "WARNING: in binary, n = %d\n", n);
    }

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

// Count the number of bits in a string representation of a binary number
// that are of the flavour specified.

int count_bits (char * binary_string, char bit_value) {
    char * p = NULL;
    int count = 0;

    p = binary_string;

    assert (p != NULL);

    while (*p) {
        if (bit_value == *p) {
            ++ count;
        }
        ++ p;
    }
    return count;
}

// Count the `1' bits in a string representation of a binary number.

int count_1_bits (char * binary_string) {
    return count_bits (binary_string, '1');
}

// Count the `0' bits in a string representation of a binary number.

int count_0_bits (char * binary_string) {
    return count_bits (binary_string, '0');
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
    if ( 1 == count_1_bits (binary (( from_col ^ to_col ), n))
        && (count_1_bits (binary (from_col, n)) == cardinality[from_row])
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
        p = NULL;
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

// Compute the factorial of n.

long long factorial (long n) {
    assert (n > 0);

    if (1 == n) {
        return 1;
    }
    else {
        return n * factorial (n - 1);
    }
}

// Number of subsets of $k$ elements from a set of size $n$

long long n_choose_k (int n, int k) {
    assert (n > 0);
    assert (k > 0);
    assert (k <= n);

    switch (n) {
        case 1:
            return k;
            break;
        default:
            return factorial (n) / (factorial (k) * factorial (n - k));
            break;
    }
    assert (1 == 0); // This should never happen.
}

// Return a newly allocated array of length $n+1$ elements containing the
// cardinality sequence for any $n$.
//
// The caller is responsible for freeing the returned array.

int * new_cardinality_array (int n) {
    int * a = NULL;
    int i = 0;

    a = malloc (((1 << n) + 1) * sizeof (int));
    assert (a);

    a[0] = 0;
    a[n] = -1;

    for (i = 1; i <= n_choose_k (n, 1); i += 2) {
        a[i] = 1;
    }

    return a;
}

// Display large numbers with thousands separators.
//
// Source of this neat code: http://stackoverflow.com/questions/1449805/
// how-to-format-a-number-from-1123456789-to-1-123-456-789-in-c

void printfcomma2 (long long n) {
    if (n < 1000) {
        fprintf (stderr, "%lld", n);
        return;
    }
    printfcomma2 (n / 1000);
    fprintf (stderr, ",%03lld", n % 1000);
}

void printfcomma (long long n) {
    if (n < 0) {
        fprintf (stderr, "-");
        n = -n;
    }
    printfcomma2 (n);
}

// Display an entire sequence.

void emit_sequence (int * sequence, int n) {
    int i = 0;

    fprintf (stderr, "found ");
    printfcomma (good_sequences);
    fprintf (stderr, ":");
    for (i = 0; i < (1 << n); i ++) {
        fprintf (stderr, "%2d ", sequence[i]);
    }
    fprintf (stderr, "\n");
    return;
}

// Check that a sequence satisfies the requirements.

void sanity_check_sequence (int * sequence, int * cardinality, int n) {
    int i = 0;
    int * dup_check_accumulator = NULL;

    // Preconditions:
    assert (sequence);
    assert (cardinality);
    assert (n > 0);

    // First check: that the sequence begins with zero.
    assert (sequence[0] == 0);

    // There isn't a good way to check that the sequence is the right
    // length; consider putting a terminating -1 on the end of all
    // sequences as a sentinel.

    // Second check: that the cardinality of every element is right.
    for (i = 0; i < (1 << n); i ++) {
        if (count_1_bits (binary (sequence[i], n)) != cardinality[i]) {
            int j = 0;

            fprintf (stderr, "sanity check failed on sequence ");
            for (j = 0; j < (1 << n); j ++) {
                fprintf (stderr, " %2d", sequence[j]);
            }
            fprintf (stderr, ", count_1_bits (\"%d\") != %d\n",
                count_1_bits (binary (sequence[i], n)), cardinality[i]);

            assert (0); // Fail on purpose.
        }
    }

    // Third check: that there are no duplicates in the sequence.
    dup_check_accumulator = calloc (1 << n, sizeof (int));
    assert (dup_check_accumulator);
    for (i = 0; i < (1 << n); i++) {
        ++ dup_check_accumulator[sequence[i]];
        assert (dup_check_accumulator[sequence[i]] == 1);
    }
    free (dup_check_accumulator);
    dup_check_accumulator = NULL;
}

// Display a single node of the digraph.

void display_digraph_node (aluminium_Christmas_tree * p, int n) {
    if (NULL == p) {
        fprintf (stderr, "NULL digraph node.\n");
    }
    else {
        fprintf (stderr, "digraph node %p: level %d, value %s has ",
            (void *) p, p->level, binary (p->value, n));
        if (p->num_children > 0) {
            int i = 0;

            fprintf (stderr, "%d ", p->num_children);
            switch (p->num_children) {
                case 1:
                    fprintf (stderr, "child");
                    break;
                default:
                    fprintf (stderr, "children");
                    break;
            }
            for (i = 0; i < p->num_children; i ++) {
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

// Depth-first search entire digraph.

void depth_first_search (aluminium_Christmas_tree * p, int * cardinality_sequence, int n) {
    int i = 0;
    int * early_dup_check = NULL;

    assert (p);

    sequence_accumulator[p->level] = p->value;

    // Make sure the sequence so far has not repeated.

    early_dup_check = calloc (1 << n, sizeof (int));
    assert (early_dup_check);

    for (i = 0; i < p->level; i ++) {
        ++ early_dup_check[sequence_accumulator[i]];
        if (early_dup_check[sequence_accumulator[i]] > 1) {
            free (early_dup_check);
            early_dup_check = NULL;
            return;
        }
    }
    free (early_dup_check);
    early_dup_check = NULL;

    // The following loop could be multi-threaded for up to $n$ cores.

    for (i = 0; i < p->num_children; i ++) {
        depth_first_search (p->next[i], cardinality_sequence, n);
    }

    // The following check might be redundant with early dup check; I'm not sure yet.

    // As soon as we have a full sequence, check for duplicates.

    if (p->level == ((1 << n) - 1)) {
        int * duplicate_check = NULL;
        int j = 0;

        duplicate_check = calloc (1 << n, sizeof (int));
        assert (duplicate_check);

        for (j = 0; j < (1 << n); j ++) {
            ++ duplicate_check[sequence_accumulator[j]];
            if (duplicate_check[sequence_accumulator[j]] != 1) {
                break;
            }
        }

        if (j == (1 << n)) {
            sanity_check_sequence (sequence_accumulator, cardinality_sequence, n);
            emit_sequence (sequence_accumulator, n);
            ++ good_sequences;
        }

        free (duplicate_check);
        duplicate_check = NULL;
    }
}

