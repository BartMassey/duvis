/*
 * Copyright © 2014 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */ 

/* Number of entries to consider "largest small". */
#define DU_INIT_ENTRIES_SIZE (128 * 1024)

/* For portability. */
#define DU_PATH_MAX 4096
#define DU_COMPONENTS_MAX DU_PATH_MAX

/* Number of spaces of indent per level. */
#define N_INDENT 2

struct entry {
    uint64_t size;		
    uint32_t n_components;    // # of components that makeup this entry
    char *path;               // for later free 
    char **components;        // The actual components of this entry
    uint32_t depth;	      // The depth of this entry in the directory tree
    uint32_t n_children;      // # of children directories at this entry level
    struct entry **children;  // Children entries of this entry
};

extern int n_entries;
extern struct entry *entries;
extern struct entry *root_entry;
extern int base_depth;

extern int gui(int argv, char **argc);
