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

Filetree filetree_init(const char* root, GSList **errors);
void filetree_print(Filetree tree);
void filetree_destroy(Filetree tree);

#endif /* end of include guard: FILETREE_H */
