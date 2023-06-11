#include <stdio.h>
#include <elf.h>
#include "elf_file.h"

void
elf_write_header(FILE *stream)
{
        Elf32_Ehdr header = {};
        
        header.e_ident[EI_MAG0] = ELFMAG0;
        header.e_ident[EI_MAG1] = ELFMAG1;
        header.e_ident[EI_MAG2] = ELFMAG2;
        header.e_ident[EI_MAG3] = ELFMAG3;

        header.e_ident[EI_CLASS] = ELFCLASS32;
        header.e_ident[EI_DATA] = ELFDATA2LSB;
        header.e_ident[EI_VERSION] = EV_CURRENT;
        header.e_ident[EI_OSABI] = ELFOSABI_SYSV;
        header.e_type = ET_EXEC;
        header.e_machine = EM_X86_64;
        header.e_version = EV_CURRENT;

        header.e_entry = 0; // Generally, can be a constant?
        header.e_phoff = 0x40; // Program header right after ELF header.
        header.e_shoff = 0; // Should be computered considering the size of program.

        header.e_flags = 0;
        header.e_ehsize = 0x40;

        header.e_phentsize = 0x38;
        header.e_phnum = 0;

        header.e_shentsize = 0x40;
        header.e_shnum = 0; // Number of sections.

        header.e_shstrndx = 0;
}

void
elf_write_prg_hdr()
{
        Elf32_Phdr hdr = {};

        hdr.p_type = PT_LOAD;

        hdr.p_offset = 0;

        hdr.p_vaddr = 0;
        hdr.p_paddr = 0;

        hdr.p_filesz = 0;
        hdr.p_memsz = 0;

        hdr.p_flags = PF_X | PF_R;

        hdr.p_align = 0;
}

void
elf_write_exec(const code_t *code, FILE *elf_file)
{
        fwrite(const void *__restrict ptr, size_t size, size_t n, FILE *__restrict s);
}
