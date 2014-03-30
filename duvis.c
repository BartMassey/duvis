/* Copyright © 2014 Bart Massey */
/* xdu replacement with reasonable performance */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DU_INIT_ENTRIES_SIZE (128 * 1024)
/* for portability */
#define DU_PATH_MAX 4096
#define DU_COMPONENTS_MAX 4096

struct entry {
    uint64_t size;
    uint32_t n_components;
    char **components;
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
        /* Parse the line. */
        char *index = path;
        while (isdigit(*index))
            index++;
        if (index == path || (*index != ' ' && *index != '\t')) {
            fprintf(stderr, "line %d: buffer format error\n", line_number);
            exit(1);
        }
        /* While parsing the line, allocate a new entry to it. */
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
            } else {
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

int main() {
    read_entries(stdin);
    return 0;
}
