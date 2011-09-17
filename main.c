#include "filetree.h"

#ifdef G_OS_WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <sys/stat.h>
#include <glib/gstdio.h>

#include "input.h"


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
    GStatBuf stat_buf;
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

static void get_number_of_frames(gpointer user, gpointer user_data)
{
    struct filename_list_node *fln = (struct filename_list_node *) user;
    size_t *number_of_frames;

    struct input_ops* ops = NULL;
    struct input_handle* ih = NULL;
    FILE *file = NULL;
    int result;


    (void) user_data;
    fln->d = g_malloc(sizeof(size_t));
    number_of_frames = (size_t *) fln->d;
    *number_of_frames = 0;


    ops = input_get_ops(fln->fr->raw);
    if (!ops) {
        fprintf(stderr, "no ops");
        goto free;
    }
    ih = ops->handle_init();

    file = g_fopen(fln->fr->raw, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file '%s'\n", fln->fr->display);
        goto free;
    }
    result = ops->open_file(ih, file, fln->fr->raw);
    if (result) {
        fprintf(stderr, "result fail");
        goto free;
    }

    *number_of_frames = ops->get_total_frames(ih);

  free:
    if (file) ops->close_file(ih, file);
    if (ih) ops->handle_destroy(&ih);
}

static void print_file_size(gpointer user, gpointer user_data)
{
    struct filename_list_node *fln = (struct filename_list_node *) user;
    guint64 *file_size = (guint64 *) fln->d;

    (void) user_data;
    print_utf8_string(fln->fr->display);
    g_print(", %" G_GUINT64_FORMAT "\n", *file_size);
}

static void free_list_entry(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_free(((struct filename_list_node *) user)->d);
    g_free(user);
}

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

#ifdef G_OS_WIN32
extern int __wgetmainargs(int *_Argc, wchar_t ***_Argv, wchar_t ***_Env,
                          int _DoWildCard, STARTUPINFO *_StartInfo);
#endif

static void parse_args(int *argc, char ***argv)
{
    GError *error = NULL;
    GOptionContext *context = g_option_context_new("- walk file hierarchy");
#ifdef G_OS_WIN32
    wchar_t **wargv, **wenv;
    STARTUPINFO si = {0};
    int i;

    __wgetmainargs(argc, &wargv, &wenv, TRUE, &si);
    *argv = g_new(gchar *, *argc + 1);
    for (i = 0; i < *argc; ++i) {
        (*argv)[i] = g_utf16_to_utf8(wargv[i], -1, NULL, NULL, NULL);
    }
    (*argv)[i] = NULL;
#endif

    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, argc, argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(EXIT_FAILURE);
    }
    if (*argc == 1) {
#if GLIB_CHECK_VERSION(2, 14, 0)
        gchar* help = g_option_context_get_help(context, FALSE, NULL);
        fprintf(stderr, "%s", help);
        g_free(help);
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
    g_thread_init(NULL);
    input_init(argv[0], NULL);


    setlocale(LC_COLLATE, "");
    setlocale(LC_CTYPE, "");
    tree = filetree_init(&argv[1], argc - 1,
                         recursive, follow_symlinks, &errors);
    /* filetree_print(tree); */

    g_slist_foreach(errors, print_error, NULL);
    g_slist_foreach(errors, free_error, NULL);
    g_slist_free(errors);

    filetree_file_list(tree, &files);
    g_slist_foreach(files, get_number_of_frames, NULL);
    g_slist_foreach(files, print_file_size, NULL);
    g_slist_foreach(files, free_list_entry, NULL);
    g_slist_free(files);

    filetree_destroy(tree);
    g_strfreev(file_names);
    input_deinit();

    return EXIT_SUCCESS;
}
