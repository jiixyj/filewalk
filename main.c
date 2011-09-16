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

static gboolean recursive = FALSE;
static gboolean follow_symlinks = FALSE;
static gchar **file_names;

static GOptionEntry entries[] =
{
    { "recursive", 'r', 0, G_OPTION_ARG_NONE, &recursive,
      "recurse into directories", NULL },
    { "follow-symlinks", 'L', 0, G_OPTION_ARG_NONE, &follow_symlinks,
      "follow symbolic links", NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &file_names,
      "<input>" , "[FILE|DIRECTORY]..."},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, 0 }
};

static void parse_args(int *argc, char **argv[])
{
    GError *error = NULL;
    GOptionContext *context = g_option_context_new("- walk file hierarchy");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, argc, argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(EXIT_FAILURE);
    }
    if (!file_names) {
#if GLIB_CHECK_VERSION(2, 14, 0)
        gchar* help = g_option_context_get_help(context, FALSE, NULL);
        fprintf(stderr, "%s", help);
#else
        fprintf(stderr, "Get help with -h or --help.\n");
#endif
        g_option_context_free(context);
        exit(EXIT_FAILURE);
    }
    g_option_context_free(context);
}


int main(int argc, char *argv[])
{
    GSList *errors = NULL, *files = NULL;
    Filetree tree;

    parse_args(&argc, &argv);

    setlocale(LC_COLLATE, "");
    setlocale(LC_CTYPE, "");
    tree = filetree_init(file_names, g_strv_length(file_names),
                         recursive, follow_symlinks, &errors);
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

    g_strfreev(file_names);

    return EXIT_SUCCESS;
}
