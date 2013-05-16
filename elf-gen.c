#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "elf-gen.h"

void elf_gen (const char const* code)
{
    const long page_size = sysconf (_SC_PAGESIZE);
    Elf64_Ehdr e;
    /* code */
    Elf64_Phdr p;
    /* stack */
    Elf64_Phdr s;

    memset (&e, 0, sizeof (e));
    memset (&p, 0, sizeof (p));
    memset (&s, 0, sizeof (s));

    e.e_ident [EI_MAG0] = ELFMAG0;
    e.e_ident [EI_MAG1] = ELFMAG1;
    e.e_ident [EI_MAG2] = ELFMAG2;
    e.e_ident [EI_MAG3] = ELFMAG3;
    e.e_ident [EI_CLASS] = ELFCLASS64;
    e.e_ident [EI_DATA] = ELFDATA2LSB;
    e.e_ident [EI_VERSION] = EV_CURRENT;
    e.e_ident [EI_OSABI] = ELFOSABI_LINUX;
    e.e_ident [EI_NIDENT] = EI_NIDENT;

    e.e_type = ET_EXEC;
    e.e_machine = EM_X86_64;
    e.e_version = EV_CURRENT;
    e.e_ehsize = sizeof (e);
    e.e_phentsize = sizeof (p);
    e.e_phnum = 1;
    e.e_entry = 0x400000;
    e.e_phoff = sizeof (e);

    /* code */
    p.p_type = PT_LOAD;
    p.p_offset = page_size;
    p.p_vaddr = 0x400000;
    p.p_filesz = page_size;
    p.p_memsz = page_size;
    p.p_flags = ( PF_X | PF_R );

    /* stack */
    s.p_type = PT_GNU_STACK;
    s.p_flags = ( PF_W | PF_R );

    assert (page_size >= (sizeof (e) + sizeof (p) + sizeof (s)));

    /* bad! */
    const int fd = open ("elf.out", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IXUSR);
    write (fd, &e, sizeof (e));
    write (fd, &p, sizeof (p));
    write (fd, &s, sizeof (s));

    lseek (fd, page_size, SEEK_SET);
    write (fd, code, page_size);
    close (fd);
}
