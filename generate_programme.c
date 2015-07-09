#include "generate_programme.h"

char * binary (int n, int m);
void blank_line (void);
int count_bits (char * binary_string, char digit);
void test_count_bits (void);

int hand_generated_cardinality_sequence[] = { 0, 1, 2, 1, 2, 1, 2, 1, 2, 1,
    2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 5};

int main (int argc, char ** argv) {
    int n = 0;
    int row = 0;
    int col = 0;
    int * cardinality = NULL;

    switch (argc) {
        case 2:
            n = atoi (argv[1]);
            break;
        default:
            fprintf (stderr, "Usage: %s n\n", argv[0]);
            exit (EXIT_FAILURE);
    }

    if (5 == n) {
        cardinality = hand_generated_cardinality_sequence;
    }
    else {
        int i = 0;

        cardinality = (int *) malloc (n);
        assert (cardinality != NULL);
        for (i = 0; i < n; i++) {
            cardinality[i] = -1;
        }
    }

    printf ("/*\n");
    printf ("    dot -T pdf order-%d_graph_generated.dot -o order-%d_graph_generated.pdf\n",
        n, n);
    printf ("*/\n");
    blank_line ();
    printf ("digraph order%d {\n", n);
    blank_line ();
    printf ("    node [shape=plaintext]\n");
    blank_line ();

    // left side row markers

    for (row = 0; row < (0x1 << n); row ++) {
        printf ("    level_%d [label=\"%d (%d)\"]\n",
            row, row, cardinality[row]);
    }

    blank_line ();
    printf ("    /* Connect the left side row markers invisibly so they stay lined up. */\n");

    blank_line ();
    printf ("    edge [style=invis]\n");
    blank_line ();

    printf ("    level_0");
    for (row = 0; row < (0x1 << n); row ++) {
        printf (" -> level_%d", row);

        // break long lines

        if ((row % 5) == 4) {
            blank_line ();
            printf ("        ");
        }
    }

    blank_line ();
    blank_line ();
    printf ("    graph [ordering=out]\n");
    printf ("    node [shape=rect]\n");
    blank_line ();

    printf ("    /* set of all possible states */\n");
    blank_line ();

    for (row=0; row < (0x1 << n); row ++) {
        printf ("    {\n");
        printf ("        rank=same; level_%d\n", row);
        blank_line ();

        for (col=0; col < (0x1 << n); col ++) {
            char * p = NULL;

            p = binary (col, n);

            printf ("        level_%d_%s [label=\"%s\"", row, p, p);

            if (((row == 0) && (count_bits (p, '1') == 0))
                || ((row == (1 << n) - 1) && (count_bits (p, '1') == n))) {
                printf (",color=red,textcolor=red");
            }
            else if (count_bits (p, '1') != cardinality[row]) {
                printf (",color=grey,textcolor=grey");
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

    for (col=0; col < (0x1 << n); col ++) {
        blank_line ();

        for (row = 0; row < ((0x1 << n) - 1); row ++) {
            char * p = NULL;

            p = binary (col, n);
            printf ("    level_%d_%s -> level_%d_%s\n", row, p, row + 1, p);
            free (p);
        }
    }

    blank_line ();
    printf ("    /* Connect the states invisibly so they stay lined up horizontally. */\n");

    for (row = 0; row < (0x1 << n); row ++) {
        char * p = NULL;

        blank_line ();
        p = binary (0, n);
        printf ("    level_%d_%s", row, p);
        free (p);

        for (col = 1; col < (0x1 << n); col ++) {
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
    printf ("    edge [style=solid,color=black]\n");
    blank_line ();

    // End of DOT source file.

    printf ("}\n");
    blank_line ();

    return EXIT_SUCCESS;
}

// Return a string containing the binary representation of $n$ in $m$ bits.
//
// The caller is responsible for freeing the string.

char * binary (int n, int m) {
    int i = 0;
    char * s = NULL;

    // Make sure the result fits in $m$ bits.

    assert ( log (n) <= (double) m );

    s = malloc (m + 1);
    if (!s) {
        fprintf (stderr, "malloc() failed\n");
        exit (EXIT_FAILURE);
    }
    s[0] = '\0';

    for (i = (0x1 << (m - 1)); i > 0; i >>= 1) {
        strcat (s, ((n & i) == i) ? "1" : "0");
    }

    return s;
}

// Put a blank line in the output.

void blank_line (void) {
    printf ("\n");
}

// Count the `1' bits in a string representation of a binary number.

int count_bits (char * binary_string, char digit) {
    char * p = NULL;
    int count = 0;

    p = binary_string;

    assert (p != NULL);

    while (*p) {
        if (digit == *p) {
            ++ count;
        }
        ++ p;
    }
    return count;
}

void test_count_bits (void) {
    assert (count_bits ("0", '1') == 0);
    assert (count_bits ("1", '1') == 1);
    assert (count_bits ("10101", '1') == 3);
    assert (count_bits ("10101", '0') == 2);
    assert (count_bits ("10101", 'a') == 0);
    assert (count_bits ("abc", '1') == 0);
    assert (count_bits ("", '1') == 0);
}

