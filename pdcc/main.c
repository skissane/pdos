/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "cpplib.h"
#include "c_ppout.h"
#include "inc_path.h"
#include "xmalloc.c"
#include "xrealloc.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct deferred_option {
    int code;
    const char *arg;
};

typedef struct {
    cpp_reader *reader;
    include_paths *ic;
    const char **input_names;
    unsigned int input_name_count;
    const char *output_name;
    int preprocess_only_flag;
    int compile_flag;
    void *processor;
    struct deferred_option *deferred_options;
    unsigned int deferred_count;
    int hosted;
} global_core;

global_core *global_create_core(void)
{
    global_core *core = xmalloc(sizeof(*core));
    core->reader = NULL;
    core->ic = NULL;
    core->input_names = NULL;
    core->input_name_count = 0;
    core->output_name = NULL;
    core->preprocess_only_flag = 0;
    core->compile_flag = 1;

    core->deferred_options = NULL;
    core->deferred_count = 0;

    core->hosted = 1;

    return (core);
}

void global_destroy_core(global_core *core)
{
    free(core->input_names);
    free(core->deferred_options);
    free(core);
}

enum {
    OPT_D
};

static void defer_option(global_core *core, int code, const char *arg)
{
    core->deferred_options[core->deferred_count].code = code;
    core->deferred_options[core->deferred_count].arg = arg;
    core->deferred_count++;
}

static void diagnostics(cpp_reader *reader,
                        enum cpp_diagnostic_level level,
                        int ignored,
                        location_t loc,
                        const char *message,
                        va_list *vl)
{
    fprintf(stderr, "\n");
    if (loc.file)
    {
        fprintf(stderr, "%s:%lu:%i: ", loc.file, loc.line, loc.column);
    }
    else
    {
        fprintf(stderr, "unknown location: ");
    }
    switch(level)
    {
        case CPP_DL_WARNING: fprintf(stderr, "warning: "); break;
        case CPP_DL_ERROR: fprintf(stderr, "error: "); break;
        case CPP_DL_CRITICAL: fprintf(stderr, "critical error: "); break;
        case CPP_DL_INTERNAL_ERROR: fprintf(stderr, "internal error: "); break;
    }
    vfprintf(stderr, message, *vl);
    fprintf(stderr, "\n");
    if (level == CPP_DL_CRITICAL)
    {
        /* +++Compilation failure. */
    }
}

void c_init_options(global_core *core,
                    unsigned int option_count,
                    void *options)
{
    core->reader = cpp_create_reader(STD_C17, NULL);
    core->ic = ic_create_include_paths();

    cpp_get_callbacks(core->reader)->diagnostics = &diagnostics;

    core->deferred_options = xmalloc(sizeof(struct deferred_option)
                                    * option_count);
    core->deferred_count = 0;
}

void c_after_options(global_core *core)
{
    ic_register_include_chains(core->ic, core->reader);
    
    if (core->preprocess_only_flag)
    {
        FILE *output;
        if ((core->output_name == NULL)
            || (core->output_name[0] == '\0'))
        {
            output = stdout;
        }
        else output = fopen(core->output_name, "w");

        core->processor = init_pp_output(core->reader, output);
        if (cpp_read_main_file(core->reader, core->input_names[0]) == NULL)
        {
            printf("+++Error c_after_options\n");
            return;
        }
    }
    else
    {
        printf("+++Finish c_after_options\n");
    }
}

static void c_finish_options(global_core *core)
{
    location_t loc;
    unsigned int i;

    loc.file = "<built-in>";
    loc.line = 1;
    loc.column = 0;
    cpp_get_callbacks(core->reader)->file_change(core->reader,
                                                 loc,
                                                 CPP_REASON_RENAME);

    cpp_force_token_locations(core->reader, loc);
    cpp_init_builtins(core->reader, core->hosted);

    loc.file = "<command-line>";
    loc.line = 1;
    loc.column = 0;
    cpp_get_callbacks(core->reader)->file_change(core->reader,
                                                 loc,
                                                 CPP_REASON_RENAME);

    cpp_force_token_locations(core->reader, loc);

    for (i = 0; i < core->deferred_count; i++)
    {
        struct deferred_option *option;
        option = &(core->deferred_options[i]);

        switch (option->code)
        {
            case OPT_D:
                cpp_define(core->reader, option->arg);
                break;
        }
    }

    cpp_stop_forcing_token_locations(core->reader);
}

static void c_end(global_core *core)
{
    int i;
    cpp_destroy_reader(core->reader);
    for (i = 0; i < 4; i++)
    {
        cpp_dir *d = core->ic->tails[i];
        core->ic->tails[i] = 0;
        while (d != NULL)
        {
            cpp_dir *dn = d->next;
            ic_free_cpp_dir(d);
            d = dn;
        }
    }

    free(core->ic);
    if (core->preprocess_only_flag)
    {
        free(core->processor);
    }
}

static char *xstrdup(const char *str)
{
    size_t len = strlen(str);
    char *out = xmalloc(len + 1);
    strcpy(out, str);
    return (out);
}

static void assemble_file(cpp_reader *reader)
{
    return;
}

int main(int argc, char **argv)
{
    int i;
    
    global_core *core;

    for (i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--help") == 0)
            || (strcmp(argv[i], "-h") == 0))
        {
            printf("Usage: pdcc [options] file...\n");
            printf("Options:\n");
            printf("  -h, --help      Displays this information.\n");
            printf("  -E              Preprocess only; do not compile.\n");
            printf("  -o <file>       Puts output into <file>.\n");
            printf("  -I <directory>  #include searches in <directory>.\n");
            printf("  -D <macro>      Defines <macro>.\n");

            return (0);
        }
    }

    core = global_create_core();

    c_init_options(core, argc - 1, NULL);

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            argv[i]++;
            switch (argv[i][0])
            {
                case 'C':
                    core->compile_flag = 0;
                    break;
                case 'E':
                    core->preprocess_only_flag = 1;
                    break;
                case 'I':
                    if (argv[i][1] != '\0')
                    {
                        ic_add_path(core->ic,
                                    xstrdup(argv[i] + 1),
                                    INCLUDE_PATH_ANGLED);
                    }
                    else
                    {
                        ic_add_path(core->ic,
                                    xstrdup(argv[++i]),
                                    INCLUDE_PATH_ANGLED);
                    }
                    break;
                case 'o':
                    if (argv[i][1] != '\0')
                    {
                        core->output_name = argv[i] + 1;
                    }
                    else
                    {
                        core->output_name = argv[++i];
                    }
                    break;
                case 'D':
                    if (argv[i][1] != '\0')
                    {
                        defer_option(core, OPT_D, argv[i] + 1);
                    }
                    else
                    {
                        defer_option(core, OPT_D, argv[++i]);
                    }
                    break;
            }
        }
        else
        {
            core->input_name_count++;
            core->input_names = xrealloc(core->input_names,
                                        (sizeof(*(core->input_names))
                                         * (core->input_name_count)));
            core->input_names[core->input_name_count - 1] = argv[i];
        }
    }

    if (core->input_name_count == 0)
    {
        printf("Error: no input files.\n");
        printf("Use --help for help.\n");
        goto end;
    }

    if (!(core->preprocess_only_flag))
    {
        printf("+++Only preprocessor is currently working\n");
    }
    
    c_after_options(core);

    /* Call for each file. */
    c_finish_options(core);

    if (core->preprocess_only_flag)
    {
        preprocess_file(core->processor, core->reader);
    }
    if (core->compile_flag)
    {
        assemble_file(core->reader);
    }

end:
    c_end(core);
    global_destroy_core(core);
    return (0);
}
