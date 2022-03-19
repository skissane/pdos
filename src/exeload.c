/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  exeload.c - functions for loading executables                    */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exeload.h"
#include "pos.h"
#include "liballoc.h" /* For kmalloc() and kfree(). */
#include "support.h"

/* Headers for executable support. */
#include "a_out.h"
#include "elf.h"
#include "mz.h"
#include "pecoff.h"

/* For reading files Pos API must be used directly
 * as PDPCLIB allocates memory from the process address space. */

static int exeloadLoadAOUT(unsigned long *entry_point, int fhandle);
static int exeloadLoadELF(unsigned long *entry_point, int fhandle);
static int exeloadLoadMZ(unsigned long *entry_point, int fhandle);
/* Subfunctions of exeloadLoadMZ() for loading extensions to MZ. */
static int exeloadLoadPE(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew);
static int exeloadLoadLX(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew);
static int exeloadLoadNE(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew);

/* Support function for loading DLLs. */
static int exeloadLoadPEDLL(unsigned char *exeStart,
                            IMAGE_IMPORT_DESCRIPTOR *import_desc);

extern char kernel32[];
extern char msvcrt[];
extern unsigned char *loadaddr;
static int warnkernel = 1;

int exeloadDoload(unsigned long *entry_point, char *prog)
{
    int fhandle;
    int ret;

    if (PosOpenFile(prog, 0, &fhandle))
    {
        printf("Failed to open %s for loading\n", prog);
        return (1);
    }
    /* Tries to load the executable as different formats.
     * Returned 0 means the executable was loaded successfully.
     * 1 means it is not the format the function loads.
     * 2 means correct format, but error occured. */
    ret = exeloadLoadAOUT(entry_point, fhandle);
#if WARNEXE
    if (ret == 0) printf("warning - still using a.out format\n");
#endif
    if (ret == 1) ret = exeloadLoadELF(entry_point, fhandle);
    if (ret == 1) ret = exeloadLoadMZ(entry_point, fhandle);
    if (ret != 0)
    {
        PosCloseFile(fhandle);
        return (1);
    }

    return (0);
}

static int exeloadLoadAOUT(unsigned long *entry_point, int fhandle)
{
    int doing_zmagic = 0;
    int doing_nmagic = 0;
    struct exec firstbit;
    unsigned int headerLen;
    unsigned char *header = NULL;
    unsigned long exeLen;
    unsigned char *exeStart;
    unsigned char *bss;
    long newpos;
    size_t readbytes;

    if (PosMoveFilePointer(fhandle, 0, SEEK_SET, &newpos)
        || PosReadFile(fhandle, &firstbit, sizeof(firstbit), &readbytes)
        || readbytes != sizeof(firstbit))
    {
        return (1);
    }
    /* ZMAGIC and NMAGIC are currently not supported
     * as they should be loaded at 0x10000,
     * but the kernel reserves the first 4 MiB. */
    if ((firstbit.a_info & 0xffff) == ZMAGIC)
    {
        doing_zmagic = 1;
        printf("a.out ZMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) == NMAGIC)
    {
        doing_nmagic = 1;
        printf("a.out NMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) == QMAGIC)
    {
        printf("a.out QMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) != OMAGIC)
    {
        /* The file is not A.OUT. */
        return (1);
    }
    if (doing_zmagic)
    {
        headerLen = N_TXTOFF(firstbit);
        header = kmalloc(headerLen);
        memcpy(header, &firstbit, sizeof firstbit);
        if (PosReadFile(fhandle, header + sizeof firstbit,
                        headerLen - sizeof firstbit, &readbytes)
            || (readbytes != (headerLen - sizeof firstbit)))
        {
            kfree(header);
            return (2);
        }
    }

    if (doing_zmagic || doing_nmagic)
    {
        exeLen = N_BSSADDR(firstbit) - N_TXTADDR(firstbit) + firstbit.a_bss;
    }
    else
    {
        exeLen = firstbit.a_text + firstbit.a_data + firstbit.a_bss;
    }
    /* Allocates memory for the process. */
    if (doing_zmagic || doing_nmagic)
    {
        /* a.out ZMAGIC and NMAGIC must be loaded at 0x10000. */
        exeStart = PosVirtualAlloc((void *)0x10000, exeLen);
    }
    else
    {
        /* a.out OMAGIC can be loaded anywhere. */
        exeStart = PosVirtualAlloc(0, exeLen);
    }
    if (exeStart == NULL)
    {
        printf("Insufficient memory to load A.OUT program\n");
        if (doing_zmagic) kfree(header);
        return (2);
    }


    if (PosReadFile(fhandle, exeStart, firstbit.a_text, &readbytes)
        || (readbytes != (firstbit.a_text)))
    {
        printf("Error occured while reading A.OUT text\n");
        if (doing_zmagic) kfree(header);
        return (2);
    }
    if (doing_zmagic || doing_nmagic)
    {
        if (PosReadFile(fhandle,
                        (exeStart + N_DATADDR(firstbit) - N_TXTADDR(firstbit)),
                        firstbit.a_data,
                        &readbytes)
            || (readbytes != (firstbit.a_data)))
        {
            printf("Error occured while reading A.OUT data\n");
            if (doing_zmagic) kfree(header);
            return (2);
        }
    }
    else
    {
        if (PosReadFile(fhandle, exeStart + firstbit.a_text, firstbit.a_data,
                        &readbytes)
            || (readbytes != (firstbit.a_data)))
        {
            printf("Error occured while reading A.OUT data\n");
            return (2);
        }
    }
    /* Closes the file for ZMAGIC and NMAGIC as there is no need for it now. */
    if (doing_zmagic || doing_nmagic) PosCloseFile(fhandle);
    if (doing_zmagic) kfree(header);

    /* initialise BSS */
    if (doing_zmagic || doing_nmagic)
    {
        bss = exeStart + N_BSSADDR(firstbit);
    }
    else
    {
        bss = exeStart + firstbit.a_text + firstbit.a_data;
    }
    memset(bss, '\0', firstbit.a_bss);
    /* Relocations. */
    if (!doing_zmagic && !doing_nmagic)
    {
        unsigned int *corrections;
        unsigned int i;
        unsigned int offs;
        unsigned int type;
        unsigned int zapdata;
        unsigned char *zap;

        zap = exeStart;
        zapdata = (unsigned int)zap;
        if (firstbit.a_trsize != 0)
        {
            corrections = kmalloc(firstbit.a_trsize);
            if (corrections == NULL)
            {
                printf("insufficient memory %lu\n", firstbit.a_trsize);
                return (2);
            }
            if (PosReadFile(fhandle, corrections, firstbit.a_trsize,
                            &readbytes)
                || (readbytes != (firstbit.a_trsize)))
            {
                printf("Error occured while reading A.OUT text relocations\n");
                kfree(corrections);
                return (2);
            }
            for (i = 0; i < firstbit.a_trsize / 4; i += 2)
            {
                offs = corrections[i];
                type = corrections[i + 1];
                if (((type >> 24) & 0xff) != 0x04)
                {
                    continue;
                }
                *(unsigned int *)(zap + offs) += zapdata;
            }
            kfree(corrections);
        }
        if (firstbit.a_drsize != 0)
        {
            corrections = kmalloc(firstbit.a_drsize);
            if (corrections == NULL)
            {
                printf("insufficient memory %lu\n", firstbit.a_drsize);
                return (2);
            }
            if (PosReadFile(fhandle, corrections, firstbit.a_drsize,
                            &readbytes)
                || (readbytes != (firstbit.a_drsize)))
            {
                printf("Error occured while reading A.OUT data relocations\n");
                kfree(corrections);
                return (2);
            }
            zap = exeStart + firstbit.a_text;
            for (i = 0; i < firstbit.a_drsize / 4; i += 2)
            {
                offs = corrections[i];
                type = corrections[i + 1];
                if (((type >> 24) & 0xff) != 0x04)
                {
                    continue;
                }
                *(unsigned int *)(zap + offs) += zapdata;
            }
            kfree(corrections);
        }
        firstbit.a_entry += (unsigned long)exeStart;
        PosCloseFile(fhandle);
    }

    *entry_point = firstbit.a_entry;
    loadaddr = exeStart;

    return (0);
}

static int exeloadLoadELF(unsigned long *entry_point, int fhandle)
{
    int doing_elf_rel = 0;
    int doing_elf_exec = 0;
    Elf32_Ehdr *elfHdr;
    Elf32_Phdr *program_table = NULL;
    Elf32_Phdr *segment;
    Elf32_Shdr *section_table = NULL;
    Elf32_Shdr *section;
    unsigned char *elf_other_sections;
    Elf32_Addr lowest_p_vaddr = 0;
    Elf32_Word lowest_segment_align = 0;
    unsigned char *exeStart;
    unsigned char *bss;
    unsigned long exeLen;
    unsigned char firstbit[4];
    long newpos;
    size_t readbytes;

    if (PosMoveFilePointer(fhandle, 0, SEEK_SET, &newpos)
        || PosReadFile(fhandle, firstbit, sizeof(firstbit), &readbytes)
        || (readbytes != sizeof(firstbit))
        || (memcmp((&firstbit), "\x7f""ELF", 4) != 0))
    {
        return (1);
    }
    {
        int elf_invalid = 0;

        /* Loads entire ELF header into memory. */
        elfHdr = kmalloc(sizeof(Elf32_Ehdr));
        if (elfHdr == NULL)
        {
            printf("Insufficient memory for ELF header\n");
            return (2);
        }
        if (PosMoveFilePointer(fhandle, 0, SEEK_SET, &newpos)
            || PosReadFile(fhandle, elfHdr, sizeof(Elf32_Ehdr), &readbytes)
            || (readbytes != sizeof(Elf32_Ehdr)))
        {
            printf("Error occured while reading ELF header\n");
            kfree(elfHdr);
            return (2);
        }

        /* Checks e_ident if the program can be used on PDOS-32. */
        if (elfHdr->e_ident[EI_CLASS] != ELFCLASS32)
        {
            if (elfHdr->e_ident[EI_CLASS] == ELFCLASS64)
            {
                printf("64-bit ELF is not supported\n");
            }
            else if (elfHdr->e_ident[EI_CLASS] == ELFCLASSNONE)
            {
                printf("Invalid ELF class\n");
            }
            else
            {
                printf("Unknown ELF class: %u\n", elfHdr->e_ident[EI_CLASS]);
            }
            elf_invalid = 1;
        }
        if (elfHdr->e_ident[EI_DATA] != ELFDATA2LSB)
        {
            if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB)
            {
                printf("Big-endian ELF encoding is not supported\n");
            }
            else if (elfHdr->e_ident[EI_DATA] == ELFDATANONE)
            {
                printf("Invalid ELF data encoding\n");
            }
            else
            {
                printf("Unknown ELF data encoding: %u\n",
                       elfHdr->e_ident[EI_DATA]);
            }
            elf_invalid = 1;
        }
        if (elfHdr->e_ident[EI_OSABI] != ELFOSABI_NONE)
        {
            printf("No OS or ABI specific extensions for ELF supported\n");
            elf_invalid = 1;
        }
        /* Checks other parts of the header if the file can be loaded. */
        if (elfHdr->e_type == ET_REL)
        {
            doing_elf_rel = 1;
        }
        else if (elfHdr->e_type == ET_EXEC)
        {
            doing_elf_exec = 1;
        }
        else
        {
            printf("Only ELF relocatable and executable "
                   "files are supported\n");
            elf_invalid = 1;
        }
        if (elfHdr->e_machine != EM_386)
        {
            printf("Only Intel 386 architecture is supported\n");
            elf_invalid = 1;
        }
        if (doing_elf_exec)
        {
            if (elfHdr->e_phoff == 0 || elfHdr->e_phnum == 0)
            {
                printf("Executable file is missing Program Header Table\n");
                elf_invalid = 1;
            }
            if (elfHdr->e_phnum >= SHN_LORESERVE)
            {
                printf("Reserved indexes for e_phnum are not supported\n");
                printf("e_phnum is %04x\n", elfHdr->e_phnum);
                elf_invalid = 1;
            }
            if (elfHdr->e_phentsize != sizeof(Elf32_Phdr))
            {
                printf("Program Header Table entries have unsupported size\n");
                printf("e_phentsize: %u supported size: %lu\n",
                       elfHdr->e_phentsize, sizeof(Elf32_Phdr));
                elf_invalid = 1;
            }
        }
        else if (doing_elf_rel)
        {
            if (elfHdr->e_shoff == 0 || elfHdr->e_shnum == 0)
            {
                printf("Relocatable file is missing Section Header Table\n");
                elf_invalid = 1;
            }
            if (elfHdr->e_shnum >= SHN_LORESERVE)
            {
                printf("Reserved indexes for e_shnum are not supported\n");
                printf("e_shnum is %04x\n", elfHdr->e_shnum);
                elf_invalid = 1;
            }
            if (elfHdr->e_shentsize != sizeof(Elf32_Shdr))
            {
                printf("Section Header Table entries have unsupported size\n");
                printf("e_shentsize: %u supported size: %lu\n",
                       elfHdr->e_shentsize, sizeof(Elf32_Shdr));
                elf_invalid = 1;
            }
        }
        if (elf_invalid)
        {
            /* All problems with ELF header are reported
             * and loading is stopped. */
            printf("This ELF file cannot be loaded\n");
            kfree(elfHdr);
            return (2);
        }
        /* Loads Program Header Table if it is present. */
        if (!(elfHdr->e_phoff == 0 || elfHdr->e_phnum == 0))
        {
            program_table = kmalloc(elfHdr->e_phnum * elfHdr->e_phentsize);
            if (program_table == NULL)
            {
                printf("Insufficient memory for ELF Program Header Table\n");
                kfree(elfHdr);
                return (2);
            }
            if (PosMoveFilePointer(fhandle, elfHdr->e_phoff, SEEK_SET, &newpos)
                || PosReadFile(fhandle, program_table,
                               elfHdr->e_phnum * elfHdr->e_phentsize,
                               &readbytes)
                || (readbytes != (elfHdr->e_phnum * elfHdr->e_phentsize)))
            {
                printf("Error occured while reading "
                       "ELF Program Header Table\n");
                kfree(elfHdr);
                kfree(program_table);
                return (2);
            }
        }
        /* Loads Section Header Table if it is present. */
        if (!(elfHdr->e_shoff == 0 || elfHdr->e_shnum == 0))
        {
            section_table = kmalloc(elfHdr->e_shnum * elfHdr->e_shentsize);
            if (section_table == NULL)
            {
                printf("Insufficient memory for ELF Section Header Table\n");
                kfree(elfHdr);
                return (2);
            }
            if (PosMoveFilePointer(fhandle, elfHdr->e_shoff, SEEK_SET, &newpos)
                || PosReadFile(fhandle, section_table,
                               elfHdr->e_shnum * elfHdr->e_shentsize,
                               &readbytes)
                || (readbytes != (elfHdr->e_shnum * elfHdr->e_shentsize)))
            {
                printf("Error occured while reading "
                       "ELF Section Header Table\n");
                kfree(elfHdr);
                kfree(section_table);
                return (2);
            }
        }
    }

    if (doing_elf_rel || doing_elf_exec)
    {
        /* Calculates how much memory is needed
         * and allocates memory for sections used only for loading. */
        unsigned long otherLen = 0;

        exeLen = 0;
        if (doing_elf_exec)
        {
            Elf32_Addr highest_p_vaddr = 0;
            Elf32_Word highest_segment_memsz = 0;

            for (segment = program_table;
                 segment < program_table + elfHdr->e_phnum;
                 segment++)
            {
                if (segment->p_type == PT_LOAD)
                {
                    if (!lowest_p_vaddr || lowest_p_vaddr > segment->p_vaddr)
                    {
                        lowest_p_vaddr = segment->p_vaddr;
                        lowest_segment_align = segment->p_align;
                    }
                    if (highest_p_vaddr < segment->p_vaddr)
                    {
                        highest_p_vaddr = segment->p_vaddr;
                        highest_segment_memsz = segment->p_memsz;
                    }
                }
            }
            exeLen = highest_p_vaddr - lowest_p_vaddr + highest_segment_memsz;
            if (lowest_segment_align > 1)
            {
                /* Ensures alignment of the lowest segment.
                 * 0 and 1 mean no alignment restrictions. */
                exeLen += lowest_segment_align;
            }
        }
        if (section_table)
        {
            for (section = section_table;
                 section < section_table + elfHdr->e_shnum;
                 section++)
            {
                unsigned long section_size = section->sh_size;
                if (section->sh_addralign > 1)
                {
                    /* Some sections must be aligned
                     * on sh_addralign byte boundaries.
                     * 0 and 1 mean no alignment restrictions. */
                    section_size += section->sh_addralign;
                }
                if (section->sh_flags & SHF_ALLOC)
                {
                    /* Section is needed while the program is running,
                     * but if we are loading an executable file,
                     * the memory is already counted
                     * using Program Header Table. */
                    if (doing_elf_exec) continue;
                    exeLen += section_size;
                }
                else
                {
                    /* Section is used only for loading. */
                   otherLen += section_size;
                }
            }
            elf_other_sections = kmalloc(otherLen);
            if (elf_other_sections == NULL)
            {
                printf("Insufficient memory to load ELF sections\n");
                kfree(elfHdr);
                kfree(section_table);
                return (2);
            }
        }
    }
    /* Allocates memory for the process. */
    if (doing_elf_exec)
    {
        exeStart = PosVirtualAlloc((void *)lowest_p_vaddr, exeLen);
    }
    else if (doing_elf_rel)
    {
        exeStart = PosVirtualAlloc(0, exeLen);
    }
    if (exeStart == NULL)
    {
        printf("Insufficient memory to load ELF program\n");
        kfree(elfHdr);
        if (program_table) kfree(program_table);
        if (section_table)
        {
            kfree(section_table);
            kfree(elf_other_sections);
        }
        return (2);
    }

    if (doing_elf_rel || doing_elf_exec)
    {
        /* Loads all sections of ELF file with proper alignment,
         * clears all SHT_NOBITS sections and stores the addresses
         * in sh_addr of each section.
         * bss is set now too. */
        unsigned char *exe_addr = exeStart;
        unsigned char *other_addr = elf_other_sections;

        bss = 0;
        if (doing_elf_exec)
        {
            /* Aligns the exeStart on lowest segment alignment boundary. */
            /*exeStart = (unsigned char *)((((unsigned long)exeStart
                                           / lowest_segment_align) + 1)
                                         * lowest_segment_align);*/
            /* +++Enable aligning. */
            for (segment = program_table;
                 segment < program_table + elfHdr->e_phnum;
                 segment++)
            {
                if (segment->p_type == PT_LOAD)
                {
                    exe_addr = exeStart + (segment->p_vaddr - lowest_p_vaddr);

                    if (PosMoveFilePointer(fhandle, segment->p_offset,
                                           SEEK_SET, &newpos)
                        || PosReadFile(fhandle, exe_addr, segment->p_filesz,
                                       &readbytes)
                        || (readbytes != (segment->p_filesz)))
                    {
                        printf("Error occured while reading ELF segment\n");
                        kfree(program_table);
                        if (section_table)
                        {
                            kfree(section_table);
                            kfree(elf_other_sections);
                        }
                        return (2);
                    }

                    /* Bytes that are not present in file,
                     * but must be present in memory must be set to 0. */
                    if (segment->p_filesz < segment->p_memsz)
                    {
                        bss = exe_addr + (segment->p_filesz);
                        memset(bss, '\0',
                               segment->p_memsz - segment->p_filesz);
                    }
                }
            }
        }

        for (section = section_table;
             section < section_table + elfHdr->e_shnum;
             section++)
        {
            if (section->sh_flags & SHF_ALLOC)
            {
                /* If we are loading executable file,
                 * SHF_ALLOC sections are already loaded in segments. */
                if (doing_elf_exec) continue;
                if (section->sh_addralign > 1)
                {
                    exe_addr = (void *)((((unsigned long)exe_addr
                                          / (section->sh_addralign)) + 1)
                                        * (section->sh_addralign));
                }
                if (section->sh_type != SHT_NOBITS)
                {
                    if (PosMoveFilePointer(fhandle, section->sh_offset,
                                           SEEK_SET, &newpos)
                        || PosReadFile(fhandle, exe_addr, section->sh_size,
                                       &readbytes)
                        || (readbytes != (section->sh_size)))
                    {
                        printf("Error occured while reading ELF section\n");
                        kfree(elfHdr);
                        if (program_table) kfree(program_table);
                        kfree(section_table);
                        kfree(elf_other_sections);
                        return (2);
                    }
                }
                else
                {
                    /* The section is probably BSS. */
                    if (bss != 0)
                    {
                        printf("Multiple SHT_NOBITS with SHF_ALLOC "
                               "present in ELF file\n");
                    }
                    bss = exe_addr;
                    /* All SHT_NOBITS should be cleared to 0. */
                    memset(bss, '\0', section->sh_size);
                }
                /* sh_addr is 0 in relocatable files,
                 * so we can use it to store the real address. */
                section->sh_addr = (Elf32_Addr)exe_addr;
                exe_addr += section->sh_size;
            }
            else
            {
                if (section->sh_addralign > 1)
                {
                    other_addr = (void *)((((unsigned long)other_addr
                                            / (section->sh_addralign)) + 1)
                                          * (section->sh_addralign));
                }
                if (section->sh_type != SHT_NOBITS)
                {
                    if (PosMoveFilePointer(fhandle, section->sh_offset,
                                           SEEK_SET, &newpos)
                        || PosReadFile(fhandle, other_addr, section->sh_size,
                                       &readbytes)
                        || (readbytes != (section->sh_size)))
                    {
                        printf("Error occured while reading ELF section\n");
                        kfree(elfHdr);
                        if (program_table) kfree(program_table);
                        kfree(section_table);
                        kfree(elf_other_sections);
                        return (2);
                    }
                }
                else
                {
                    /* All SHT_NOBITS should be cleared to 0. */
                    memset(other_addr, '\0', section->sh_size);
                }
                /* sh_addr is 0 in relocatable files,
                 * so we can use it to store the real address. */
                section->sh_addr = (Elf32_Addr)other_addr;
                other_addr += section->sh_size;
            }
        }
    }
    /* Program was successfully loaded from the file,
     * no more errors can occur. */
    PosCloseFile(fhandle);

    /* Relocations. */
    if (doing_elf_rel)
    {
        for (section = section_table;
             section < section_table + elfHdr->e_shnum;
             section++)
        {
            if (section->sh_type == SHT_RELA)
            {
                printf("ELF Relocations with explicit addend "
                       "are not supported\n");
            }
            else if (section->sh_type == SHT_REL)
            {
                /* sh_link specifies the symbol table
                 * and sh_info section being modified. */
                Elf32_Sym *sym_table = (Elf32_Sym *)(section_table
                                        + (section->sh_link))->sh_addr;
                unsigned char *target_base = (unsigned char *)(section_table
                                             + (section->sh_info))->sh_addr;
                Elf32_Rel *startrel = (Elf32_Rel *)section->sh_addr;
                Elf32_Rel *currel;

                if (section->sh_entsize != sizeof(Elf32_Rel))
                {
                    printf("Invalid size of relocation entries in ELF file\n");
                    continue;
                }

                for (currel = startrel;
                     currel < (startrel
                               + ((section->sh_size) / (section->sh_entsize)));
                     currel++)
                {
                    long *target = (long *)(target_base + currel->r_offset);
                    Elf32_Sym *symbol = (sym_table
                                         + ELF32_R_SYM(currel->r_info));
                    Elf32_Addr sym_value = 0;

                    if (ELF32_R_SYM(currel->r_info) != STN_UNDEF)
                    {
                        if (symbol->st_shndx == SHN_ABS)
                        {
                            /* Absolute symbol, stores absolute value. */
                            sym_value = symbol->st_value;
                        }
                        else if (symbol->st_shndx == SHN_UNDEF)
                        {
                            /* Dynamic linker should fill this symbol. */
                            printf("Undefined symbol in ELF file\n");
                            continue;
                        }
                        else if (symbol->st_shndx == SHN_XINDEX)
                        {
                            printf("Unsupported value in ELF symbol\n");
                            printf("symbol->st_shndx: %x\n", symbol->st_shndx);
                            continue;
                        }
                        else
                        {
                            /* Internal symbol. Must be converted
                             * to absolute symbol.*/
                            sym_value = symbol->st_value;
                            /* Adds the address of the related section
                             * so the symbol stores absolute address. */
                            sym_value += ((section_table
                                           + symbol->st_shndx)->sh_addr);
                        }
                    }
                    switch (ELF32_R_TYPE(currel->r_info))
                    {
                        case R_386_NONE:
                            break;
                        case R_386_32:
                            /* Symbol value + offset. */
                            *target = sym_value + *target;
                            break;
                        case R_386_PC32:
                            /* Symbol value + offset - absolute address
                             * of the modified field. */
                            *target = (sym_value + (*target)
                                       - (unsigned long)target);
                            break;
                        default:
                            printf("Unknown relocation type in ELF file\n");
                    }
                }
            }
        }
    }
    else if (doing_elf_exec)
    {
        /* +++Implement relocations for ELF Executables. */
        ;
    }

    if (doing_elf_rel)
    {
        *entry_point = ((unsigned long)exeStart) + (elfHdr->e_entry);
    }
    else if (doing_elf_exec)
    {
        *entry_point = elfHdr->e_entry;
    }
    loadaddr = exeStart;

    /* Frees memory not needed by the process. */
    kfree(elfHdr);
    if (program_table) kfree(program_table);
    if (section_table)
    {
        kfree(section_table);
        kfree(elf_other_sections);
    }

    return (0);
}

static int exeloadLoadMZ(unsigned long *entry_point, int fhandle)
{
    Mz_hdr firstbit;
    long newpos;
    size_t readbytes;

    /* The header size is in paragraphs,
     * so the smallest possible header is 16 bytes (paragraph) long.
     * Next is the magic number checked. */
    if (PosMoveFilePointer(fhandle, 0, SEEK_SET, &newpos)
        || PosReadFile(fhandle, &firstbit, 16, &readbytes)
        || (readbytes != 16)
        || (memcmp(firstbit.magic, "MZ", 2) != 0
            && memcmp(&firstbit.magic, "ZM", 2) != 0))
    {
        return (1);
    }
    if (firstbit.header_size == 0)
    {
        printf("MZ Header has 0 size\n");
        return (2);
    }
    if (firstbit.header_size * 16 > sizeof(firstbit))
    {
        printf("MZ Header is too large, size: %u\n",
               firstbit.header_size * 16);
        return (2);
    }
    /* Loads the rest of the header. */
    if (PosReadFile(fhandle, ((char *)&firstbit) + 16,
                    (firstbit.header_size - 1) * 16, &readbytes)
        || (readbytes != (firstbit.header_size - 1) * 16))
    {
        printf("Error occured while reading MZ header\n");
        return (2);
    }
    /* Determines whether the executable has extensions or is a pure MZ.
     * Extensions are at offset in e_lfanew,
     * so the header must have at least 4 paragraphs. */
    if ((firstbit.header_size >= 4)
        && (firstbit.e_lfanew != 0))
    {
        int ret;

        /* Same logic as in exeloadDoload(). */
        ret = exeloadLoadPE(entry_point, fhandle, firstbit.e_lfanew);
        if (ret == 1)
            ret = exeloadLoadLX(entry_point, fhandle, firstbit.e_lfanew);
        if (ret == 1)
            ret = exeloadLoadNE(entry_point, fhandle, firstbit.e_lfanew);
        if (ret == 1)
            printf("Unknown MZ extension\n");
        return (ret);
    }
    /* Pure MZ executables are for 16-bit DOS, so we cannot run them. */
    printf("Pure MZ executables are not supported\n");

    return (2);
}

static int exeloadLoadPE(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew)
{
    Coff_hdr coff_hdr;
    Pe32_optional_hdr *optional_hdr;
    Coff_section *section_table, *section;
    unsigned char *exeStart;
    long newpos;
    size_t readbytes;

    {
        unsigned char firstbit[4];

        if (PosMoveFilePointer(fhandle, e_lfanew, SEEK_SET, &newpos)
            || PosReadFile(fhandle, firstbit, 4, &readbytes)
            || (readbytes != 4)
            || (memcmp(firstbit, "PE\0\0", 4) != 0))
        {
            return (1);
        }
    }
    if (PosReadFile(fhandle, &coff_hdr, sizeof(coff_hdr), &readbytes)
        || (readbytes != sizeof(coff_hdr)))
    {
        printf("Error occured while reading COFF header\n");
        return (2);
    }
    if ((coff_hdr.Machine != IMAGE_FILE_MACHINE_UNKNOWN)
        && (coff_hdr.Machine != IMAGE_FILE_MACHINE_I386))
    {
        if (coff_hdr.Machine == IMAGE_FILE_MACHINE_AMD64)
        {
            printf("64-bit PE programs are not supported\n");
        }
        else
        {
            printf("Unknown PE machine type: %04x\n", coff_hdr.Machine);
        }
        return (2);
    }

    optional_hdr = kmalloc(coff_hdr.SizeOfOptionalHeader);
    if (optional_hdr == NULL)
    {
        printf("Insufficient memory to load PE optional header\n");
        return (2);
    }
    if (PosReadFile(fhandle, optional_hdr, coff_hdr.SizeOfOptionalHeader,
                    &readbytes)
        || (readbytes != (coff_hdr.SizeOfOptionalHeader)))
    {
        printf("Error occured while reading PE optional header\n");
        kfree(optional_hdr);
        return (2);
    }
    if (optional_hdr->Magic != MAGIC_PE32)
    {
        printf("Unknown PE optional header magic: %04x\n",
               optional_hdr->Magic);
        kfree(optional_hdr);
        return (2);
    }

    section_table = kmalloc(coff_hdr.NumberOfSections * sizeof(Coff_section));
    if (section_table == NULL)
    {
        printf("Insufficient memory to load PE section headers\n");
        kfree(optional_hdr);
        return (2);
    }
    if (PosReadFile(fhandle, section_table,
                    coff_hdr.NumberOfSections * sizeof(Coff_section),
                    &readbytes)
        || (readbytes != (coff_hdr.NumberOfSections * sizeof(Coff_section))))
    {
        printf("Error occured while reading PE optional header\n");
        kfree(section_table);
        kfree(optional_hdr);
        return (2);
    }

    if (coff_hdr.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
    {
        printf("error - executable has had relocations stripped\n");
        kfree(section_table);
        kfree(optional_hdr);
        return (2);
    }

    /* Allocates memory for the process.
     * Size of image is obtained from the optional header. */
    exeStart = PosVirtualAlloc(0, optional_hdr->SizeOfImage);

    if (exeStart == NULL)
    {
        printf("Insufficient memory to load PE program\n");
        kfree(section_table);
        kfree(optional_hdr);
        return (2);
    }

    /* Loads all sections at their addresses. */
    for (section = section_table;
         section < section_table + (coff_hdr.NumberOfSections);
         section++)
    {
        unsigned long size_in_file;

        /* Virtual size of a section is larger than in file,
         * so the difference is filled with 0. */
        if ((section->VirtualSize) > (section->SizeOfRawData))
        {
            memset((exeStart
                    + (section->VirtualAddress)
                    + (section->SizeOfRawData)),
                   0,
                   (section->VirtualSize) - (section->SizeOfRawData));
            size_in_file = section->SizeOfRawData;
        }
        /* SizeOfRawData is rounded up,
         * so it can be larger than VirtualSize. */
        else
        {
            size_in_file = section->VirtualSize;
        }
        if (PosMoveFilePointer(fhandle, section->PointerToRawData, SEEK_SET,
                               &newpos)
            || PosReadFile(fhandle, exeStart + (section->VirtualAddress),
                           size_in_file, &readbytes)
            || (readbytes != size_in_file))
        {
            printf("Error occured while reading PE section\n");
            kfree(section_table);
            kfree(optional_hdr);
            return (2);
        }
    }
    PosCloseFile(fhandle);

    if (((unsigned long)exeStart) != (optional_hdr->ImageBase))
    {
        /* Relocations are not stripped, so the executable can be relocated. */
        if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_REL)
        {
            IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                               (optional_hdr + 1))
                                              + DATA_DIRECTORY_REL);
            /* Difference between preferred address and real address. */
            unsigned long image_diff;
            int lower_exeStart;
            Base_relocation_block *rel_block = ((Base_relocation_block *)
                                                (exeStart
                                                 + (data_dir
                                                    ->VirtualAddress)));
            Base_relocation_block *end_rel_block;

            if (optional_hdr->ImageBase > (unsigned long)exeStart)
            {
                /* Image is loaded  at lower address than preferred. */
                image_diff = optional_hdr->ImageBase
                             - (unsigned long)exeStart;
                lower_exeStart = 1;
            }
            else
            {
                image_diff = (unsigned long)exeStart
                             - optional_hdr->ImageBase;
                lower_exeStart = 0;
            }
            end_rel_block = rel_block + ((data_dir->Size)
                                         / sizeof(Base_relocation_block));

            for (; rel_block < end_rel_block;)
            {
                unsigned short *cur_rel = (unsigned short *)rel_block;
                unsigned short *end_rel;

                end_rel = (cur_rel
                           + ((rel_block->BlockSize)
                              / sizeof(unsigned short)));
                cur_rel = (unsigned short *)(rel_block + 1);

                for (; cur_rel < end_rel; cur_rel++)
                {
                    /* Top 4 bits indicate the type of relocation. */
                    unsigned char rel_type = (*cur_rel) >> 12;
                    void *rel_target = (exeStart + (rel_block->PageRva)
                                        + ((*cur_rel) & 0x0fff));

                    if (rel_type == IMAGE_REL_BASED_ABSOLUTE) continue;
                    if (rel_type == IMAGE_REL_BASED_HIGHLOW)
                    {
                        if (lower_exeStart)
                            (*((unsigned long *)rel_target)) -= image_diff;
                        else (*((unsigned long *)rel_target)) += image_diff;
                    }
                    else
                    {
                        printf("Unknown PE relocation type: %u\n",
                               rel_type);
                    }
                }

                rel_block = (Base_relocation_block *)cur_rel;
            }
        }
    }

    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_IMPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_IMPORT_TABLE);
        if (data_dir->Size != 0)
        {
            IMAGE_IMPORT_DESCRIPTOR *import_desc = ((void *)
                                                    (exeStart
                                                     + (data_dir
                                                        ->VirtualAddress)));

            /* Each descriptor is for one DLL
             * and the array has a null terminator. */
            for (; import_desc->OriginalFirstThunk; import_desc++)
            {
                if (exeloadLoadPEDLL(exeStart, import_desc))
                {
                    printf("Failed to load DLL %s\n",
                           exeStart + (import_desc->Name));
                    return (2);
                }
            }
        }
    }

    *entry_point = (((unsigned long)exeStart)
                    + (optional_hdr->AddressOfEntryPoint));
    loadaddr = exeStart;
    /* Frees memory not needed by the process. */
    kfree(section_table);
    kfree(optional_hdr);

    return (0);
}

static int exeloadLoadPEDLL(unsigned char *exeStart,
                            IMAGE_IMPORT_DESCRIPTOR *import_desc)
{
    Mz_hdr mzFirstbit;
    int fhandle;
    Coff_hdr coff_hdr;
    Pe32_optional_hdr *optional_hdr;
    Coff_section *section_table, *section;
    unsigned char *dllStart;
    long newpos;
    size_t readbytes;
    char *name1;
    char *name2;

    /* PE DLL is being loaded,
     * so it is loaded in the same way as PE executable,
     * but the MZ stage is integrated. */
    name1 = exeStart + import_desc->Name;
    if ((strcmp(name1, "kernel32.dll") == 0)
        || (strcmp(name1, "KERNEL32.dll") == 0))
    {
        name2 = kernel32;
        if (warnkernel)
        {
#if WARNEXE
            printf("warning - this executable is dependent on"
                   " kernel32 instead of just msvcrt\n");
#endif
        }
    }
    else if (strcmp(name1, "msvcrt.dll") == 0)
    {
        name2 = msvcrt;
        warnkernel = 0; /* MSVCRT is the only thing allowed to use
            kernel32 without a warning */
    }
    else
    {
#if WARNEXE
        printf("warning - this executable uses a non-standard DLL %s\n",
               name1);
#endif
        name2 = name1;
    }
    if (PosOpenFile(name2, 0, &fhandle))
    {
        printf("Failed to open %s for loading\n",
               exeStart + (import_desc->Name));
        return (1);
    }

    /* The header size is in paragraphs,
     * so the smallest possible header is 16 bytes (paragraph) long.
     * Next is the magic number checked. */
    if (PosMoveFilePointer(fhandle, 0, SEEK_SET, &newpos)
        || PosReadFile(fhandle, &mzFirstbit, 16, &readbytes)
        || (readbytes != 16)
        || (memcmp(mzFirstbit.magic, "MZ", 2) != 0
            && memcmp(&mzFirstbit.magic, "ZM", 2) != 0))
    {
        PosCloseFile(fhandle);
        return (1);
    }
    if (mzFirstbit.header_size == 0)
    {
        printf("MZ Header has 0 size\n");
        PosCloseFile(fhandle);
        return (1);
    }
    if (mzFirstbit.header_size * 16 > sizeof(mzFirstbit))
    {
        printf("MZ Header is too large, size: %u\n",
               mzFirstbit.header_size * 16);
        PosCloseFile(fhandle);
        return (1);
    }
    /* Loads the rest of the header. */
    if (PosReadFile(fhandle, ((char *)&mzFirstbit) + 16,
                    (mzFirstbit.header_size - 1) * 16, &readbytes)
        || (readbytes != (mzFirstbit.header_size - 1) * 16))
    {
        printf("Error occured while reading MZ header\n");
        PosCloseFile(fhandle);
        return (1);
    }
    /* Determines whether the executable has extensions or is a pure MZ.
     * Extensions are at offset in e_lfanew,
     * so the header must have at least 4 paragraphs. */
    if ((mzFirstbit.header_size < 4)
        || (mzFirstbit.e_lfanew == 0))
    {
        PosCloseFile(fhandle);
        return (1);
    }

    /* PE stage begins. */
    {
        unsigned char firstbit[4];

        if (PosMoveFilePointer(fhandle, mzFirstbit.e_lfanew, SEEK_SET, &newpos)
            || PosReadFile(fhandle, firstbit, 4, &readbytes)
            || (readbytes != 4)
            || (memcmp(firstbit, "PE\0\0", 4) != 0))
        {
            PosCloseFile(fhandle);
            return (1);
        }
    }
    if (PosReadFile(fhandle, &coff_hdr, sizeof(coff_hdr), &readbytes)
        || (readbytes != sizeof(coff_hdr)))
    {
        printf("Error occured while reading COFF header\n");
        PosCloseFile(fhandle);
        return (1);
    }
    if ((coff_hdr.Machine != IMAGE_FILE_MACHINE_UNKNOWN)
        && (coff_hdr.Machine != IMAGE_FILE_MACHINE_I386))
    {
        if (coff_hdr.Machine == IMAGE_FILE_MACHINE_AMD64)
        {
            printf("64-bit PE programs are not supported\n");
        }
        else
        {
            printf("Unknown PE machine type: %04x\n", coff_hdr.Machine);
        }
        PosCloseFile(fhandle);
        return (1);
    }

    optional_hdr = kmalloc(coff_hdr.SizeOfOptionalHeader);
    if (optional_hdr == NULL)
    {
        printf("Insufficient memory to load PE optional header\n");
        PosCloseFile(fhandle);
        return (1);
    }
    if (PosReadFile(fhandle, optional_hdr, coff_hdr.SizeOfOptionalHeader,
                    &readbytes)
        || (readbytes != (coff_hdr.SizeOfOptionalHeader)))
    {
        printf("Error occured while reading PE optional header\n");
        kfree(optional_hdr);
        PosCloseFile(fhandle);
        return (1);
    }
    if (optional_hdr->Magic != MAGIC_PE32)
    {
        printf("Unknown PE optional header magic: %04x\n",
               optional_hdr->Magic);
        kfree(optional_hdr);
        PosCloseFile(fhandle);
        return (1);
    }

    section_table = kmalloc(coff_hdr.NumberOfSections * sizeof(Coff_section));
    if (section_table == NULL)
    {
        printf("Insufficient memory to load PE section headers\n");
        kfree(optional_hdr);
        return (2);
    }
    if (PosReadFile(fhandle, section_table,
                    coff_hdr.NumberOfSections * sizeof(Coff_section),
                    &readbytes)
        || (readbytes != (coff_hdr.NumberOfSections * sizeof(Coff_section))))
    {
        printf("Error occured while reading PE optional header\n");
        kfree(section_table);
        kfree(optional_hdr);
        PosCloseFile(fhandle);
        return (2);
    }

    if (coff_hdr.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
    {
        printf("error - DLL has had relocations stripped\n");
        kfree(section_table);
        kfree(optional_hdr);
        PosCloseFile(fhandle);
        return (2);
    }

    /* Allocates memory for the process.
     * Size of image is obtained from the optional header. */
    dllStart = PosVirtualAlloc(0, optional_hdr->SizeOfImage);
    if (dllStart == NULL)
    {
        printf("Insufficient memory to load PE program\n");
        kfree(section_table);
        kfree(optional_hdr);
        PosCloseFile(fhandle);
        return (2);
    }

    /* Loads all sections at their addresses. */
    for (section = section_table;
         section < section_table + (coff_hdr.NumberOfSections);
         section++)
    {
        unsigned long size_in_file;

        /* Virtual size of a section is larger than in file,
         * so the difference is filled with 0. */
        if ((section->VirtualSize) > (section->SizeOfRawData))
        {
            memset((dllStart
                    + (section->VirtualAddress)
                    + (section->SizeOfRawData)),
                   0,
                   (section->VirtualSize) - (section->SizeOfRawData));
            size_in_file = section->SizeOfRawData;
        }
        /* SizeOfRawData is rounded up,
         * so it can be larger than VirtualSize. */
        else
        {
            size_in_file = section->VirtualSize;
        }
        if (PosMoveFilePointer(fhandle, section->PointerToRawData, SEEK_SET,
                               &newpos)
            || PosReadFile(fhandle, dllStart + (section->VirtualAddress),
                           size_in_file, &readbytes)
            || (readbytes != size_in_file))
        {
            printf("Error occured while reading PE section\n");
            kfree(section_table);
            kfree(optional_hdr);
            PosCloseFile(fhandle);
            return (2);
        }
    }
    PosCloseFile(fhandle);

    if (((unsigned long)dllStart) != (optional_hdr->ImageBase))
    {
        /* Relocations are not stripped, so the executable can be relocated. */
        if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_REL)
        {
            IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                               (optional_hdr + 1))
                                              + DATA_DIRECTORY_REL);
            /* Difference between preferred address and real address. */
            unsigned long image_diff;
            int lower_dllStart = 0;
            Base_relocation_block *rel_block = ((Base_relocation_block *)
                                                (dllStart
                                                 + (data_dir
                                                    ->VirtualAddress)));
            Base_relocation_block *end_rel_block;

            if (optional_hdr->ImageBase > (unsigned long)dllStart)
            {
                /* Image is loaded  at lower address than preferred. */
                image_diff = optional_hdr->ImageBase
                             - (unsigned long)dllStart;
                lower_dllStart = 1;
            }
            else
            {
                image_diff = (unsigned long)dllStart
                             - optional_hdr->ImageBase;
                lower_dllStart = 0;
            }
            end_rel_block = rel_block + ((data_dir->Size)
                                         / sizeof(Base_relocation_block));

            for (; rel_block < end_rel_block;)
            {
                unsigned short *cur_rel = (unsigned short *)rel_block;
                unsigned short *end_rel;

                end_rel = (cur_rel
                           + ((rel_block->BlockSize)
                              / sizeof(unsigned short)));
                cur_rel = (unsigned short *)(rel_block + 1);

                for (; cur_rel < end_rel; cur_rel++)
                {
                    /* Top 4 bits indicate the type of relocation. */
                    unsigned char rel_type = (*cur_rel) >> 12;
                    void *rel_target = (dllStart + (rel_block->PageRva)
                                        + ((*cur_rel) & 0x0fff));

                    if (rel_type == IMAGE_REL_BASED_ABSOLUTE) continue;
                    if (rel_type == IMAGE_REL_BASED_HIGHLOW)
                    {
                        if (lower_dllStart)
                            (*((unsigned long *)rel_target)) -= image_diff;
                        else (*((unsigned long *)rel_target)) += image_diff;
                    }
                    else
                    {
                        printf("Unknown PE relocation type: %u\n",
                               rel_type);
                    }
                }

                rel_block = (Base_relocation_block *)cur_rel;
            }
        }
    }

    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_EXPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_EXPORT_TABLE);
        IMAGE_EXPORT_DIRECTORY *export_dir;
        unsigned long *functionTable;
        unsigned long *nameTable;
        unsigned short *ordinalTable;
        unsigned long *thunk;

        if (data_dir->Size == 0)
        {
            printf("DLL has no export tables\n");
            kfree(section_table);
            kfree(optional_hdr);
            return (1);
        }

        export_dir = ((void *)(dllStart + (data_dir->VirtualAddress)));
        functionTable = (void *)(dllStart + (export_dir->AddressOfFunctions));
        nameTable = (void *)(dllStart + (export_dir->AddressOfNames));
        ordinalTable = (void *)(dllStart
                                + (export_dir->AddressOfNameOrdinals));

        /* The importing itself.
         * Import Address Table of the executable is parsed
         * and function addresses are written
         * when they are found. */
        for (thunk = (void *)(exeStart + (import_desc->FirstThunk));
             *thunk != 0;
             thunk++)
        {
            if ((*thunk) & 0x80000000)
            {
                /* Bit 31 set, import by ordinal. */
                *thunk = ((unsigned long)
                          (dllStart
                           + (functionTable[((*thunk) & 0x7fffffff)
                                            - (export_dir->Base)])));
            }
            else
            {
                /* Import by name. */
                unsigned char *hintname = exeStart + ((*thunk) & 0x7fffffff);
                int i;

                /* The first 2 bytes are hint index,
                 * so they are skipped to get the name. */
                hintname += 2;
                for (i = 0; i < (export_dir->NumberOfNames); i++)
                {
                    if (strcmp(hintname, dllStart + (nameTable[i]))) continue;
                    break;
                }
                if (i == (export_dir->NumberOfNames))
                {
                    printf("Function %s not found in DLL\n", hintname);
                    kfree(section_table);
                    kfree(optional_hdr);
                    return (1);
                }
                /* PE specification says that the ordinals in ordinal table
                 * are biased by Ordinal Base,
                 * but it does not seem to be correct. */
                *thunk = (unsigned long)(dllStart
                                         + (functionTable[ordinalTable[i]]));
            }
        }
        /* After the DLL is bound, time/date stamp is copied. */
        import_desc->TimeDateStamp = export_dir->TimeDateStamp;
    }
    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_IMPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_IMPORT_TABLE);
        if (data_dir->Size != 0)
        {
            IMAGE_IMPORT_DESCRIPTOR *import_desc = ((void *)
                                                    (dllStart
                                                     + (data_dir
                                                        ->VirtualAddress)));

            /* Each descriptor is for one DLL
             * and the array has a null terminator. */
            for (; import_desc->OriginalFirstThunk; import_desc++)
            {
                if (exeloadLoadPEDLL(dllStart, import_desc))
                {
                    printf("Failed to load DLL %s\n",
                           dllStart + (import_desc->Name));
                    return (1);
                }
            }
        }
    }

    /* Entry point is optional for DLLs. */
    if (optional_hdr->AddressOfEntryPoint)
    {
        int ret = callDllEntry(dllStart + (optional_hdr->AddressOfEntryPoint),
                               dllStart,
                               1, /* fdwReason = DLL_PROCESS_ATTACH */
                               (void *)1); /* lpvReserved = non-NULL */

        if (ret == 0)
        {
            printf("Initialization of DLL %s failed\n",
                   exeStart + (import_desc->Name));
            kfree(section_table);
            kfree(optional_hdr);
            return (1);
        }
    }

    /* Frees memory not needed by the DLL. */
    kfree(section_table);
    kfree(optional_hdr);

    if (name2 == msvcrt)
    {
        warnkernel = 1;
    }
    return (0);
}

static int exeloadLoadLX(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew)
{
    unsigned char firstbit[2];
    long newpos;
    size_t readbytes;

    if (PosMoveFilePointer(fhandle, e_lfanew, SEEK_SET, &newpos)
        || PosReadFile(fhandle, firstbit, sizeof(firstbit), &readbytes)
        || (readbytes != sizeof(firstbit))
        || (memcmp(firstbit, "LX", 2) != 0))
    {
        return (1);
    }
    /* LX seems to be called LE sometimes. */
    printf("LX (other name LE) is not supported\n");

    return (2);
}

static int exeloadLoadNE(unsigned long *entry_point,
                         int fhandle,
                         unsigned long e_lfanew)
{
    unsigned char firstbit[2];
    long newpos;
    size_t readbytes;

    if (PosMoveFilePointer(fhandle, e_lfanew, SEEK_SET, &newpos)
        || PosReadFile(fhandle, firstbit, sizeof(firstbit), &readbytes)
        || (readbytes != sizeof(firstbit))
        || (memcmp(firstbit, "NE", 2) != 0))
    {
        return (1);
    }
    printf("NE is not supported\n");

    return (2);
}
