#ifndef FILETREE_H
#define FILETREE_H

#include <glib.h>

typedef void *Filetree;

#define FILETREE_DIR 1
#define FILETREE_FILE 2

/* raw: glib filename encoding
 * display: UTF-8 encoded string
 * collate_key: for sorting
 * type: either FILETREE_DIR or FILETREE_FILE */
struct filename_representations {
    char *raw;
    char *display;
    char *collate_key;
    int type;
    int _; /* padding */
};

struct filename_list_node {
    struct filename_representations *fr;
    void *d;
};

Filetree filetree_init(char *roots[],
                       size_t roots_size,
                       gboolean recursive,
                       gboolean follow_symlinks,
                       GSList **errors);
void filetree_print(Filetree tree);
void filetree_destroy(Filetree tree);
void filetree_file_list(Filetree tree, GSList **files);

void print_utf8_string(const char *string);

#endif /* end of include guard: FILETREE_H */
