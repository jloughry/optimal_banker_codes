#include "generate_programme.h"

int sequence_accumulator[MAX_n];

static mpz_t good_sequences; // Using the GNU multiple precision library.
static mpz_t predicted_number_of_candidate_sequences;
static mpz_t predicted_number_of_candidate_sequences_at_row_n;

// For some reason I don't understand, if the following variable is defined
// inside main(), the programme segfaults.

int children_per_node_at_level[MAX_n];

int main (int argc, char ** argv) {
    int n = 0;
    int row = 0;
    int col = 0;
    int * cardinality = NULL;
    aluminium_Christmas_tree big_dumb_array[1 << MAX_n][1 << MAX_n];
    aluminium_Christmas_tree * start = &big_dumb_array[0][0];
    int i = 0;

    mpz_init (good_sequences);
    mpz_init (predicted_number_of_candidate_sequences);
    mpz_init (predicted_number_of_candidate_sequences_at_row_n);

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

    test_count_1_bits ();

    // Initialise the big dumb array with sentinel values. Is it possible
    // directly to malloc a two-dimensional array and still index it?

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

    verify_all_hand_made_cardinality_sequence_data ();
    test_generate_cardinality_sequence_function ();

    cardinality = generate_cardinality_sequence (n);
    acid_test_for_cardinality_sequence (cardinality, n);

    fprintf (stderr, "Version %d\n", VERSION);

    // Write the header of the DOT source file to stdout.

    printf ("/*\n");
    printf (TAB "dot -T pdf order-%d_graph_generated.dot -o order-%d_graph_generated.pdf\n",
        n, n);
    blank_line ();
    printf (TAB "This was made by Version %d of the generator.\n", VERSION);
    printf ("*/\n");
    blank_line ();
    printf ("digraph order%d {\n", n);
    blank_line ();
    printf (TAB "node [shape=plaintext]\n");
    blank_line ();

    // Draw row markers down the left side of the graph.

    for (row = 0; row < (1 << n); row ++) {
        printf (TAB "level_%d [label=\"%d (%d)\"]\n",
            row, row, cardinality[row]);
    }

    blank_line ();
    printf (TAB "/* Connect the left side row markers invisibly so they stay lined up. */\n");

    blank_line ();
    printf (TAB "edge [style=invis]\n");
    blank_line ();

    printf (TAB "level_0");
    for (row = 0; row < (1 << n); row ++) {
        printf (" -> level_%d", row);

        // break long lines

        if ((row % 4) == 3) {
            blank_line ();
            printf (TAB TAB);
        }
    }

    // The purpose of `ordering=out' is to keep DOT from re-ordering nodes
    // on the page.

    blank_line ();
    blank_line ();
    printf (TAB "graph [ordering=out]\n");
    printf (TAB "node [shape=rect]\n");
    blank_line ();

    printf (TAB "/* set of all possible states */\n");
    blank_line ();

    for (row = 0; row < (1 << n); row ++) {
        printf (TAB "{\n");
        printf (TAB TAB "rank=same; level_%d\n", row);
        blank_line ();

        for (col = 0; col < (1 << n); col ++) {
            char * p = NULL;

            p = binary (col, n);

            printf (TAB TAB "level_%d_%s [label=\"%s\"", row, p, p);

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
        printf (TAB "}\n");
        blank_line ();
    }

    printf (TAB "edge [style=invis]\n");

    blank_line ();
    printf (TAB "/* Connect the states invisibly so they stay lined up vertically. */\n");

    for (col = 0; col < (1 << n); col ++) {
        char * p = NULL;

        blank_line ();
        p = binary (col, n);
        printf (TAB "level_%d_%s -> level_%d_%s; ", 0, p, 1, p);
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
                printf ("\n" TAB);
            }
            else if ( row < ((1 << n) - 2)) {
                printf (" ");
            }
        }
        blank_line ();
    }

    blank_line ();
    printf (TAB "/* Connect the states invisibly so they stay lined up horizontally. */\n");

    for (row = 0; row < (1 << n); row ++) {
        char * p = NULL;

        blank_line ();
        p = binary (0, n);
        printf (TAB "level_%d_%s", row, p);
        free (p);
        p = NULL;

        for (col = 1; col < (1 << n); col ++) {
            p = binary (col, n);
            printf (" -> level_%d_%s", row, p);
            free (p);
            p = NULL;

            // break long lines

            if ((col % 4) == 3) {
                printf ("\n" TAB);
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

        printf (TAB "/* These are the allowable transitions. */\n");
        blank_line ();
        printf (TAB "edge [style=solid,color=black]\n");
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

                        printf (TAB "level_%d_%s -> level_%d_%s [label=\"%p to %p\"]\n",
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
                        big_dumb_array[row][col].num_children ++;
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

        mpz_set_ui (predicted_number_of_candidate_sequences, 1);

        for (row = 0; row < (1 << n) - 1; row ++) {
            mpz_mul_ui (predicted_number_of_candidate_sequences_at_row_n,
                predicted_number_of_candidate_sequences,
                children_per_node_at_level[row]);
            mpz_set (predicted_number_of_candidate_sequences,
                predicted_number_of_candidate_sequences_at_row_n);
        }

        fill_factor_n = (double) number_of_nodes_used / ( pow(2.0, n) * pow (2.0, n) );
        fill_factor_MAX_n = (double) number_of_nodes_used / ((1 << MAX_n) * (1 << MAX_n));

        fprintf (stderr, "%d nodes used; fill factor = %lf based on n = %d",
            number_of_nodes_used, fill_factor_n, n);

        if (n < MAX_n) {
            fprintf (stderr, " (or %lf based on %d)", fill_factor_MAX_n, MAX_n);
        }
        fprintf (stderr, "\n");

        fprintf (stderr, "The predicted number of candidate sequences is ");
        gmp_printfcomma (predicted_number_of_candidate_sequences);
        fprintf (stderr, ".\n");

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

    printf (TAB "/* end of .dot file */\n");
    printf ("}\n");
    blank_line ();

    fprintf (stderr, "\n");

    // Initialise the sequence accumulator (outside of the recursive fn).

    for (i = 0; i < MAX_n; i ++) {
        sequence_accumulator[i] = -1;
    }

    depth_first_search (start, cardinality, n);

    fprintf (stderr, "\n");
    gmp_printfcomma (good_sequences);
    if (mpz_cmp_ui (good_sequences, 1) == 0) {
        fprintf (stderr, " sequence was");
    }
    else {
        fprintf (stderr, " sequences were");
    }
    fprintf (stderr, " found in all.\n");

    fprintf (stderr, "The predicted number of candidate sequences was ");
    gmp_printfcomma (predicted_number_of_candidate_sequences);
    fprintf (stderr, ".\n");

    free (cardinality);
    cardinality = NULL;

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            if (big_dumb_array[row][col].next) {
                free (big_dumb_array[row][col].next);
                big_dumb_array[row][col].next = NULL;
            }
        }
    }

    mpz_clear (good_sequences);
    mpz_clear (predicted_number_of_candidate_sequences);
    mpz_clear (predicted_number_of_candidate_sequences_at_row_n);

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

    return;
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

    return;
}

// Test the function that generates an arbitrary cardinality sequence.

void test_generate_cardinality_sequence_function (void) {
    int i = 0;

    for (i = 1; i <= 6; i ++) {
        test_generate_cardinality_sequence_function_helper (i);
    }
    return;
}

void test_generate_cardinality_sequence_function_helper (int order) {
    int * new_cardinality_sequence = NULL;
    int * known_good_sequence = NULL;
    int i = 0;
    int length_of_sequence = 0;

    assert (order > 0);
    assert (order <= 6);

    switch (order) {
        case 1:
            known_good_sequence = hand_generated_cardinality_sequence_data_first_order[1];
            break;
        case 2:
            known_good_sequence = hand_generated_cardinality_sequence_data_second_order[1];
            break;
        case 3:
            known_good_sequence = hand_generated_cardinality_sequence_data_third_order[1];
            break;
        case 4:
            known_good_sequence = hand_generated_cardinality_sequence_data_fourth_order[1];
            break;
        case 5:
            known_good_sequence = hand_generated_cardinality_sequence_data_fifth_order[1];
            break;
        case 6:
            known_good_sequence = hand_generated_cardinality_sequence_data_sixth_order[1];
            break;
        default:
            assert (1 == 0); // This should never happen.
            break;
    }

    length_of_sequence = 1 << order;
    new_cardinality_sequence = generate_cardinality_sequence (order);
    assert (-1 == new_cardinality_sequence[length_of_sequence]);
    assert (-1 == known_good_sequence[length_of_sequence]);
    for (i = 0; i < length_of_sequence; i ++) {
        assert (new_cardinality_sequence[i] == known_good_sequence[i]);
    }
    acid_test_for_cardinality_sequence (new_cardinality_sequence, order);
    free (new_cardinality_sequence);
    new_cardinality_sequence = NULL;

    return;
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
    return;
}

// Verify that hand-made cardinality sequence data are well-formed.

void verify_one_cardinality_sequence_data (int * index, int * sequence, int order) {
    int i = 0;
    int length = 0;

    length = 1 << order;

    // Verify that the index is well-formed.
    for (i = 0; i < length; i ++) {
        assert (index[i] == i);
    }

    // Verify the sequence itself.
    acid_test_for_cardinality_sequence (sequence, order);

    return;
}

// Sanity check any cardinality sequence.

// Since 1,2,1,2,... is a valid subsequence, we can't really test for
// quasi-monotonicity. However, by verifying that the sequence starts 
// at 0 and ends with $n$, and that adjacent values always differ by
// exactly 1, it can be proved that the sequence must ratchet upward.

void acid_test_for_cardinality_sequence (int * sequence, int n) {
    int i = 0;
    int length = 0;

    length = 1 << n;

    assert (0 == sequence[0]);
    assert (-1 == sequence[length]);

    if (odd (n)) {
        assert (n == sequence[length - 1]);
    }
    else {
        assert (n == sequence[length - 2]);
    }

    for (i = 0; i < length; i ++) {
        // Verify all values are within the allowed range.
        assert ( (sequence[i] >= 0) && (sequence[i] <= n) );

        // Verify all adjacent values differ by exactly 1.
        assert (1 == (sequence[1] - sequence[0]));
        if (i > 0) {
            assert (1 == abs (sequence[i] - sequence[i - 1]));
        }
        if (i < length - 1) {
            assert (1 == abs (sequence[i] - sequence[i + 1]));
        }
    }
    return;
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

// Return the index of the first -1 value in the array.

int first_empty_slot (int * a, int length) {
    int i = 0;

    while (i < length) {
        if (-1 == a[i]) {
            break;
        }
        ++ i;
    }
    return i;
}

// Return a newly allocated array of length $n+1$ elements containing the
// cardinality sequence for any $n$.
//
// The caller is responsible for freeing the returned array.

int * generate_cardinality_sequence (int n) {
    int * cardinality_sequence = NULL;
    int length = 0;
    int i = 0;
    int k = 0;

    length = (1 << n) + 1;

    cardinality_sequence = malloc (length * sizeof (int));
    assert (cardinality_sequence);

    // Initialise the cardinality array to `empty'.

    for (i = 0; i < length; i ++) {
        cardinality_sequence[i] = -1;
    }

    for (k = 0; k <= n; k ++) {
        int starting_position = 0;
        int how_many = 0;
        mpz_t binomial_coefficient;

        mpz_init (binomial_coefficient);
        mpz_bin_uiui (binomial_coefficient, n, k); // same as "n choose k"
        how_many = mpz_get_ui (binomial_coefficient);
        mpz_clear (binomial_coefficient);

        starting_position = first_empty_slot (cardinality_sequence, length);

        for (i = 0; i < how_many; i ++) {
            cardinality_sequence[starting_position + (2 * i)] = k;
        }
    }

    return cardinality_sequence;
}

// Display large numbers with thousands separators.
//
// Modified from: http://stackoverflow.com/questions/1449805/
// how-to-format-a-number-from-1123456789-to-1-123-456-789-in-c

void gmp_printfcomma2 (mpz_t n) {
    mpz_t n_div_1000;
    mpz_t n_mod_1000;

    mpz_init (n_div_1000);
    mpz_init (n_mod_1000);

    if (mpz_cmp_ui (n, 1000) < 0) {
        gmp_fprintf (stderr, "%Zd", n);
        return;
    }

    mpz_tdiv_q_ui (n_div_1000, n, 1000);
    mpz_mod_ui (n_mod_1000, n, 1000);

    gmp_printfcomma2 (n_div_1000);
    gmp_fprintf (stderr, ",%03Zd", n_mod_1000);

    mpz_clear (n_div_1000);
    mpz_clear (n_mod_1000);

    return;
}

void gmp_printfcomma (mpz_t n) {
    if (mpz_cmp_ui (n, 0) < 0) {
        gmp_fprintf (stderr, "-");
        mpz_neg (n, n);
    }
    gmp_printfcomma2 (n);

    return;
}

// Display an entire sequence.

void emit_sequence (int * sequence, int n) {
    gmp_printfcomma (good_sequences);
    fprintf (stderr, " sequence");
    if (mpz_cmp_ui (good_sequences, 1) != 0) {
        fprintf (stderr, "s");
    }
    fprintf (stderr, " found: ");
    display_sequence_helper (sequence, n);
    fprintf (stderr, "\n");

    return;
}

void display_sequence_helper (int * sequence, int n) {
    int two_to_the_n = 0;
    int field_width = 0;
    int i = 0;

    two_to_the_n = 1 << n;
    field_width = log10 (two_to_the_n) + 1;
    for (i = 0; i < two_to_the_n; i ++) {
        switch (field_width) {
            case 1:
            default:
                fprintf (stderr, "%d ", sequence[i]);
                break;
            case 2:
                fprintf (stderr, "%2d ", sequence[i]);
                break;
            case 3:
                fprintf (stderr, "%3d ", sequence[i]);
                break;
            case 4:
                fprintf (stderr, "%4d ", sequence[i]);
                break;
            case 5:
                fprintf (stderr, "%4d ", sequence[i]);
                break;
        }
    }
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
    // length; TODO: consider putting a terminating -1 on the end of all
    // sequences as a sentinel.

    // Second check: that the cardinality of every element is right.
    for (i = 0; i < (1 << n); i ++) {
        if (count_1_bits (binary (sequence[i], n)) != cardinality[i]) {
            fprintf (stderr, "sanity check failed on sequence ");
            display_sequence_helper (sequence, n);
            fprintf (stderr, ", count_1_bits (\"%d\") != %d\n",
                count_1_bits (binary (sequence[i], n)), cardinality[i]);

            assert (0); // Fail on purpose.
        }
    }

    // Third check: that there are no duplicates in the sequence.
    dup_check_accumulator = calloc (1 << n, sizeof (int));
    assert (dup_check_accumulator);
    for (i = 0; i < (1 << n); i ++) {
        ++ dup_check_accumulator[sequence[i]];
        assert (dup_check_accumulator[sequence[i]] == 1);
    }
    free (dup_check_accumulator);
    dup_check_accumulator = NULL;

    return;
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

    // Make sure the sequence so far has not repeated. This is the key to
    // early disqualification or early termination of the search.

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

    // As soon as we have a full sequence, check for duplicates. NOTE:
    // waiting until we have a full sequence to check if it contains any
    // duplicates is far too slow to be useful.

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
            mpz_add_ui (good_sequences, good_sequences, 1);
            emit_sequence (sequence_accumulator, n);
        }

        free (duplicate_check);
        duplicate_check = NULL;
    }
    return;
}

