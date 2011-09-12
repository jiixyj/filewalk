#include "filetree.h"

#include <stdio.h>

static const unsigned char strescape_exceptions[] = {
  0x7f,  0x80,  0x81,  0x82,  0x83,  0x84,  0x85,  0x86,
  0x87,  0x88,  0x89,  0x8a,  0x8b,  0x8c,  0x8d,  0x8e,
  0x8f,  0x90,  0x91,  0x92,  0x93,  0x94,  0x95,  0x96,
  0x97,  0x98,  0x99,  0x9a,  0x9b,  0x9c,  0x9d,  0x9e,
  0x9f,  0xa0,  0xa1,  0xa2,  0xa3,  0xa4,  0xa5,  0xa6,
  0xa7,  0xa8,  0xa9,  0xaa,  0xab,  0xac,  0xad,  0xae,
  0xaf,  0xb0,  0xb1,  0xb2,  0xb3,  0xb4,  0xb5,  0xb6,
  0xb7,  0xb8,  0xb9,  0xba,  0xbb,  0xbc,  0xbd,  0xbe,
  0xbf,  0xc0,  0xc1,  0xc2,  0xc3,  0xc4,  0xc5,  0xc6,
  0xc7,  0xc8,  0xc9,  0xca,  0xcb,  0xcc,  0xcd,  0xce,
  0xcf,  0xd0,  0xd1,  0xd2,  0xd3,  0xd4,  0xd5,  0xd6,
  0xd7,  0xd8,  0xd9,  0xda,  0xdb,  0xdc,  0xdd,  0xde,
  0xdf,  0xe0,  0xe1,  0xe2,  0xe3,  0xe4,  0xe5,  0xe6,
  0xe7,  0xe8,  0xe9,  0xea,  0xeb,  0xec,  0xed,  0xee,
  0xef,  0xf0,  0xf1,  0xf2,  0xf3,  0xf4,  0xf5,  0xf6,
  0xf7,  0xf8,  0xf9,  0xfa,  0xfb,  0xfc,  0xfd,  0xfe,
  0xff,
  '\0'
};

static int compare_filenames(gconstpointer lhs,
                             gconstpointer rhs,
                             gpointer user_data) {
    (void) user_data;
    struct filename_representations const *fr_l = lhs;
    struct filename_representations const *fr_r = rhs;
    if (fr_l->type != fr_r->type) {
        return fr_l->type - fr_r->type;
    }
    return strcmp(fr_l->collate_key, fr_r->collate_key);
}

static struct filename_representations
*filename_representations_new(const char *raw, int type)
{
    struct filename_representations *fr =
                             g_malloc(sizeof(struct filename_representations));

    char *display_pre;

    fr->type = type;
    fr->raw = g_strdup(raw);

    display_pre = g_filename_display_name(fr->raw);
    fr->collate_key = g_utf8_collate_key_for_filename(display_pre, -1);

    fr->display = g_strescape(display_pre, (const gchar *) strescape_exceptions);
    g_free(display_pre);
    return fr;
}

static void filename_representations_free(void *fr)
{
    g_free(((struct filename_representations *) fr)->raw);
    g_free(((struct filename_representations *) fr)->display);
    g_free(((struct filename_representations *) fr)->collate_key);
    g_free(fr);
}

static void walk_recursive(const char *current_dir_string,
                           GDir *current_dir,
                           GTree *current_tree,
                           GSList **errors)
{
    const gchar *basename;
    gchar *filename;

    while ((basename = g_dir_read_name(current_dir))) {
        filename = g_build_filename(current_dir_string, basename, NULL);
        if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
            GError *err = NULL;
            GDir *sub_dir = g_dir_open(filename, 0, &err);
            if (err) {
                *errors = g_slist_prepend(*errors, err);
                goto next;
            }
            struct filename_representations *fr =
                        filename_representations_new(filename, FILETREE_DIR);
            GTree *sub_dir_tree = g_tree_new_full(compare_filenames, NULL,
                                                  filename_representations_free,
                                                  filetree_destroy);
            g_tree_insert(current_tree, fr, sub_dir_tree);
            walk_recursive(filename, sub_dir, sub_dir_tree, errors);
            g_dir_close(sub_dir);
        } else if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
            struct filename_representations *fr =
                        filename_representations_new(filename, FILETREE_FILE);
            g_tree_insert(current_tree, fr, NULL);
        }
      next:
        g_free(filename);
    }
}

/* root: directory in glib file name encoding */
Filetree filetree_init(const char* root, GSList **errors)
{
    GDir *dir;
    GTree *root_tree, *sub_dir_tree;
    GError *err = NULL;

    if (!g_file_test(root, G_FILE_TEST_IS_DIR)) {
        return NULL;
    }
    dir = g_dir_open(root, 0, &err);
    if (err) {
        *errors = g_slist_prepend(*errors, err);
        return NULL;
    }

    root_tree = g_tree_new_full(compare_filenames, NULL,
                                filename_representations_free,
                                filetree_destroy);
    struct filename_representations *fr =
                            filename_representations_new(root, FILETREE_DIR);
    sub_dir_tree = g_tree_new_full(compare_filenames, NULL,
                                   filename_representations_free,
                                   filetree_destroy);

    g_tree_insert(root_tree, fr, sub_dir_tree);
    walk_recursive(root, dir, sub_dir_tree, errors);
    g_dir_close(dir);

    return root_tree;
}

static gboolean print_tree_entries(gpointer key,
                                   gpointer value,
                                   gpointer data)
{
    int i;
    for (i = 0; i < GPOINTER_TO_INT(data); ++i) {
        putchar(' ');
    }
    g_print("%s\n", ((struct filename_representations*) key)->display);
    if (value) {
        g_tree_foreach((Filetree) value,
                       print_tree_entries,
                       GINT_TO_POINTER(GPOINTER_TO_INT(data) + 2));
    }
    return FALSE;
}

void filetree_print(Filetree tree)
{
    g_tree_foreach(tree, print_tree_entries, GINT_TO_POINTER(0));
}

void filetree_destroy(Filetree tree)
{
    if (tree) {
        g_tree_destroy(tree);
    }
}
