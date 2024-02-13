/******************************************************************************
 * @file            i386_gen.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "i386_opc.h"

void *xmalloc (size_t size)
{
    void *p = malloc (size);

    if (p == NULL) {
        fprintf (stderr, "Failed to allocate memory!\n");
        exit (EXIT_FAILURE);
    }
    
    return p;
}

void *xrealloc (void *p, size_t size)
{
    p = realloc (p, size);

    if (p == NULL) {
        fprintf (stderr, "Failed to allocate memory!\n");
        exit (EXIT_FAILURE);
    }

    return p;
}

#define READ_MEM_INCREMENT 60000 /* Arbitrary. */

int read_text_file_into_memory (const char *filename, unsigned char **memory_p, size_t *size_p)
{
    size_t mem_size = READ_MEM_INCREMENT;
    size_t read_bytes = 0;
    size_t change;
    unsigned char *memory;
    FILE *infile;

    if ((infile = fopen (filename, "r")) == NULL) return 1;

    memory = xmalloc (mem_size + 2);
    while ((change = fread (memory + read_bytes, 1, mem_size - read_bytes, infile)) > 0) {
        read_bytes += change;
        if (read_bytes == mem_size) {
            mem_size += READ_MEM_INCREMENT;
            memory = xrealloc (memory, mem_size + 2);
        }
    }

    memory[read_bytes] = '\0';
    memory[read_bytes + 1] = '\0';

    fclose (infile);
    *memory_p = memory;
    *size_p = read_bytes;

    return 0;
}

void skip_spaces (char **pp)
{
    while (isspace ((unsigned char)**pp)) ++*pp;
}

int xstrcasecmp (const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;
    
    p1 = (const unsigned char *) s1;
    p2 = (const unsigned char *) s2;
    
    while (*p1 != '\0') {
        if (toupper (*p1) < toupper (*p2)) return (-1);
        else if (toupper (*p1) > toupper (*p2)) return (1);
        
        p1++;
        p2++;
    }
    
    if (*p2 == '\0') return (0);

    return (-1);
}

static int isident (int c)
{
    return isalnum (c) || c == '_';
}

#define safe_fputs(_text, _file) do { if (fputs ((_text), (_file)) < 0) return 1; } while (0)

struct name_bitfield { 
    const char *name;
    unsigned int value;
};

#define GENERATOR_MACRO(a) { #a, 0 },
static struct name_bitfield cpu_flags_nb[] = {

    GENERATOR_CPU_FLAGS_LIST
    {0, 0}
};
#undef GENERATOR_MACRO

#define GENERATOR_MACRO(a) { #a, 0 },
static struct name_bitfield opcode_modifier_nb[] = {

    GENERATOR_OPCODE_MODIFIER_LIST
    {0, 0}
};
#undef GENERATOR_MACRO

#define GENERATOR_MACRO(a) { #a, 0 },
static struct name_bitfield operand_type_nb[] = {

    GENERATOR_OPERAND_TYPE_LIST
    {0, 0}
};
#undef GENERATOR_MACRO

static int process_bitfield_init (FILE *outfile, char *line, struct name_bitfield *start_nb)
{
    struct name_bitfield *nb;

    for (nb = start_nb; nb->name; nb++) nb->value = 0;

    while (*line) {
        char *p, saved_c;
        
        skip_spaces (&line);
        if (!*line) break;
        if (*line == '|') {
            line++;
            continue;
        }

        for (p = line; isident (*p); p++) {}
        saved_c = *p;
        *p = '\0';

        if (*line == '0' && !line[1]) goto skip;
            
        for (nb = start_nb; nb->name && xstrcasecmp (nb->name, line); nb++) {}

        if (nb->name) nb->value = 1;
        else {
            fprintf (stderr, "error: unrecognized bit-field name '%s'\n", line);
            return 1;
        }

    skip:
        *p = saved_c;
        line = p;
    }

    safe_fputs ("{ ", outfile);

    for (nb = start_nb; nb->name; nb++) {
        if (nb != start_nb) {
            if (fprintf (outfile, ", %u", nb->value) < 0) return 1;
        } else {
            if (fprintf (outfile, "%u", nb->value) < 0) return 1;
        }
    }

    safe_fputs (" }", outfile);

    return 0;
}

static int generate_cpu_flags (FILE *outfile, char *line)
{
    char *p = strchr (line, ',');
    
    if (!p) return 1;
    *p = '\0';

    return process_bitfield_init (outfile, line, cpu_flags_nb);
}

static int generate_opcode_modifier (FILE *outfile, char *line)
{
    char *p = strchr (line, ',');
    
    if (!p) return 1;
    *p = '\0';

    return process_bitfield_init (outfile, line, opcode_modifier_nb);
}

static int generate_operands (FILE *outfile, char *line)
{
    char *p;

    line = strchr (line, '{');
    if (!line) return 1;
    line++;
    skip_spaces (&line);

    if (*line == '}') {
        safe_fputs ("{ {0} }", outfile);
        return 0;
    }

    p = strchr (line, '}');
    if (!p) return 1;
    *p = '\0';

    safe_fputs ("{ ", outfile);
    
    p = line;
    while (1) {
        int end = 0;
        
        if (line != p) safe_fputs (", ", outfile);

        p = strchr (line, ',');
        if (!p) {
            end = 1;
            p = line + strlen (line);
        }

        *p = '\0';
        if (process_bitfield_init (outfile, line, operand_type_nb)) return 1;

        line = p + 1;
        if (end) break;
    }

    safe_fputs (" }", outfile);

    return 0;
}

static int generate_template (FILE *outfile, char *line)
{
    char *cpu;
    char *opcode_modifier;

    skip_spaces (&line);

    if (!*line) return 0;

    safe_fputs ("    { ", outfile);

    /* First three fields should be copied verbatim. */
    {
        char *p, saved_c;
        int i;

        p = line;
        for (i = 0; i < 3; i++) {
            p = strchr (p, ',');
            if (!p) return 1;
            p++;
        }

        saved_c = *p;
        *p = '\0';
        safe_fputs (line, outfile);
        *p = saved_c;

        line = p;
    }

    cpu = line;
    line = strchr (line, ',');
    if (!line) return 1;
    line++;

    opcode_modifier = line;
    line = strchr (line, ',');
    if (!line) return 1;
    line++;

    {
        unsigned int operand_count;
        char *p;

        p = strchr (line, '{');
        if (!p) return 1;

        p++;
        skip_spaces (&p);
        if (!*p) return 1;
        if (*p == '}') operand_count = 0;
        else {
            for (operand_count = 1; *p && *p != '}'; p++) {
                if (*p == ',') operand_count++;
            }
            if (!*p) return 1;
        }

        if (fprintf (outfile, " %u, ", operand_count) < 0) return 1;
    }

    if (generate_cpu_flags (outfile, cpu)) return 1;
    safe_fputs (", ", outfile);

    if (generate_opcode_modifier (outfile, opcode_modifier)) return 1;
    safe_fputs (", ", outfile);

    if (generate_operands (outfile, line)) return 1;
    safe_fputs (" },\n", outfile);

    return 0;
}

static int generate_templates (FILE *outfile, char **pos_p)
{
    char *pos = *pos_p;
    
    safe_fputs ("\nstatic const struct template template_table[] = {\n\n", outfile);

    while (*pos && *pos != '=') {
        char *p;
        
        p = strchr (pos, '\n');
        if (!p) {
            fprintf (stderr, "end of file not at end of line\n");
            break;
        }

        *p = '\0';
        if (p > pos && p[-1] == '\r') p[-1] = '\0';

        if (*pos != '#' && generate_template (outfile, pos)) return 1;

        pos = p + 1;
    }

    safe_fputs ("    { 0 }\n\n};\n", outfile);

    *pos_p = pos;
    return 0;
}

static int generate_reg (FILE *outfile, char *line)
{
    skip_spaces (&line);

    if (!*line) return 0;

    safe_fputs ("    { ", outfile);

    {
        char *p = strchr (line, ',');
    
        if (!p) return 1;
        *p = '\0';

        safe_fputs (line, outfile);
        safe_fputs (", ", outfile);

        line = p + 1;
    }

    {
        char *p = strchr (line, ',');
    
        if (!p) return 1;
        *p = '\0';
        
        if (process_bitfield_init (outfile, line, operand_type_nb)) return 1;
        safe_fputs (", ", outfile);

        line = p + 1;
    }

    skip_spaces (&line);
    safe_fputs (line, outfile);
    
    safe_fputs (" },\n", outfile);

    return 0;
}

static int generate_regs (FILE *outfile, char **pos_p)
{
    char *pos = *pos_p;

    safe_fputs ("\nstatic const struct reg_entry reg_table[] = {\n\n", outfile);

    while (*pos && *pos != '=') {
        char *p;
        
        p = strchr (pos, '\n');
        if (!p) {
            fprintf (stderr, "end of file not at end of line\n");
            break;
        }

        *p = '\0';
        if (p > pos && p[-1] == '\r') p[-1] = '\0';

        if (*pos != '#' && generate_reg (outfile, pos)) return 1;

        pos = p + 1;
    }

    safe_fputs ("    { 0 }\n\n};\n", outfile);

    *pos_p = pos;
    return 0;
}

static int generate_cpu_flags_initializer (FILE *outfile, char *line)
{
    skip_spaces (&line);

    if (!*line) return 0;

    safe_fputs ("#define ", outfile);

    {
        char *p = strchr (line, ',');
    
        if (!p) return 1;
        *p = '\0';

        safe_fputs (line, outfile);
        safe_fputs (" ", outfile);

        line = p + 1;
    }

    if (process_bitfield_init (outfile, line, cpu_flags_nb)) return 1;
    safe_fputs ("\n", outfile);

    return 0;
}

static int generate_operand_type_initializer (FILE *outfile, char *line)
{
    skip_spaces (&line);

    if (!*line) return 0;

    safe_fputs ("#define ", outfile);

    {
        char *p = strchr (line, ',');
    
        if (!p) return 1;
        *p = '\0';

        safe_fputs (line, outfile);
        safe_fputs (" ", outfile);

        line = p + 1;
    }

    if (process_bitfield_init (outfile, line, operand_type_nb)) return 1;
    safe_fputs ("\n", outfile);

    return 0;
}

static int generate_initializers (FILE *outfile,
                                  char **pos_p,
                                  int (*generate_func) (FILE *outfile, char *line))
{
    char *pos = *pos_p;

    safe_fputs ("\n", outfile);

    while (*pos && *pos != '=') {
        char *p;
        
        p = strchr (pos, '\n');
        if (!p) {
            fprintf (stderr, "end of file not at end of line\n");
            break;
        }

        *p = '\0';
        if (p > pos && p[-1] == '\r') p[-1] = '\0';

        if (*pos != '#' && (*generate_func) (outfile, pos)) return 1;

        pos = p + 1;
    }

    safe_fputs ("\n", outfile);

    *pos_p = pos;
    return 0;
}

static int generate_start (FILE *outfile)
{
    safe_fputs ("/* Automatically generated file, do not edit! */\n", outfile);

    return 0;
}

static void skip_equal_separator (char **pos_p)
{
    char *pos = *pos_p;

    if (*pos == '=') {
        char *p = strchr (pos, '\n');

        if (p) pos = p + 1;
        else pos += strlen (pos);
    }

    *pos_p = pos;
}

static int generate (FILE *outfile, const char *in_template_filename)
{
    unsigned char *real_file;
    size_t file_size;
    char *pos;
    
    if (read_text_file_into_memory (in_template_filename, &real_file, &file_size)) return 1;
    pos = (char *)real_file;

    if (generate_start (outfile)) {
        fprintf (stderr, "failed to generate beginning\n");
        return 1;
    }
    
    if (generate_templates (outfile, &pos)) {
        fprintf (stderr, "failed to generate templates\n");
        return 1;
    }

    skip_equal_separator (&pos);

    if (generate_regs (outfile, &pos)) {
        fprintf (stderr, "failed to generate registers\n");
        return 1;
    }

    skip_equal_separator (&pos);

    if (generate_initializers (outfile, &pos, &generate_cpu_flags_initializer)) {
        fprintf (stderr, "failed to generate initializers\n");
        return 1;
    }

    skip_equal_separator (&pos);

    if (generate_initializers (outfile, &pos, &generate_operand_type_initializer)) {
        fprintf (stderr, "failed to generate initializers\n");
        return 1;
    }

    free (real_file);
    return 0;
}

int main (int argc, char **argv)
{
    FILE *outfile;
    const char *in_template_filename, *out_filename;

    if (argc != 3) {
        printf ("usage: %s input_file output_file\n", argv[0]);
        return EXIT_SUCCESS;
    }

    in_template_filename = argv[1];
    out_filename = argv[2];

    if (strcmp (out_filename, "-")) {
        if (!(outfile = fopen (out_filename, "w"))) {
            fprintf (stderr, "failed to open '%s'\n", out_filename);
            return EXIT_FAILURE;
        }
    } else outfile = stdout;

    if (generate (outfile, in_template_filename)) {
        fprintf (stderr, "failed to generate output\n");
        goto failure;
    }

    if (outfile != stdout) fclose (outfile);

    return EXIT_SUCCESS;

failure:
    if (outfile != stdout) fclose (outfile);
    return EXIT_FAILURE;
}
