/*
 * Copyright © 2014 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */

/* ASCII xdu replacement with reasonable performance. */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Number of entries to consider "largest small". */
#define DU_INIT_ENTRIES_SIZE (128 * 1024)

/* For portability. */
#define DU_PATH_MAX 4096
#define DU_COMPONENTS_MAX DU_PATH_MAX

/* Number of spaces of indent per level. */
#define N_INDENT 2

struct entry {
    uint64_t size;
    uint32_t n_components;
    char *path;   /* for later free */
    char **components;
    uint32_t n_children;
    struct entry **children;
};

int n_entries = 0;
int max_entries = 0;
struct entry *entries = 0;

void read_entries(FILE *f) {
    int line_number = 0;
    /* Size field is at most 2**64 bytes, but in kB.
     * Thus max digits in 64-bit size is
     *   ceil(log10(2**64 / 1024)) = 17
     * Buffer is size field + separator + "./" + path + newline.
     */
    int du_buffer_length = 17 + 1 + (2 + DU_PATH_MAX) + 1;
    /* Loop reading lines from the du file and processing them. */
    while (1) {
        /* Get a buffer for the line data. */
        char *path = malloc(du_buffer_length);
        if (!path) {
            perror("malloc");
            exit(1);
        }
        /* Read the next line. */
        path[du_buffer_length - 1] = '\0';
        errno = 0;
        char *result = fgets(path, du_buffer_length, f);
        if (!result) {
            if (errno != 0) {
                perror("fgets");
                exit(1);
            }
            free(path);
            entries = realloc(entries, n_entries * sizeof(entries[0]));
            if (!entries) {
                perror("realloc");
                exit(1);
            }
            return;
        }
        line_number++;
        if (path[du_buffer_length - 1] != '\0') {
            if (path[du_buffer_length - 1] == '\n')
                path[du_buffer_length - 1] = '\0';
            fprintf(stderr, "line %d: buffer overrun\n", line_number);
            exit(1);
        }
        /*
         * Don't leak a ton of data on each path. The size
         * field and separator and newline are still leaked.
         * C'est la vie.
         */
        path = realloc(path, strlen(path) + 1);
        if (!path) {
            perror("realloc");
            exit(1);
        }
        /* Allocate a new entry for the line. */
        while (n_entries >= max_entries) {
            if (max_entries == 0)
                max_entries = DU_INIT_ENTRIES_SIZE;
            else
                max_entries *= 2;
            entries = realloc(entries, max_entries * sizeof(entries[0]));
            if (!entries) {
                perror("realloc");
                exit(1);
            }
        }
        struct entry *entry = &entries[n_entries++];
        entry->path = path;
        entry->n_children = 0;
        entry->children = 0;
        /* Start to parse the line. */
        char *index = path;
        while (isdigit(*index))
            index++;
        if (index == path || (*index != ' ' && *index != '\t')) {
            fprintf(stderr, "line %d: buffer format error\n", line_number);
            exit(1);
        }
        /* Parse the size field. */
        *index++ = '\0';
        int n_scanned = sscanf(path, "%lu", &entry->size);
        if (n_scanned != 1) {
            fprintf(stderr, "line %d: size parse failure\n", line_number);
            exit(1);
        }
        /*
         * Parse the path. Note that we don't skip extra separator
         * chars, on the off chance that there's a leading path that
         * starts with a whitespace character.
         */
        entry->components =
            malloc(DU_COMPONENTS_MAX * sizeof(entry->components[0]));
        if (!entry->components) {
            perror("malloc");
            exit(1);
        }
        entry->components[0] = index;
        entry->n_components = 1;
        while (1) {
            if (*index == '\n' || *index == '\0') {
                *index = '\0';
                break;
            }
            else if (*index == '/') {
                *index++ = '\0';
                entry->components[entry->n_components++] = index;
                assert(entry->n_components < DU_COMPONENTS_MAX);
            }
            else {
                index++;
            }
        }
        /* Don't leak a ton of data on each entry. */
        entry->components =
            realloc(entry->components,
                    entry->n_components * sizeof(entry->components[0]));
        if (!entry->components) {
            perror("realloc");
            exit(1);
        }
    }
    assert(0);
}

/*
 * Priorities for sort:
 *   (1) Prefixes before path extensions.
 *   (2) Ascending alphabetical order.
 */
int compare_entries(const void *p1, const void * p2) {
    const struct entry *e1 = p1;
    const struct entry *e2 = p2;
    int n1 = e1->n_components;
    int n2 = e2->n_components;
    for (int i = 0; i < n1 && i < n2; i++) {
        int q = strcmp(e1->components[i], e2->components[i]);
        if (q != 0)
            return q;
    }
    if (n1 != n2)
        return (n1 - n2);
    assert(0);
}

/*
 * Build a tree in the entry structure. The two-pass
 * design means monotonic malloc() usage, because efficiency.
 */
void build_tree(uint32_t start, uint32_t end, int offset) {
    /* Set up for calculation. */
    struct entry *e = &entries[start];
    if (e->n_components != offset + 1) {
        fprintf(stderr, "index %d: unexpected entry\n", start + 1);
        exit(1);
    }
    /* Pass 1: Count and allocate direct children. */
    for (int i = start + 1; i < end; i++)
        if (entries[i].n_components == offset + 2)
            e->n_children++;
    e->children = malloc(e->n_children * sizeof(e->children[0]));
    if (!e->children) {
        perror("malloc");
        exit(1);
    }
    /* Pass 2: Fill direct children and build subtrees. */
    int n_children = 0;
    int i = start + 1;
    while (i < end) {
        if (entries[i].n_components != offset + 2) {
            fprintf(stderr, "index %d: missing entry\n", i + 1);
            exit(1);
        }
        e->children[n_children++] = &entries[i];
        int j = i + 1;
        /* Walk to end of subtree. */
        while (j < end && entries[j].n_components > offset + 2 &&
               !strcmp(entries[i].components[offset + 1],
                       entries[j].components[offset + 1]))
            j++;
        /* If subtree is found, build it. */
        if (j > i + 1)
            build_tree(i, j, offset + 1);
        i = j;
    }
}

void indent(int depth) {
    for (int i = 0; i < N_INDENT * depth; i++)
        putchar(' ');
}

void show_entries(struct entry *root, int depth) {
    indent(depth);
    assert(root->n_components == depth + 1);
    printf("%s %lu\n", root->components[depth], root->size);
    for (int i = 0; i < root->n_children; i++)
        show_entries(root->children[i], depth + 1);
}

int main() {
    fprintf(stderr, "(1) Parsing du file.\n");
    read_entries(stdin);
    fprintf(stderr, "(2) Sorting entries.\n");
    qsort(entries, n_entries, sizeof(entries[0]), compare_entries);
    fprintf(stderr, "(3) Building tree.\n");
    build_tree(0, n_entries, 0);
    fprintf(stderr, "(4) Rendering tree.\n");
    show_entries(&entries[0], 0);
    return 0;
}
