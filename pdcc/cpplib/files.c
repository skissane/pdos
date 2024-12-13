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
#include "internal.h"
#include "filename.h"
#include "support.h"
#include "xmalloc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IS_PATH_SEPARATOR(a) ((a == '\\') || (a == '/'))

struct _cpp_file {
    char *name;
    FILE *f;
    cpp_dir *dir;
    char *path;

    char *dir_name;

    int not_found;
};

/* Chain for <> is part of the "", otherwise "" chain is set. */
void cpp_set_include_chains(cpp_reader *reader,
                            cpp_dir *quote,
                            cpp_dir *angled,
                            int quote_ignores_source_dir)
{
    reader->quote_include = quote;
    reader->angled_include = quote;
    reader->quote_ignores_source_dir = quote_ignores_source_dir;

    for (; quote; quote = quote->next)
    {
        if (quote == angled)
        {
            reader->angled_include = angled;
        }
    }
}

static char *dir_name_of_file(_cpp_file *file)
{
    if (file->dir_name == NULL)
    {
        size_t len = filename(file->path) - (file->path);
        char *name;

        if (len || IS_PATH_SEPARATOR (file->path[0])) {
            name = xstrndup (file->path, len + 1);
        } else name = xstrdup ("");
        file->dir_name = name;
    }

    return (file->dir_name);
}

static cpp_dir *make_cpp_dir(cpp_reader *reader,
                             const char *name)
{
    cpp_dir *dir = xmalloc(sizeof(*dir));

    dir->next = reader->quote_include;
    {
        size_t len = strlen(name);
        dir->name = xmalloc(len + 1);
        memcpy(dir->name, name, len + 1);
    }

    return (dir);
}

static cpp_dir *search_path_head(cpp_reader *reader,
                                 const char *name,
                                 int angled,
                                 enum include_type typ)
{
    cpp_dir *dir;
    _cpp_file *file;

    if (IS_ABSOLUTE_PATH(name))
        return (&(reader->no_search_path));

    file = reader->mffc == NULL ? reader->main_file : reader->mffc->file;
    if (angled) dir = reader->angled_include;
    else if (typ == INCLUDE_TYPE_COMMAND_LINE)
        return(make_cpp_dir(reader, "./"));
    else if (reader->quote_ignores_source_dir)
        dir = reader->quote_include;
    else
        return(make_cpp_dir(reader, dir_name_of_file(file)));

    if (dir == NULL)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "No include path in which to search for %s", name);
    }

    return (dir);
}

int _cpp_add_include(cpp_reader *reader,
                     const char *name,
                     int angled_name,
                     enum include_type type,
                     location_t loc)
{
    cpp_dir *dir;
    _cpp_file *file;

    dir = search_path_head(reader, name, angled_name, type);
    if (dir == NULL) return (1);

    file = _cpp_find_file(reader, name, dir, 0,
                          angled_name, (type == INCLUDE_TYPE_DEFAULT), loc);
    if ((type == INCLUDE_TYPE_DEFAULT) && (file == NULL)) return (1);
    return (_cpp_add_file(reader, file, 0, loc));
}

static void open_file_failed(cpp_reader *reader,
                             _cpp_file *file,
                             location_t loc)
{
    cpp_error_with_line(reader, CPP_DL_CRITICAL,
                        loc, 0,
                        "%s: No such file or directory",
                        file->path ? file->path : file->name);
}

static int read_file(cpp_reader *reader, _cpp_file *file, location_t loc)
{
    FILE *f;
    char *memory;
    size_t size = 8 * 1024;
    size_t read_bytes = 0;
    size_t change;

    if (file->f == NULL)
    {
        open_file_failed(reader, file, loc);
        return (1);
    }
    f = file->f;

    memory = malloc(size + 16);
    if (memory == NULL)
    {
        fclose(f);
        return (1);
    }

    while ((change = fread(memory + read_bytes, 1, size - read_bytes, f)) > 0)
    {
        read_bytes += change;

        if (read_bytes == size)
        {
            void *old = memory;
            size *= 2;

            if ((memory = realloc(memory, size + 16)) == NULL)
            {
                free(old);
                fclose(f);
                return (1);
            }
        }
    }

    fclose(f);
    file->f = NULL;

    memory[read_bytes] = '\n';

    _cpp_add_mffc(reader, memory, read_bytes, 0);
    reader->mffc->file = file;
    /* Position tracking. */
    reader->mffc->filename = file->name;
    reader->mffc->line = 1;

    _cpp_change_file(reader, CPP_REASON_ENTER,
                     reader->mffc->filename,
                     reader->mffc->line);

    return (0);
}

int _cpp_add_file(cpp_reader *reader,
                  _cpp_file *file,
                  int ignored,
                  location_t loc)
{
    return (read_file(reader, file, loc));
}

static _cpp_file *create__cpp_file(cpp_reader *reader,
                                   cpp_dir *dir,
                                   const char *name)
{
    _cpp_file *file = xmalloc(sizeof(*file));
    file->f = NULL;
    file->dir = dir;
    file->name = xstrdup(name);
    file->dir_name = NULL;
    file->not_found = 1;
    return (file);
}

void _cpp_free_file(_cpp_file *file)
{
    free (file->name);
    free (file->path);
    free (file->dir_name);
    free (file);
}

static char *add_file_to_dir(_cpp_file *file,
                             cpp_dir *dir)
{
    size_t file_len, dir_len;
    char *path;

    file_len = strlen(file->name);
    dir_len = strlen(dir->name);

    path = xmalloc(dir_len + 1 + file_len + 1);

    memcpy(path, dir->name, dir_len);

    if (dir_len && !IS_PATH_SEPARATOR(path[dir_len - 1]))
        path[dir_len++] = '/';
    memcpy(&(path[dir_len]), file->name, file_len + 1);

    return (path);
}

static int find_file_in_dir(cpp_reader *reader,
                            _cpp_file *file,
                            void *ignored,
                            location_t loc)
{
    char *path;

    path = add_file_to_dir(file, file->dir);

    if (path)
    {
        file->path = path;
        if ((file->f = fopen((char *)(file->path), "r")))
        {
            file->not_found = 0;
            return (1);
        }

        file->path = file->name;
        free(path);
    }
    else
    {
        file->path = NULL;
    }
    
    return (0);
}

_cpp_file *_cpp_find_file(cpp_reader *reader, const char *name,
                          cpp_dir *start_dir, int ignored, int angled,
                          int ignored2, location_t loc)
{
    _cpp_file *file = create__cpp_file(reader, start_dir, name);
    for (;;)
    {
        if (find_file_in_dir(reader, file, NULL, loc)) break;

        file->dir = file->dir->next;
        if (file->dir == NULL)
            return (file);
    }
    
    return (file);
}

int _cpp_file_not_found(_cpp_file *file)
{
    return (file->not_found);
}
