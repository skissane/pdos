/******************************************************************************
 * @file            sections.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "ld.h"
#include "xmalloc.h"

struct section *all_sections = NULL;
static struct section **last_section_p = &all_sections;

struct object_file *all_object_files = NULL;
static struct object_file **last_object_file_p = &all_object_files;

struct section *section_find (const char *name)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        if (strcmp (name, section->name) == 0) break;
    }

    return section;
}

struct section *section_find_or_make (const char *name)
{
    struct section *section = section_find (name);

    if (section) return section;

    section = xmalloc (sizeof (*section));
    section->name = xstrdup (name);
    section->flags = 0;
    section->first_part = NULL;
    section->last_part_p = &section->first_part;

    section->all_subsections = NULL;

    section->total_size = 0;
    section->is_bss = 0;
    section->rva = 0;
    section->section_alignment = 1;

    section->object_dependent_data = NULL;

    section->next = NULL;
    *last_section_p = section;
    last_section_p = &section->next;

    return section;
}

void section_write (struct section *section, unsigned char *memory)
{
    struct section_part *part;

    for (part = section->first_part; part; part = part->next) {
        memcpy (memory, part->content, part->content_size);
        memory += part->content_size;
    }
}

int section_count (void)
{
    struct section *section;
    int i = 0;

    for (section = all_sections; section; i++, section = section->next) {}

    return i;
}

struct subsection *subsection_find (struct section *section, const char *name)
{
    struct subsection *subsection;

    for (subsection = section->all_subsections; subsection; subsection = subsection->next) {
        if (strcmp (name, subsection->name) == 0) break;
    }

    return subsection;
}

struct subsection *subsection_find_or_make (struct section *section, const char *name)
{
    struct subsection *subsection, **next_p;

    for (subsection = *(next_p = &section->all_subsections);
         subsection;
         subsection = *(next_p = &subsection->next)) {

        if (strcmp (name, subsection->name) <= 0) break;

    }

    if (!subsection || strcmp (name, subsection->name) != 0) {

        subsection = xmalloc (sizeof (*subsection));
        subsection->name = xstrdup (name);
        subsection->first_part = NULL;
        subsection->last_part_p = &subsection->first_part;

        subsection->next = *next_p;
        *next_p = subsection;
    }

    return subsection;
}

struct section_part *section_part_new (struct section *section, struct object_file *of)
{
    struct section_part *part = xmalloc (sizeof (*part));

    part->section = section;
    part->of = of;

    part->content = NULL;
    part->content_size = 0;

    part->relocation_array = NULL;
    part->relocation_count = 0;
    part->rva = 0;

    part->next = NULL;

    return part;
}

void section_append_section_part (struct section *section, struct section_part *part)
{
    *section->last_part_p = part;
    section->last_part_p = &part->next;
}

void subsection_append_section_part (struct subsection *subsection, struct section_part *part)
{
    *subsection->last_part_p = part;
    subsection->last_part_p = &part->next;
}

struct object_file *object_file_make (size_t symbol_count, const char *filename)
{
    struct object_file *of = xmalloc (sizeof (*of));

    of->filename = xstrdup (filename);

    of->symbol_array = symbol_count ? xmalloc (sizeof (*of->symbol_array) * (symbol_count)) : NULL;
    of->symbol_count = symbol_count;

    of->next = NULL;
    *last_object_file_p = of;
    last_object_file_p = &of->next;

    return of;
}

void sections_destroy (void)
{
    {
        struct section *section, *next_section;

        for (section = all_sections; section; section = next_section) {  
            struct subsection *subsection;
            struct section_part *part, *next_part;

            for (subsection = section->all_subsections; subsection; subsection = section->all_subsections) {
                section->all_subsections = subsection->next;
                free (subsection->name);
                free (subsection);
            }

            next_section = section->next;

            for (part = section->first_part; part; part = next_part) {
                next_part = part->next;
                free (part->content);
                free (part->relocation_array);
                free (part);
            }

            free (section->name);
            free (section);
        }

    }

    {
        struct object_file *of, *next_of;

        for (of = all_object_files; of; of = next_of) {
            size_t i;

            next_of = of->next;

            for (i = 0; i < of->symbol_count; i++) {
                free (of->symbol_array[i].name);
            }
            
            free (of->filename);
            free (of->symbol_array);
            free (of);
        }

    }
}
