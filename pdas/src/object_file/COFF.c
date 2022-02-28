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

#include "coff.h"

int output_relocation(FILE *outfile, fixupS *fixup)
{
    struct Relocation_entry reloc_entry;

    reloc_entry.VirtualAddress = fixup->frag->address + fixup->where;

    if (fixup->add_symbol == NULL)
    {
        as_error("+++output relocation\n");
        return (1);
    }
    else
    {
        (reloc_entry.SymbolTableIndex
         = symbol_get_symbol_table_index(fixup->add_symbol));
    }

    if (fixup->pcrel)
    {
        reloc_entry.Type = IMAGE_REL_I386_REL32;
    }
    else reloc_entry.Type = IMAGE_REL_I386_DIR32;

    if (fwrite(&reloc_entry, Relocation_entry_size, 1, outfile) != 1)
    {
        return (1);
    }

    return (0);
}

#define NUMBER_OF_SECTIONS 3 /* .text, .data, .bss. */
 
void object_dependent_write_object_file(void)
{
    FILE *outfile;
    struct COFF_file_header file_header;
    struct Section_Table_entry section_headers[NUMBER_OF_SECTIONS];
    struct Section_Table_entry *text_section_header;
    struct Section_Table_entry *data_section_header;
    struct Section_Table_entry *bss_section_header;
    unsigned long string_table_size = 4; /* The size includes the size field.*/

    if ((outfile = fopen(output_file_name, "wb")) == NULL)
    {
        as_error("Failed to open `%s'!\n", output_file_name);
        return;
    }

    memset(&file_header, '\0', sizeof(file_header));
    
    file_header.Machine = IMAGE_FILE_MACHINE_I386;
    file_header.NumberOfSections = NUMBER_OF_SECTIONS;
    file_header.SizeOfOptionalHeader = 0;
    file_header.Characteristics = IMAGE_FILE_LINE_NUMS_STRIPPED;

    text_section_header = &section_headers[0];
    memset(text_section_header, '\0', sizeof(*text_section_header));
    strcpy(text_section_header->Name, ".text");
    text_section_header->Characteristics = (IMAGE_SCN_CNT_CODE
                                            | IMAGE_SCN_MEM_EXECUTE
                                            | IMAGE_SCN_MEM_READ);
    
    data_section_header = &section_headers[1];
    memset(data_section_header, '\0', sizeof(*data_section_header));
    strcpy(data_section_header->Name, ".data");
    data_section_header->Characteristics = (IMAGE_SCN_CNT_INITIALIZED_DATA
                                            | IMAGE_SCN_MEM_WRITE
                                            | IMAGE_SCN_MEM_READ);

    bss_section_header = &section_headers[2];
    memset(bss_section_header, '\0', sizeof(*bss_section_header));
    strcpy(bss_section_header->Name, ".bss");
    bss_section_header->Characteristics = (IMAGE_SCN_CNT_UNINITIALIZED_DATA
                                           | IMAGE_SCN_MEM_WRITE
                                           | IMAGE_SCN_MEM_READ);

    /* Seeks at the end of file header and section table
     * as these are the only parts with fixed position. */
    if (fseek(outfile,
              (sizeof(file_header)
               + NUMBER_OF_SECTIONS * sizeof(struct Section_Table_entry)),
              SEEK_SET))
    {
        as_error("Failed to fseek!\n");
        return;
    }

    /* Outputs text. */
    {
        fragS *frag;

        section_set(text_section);

        text_section_header->PointerToRawData = ftell(outfile);
        text_section_header->SizeOfRawData = 0;
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

            text_section_header->SizeOfRawData += frag->fixed_size;
        }
    }

    /* Outputs data. */
    {
        fragS *frag;

        section_set(data_section);

        data_section_header->PointerToRawData = ftell(outfile);
        data_section_header->SizeOfRawData = 0;
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

            data_section_header->SizeOfRawData += frag->fixed_size;
        }
    }

    /* Calculates bss. */
    {
        fragS *frag;

        section_set(bss_section);
        
        bss_section_header->VirtualSize = 0;
        for (frag = current_frag_chain->first_frag;
             frag;
             frag = frag->next)
        {
            if (frag->fixed_size == 0) continue;

            bss_section_header->VirtualSize += frag->fixed_size;
        }
    }

    /* Outputs symbol table. */
    {
        symbolS *symbol;

        file_header.PointerToSymbolTable = ftell(outfile);
        file_header.NumberOfSymbols = 0;

        for (symbol = symbols;
             symbol;
             symbol = symbol->next)
        {
            struct Symbol_Table_entry symbol_table_entry;

            symbol->write_name_to_string_table = 0;
            
            if ((symbol_is_external(symbol) == 0)
                && (symbol_is_undefined(symbol) == 0)
                && (symbol_is_section_symbol(symbol) == 0)) continue;

            memset(&symbol_table_entry, '\0', sizeof(symbol_table_entry));

            if (strlen(symbol->name) <= 8)
            {
                memcpy(symbol_table_entry.Name,
                       symbol->name, strlen(symbol->name));
            }
            else
            {
                /* The name is too long, so the String Table is used. */
                memset(symbol_table_entry.Name, 0, 4);
                symbol_table_entry.Name[4] = string_table_size & 0xff;
                symbol_table_entry.Name[5] = (string_table_size >> 8) & 0xff;
                symbol_table_entry.Name[6] = (string_table_size >> 16) & 0xff;
                symbol_table_entry.Name[7] = (string_table_size >> 24) & 0xff;
                string_table_size += strlen(symbol->name) + 1;
                symbol->write_name_to_string_table = 1;
            }
                
            
            symbol_table_entry.Value = symbol_get_value(symbol);
            
            /* Section number is one based index. */
            if (symbol_is_undefined(symbol))
            {
                symbol_table_entry.SectionNumber = IMAGE_SYM_UNDEFINED;
            }
            else if (symbol_get_section(symbol) == text_section)
            {
                symbol_table_entry.SectionNumber = 1;
            }
            else if (symbol_get_section(symbol) == data_section)
            {
                symbol_table_entry.SectionNumber = 2;
            }
            else if (symbol_get_section(symbol) == bss_section)
            {
                symbol_table_entry.SectionNumber = 3;
            }
            else as_error("+++COFF symbol table\n");
            
            symbol_table_entry.Type = ((IMAGE_SYM_DTYPE_NULL << 8)
                                       | IMAGE_SYM_TYPE_NULL);

            if (symbol_is_external(symbol) || symbol_is_undefined(symbol))
            {
                symbol_table_entry.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            }
            else if (symbol_is_section_symbol(symbol))
            {
                symbol_table_entry.StorageClass = IMAGE_SYM_CLASS_STATIC;
            }
            else
            {
                as_error("+++COFF symbol table\n");
            }
            
            symbol_table_entry.NumberOfAuxSymbols = 0;

            if (fwrite(&symbol_table_entry,
                       Symbol_Table_entry_size, 1, outfile) != 1)
            {
                as_error("Failed to write symbol table!\n");
                return;
            }

            symbol_set_symbol_table_index(symbol, file_header.NumberOfSymbols);

            file_header.NumberOfSymbols++;
        }
    }

    /* Outputs String Table (it must immediately follow Symbol Table). */
    {
        symbolS *symbol;
        
        if (fwrite(&string_table_size, 4, 1, outfile) != 1)
        {
            as_error("Failed to write string table!\n");
            return;
        }

        for (symbol = symbols;
             symbol;
             symbol = symbol->next)
        {
            if (symbol->write_name_to_string_table)
            {
                if (fwrite(symbol->name, strlen(symbol->name) + 1,
                           1, outfile) != 1)
                {
                    as_error("Failed to write string table!\n");
                    return;
                }
            }
        }
    }

    /* Outputs text relocations. */
    {
        fixupS *fixup;

        text_section_header->PointerToRelocations = ftell(outfile);
        text_section_header->NumberOfRelocations = 0;

        section_set(text_section);
        for (fixup = current_frag_chain->first_fixup;
             fixup;
             fixup = fixup->next)
        {
            int ret;
            
            if (fixup->done) continue;

            ret = output_relocation(outfile, fixup);
            if (ret)
            {
                as_error("Failed to write text relocation!\n");
                return;
            }

            text_section_header->NumberOfRelocations++;
        }

        if (text_section_header->NumberOfRelocations == 0)
        {
            text_section_header->PointerToRelocations = 0;
        }
    }

    /* Outputs data relocations. */
    {
        fixupS *fixup;

        data_section_header->PointerToRelocations = ftell(outfile);
        data_section_header->NumberOfRelocations = 0;

        section_set(data_section);
        for (fixup = current_frag_chain->first_fixup;
             fixup;
             fixup = fixup->next)
        {
            int ret;
            
            if (fixup->done) continue;

            ret = output_relocation(outfile, fixup);
            if (ret)
            {
                as_error("Failed to write data relocation!\n");
                return;
            }

            data_section_header->NumberOfRelocations++;
        }

        if (data_section_header->NumberOfRelocations == 0)
        {
            data_section_header->PointerToRelocations = 0;
        }
    }

    /* Writes the file header.*/
    rewind(outfile);
    if (fwrite(&file_header, sizeof(file_header), 1, outfile) != 1)
    {
        as_error("Failed to write header!\n");
        return;
    }

    /* Writes the section headers. */
    if (fwrite(text_section_header, sizeof(*text_section_header), 1, outfile) != 1)
    {
        as_error("Failed to write header!\n");
        return;
    }

    if (fwrite(data_section_header, sizeof(*data_section_header), 1, outfile) != 1)
    {
        as_error("Failed to write header!\n");
        return;
    }

    if (fwrite(bss_section_header, sizeof(*bss_section_header), 1, outfile) != 1)
    {
        as_error("Failed to write header!\n");
        return;
    }

    if (fclose(outfile)) as_error("Failed to close file!\n");
}
