/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../as.h"

#include "a_out.h"
 
void object_dependent_write_object_file(void)
{
    FILE *outfile;
    struct exec header;
    long string_table_pos = 4; /* Includes the length field. */

    memset(&header, '\0', sizeof(header));
    header.a_info = 0x00640000 | OMAGIC;

    if ((outfile = fopen(output_file_name, "wb")) == NULL)
    {
        as_error("Failed to open `%s'!\n", output_file_name);
        return;
    }

    if (fseek(outfile, sizeof(header), SEEK_SET))
    {
        as_error("Failed to fseek!\n");
        return;
    }

    /* Outputs text. */
    {
        fragS *frag;

        section_set(text_section);

        header.a_text = 0;
        for (frag = current_frag_chain->first_frag;
             frag;
             frag = frag->next)
        {
            if (frag->fixed_size == 0) continue;
            
            if (fwrite(frag->buf, frag->fixed_size, 1, outfile) != 1)
            {
                as_error("Failed to write text!\n");
                return;
            }

            header.a_text += frag->fixed_size;
        }
    }
    
    /* Outputs data. */
    {
        fragS *frag;

        section_set(data_section);

        header.a_data = 0;
        for (frag = current_frag_chain->first_frag;
             frag;
             frag = frag->next)
        {
            if (frag->fixed_size == 0) continue;
            
            if (fwrite(frag->buf, frag->fixed_size, 1, outfile) != 1)
            {
                as_error("Failed to write data!\n");
                return;
            }

            header.a_data += frag->fixed_size;
        }
    }

    /* Outputs text relocations. */
    {
        fixupS *fixup;

        header.a_trsize = 0;
        
        section_set(text_section);
        for (fixup = current_frag_chain->first_fixup;
             fixup;
             fixup = fixup->next)
        {
            struct relocation_info reloc;

            if (fixup->done) continue;

            reloc.r_address = fixup->frag->address + fixup->where;

            if (symbol_is_section_symbol(fixup->add_symbol))
            {
                if (symbol_get_section(fixup->add_symbol) == text_section)
                {
                    reloc.r_symbolnum = N_TEXT;
                }
                else if (symbol_get_section(fixup->add_symbol) == data_section)
                {
                    reloc.r_symbolnum = N_DATA;
                }
                else
                {
                    as_error("+++object_dependent_write_object_file reloc!\n");
                }
            }
            else
            {
                symbolS *symbol;
                int symbol_number;

                for (symbol = symbols, symbol_number = 0;
                     symbol && (symbol != fixup->add_symbol);
                     symbol = symbol->next)
                {
                    /* Only external and undefined symbols are counted. */
                    if ((symbol->flags & SYMBOL_FLAG_EXTERNAL)
                        || (symbol->section == undefined_section))
                    {
                        symbol_number++;
                    }
                }
                
                reloc.r_symbolnum = symbol_number;
                
                /* r_extern. */
                reloc.r_symbolnum |= 1 << 27;
            }

            /* r_pcrel. */
            reloc.r_symbolnum |= (!!(fixup->pcrel)) << 24;
            /* r_length. */
            {
                int log2_of_size, size;

                for (log2_of_size = -1, size = fixup->size;
                     size; size >>= 1, log2_of_size++) ;

                reloc.r_symbolnum |= log2_of_size << 25;
            }

            if (fwrite(&reloc, sizeof(reloc), 1, outfile)
                    != 1)
            {
                as_error("Error writing text relocations!\n");
                return;
            }

            header.a_trsize += sizeof(reloc);
        }
    }

    /* +++Outputs data relocations. */
    {
        fixupS *fixup;
        unsigned long start_address_of_data;

        header.a_drsize = 0;

        section_set(data_section);
        start_address_of_data = current_frag_chain->first_frag->address;
        for (fixup = current_frag_chain->first_fixup;
             fixup;
             fixup = fixup->next)
        {
            struct relocation_info reloc;

            if (fixup->done) continue;

            /* r_address is relative to the start of data section. */
            reloc.r_address = (fixup->frag->address + fixup->where
                               - start_address_of_data);
            
            if (symbol_is_section_symbol(fixup->add_symbol))
            {
                if (symbol_get_section(fixup->add_symbol) == text_section)
                {
                    reloc.r_symbolnum = N_TEXT;
                }
                else if (symbol_get_section(fixup->add_symbol) == data_section)
                {
                    reloc.r_symbolnum = N_DATA;
                }
                else
                {
                    as_error("+++object_dependent_write_object_file reloc!\n");
                }
            }
            else
            {
                symbolS *symbol;
                int symbol_number;

                for (symbol = symbols, symbol_number = 0;
                     symbol && (symbol != fixup->add_symbol);
                     symbol = symbol->next)
                {
                    /* Only external and undefined symbols are counted. */
                    if ((symbol->flags & SYMBOL_FLAG_EXTERNAL)
                        || (symbol->section == undefined_section))
                    {
                        symbol_number++;
                    }
                }
                
                reloc.r_symbolnum = symbol_number;
                
                /* r_extern. */
                reloc.r_symbolnum |= 1 << 27;
            }

            /* r_pcrel. */
            reloc.r_symbolnum |= (!!(fixup->pcrel)) << 24;
            /* r_length. */
            {
                int log2_of_size, size;

                for (log2_of_size = -1, size = fixup->size;
                     size; size >>= 1, log2_of_size++) ;

                reloc.r_symbolnum |= log2_of_size << 25;
            }

            if (fwrite(&reloc, sizeof(reloc), 1, outfile)
                    != 1)
            {
                as_error("Error writing data relocations!\n");
                return;
            }

            header.a_drsize += sizeof(reloc);
        }
    }

    /* Outputs symbols. */
    {
        symbolS *symbol;
        unsigned long symbol_table_size = 0;

        for (symbol = symbols;
             symbol;
             symbol = symbol->next)
        {
            /* Guess only external and undefined symbols need to be written. */
            if ((symbol->flags & SYMBOL_FLAG_EXTERNAL)
                || (symbol->section == undefined_section))
            {
                struct nlist symbol_entry;

                memset(&symbol_entry, '\0', sizeof(symbol_entry));

                symbol_entry.n_un.n_strx = string_table_pos;
                string_table_pos += strlen(symbol->name) + 1;

                if (symbol->section == undefined_section)
                {
                    symbol_entry.n_type = N_UNDF;
                }
                else if (symbol->section == text_section)
                {
                    symbol_entry.n_type = N_TEXT;
                }
                else if (symbol->section == data_section)
                {
                    symbol_entry.n_type = N_DATA;
                }
                else
                {
                    as_error("+++other n_type\n");
                }

                /* All our symbols are external. */
                symbol_entry.n_type |= N_EXT;

                symbol_entry.n_value = symbol->value;

                if (fwrite(&symbol_entry, sizeof(symbol_entry), 1, outfile)
                    != 1)
                {
                    as_error("Error writing symbol table!\n");
                    return;
                }
                
                symbol_table_size += sizeof(symbol_entry);
            }
        }

        header.a_syms = symbol_table_size;
    }

    /* Writes the string table. */
    {
        symbolS *symbol;

        if (fwrite(&string_table_pos, sizeof(string_table_pos), 1, outfile)
            != 1)
        {
            as_error("Failed to write string table!\n");
            return;
        }

        for (symbol = symbols;
             symbol;
             symbol = symbol->next)
        {
            /* We are writing only names of external and undefined symbols. */
            if ((symbol->flags & SYMBOL_FLAG_EXTERNAL)
                || (symbol->section == undefined_section))
            {
                if (fwrite(symbol->name, strlen(symbol->name) + 1, 1, outfile)
                    != 1)
                {
                    as_error("Failed to write string table!\n");
                    return;
                }
            }
        }
    }

    /* Writes the header.*/
    rewind(outfile);
    if (fwrite(&header, sizeof(header), 1, outfile) != 1)
    {
        as_error("Failed to write header!\n");
        return;
    }

    if (fclose(outfile)) as_error("Failed to close file!\n");
}
