#include "filetree.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

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

int main(int argc, char *argv[])
{
    GSList *errors = NULL;

    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        return EXIT_FAILURE;
    }
    setlocale(LC_COLLATE, "");
    setlocale(LC_CTYPE, "");
    Filetree tree = filetree_init(argv[1], &errors);
    filetree_print(tree);
    filetree_destroy(tree);

    if (errors) {
        g_slist_foreach(errors, print_error, NULL);
        g_slist_foreach(errors, free_error, NULL);
        g_slist_free(errors);
    }

    return EXIT_SUCCESS;
}
