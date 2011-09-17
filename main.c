#ifdef G_OS_WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "filetree.h"
#include "parse_global_args.h"

static gboolean recursive = FALSE;
static gboolean follow_symlinks = FALSE;

static GOptionEntry entries[] =
{
    { "recursive", 'r', 0, G_OPTION_ARG_NONE, &recursive,
      "recurse into directories", NULL },
    { "follow-symlinks", 'L', 0, G_OPTION_ARG_NONE, &follow_symlinks,
      "follow symbolic links", NULL },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, 0 }
};

int main(int argc, char *argv[])
{
    GSList *errors = NULL, *files = NULL;
    Filetree tree;

    parse_global_args(&argc, &argv, entries, FALSE);
    g_thread_init(NULL);


    setlocale(LC_COLLATE, "");
    setlocale(LC_CTYPE, "");
    tree = filetree_init(&argv[1], (size_t) (argc - 1),
                         recursive, follow_symlinks, FALSE, &errors);
    /* filetree_print(tree); */

    g_slist_foreach(errors, filetree_print_error, NULL);
    g_slist_foreach(errors, filetree_free_error, NULL);
    g_slist_free(errors);

    filetree_file_list(tree, &files);
    g_slist_foreach(files, filetree_get_file_size, NULL);
    g_slist_foreach(files, filetree_print_file_size, NULL);
    g_slist_foreach(files, filetree_free_list_entry, NULL);
    g_slist_free(files);

    filetree_destroy(tree);

    return EXIT_SUCCESS;
}
