#include "filetree.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <sys/stat.h>
#include <glib/gstdio.h>

static void print_error(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_warning("%s\n", ((GError *) user)->message);
}

static void free_error(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_error_free((GError *) user);
}

static void get_file_size(gpointer user, gpointer user_data)
{
#ifdef GStatBuf
    GStatBuf stat_buf;
#else
    struct stat stat_buf;
#endif
    struct filename_list_node *fln = (struct filename_list_node *) user;
    guint64 *file_size;

    (void) user_data;
    fln->d = g_malloc(sizeof(guint64));
    file_size = (guint64 *) fln->d;
    if (g_stat(fln->fr->raw, &stat_buf)) {
        *file_size = 0;
    } else {
        *file_size = (guint64) stat_buf.st_size;
    }
}

static void print_file_size(gpointer user, gpointer user_data)
{
    struct filename_list_node *fln = (struct filename_list_node *) user;
    guint64 *file_size = (guint64 *) fln->d;

    (void) user_data;
    g_print("%s, %" G_GUINT64_FORMAT "\n",
            fln->fr->display,
            *file_size);
    /* g_print("%s\n", fln->fr->display); */
}

static void free_list_entry(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_free(((struct filename_list_node *) user)->d);
    g_free(user);
}

int main(int argc, char *argv[])
{
    GSList *errors = NULL, *files = NULL;
    Filetree tree;

    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        return EXIT_FAILURE;
    }
    setlocale(LC_COLLATE, "");
    setlocale(LC_CTYPE, "");
    tree = filetree_init(argv[1], &errors);
    /* filetree_print(tree); */

    g_slist_foreach(errors, print_error, NULL);
    g_slist_foreach(errors, free_error, NULL);
    g_slist_free(errors);

    filetree_file_list(tree, &files);
    g_slist_foreach(files, get_file_size, NULL);
    g_slist_foreach(files, print_file_size, NULL);
    g_slist_foreach(files, free_list_entry, NULL);
    g_slist_free(files);

    filetree_destroy(tree);

    return EXIT_SUCCESS;
}
