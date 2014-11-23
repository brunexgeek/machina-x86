#ifndef MACHINA_OS_ELF_H
#define MACHINA_OS_ELF_H

#define EI_NIDENT (16)

#define ELF32_R_SYM(val)        ((val) >> 8)
#define ELF32_R_TYPE(val)       ((val) & 0xff)

#define R_386_NONE          0    // No reloc
#define R_386_32            1    // Direct 32 bit
#define R_386_PC32          2    // PC relative 32 bit
#define R_386_GOT32         3    // 32 bit GOT entry
#define R_386_PLT32         4    // 32 bit PLT address
#define R_386_COPY          5    // Copy symbol at runtime
#define R_386_GLOB_DAT      6    // Create GOT entry
#define R_386_JMP_SLOT      7    // Create PLT entry
#define R_386_RELATIVE      8    // Adjust by program base
#define R_386_GOTOFF        9    // 32 bit offset to GOT
#define R_386_GOTPC         10   // 32 bit PC relative offset to GOT
#define R_386_32PLT         11
#define R_386_TLS_TPOFF     14   // Offset in static TLS block
#define R_386_TLS_IE        15   // Address of GOT entry for static TLS block offset
#define R_386_TLS_GOTIE     16   // GOT entry for static TLS block offset
#define R_386_TLS_LE        17   // Offset relative to static TLS block
#define R_386_TLS_GD        18   // Direct 32 bit for GNU version of general dynamic thread local data
#define R_386_TLS_LDM       19   // Direct 32 bit for GNU version of local dynamic thread local data in LE code
#define R_386_16            20
#define R_386_PC16          21
#define R_386_8             22
#define R_386_PC8           23
#define R_386_TLS_GD_32     24   // Direct 32 bit for general dynamic thread local data
#define R_386_TLS_GD_PUSH   25   // Tag for pushl in GD TLS code
#define R_386_TLS_GD_CALL   26   // Relocation for call to __tls_get_addr()
#define R_386_TLS_GD_POP    27   // Tag for popl in GD TLS code
#define R_386_TLS_LDM_32    28   // Direct 32 bit for local dynamic thread local data in LE code
#define R_386_TLS_LDM_PUSH  29   // Tag for pushl in LDM TLS code
#define R_386_TLS_LDM_CALL  30   // Relocation for call to __tls_get_addr() in LDM code
#define R_386_TLS_LDM_POP   31   // Tag for popl in LDM TLS code
#define R_386_TLS_LDO_32    32   // Offset relative to TLS block
#define R_386_TLS_IE_32     33   // GOT entry for negated static TLS block offset
#define R_386_TLS_LE_32     34   // Negated offset relative to static TLS block
#define R_386_TLS_DTPMOD32  35   // ID of module containing symbol
#define R_386_TLS_DTPOFF32  36   // Offset in TLS block
#define R_386_TLS_TPOFF32   37   // Negated offset in static TLS block
// 38?
#define R_386_TLS_GOTDESC   39   // GOT offset for TLS descriptor.
#define R_386_TLS_DESC_CALL 40   // Marker of call through TLS descriptor for relaxation.
#define R_386_TLS_DESC      41   // TLS descriptor containing pointer to code and to argument, returning the TLS offset for the symbol
#define R_386_IRELATIVE     42   // Adjust indirectly by program base
#define R_386_NUM           43   // Keep this the last entry.


//  Legal values for p_type (segment type).

#define PT_NULL         0           //  Program header table entry unused
#define PT_LOAD         1           //  Loadable program segment
#define PT_DYNAMIC      2           //  Dynamic linking information
#define PT_INTERP       3           //  Program interpreter
#define PT_NOTE         4           //  Auxiliary information
#define PT_SHLIB        5           //  Reserved
#define PT_PHDR         6           //  Entry for header table itself
#define PT_TLS          7           //  Thread-local storage segment
#define PT_NUM          8           //  Number of defined types
#define PT_LOOS         0x60000000  //  Start of OS-specific
#define PT_GNU_EH_FRAME 0x6474e550  //  GCC .eh_frame_hdr segment
#define PT_GNU_STACK    0x6474e551  //  Indicates stack executability
#define PT_GNU_RELRO    0x6474e552  //  Read-only after relocation
#define PT_LOSUNW       0x6ffffffa
#define PT_SUNWBSS      0x6ffffffa  //  Sun Specific segment
#define PT_SUNWSTACK    0x6ffffffb  //  Stack segment
#define PT_HISUNW       0x6fffffff
#define PT_HIOS         0x6fffffff  //  End of OS-specific
#define PT_LOPROC       0x70000000  //  Start of processor-specific
#define PT_HIPROC       0x7fffffff  //  End of processor-specific

//  Legal values for p_flags (segment flags).

#define PF_X            (1 << 0)    //  Segment is executable
#define PF_W            (1 << 1)    //  Segment is writable
#define PF_R            (1 << 2)    //  Segment is readable
#define PF_MASKOS       0x0ff00000  //  OS-specific
#define PF_MASKPROC     0xf0000000  //  Processor-specific


//  Legal values for sh_type (section type).

#define SHT_NULL      0     //  Section header table entry unused
#define SHT_PROGBITS      1     //  Program data
#define SHT_SYMTAB    2     //  Symbol table
#define SHT_STRTAB    3     //  String table
#define SHT_RELA      4     //  Relocation entries with addends
#define SHT_HASH      5     //  Symbol hash table
#define SHT_DYNAMIC   6     //  Dynamic linking information
#define SHT_NOTE      7     //  Notes
#define SHT_NOBITS    8     //  Program space with no data (bss)
#define SHT_REL       9     //  Relocation entries, no addends
#define SHT_SHLIB     10        //  Reserved
#define SHT_DYNSYM    11        //  Dynamic linker symbol table
#define SHT_INIT_ARRAY    14        //  Array of constructors
#define SHT_FINI_ARRAY    15        //  Array of destructors
#define SHT_PREINIT_ARRAY 16        //  Array of pre-constructors
#define SHT_GROUP     17        //  Section group
#define SHT_SYMTAB_SHNDX  18        //  Extended section indeces
#define SHT_NUM       19        //  Number of defined types.
#define SHT_LOOS      0x60000000    //  Start OS-specific.
#define SHT_GNU_ATTRIBUTES 0x6ffffff5   //  Object attributes.
#define SHT_GNU_HASH      0x6ffffff6    //  GNU-style hash table.
#define SHT_GNU_LIBLIST   0x6ffffff7    //  Prelink library list
#define SHT_CHECKSUM      0x6ffffff8    //  Checksum for DSO content.
#define SHT_LOSUNW    0x6ffffffa    //  Sun-specific low bound.
#define SHT_SUNW_move     0x6ffffffa
#define SHT_SUNW_COMDAT   0x6ffffffb
#define SHT_SUNW_syminfo  0x6ffffffc
#define SHT_GNU_verdef    0x6ffffffd    //  Version definition section.
#define SHT_GNU_verneed   0x6ffffffe    //  Version needs section.
#define SHT_GNU_versym    0x6fffffff    //  Version symbol table.
#define SHT_HISUNW    0x6fffffff    //  Sun-specific high bound.
#define SHT_HIOS      0x6fffffff    //  End OS-specific type
#define SHT_LOPROC    0x70000000    //  Start of processor-specific
#define SHT_HIPROC    0x7fffffff    //  End of processor-specific
#define SHT_LOUSER    0x80000000    //  Start of application-specific
#define SHT_HIUSER    0x8fffffff    //  End of application-specific

//  Legal values for sh_flags (section flags).

#define SHF_WRITE        (1 << 0)   //  Writable
#define SHF_ALLOC        (1 << 1)   //  Occupies memory during execution
#define SHF_EXECINSTR        (1 << 2)   //  Executable
#define SHF_MERGE        (1 << 4)   //  Might be merged
#define SHF_STRINGS      (1 << 5)   //  Contains nul-terminated strings
#define SHF_INFO_LINK        (1 << 6)   //  `sh_info' contains SHT index
#define SHF_LINK_ORDER       (1 << 7)   //  Preserve order after combining
#define SHF_OS_NONCONFORMING (1 << 8)   //  Non-standard OS specific handling                       required
#define SHF_GROUP        (1 << 9)   //  Section is member of a group.
#define SHF_TLS          (1 << 10)  //  Section hold thread-local data.
#define SHF_MASKOS       0x0ff00000 //  OS-specific.
#define SHF_MASKPROC         0xf0000000 //  Processor-specific
#define SHF_ORDERED      (1 << 30)  //  Special ordering requirement                       (Solaris).
#define SHF_EXCLUDE      (1 << 31)  //  Section is excluded unless                    referenced or allocated (Solaris).

// Legal values for e_type (ELF type)
#define ET_NONE         0       // Unknown type.
#define ET_REL          1       // Relocatable.
#define ET_EXEC         2       // Executable.
#define ET_DYN          3       // Shared object.
#define ET_CORE         4       // Core file.


// Legal values for e_machine (machine type)
#define EM_386          3       // Intel 386
#define EM_ARM          40      // ARM
#define EM_IA_64        50      // Intel IA-64 processor architecture
#define EM_X86_64       62      // AMD x86-64 architecture
#define EM_AARCH64      183     // ARM AArch64

#include <stdint.h>

typedef struct elf32_file_header_t
{
    uint8_t e_ident[EI_NIDENT]; // Magic number and other info
    uint16_t e_type;            // Object file type
    uint16_t e_machine;         // Machine architecture
    uint32_t e_version;         // Object file version
    uint32_t e_entry;           // Entry point virtual address
    uint32_t e_phoff;           // Program header table file offset
    uint32_t e_shoff;           // Section header table file offset
    uint32_t e_flags;           // Processor-specific flags
    uint16_t e_eh_size;         // ELF header size in bytes
    uint16_t e_phentsize;       // Program header table entry size
    uint16_t e_phnum;           // Program header table entry count
    uint16_t e_shentsize;       // Section header table entry size
    uint16_t e_shnum;           // Section header table entry count
    uint16_t e_shstrndx;        // Section header string table index
} elf32_file_header_t;


typedef struct elf32_sect_header_t
{
    uint32_t sh_name;        // Section name (string tbl index)
    uint32_t sh_type;        // Section type
    uint32_t sh_flags;       // Section flags
    uint32_t sh_addr;        // Section virtual addr at execution
    uint32_t sh_offset;      // Section file offset
    uint32_t sh_size;        // Section size in bytes
    uint32_t sh_link;        // Link to another section
    uint32_t sh_info;        // Additional section information
    uint32_t sh_addr_align;  // Section alignment
    uint32_t sh_entsize;     // Entry size if section holds table
} elf32_sect_header_t;


typedef struct elf32_symb_t
{
    uint32_t st_name;        // Symbol name (string tbl index)
    uint32_t st_value;       // Symbol value
    uint32_t st_size;        // Symbol size
    uint8_t st_info;         // Symbol type and binding
    uint8_t st_other;        // Symbol visibility
    uint16_t st_shndx;       // Section index
} elf32_symb_t;


typedef struct elf32_prog_header_t
{
    uint32_t p_type;        // Segment type
    uint32_t p_offset;      // Segment file offset
    uint32_t p_vaddr;       // Segment virtual address
    uint32_t p_paddr;       // Segment physical address
    uint32_t p_filesz;      // Segment size in file
    uint32_t p_memsz;       // Segment size in memory
    uint32_t p_flags;       // Segment flags
    uint32_t p_align;       // Segment alignment
} elf32_prog_header_t;


//  Relocation table entry without addend (in section of type SHT_REL).

typedef struct elf32_relc_t
{
    uint32_t r_offset;       // Address
    uint32_t r_info;         // Relocation type and symbol index
} elf32_relc_t;


#endif  // MACHINA_OS_ELF_H
