#include "generate_programme.h"

int sequence_accumulator[MAX_n];
int sequence_is_valid = false;
time_t starting_time = 0;
boolean first_time_through_recursion = true;

static mpz_t good_sequences; // Using the GNU multiple precision library.
static mpz_t predicted_number_of_candidate_sequences;
static mpz_t predicted_number_of_candidate_sequences_at_row_n;

boolean found_first_solution = false;

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
    aluminium_Christmas_tree * dag = NULL;
    int i = 0;
    boolean option_run_once = false;
    boolean option_generate_graph = false;
    int option_restart_level = 0;
    int row_plus_one_col = 0;
    int child_number = 0;
    aluminium_Christmas_tree ** addressable_row_array = NULL;
    char binary_buffer[MAX_n + 1];

    starting_time = time (NULL);

    mpz_init (good_sequences);
    mpz_init (predicted_number_of_candidate_sequences);
    mpz_init (predicted_number_of_candidate_sequences_at_row_n);

    process_command_line_options (argc, argv, &option_run_once,
        &option_generate_graph, &option_restart_level, &n);

    assert (n > 0);
    assert (n <= MAX_n);
    assert (start);

    checkpoint (n);

    test_count_1_bits ();

    // Initialise the big dumb array with sentinel values. Is it possible
    // directly to malloc a two-dimensional array and still index it?

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            big_dumb_array[row][col].level = -1;
            big_dumb_array[row][col].value = -99;
            big_dumb_array[row][col].in_use = false;
            big_dumb_array[row][col].num_children = 0;
            big_dumb_array[row][col].num_children_predicted = 0;
            big_dumb_array[row][col].visited = false;
            big_dumb_array[row][col].child = NULL;
        }
    }

    verify_all_hand_made_cardinality_sequence_data ();
    test_generate_cardinality_sequence_function ();

    cardinality = generate_cardinality_sequence (n);
    acid_test_for_cardinality_sequence (cardinality, n);

    fprintf (stderr, "Version %d\n", VERSION);

    // dag is a smarter data structure, built in parallel with the
    // big_dumb_array, but meant to supplant it as soon as I'm sure.

    addressable_row_array = malloc ((1 << n) * sizeof (aluminium_Christmas_tree *));
    assert (addressable_row_array);

    row = 0;
    col = 0;

    dag = malloc (sizeof (aluminium_Christmas_tree));
    assert (dag);

    dag->level = row;
    dag->value = col;
    dag->num_children = 0;
    dag->in_use = true;
    dag->visited = false;
    dag->child = NULL;
    dag->sibling = NULL;

    switch (cardinality[row] - cardinality[row + 1]) {
        case -1:
            dag->num_children_predicted = count_0_bits (binary (col, n, binary_buffer));
            break;
        case 1:
            dag->num_children_predicted = count_1_bits (binary (col, n, binary_buffer));
            break;
        default:
            assert (false); // This should never happen.
            break;
    }

    addressable_row_array[row] = dag;

    // Now generate the graph of allowable transitions.

    // A great big shiny aluminium Christmas tree!

    assert (NULL == dag->child);
    dag->child = malloc ((dag->num_children_predicted + 1) * sizeof (aluminium_Christmas_tree *));
    assert (dag->child);
    child_number = 0;
    dag->child[child_number] = NULL; // Terminate the list.

    for (col = 0; col < (1 << n); col ++) {
        for (row_plus_one_col = 0; row_plus_one_col < (1 << n); row_plus_one_col ++) {
            if (allowable (row, col, row + 1, row_plus_one_col, cardinality, n)) {
                aluminium_Christmas_tree * here = NULL;

                here = malloc (sizeof (aluminium_Christmas_tree));
                assert (here);

                here->level = row + 1;
                here->value = row_plus_one_col;
                here->in_use = true;
                here->num_children = 0;
                here->num_children_predicted = 0;
                here->visited = false;
                here->child = NULL;
                here->sibling = NULL;

                dag->child[child_number] = here;

                if (child_number > 0) {
                    (dag->child[child_number - 1])->sibling = here;
                }
                here->sibling = NULL;

                fprintf (stderr, "%p[%d] = %p\n", (void *) dag,
                    child_number, (void *) dag->child[child_number]);

                ++ child_number;
                dag->child[child_number] = NULL; // Terminate the list.
            }
        }
    }
    dag->num_children = child_number;
    ++ row;

    addressable_row_array[row] = dag->child[0];

    // Now build a DAG in the big dumb array.

    if (cardinality[0] >= 0) {
        int number_of_nodes_used = 0;
        double fill_factor_n = 0.0;
        double fill_factor_MAX_n = 0.0;

        for (row = 0; row < ( (1 << n) - 1); row ++) {
            for (col = 0; col < (1 << n); col ++) {
                int row_plus_one_col = 0;
                int pointer_number = 0;
                int predicted_number_of_children = 0;

                for (row_plus_one_col = 0; row_plus_one_col < (1 << n); row_plus_one_col ++) {

                    if (allowable (row, col, row + 1, row_plus_one_col, cardinality, n)) {
                        char buf1[n + 1];
                        char buf2[n + 1];

                        assert (count_1_bits (binary (col, n, buf1)) == cardinality[row]);
                        assert (count_1_bits (binary (row_plus_one_col, n, buf1))
                            == cardinality[row + 1]);

                        switch (cardinality[row] - cardinality[row + 1]) {
                            case -1:
                                predicted_number_of_children = count_0_bits (binary (col, n, buf2));
                                break;
                            case 1:
                                predicted_number_of_children = count_1_bits (binary (col, n, buf2));
                                break;
                            default:
                                assert (false); // This should never happen.
                                break;
                        }

                        assert (pointer_number < predicted_number_of_children);

                        big_dumb_array[row][col].level = row;
                        big_dumb_array[row][col].value = col;
                        big_dumb_array[row][col].in_use = true;
                        if (NULL == big_dumb_array[row][col].child) {
                            big_dumb_array[row][col].child = malloc (sizeof (aluminium_Christmas_tree *)
                                * predicted_number_of_children + 1);
                            assert (big_dumb_array[row][col].child);
                            big_dumb_array[row][col].child[0] = NULL; // NULL terminate just in case.
                        }
                        big_dumb_array[row][col].child[pointer_number] =
                            &(big_dumb_array[row + 1][row_plus_one_col]);
                        big_dumb_array[row][col].num_children ++;
                        big_dumb_array[row][col].num_children_predicted = predicted_number_of_children;

                        // Why does the following line cause a segfault in error check later?

                        children_per_node_at_level[row] = predicted_number_of_children;

                        // Make sure child nodes exist if they are named.

                        big_dumb_array[row + 1][row_plus_one_col].level = row + 1;
                        big_dumb_array[row + 1][row_plus_one_col].value = row_plus_one_col;
                        big_dumb_array[row + 1][row_plus_one_col].in_use = true;

                        // However, we don't know anything about *their* children.

#ifdef DEBUG
                        fprintf (stderr,
                            "recorded node %p (%d, %d) -> %p (%d, %d) with %d ",
                            (void *) &big_dumb_array[row][col],
                            big_dumb_array[row][col].level,
                            big_dumb_array[row][col].value,
                            (void *) &big_dumb_array[row + 1][row_plus_one_col],
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
        }
        if (odd (n)) {
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].level = (1 << n) - 1;
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].value = (1 << n) - 1;
            big_dumb_array[(1 << n) - 1][(1 << n) - 1].in_use = true;

            fprintf (stderr, "node %p (%d, %d) also created with %d children.\n",
                (void *) &big_dumb_array[(1 << n) - 1][(1 << n) - 1],
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
                        row, col,
                        (void *) &big_dumb_array[row][col],
                        big_dumb_array[row][col].num_children_predicted,
                        big_dumb_array[row][col].num_children);
                }
            }
        }
    }
    else {
        fprintf (stderr, "We have no cardinality data; not attempting "
            "to generate the graph of allowed transitions.\n");
    }

    fprintf (stderr, "\n");

    // Initialise the sequence accumulator (outside of the recursive fn).

    for (i = 0; i < MAX_n; i ++) {
        sequence_accumulator[i] = -1;
    }

    depth_first_search (start, cardinality, n, option_run_once, option_restart_level);

    fprintf (stderr, "\n");
    gmp_printfcomma (good_sequences);
    if (mpz_cmp_ui (good_sequences, 1) == 0) {
        fprintf (stderr, " sequence was");
    }
    else {
        fprintf (stderr, " sequences were");
    }
    fprintf (stderr, " found in all");
    if (option_run_once) {
        fprintf (stderr, " (we were told to stop after the first one)");
    }
    fprintf (stderr, ".\n");

    fprintf (stderr, "The predicted number of candidate sequences was ");
    gmp_printfcomma (predicted_number_of_candidate_sequences);
    fprintf (stderr, ".\n");

    if (option_generate_graph) {
        write_dot_file (start, cardinality, n);
    }

    free (cardinality);
    cardinality = NULL;

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            if (big_dumb_array[row][col].child) {
                free (big_dumb_array[row][col].child);
                big_dumb_array[row][col].child = NULL;
            }
        }
    }

    free_dag (dag, n);
    dag = NULL;

    free (addressable_row_array);
    addressable_row_array = NULL;

    mpz_clear (good_sequences);
    mpz_clear (predicted_number_of_candidate_sequences);
    mpz_clear (predicted_number_of_candidate_sequences_at_row_n);

    return EXIT_SUCCESS;
}

// This is a simple depth-first search to reset all the 'visited' flags.

void reset_visited_flags (aluminium_Christmas_tree * p, int n) {
    int i = 0;

    if (false == p->visited) {
        return;
    }

    for (i = 0; i < p->num_children; i ++ ) {
        reset_visited_flags (p->child[i], n);
    }
    p->visited = false;
    return;
}

// Doing the same thing as the previous fn but by means of brute force.

void reset_visited_flags_the_hard_way (aluminium_Christmas_tree * p, int n) {
    int row = 0;
    int col = 0;

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++) {
            p->visited = false;
        }
    }
    return;
}

void write_dot_file (aluminium_Christmas_tree * root, int * cardinality, int n) {
    int row = 0;
    int col = 0;
    aluminium_Christmas_tree * p = NULL;

    p = root;
    assert (p);

    // First reset the 'visited' flag on every node in the graph.

    reset_visited_flags (p, n);

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

    blank_line ();
    printf (TAB "/* These are the allowable states. */\n");
    blank_line ();
    printf (TAB "node [shape=rect]\n");
    blank_line ();

    for (row = 0; row < (1 << n); row ++) {
        for (col = 0; col < (1 << n); col ++ ) {
            char buf[n + 1];

            if (count_1_bits (binary (col, n, buf)) == cardinality[row]) {
                char buf1[n + 1];
                char buf2[n + 1];

                printf (TAB "level_%d_%s [label=\"%s\"",
                    row, binary (col, n, buf1), binary (col, n, buf2));
                if (sequence_accumulator[row] == col) {
                    printf (" color=red, fontcolor=red");
                }
                printf ("]\n");
            }
        }
    }

    blank_line ();

    printf (TAB "/* These are the allowable transitions. */\n");
    blank_line ();
    printf (TAB "graph [ordering=out] /* keep binary numbers in order */\n");
    printf (TAB "edge [style=solid,color=black]\n");
    blank_line ();

    breadth_first_search (root, n);

    blank_line ();
    printf (TAB "/* end of .dot file */\n");
    printf ("}\n");
    blank_line ();
    if (fflush (stdout)) {
        fprintf (stderr, "fflush() returned an error (continuing)\n");
    }

    return;
}

// Return a string containing the binary representation of $n$ in $b$ bits in buffer.

char * binary (int n, int num_bits, char * buffer) {
    int i = 0;

    // Make sure buffer points to some storage. I don't think there is any
    // way for this function to know for sure it's valid storage, or that
    // it's big enough.

    assert (buffer);

    // Make sure the result fits in the specified number of bits.

    if (n < 0) {
        fprintf (stderr, "WARNING: in binary, n = %d\n", n);
    }

    assert ( log (n) <= (double) num_bits );

    buffer[0] = '\0';

    for (i = (1 << (num_bits - 1)); i > 0; i >>= 1) {
        strcat (buffer, ((n & i) == i) ? "1" : "0");
    }

    return buffer;
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

boolean allowable (int from_row, int from_col, int to_row, int to_col, int * cardinality, int n) {
    char buf1 [n + 1];
    char buf2 [n + 1];
    char buf3 [n + 1];

    if ( 1 == count_1_bits (binary (( from_col ^ to_col ), n, buf1))
        && (count_1_bits (binary (from_col, n, buf2)) == cardinality[from_row])
        && (count_1_bits (binary (to_col, n, buf3)) == cardinality[to_row]) ) {
            return true;
    }
    return false;
}

// Is $n$ odd?

boolean odd (int n) {
    return (n % 2);
}

// Is $n$ even?

boolean even (int n) {
    return !odd (n);
}

// This is a scaffolding function to count cardinalities.

void count_cardinalities (int n) {
    int i = 0;
    char * p = NULL;

    assert (n > 0);

    for (i = 0; i < (1 << n); i ++) {
        char buf [n + 1];

        p = binary (i, n, buf);
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
    int percent_done = 0;
    int percent_almost_done = 0;

    percent_done = 100 * log2f(sequence[1]) / (float) n;
    percent_almost_done = 100 * log2f(sequence[1]) / (float) (n - 1);

    gmp_printfcomma (good_sequences);
    fprintf (stderr, " sequence");
    if (mpz_cmp_ui (good_sequences, 1) != 0) {
        fprintf (stderr, "s");
    }
    if (percent_almost_done < 1) {
        fprintf (stderr, " (~0%% done) found: ");
    }
    else {
        fprintf (stderr, " (%d%% to %d%% done) found: ",
            percent_done, percent_almost_done);
    }
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
        char buf1[n + 1];

        if (count_1_bits (binary (sequence[i], n, buf1)) != cardinality[i]) {
            char buf2[n + 1];
            
            fprintf (stderr, "sanity check failed on sequence ");
            display_sequence_helper (sequence, n);
            fprintf (stderr, ", count_1_bits (\"%d\") != %d\n",
                count_1_bits (binary (sequence[i], n, buf2)), cardinality[i]);

            assert (false); // Fail on purpose.
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

// Dump a file containing enough checkpoint data to restart the process.

void checkpoint (int n) {
    FILE * fp_out = NULL;

    fp_out = fopen (CHECKPOINT_FILE, "w");
    if (NULL == fp_out) {
        fprintf (stderr, "Unable to open checkpoint file for writing: %s (continuing)\n",
            strerror (errno));
    }
    else {
        int i = 0;
        time_t now = 0;

        now = time (NULL);

        fprintf (fp_out, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
        open_XML_tag (fp_out, "checkpoint", 0);
            write_XML_integer_value (fp_out, "version", VERSION, 1);
            write_XML_integer_value (fp_out, "order", n, 1);
            write_XML_long_long_value (fp_out,
                "starting_time", (long long) starting_time, 1);
            write_XML_long_long_value (fp_out, "now", (long long) now, 1);
            write_XML_mpz_integer_value (fp_out,
                "good_sequences", good_sequences, 1);
            write_XML_mpz_integer_value (fp_out,
                "predicted_number_of_candidate_sequences",
                predicted_number_of_candidate_sequences, 1);
            if (sequence_is_valid) {
                open_XML_tag (fp_out, "solution", 1);
                for (i = 0; i < (1 << n); i ++) {
                    write_XML_integer_value (fp_out, "sequence_accumulator",
                        sequence_accumulator[i], 2);
                }
                close_XML_tag (fp_out, "solution", 1);
            }
        close_XML_tag (fp_out, "checkpoint", 0);
        if (fclose (fp_out)) {
            fprintf (stderr, "Error closing checkpoint file: %s (continuing)\n",
                strerror (errno));
        }
    }
    return;
}

void emit_tabs (FILE * fp, int how_deep) {
    int i = 0;

    for (i = 0; i < how_deep; i ++) {
        fprintf (fp, TAB);
    }
    return;
}

void open_XML_tag (FILE * fp, char * tag, int nesting) {
    emit_tabs (fp, nesting);
    fprintf (fp, "<%s>\n", tag);
    return;
}

void close_XML_tag (FILE * fp, char * tag, int nesting) {
    emit_tabs (fp, nesting);
    fprintf (fp, "</%s>\n", tag);
    return;
}

void write_XML_string_value (FILE * fp, char * tag, char * value, int nesting) {
    open_XML_tag (fp, tag, nesting);
    emit_tabs (fp, nesting + 1);
    fprintf (fp, "%s\n", value);
    close_XML_tag (fp, tag, nesting);
    return;
}

void write_XML_integer_value (FILE * fp, char * tag, int value, int nesting) {
    open_XML_tag (fp, tag, nesting);
    emit_tabs (fp, nesting + 1);
    fprintf (fp, "%d\n", value);
    close_XML_tag (fp, tag, nesting);
    return;
}

void write_XML_long_long_value (FILE * fp, char * tag, long long value, int nesting) {
    open_XML_tag (fp, tag, nesting);
    emit_tabs (fp, nesting + 1);
    fprintf (fp, "%lld\n", value);
    close_XML_tag (fp, tag, nesting);
    return;
}

void write_XML_mpz_integer_value (FILE * fp, char * tag, mpz_t value, int nesting) {
    open_XML_tag (fp, tag, nesting);
    emit_tabs (fp, nesting + 1);
    gmp_fprintf (fp, "%Zd\n", value);
    close_XML_tag (fp, tag, nesting);
    return;
}

// Print 'usage' message and quit.

void usage (char * programme_name) {
    assert (programme_name);

    fprintf (stderr, USAGE "\n", programme_name);
    exit (EXIT_FAILURE);
}

// Handle the command line.
//
// Note: isdigit() before atoi() means it won't accept negative numbers.

void process_command_line_options (int argc, char ** argv,
    boolean * option_1, boolean * option_g, int * option_r, int * n) {

    int c = 0;

    while ((c = getopt (argc, argv, "1gr:")) != -1) {
        switch (c) {
            case '1':
                *option_1 = true;
                break;
            case 'g':
                *option_g = true;
                break;
            case 'r':
                if (isdigit ((int)optarg[0])) {
                    *option_r = atoi (optarg);
                }
                else {
                    fprintf (stderr,
                        "Option -%c requires a numeric argument.\n",
                            optopt);
                    usage (argv[0]);
                }
                break;
            case '?':
                if ('r' == optopt) {
                    fprintf (stderr, "Option -%c requires an argument.\n",
                        optopt);
                    usage (argv[0]);
                }
                else if (isprint (optopt)) {
                    fprintf (stderr, "Unknown option '-%c'.\n", optopt);
                    usage (argv[0]);
                }
                else {
                    fprintf (stderr,
                        "Unknown option character '\\x%x'.\n", optopt);
                    usage (argv[0]);
                }
                return;
                break;
            default:
                usage (argv[0]);
                break;
        }
    }
    if (optind < argc) { // Then there's at least one argument after the options.
        if (isdigit ((int)argv[optind][0])) {
            *n = atoi (argv[optind]);
            return;
        }
        else {
            usage (argv[0]);
        }
    }
    else {
        usage (argv[0]);
    }
    return;
}

// Display a single node of the digraph.

void display_digraph_node (aluminium_Christmas_tree * p, int n) {
    if (NULL == p) {
        fprintf (stderr, "NULL digraph node.\n");
    }
    else {
        char buf[n + 1];

        fprintf (stderr, "digraph node %p: level %d, value %s has ",
            (void *) p, p->level, binary (p->value, n, buf));
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
                fprintf (stderr, " %p", (void *) p->child[i]);
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

void depth_first_search (aluminium_Christmas_tree * p,
    int * cardinality_sequence,
    int n,
    boolean first_solution_only,
    int restart_level) {

    int i = 0;
    int * early_dup_check = NULL;

    assert (p);

    if (first_solution_only && found_first_solution) {
        return;
    }

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

    // This is messy, but it's an attempt to provide a limited restart
    // capability if the computation is ever interrupted. (I wonder if
    // call-with-current-continuation would be a much more general way
    // of accomplishing this.)

    if (restart_level > 0) {
        if (first_time_through_recursion) {
            first_time_through_recursion = false;
            for (i = restart_level; i < p->num_children; i ++) {
                depth_first_search (p->child[i], cardinality_sequence, n,
                    first_solution_only, restart_level);
            }
        }
        else {
            for (i = 0; i < p->num_children; i ++) {
                depth_first_search (p->child[i], cardinality_sequence, n,
                    first_solution_only, restart_level);
            }
        }
    }
    else {
        // The following loop could be multi-threaded for up to $n$ cores.

        for (i = 0; i < p->num_children; i ++) {
            depth_first_search (p->child[i], cardinality_sequence, n,
                first_solution_only, restart_level);
        }
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
            sequence_is_valid = true;
            found_first_solution = true;
            mpz_add_ui (good_sequences, good_sequences, 1);
            emit_sequence (sequence_accumulator, n);
            // Don't do it if it's going to happen a thousand times every second.
            if (n < 5 || n > 7 || (true == first_solution_only) ) {
                checkpoint (n);
            }
        }

        free (duplicate_check);
        duplicate_check = NULL;
    }
    return;
}

// Breadth-first search the entire DAG.

void breadth_first_search (aluminium_Christmas_tree * p, int n) {
    int i = 0;

    assert (n > 0);
    assert (n <= MAX_n);
    assert (p);

    if (p->visited) {
        return;
    }

    if (0 == p->num_children) {
        return;
    }

    for (i = 0; i < p->num_children; i ++) {
        char buf1[n + 1];
        char buf2[n + 1];

        printf (TAB "level_%d_%s -> level_%d_%s",
            p->level, binary (p->value, n, buf1),
            (p->child[i])->level, binary ((p->child[i])->value, n, buf2));
        if (sequence_accumulator[p->level] == p->value &&
            sequence_accumulator[(p->child[i])->level] == p->child[i]->value) {
            printf (" [color=red]");
        }
        else {
            printf (" [style=invis]");
        }
        printf ("\n");
        breadth_first_search (p->child[i], n);
    }
    p->visited = true;
    return;
}

// Free the memory used by a dag.

void free_dag (aluminium_Christmas_tree * root, int n) {
    int i = 0;

    assert (root);

    fprintf (stderr, "I have been told to free %p who has %d children and sibling %p.\n",
        (void *) root, root->num_children, (void *) root->sibling);
    if (root->num_children > 0) {
        for (i = 0; i < root->num_children; i ++) {
            fprintf (stderr, "  first freeing %p's child[%d] = %p\n",
                (void *) root, i, (void *) root->child[i]);
            if (root->child[i]) {
                free_dag (root->child[i], n);
            }
            else {
                fprintf (stderr, "%p->child[%d] was null.\n", (void *) root, i);
            }
        }
    }
    fprintf (stderr, "all children freed; freeing DAG %p\n", (void *) root);
    free (root);

    return;
}

#ifdef DEBUG

// For detecting memory leaks.

void * debug_malloc (size_t size, const char * file, const int line, const char * func) {
    void * p = NULL;
    
    p = calloc (size, 1);
    assert (p);

    fprintf (stderr, "%s line %d, %s() allocated %p[%zu]\n", file, line, func, p, size);
    return p;
}

#endif

void test_process_command_line_options (int option_run_once,
    int option_generate_graph, int option_restart_level, int n) {

    fprintf (stderr,
        "run_once = %d, generate_graph = %d, restart_level = %d, n = %d\n",
        option_run_once, option_generate_graph, option_restart_level, n);
    exit (1);
}

