/*
 * Copyright © 2014 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */ 

/* Path memory allocator. */

/* Size field is at most 2**64 bytes, but in kB.
 * Thus max digits in 64-bit size is
 *   ceil(log10(2**64 / 1024)) = 17
 * Buffer is size field + separator + "./" + path + newline.
 */
#define DU_BUFFER_LENGTH (17 + 1 + (2 + DU_PATH_MAX) + 1)
#define MAX_PATH_BUFFER (1024 * DU_BUFFER_LENGTH)

static char *path_buffer = 0;
static uint64_t n_path_buffer = 0;

static inline int path_get(char *path, int npath, FILE *f, int zeroflag) {
    int nread = 0;
    while (1) {
        if (nread >= npath)
            return -1;
        int ch = fgetc(f);
        if (ch == EOF) {
            if (nread > 0) {
                fprintf(stderr, "warning: unterminated final path\n");
                nread++;
                break;
            }
            return 0;
        }
        nread++;
        if ((zeroflag && ch == '\0') || (!zeroflag && ch == '\n'))
            break;
        *path++ = ch;
    }
    *path = '\0';
    n_path_buffer -= DU_BUFFER_LENGTH - nread;
    return nread;
}

static inline char *path_alloc() {
    if (!path_buffer || n_path_buffer + DU_BUFFER_LENGTH > MAX_PATH_BUFFER) {
        path_buffer = malloc(MAX_PATH_BUFFER);
        if (!path_buffer) {
            perror("malloc");
            exit(1);
        }
        n_path_buffer = 0;
    }
    char *result = &path_buffer[n_path_buffer];
    n_path_buffer += DU_BUFFER_LENGTH;
    return result;
}

/* Don't leak spare portion of last block */
static inline void path_cleanup() {
    if (path_buffer)
        path_buffer = realloc(path_buffer, n_path_buffer);
}
