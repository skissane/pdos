/******************************************************************************
 * @file            load_line.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stddef.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"

#define     CAPACITY_INCREMENT          256

struct load_line_data {

    char *line, *real_line;
    
    size_t capacity, read_size;
    size_t end_of_prev_real_line;
    
    /**
     * Optional pointer for improving the warning message issued
     * when end of file is not at end of line.
     */
    unsigned long *new_line_number_p;

};

int load_line (char **line_p, char **line_end_p, char **real_line_p, size_t *real_line_len_p,
               unsigned long *newlines_p, FILE *input_file,
               void **load_line_internal_data_p) {

    struct load_line_data *l_l_data;
    size_t pos_in_line = 0, pos_in_real_line = 0;
    
    unsigned long newlines = 0;
    
    int in_block_comment = 0, in_line_comment = 0, skipping_spaces = 0, in_escape = 0, in_quote = 0;
    int possible_start_or_end_of_comment = 0;
    
    l_l_data = *load_line_internal_data_p;
    
    if (l_l_data->end_of_prev_real_line) {
    
        memmove (l_l_data->real_line, l_l_data->real_line + l_l_data->end_of_prev_real_line,
            l_l_data->read_size - l_l_data->end_of_prev_real_line);
        
        l_l_data->read_size -= l_l_data->end_of_prev_real_line;;
    
    }
    
    while (1) {
    
        if (pos_in_line >= l_l_data->capacity || pos_in_real_line >= l_l_data->capacity) {
        
            l_l_data->capacity += CAPACITY_INCREMENT;
            l_l_data->line = xrealloc (l_l_data->line, l_l_data->capacity + 2);
            l_l_data->real_line = xrealloc (l_l_data->real_line, l_l_data->capacity + 1);
        
        }
        
        if (pos_in_real_line >= l_l_data->read_size) {
        
            l_l_data->read_size = fread (l_l_data->real_line + pos_in_real_line,
                1, l_l_data->capacity - pos_in_real_line, input_file) + pos_in_real_line;
            
            if (ferror (input_file)) {
            
                /* Error while reading. */
                return 2;
            
            }
            
            l_l_data->real_line[l_l_data->read_size] = '\0';
        
        }
    
    copying:
    
        if (in_block_comment) {
        
            while (pos_in_real_line < l_l_data->read_size) {
            
                if (possible_start_or_end_of_comment && l_l_data->real_line[pos_in_real_line] == '/') {
                
                    possible_start_or_end_of_comment = 0;
                    pos_in_real_line++;
                    
                    in_block_comment = 0;
                    break;
                
                }
                
                possible_start_or_end_of_comment = 0;
                
                if (l_l_data->real_line[pos_in_real_line] == '*') {
                    possible_start_or_end_of_comment = 1;
                }
                
                if (l_l_data->real_line[pos_in_real_line] == '\n') {
                    newlines++;
                }
                
                pos_in_real_line++;
            
            }
        
        }
        
        if (in_line_comment) {
        
            while (pos_in_real_line < l_l_data->read_size) {
            
                if (l_l_data->real_line[pos_in_real_line] == '\n') {
                    
                    in_line_comment = 0;
                    break;
                
                }
                
                pos_in_real_line++;
            
            }
        
        }
        
        if (skipping_spaces) {
        
            while (pos_in_real_line < l_l_data->read_size) {
            
                if (l_l_data->real_line[pos_in_real_line] != ' ' && l_l_data->real_line[pos_in_real_line] != '\t') {
                    
                    skipping_spaces = 0;
                    break;
                
                }
                
                pos_in_real_line++;
            
            }
        
        }
        
        while (pos_in_real_line < l_l_data->read_size && pos_in_line < l_l_data->capacity) {
        
            l_l_data->line[pos_in_line] = l_l_data->real_line[pos_in_real_line++];
            
            if (in_quote) {
            
                if (in_escape) {
                    in_escape = 0;
                } else if (l_l_data->line[pos_in_line] == '\"') {
                    in_quote = 0;
                } else if (l_l_data->line[pos_in_line] == '\\') {
                    in_escape = 1;
                }
                
                if (l_l_data->line[pos_in_line] == '\n') {
                    newlines++;
                }
            
            } else {
            
                if (possible_start_or_end_of_comment && l_l_data->line[pos_in_line] == '*') {
                
                    possible_start_or_end_of_comment = 0;
                    l_l_data->line[pos_in_line - 1] = ' ';
                    
                    in_block_comment = 1;
                    goto copying;
                
                }
                
                possible_start_or_end_of_comment = 0;
                
                if (l_l_data->line[pos_in_line] == ' ' || l_l_data->line[pos_in_line] == '\t') {
                
                    l_l_data->line[pos_in_line++] = ' ';
                    
                    skipping_spaces = 1;
                    goto copying;
                
                } else if (l_l_data->line[pos_in_line] == '\n') {
                
                    /* Checks for a carriage return before the line feed and if present, removes it. */
                    if (pos_in_line > 0 && l_l_data->line[pos_in_line - 1] == '\r') {
                        l_l_data->line[--pos_in_line] = '\n';
                    }
                    
                    /* Done reading a line. */
                    l_l_data->line[pos_in_line + 1] = '\0';
                    l_l_data->end_of_prev_real_line = pos_in_real_line;
                    
                    *line_p = l_l_data->line;
                    *line_end_p = l_l_data->line + pos_in_line;
                    *real_line_p = l_l_data->real_line;
                    *real_line_len_p = pos_in_real_line;
                    *newlines_p = newlines;
                    
                    return 0;
                
                } else if (l_l_data->line[pos_in_line] == '#') {

                    in_line_comment = 1;
                    goto copying;

                } else if (l_l_data->line[pos_in_line] == '\"') {
                    in_quote = 1;
                } else if (l_l_data->line[pos_in_line] == '/') {
                    possible_start_or_end_of_comment = 1;
                }
            
            }
            
            pos_in_line++;
        
        }
        
        if (feof (input_file)) {
        
            const char *filename;
            unsigned long line_number;
            
            if (l_l_data->read_size == 0) {
                return 1;
            }
            
            l_l_data->line[pos_in_line] = '\n';
            l_l_data->line[pos_in_line + 1] = '\0';
            
            get_filename_and_line_number (&filename, &line_number);
            
            /**
             * The line number obtained might not be correct as it has yet to be updated, so it not used.
             * If new_line_number_p is provided, the correct line number is obtained using it.
             */
            if (l_l_data->new_line_number_p) {
                line_number = *(l_l_data->new_line_number_p);
            } else {
                line_number = 0;
            }
            
            as_warn_at (filename, line_number, "end of file not at end of line; newline inserted");
            
            l_l_data->end_of_prev_real_line = l_l_data->read_size = 0;
            
            *line_p = l_l_data->line;
            *line_end_p = l_l_data->line + pos_in_line;
            *real_line_p = l_l_data->real_line;
            *real_line_len_p = pos_in_real_line;
            *newlines_p = newlines;
            
            return 0;
        
        }
    
    }

}

void *load_line_create_internal_data (unsigned long *new_line_number_p) {

    struct load_line_data *l_l_data;
    
    l_l_data = xmalloc (sizeof (*l_l_data));
    
    l_l_data->capacity = 0;
    l_l_data->line = NULL;
    l_l_data->real_line = NULL;
    
    l_l_data->read_size = 0;
    l_l_data->end_of_prev_real_line = 0;
    
    l_l_data->new_line_number_p = new_line_number_p;
    return l_l_data;

}

void load_line_destroy_internal_data (void *load_line_internal_data)
{
    if (load_line_internal_data) {
        struct load_line_data *l_l_data;
    
        l_l_data = load_line_internal_data;
        
        free (l_l_data->line);
        free (l_l_data->real_line);
        free (l_l_data);
    }
}
