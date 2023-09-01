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
#include "cclib.h"
#include "c_ppout.h"
#include "inc_path.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int line_flag = 1;

struct deferred_option {
    int code;
    const char *arg;
};

typedef struct {
    cpp_reader *reader;
    cc_reader *cc_reader;
    include_paths *ic;
    const char **input_names;
    unsigned int input_name_count;
    const char *output_name;
    int preprocess_flag;
    int compile_flag;
    void *processor;
    void *compiler;
    struct deferred_option *deferred_options;
    unsigned int deferred_count;
    int hosted;
} global_core;

global_core *global_create_core(void)
{
    global_core *core = xmalloc(sizeof(*core));
    core->reader = NULL;
    core->cc_reader = NULL;
    core->ic = NULL;
    core->input_names = NULL;
    core->input_name_count = 0;
    core->output_name = NULL;
    core->preprocess_flag = 1;
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

#include "diag.h"
static void diagnostics(cpp_reader *reader,
                        enum cpp_diagnostic_level level,
                        int ignored,
                        location_t loc,
                        const char *message,
                        va_list *vl)
{
    if (loc.file)
        fprintf(stderr, TTY_RED TTY_BOLD "%s:%li:%i: " TTY_RESET TTY_BOLD,
            loc.file, loc.line, loc.column);
    else
        fprintf(stderr, TTY_RED TTY_BOLD "<unknown>: " TTY_RESET TTY_BOLD);
    vfprintf(stderr, message, *vl);
    fprintf(stderr, ":" TTY_RESET "\n");

    /* Print the line with the error */
    if (loc.file)
    {
        FILE *fp = fopen(loc.file, "r");
        if(fp)
        {
            unsigned long line_cnt = 0;
            char line[80];
            rewind(fp);
            while (fgets(line, 80, fp) && line_cnt != loc.line - 1)
                line_cnt++;
            
            if (line_cnt == loc.line - 1)
            {
                char *p = strchr(line, '\n');
                size_t i;
                if (p) *p = '\0';
                fprintf(stderr, "%s\n", line);
                for (i = 1; i < loc.column; i++)
                    fputc('-', stderr);
                fprintf(stderr, "^\n");
            }
            fclose(fp);
        }
    }

    if (level == CC_DL_ERROR)
        exit(EXIT_FAILURE);
}

void c_init_options(global_core *core,
                    unsigned int option_count,
                    void *options)
{
    core->reader = cpp_create_reader(STD_C89, NULL);
    core->ic = ic_create_include_paths();

    cpp_get_callbacks(core->reader)->diagnostics = &diagnostics;

    core->deferred_options = xmalloc(sizeof(struct deferred_option)
                                    * option_count);
    core->deferred_count = 0;
}

void c_after_options(global_core *core)
{
    ic_register_include_chains(core->ic, core->reader);
    
    if (core->preprocess_flag)
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

    if (core->compile_flag)
    {
        FILE *output;
        if ((core->output_name == NULL)
            || (core->output_name[0] == '\0'))
        {
            output = stdout;
        }
        else output = fopen(core->output_name, "w");

        core->cc_reader = xmalloc(sizeof(*core->cc_reader));
        memset(core->cc_reader, 0, sizeof(*core->cc_reader));
        
        core->cc_reader->output = output;
        core->cc_reader->file = xstrdup(core->input_names[0]);
        core->cc_reader->input = fopen(core->input_names[0], "r");
        core->cc_reader->char_bits = 8;
        core->cc_reader->short_bits = 16;
        core->cc_reader->int_bits = 32;
        core->cc_reader->long_bits = 64;
        core->cc_reader->llong_bits = 64;
        core->cc_reader->ptr_bits = 32;
        if (core->cc_reader->input == NULL)
        {
            printf("+++Can't open %s\n", core->input_names[0]);
            return;
        }
    }
}

static void c_finish_options(global_core *core)
{
    location_t loc;
    unsigned int i;

    if (core->preprocess_flag)
    {
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
}

static void c_end(global_core *core)
{
    int i;
    if (core->reader) cpp_destroy_reader(core->reader);
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
    if (core->preprocess_flag)
    {
        free(core->processor);
    }
}

static void compile_file(cc_reader *reader)
{
    cc_parse_file(reader);
}

static global_core *core = NULL;
static void global_exit(void)
{
    if (core != NULL)
    {
        c_end(core);
        global_destroy_core(core);
    }
}

int main(int argc, char **argv)
{    
    int i;
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            printf("Usage: pdcc [options] file...\n");
            printf("Options:\n");
            printf("  -h, --help      Displays this information.\n");
            printf("  -E              Preprocess only; do not compile.\n");
            printf("  -N              Preprocess without generating #line.\n");
            printf("  -o <file>       Puts output into <file>.\n");
            printf("  -I <directory>  #include searches in <directory>.\n");
            printf("  -D <macro>      Defines <macro>.\n");
            return (0);
        }
    }

    atexit(&global_exit);
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
                    core->compile_flag = 1;
                    core->preprocess_flag = 0;
                    break;
                case 'E':
                    core->preprocess_flag = 1;
                    core->compile_flag = 0;
                    break;
                case 'N':
                    core->preprocess_flag = 1;
                    core->compile_flag = 0;
                    line_flag = 0;
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
        printf("Error: no input files.\nUse --help for help.\n");
        goto end;
    }
    
    c_after_options(core);

    /* Call for each file. */
    c_finish_options(core);
    if (core->preprocess_flag)
        preprocess_file(core->processor, core->reader);
    if (core->compile_flag)
        compile_file(core->cc_reader);
end:
    return (0);
}
