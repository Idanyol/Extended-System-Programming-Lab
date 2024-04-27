#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

extern int startup(int argc, char **argv, void (*start)());

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg) {
    Elf32_Ehdr *start = (Elf32_Ehdr *) map_start;
    Elf32_Phdr *header = (Elf32_Phdr *)(map_start + start->e_phoff);
    
    int hnum = start->e_phnum;

    printf("Type\tOffset\tVirtual Address\tPhysical Address\tFile Size\tMemory Size\tFlags\tAlignment\tProtection Flags\tMapping Flags\n");

    for (int i=0 ; i<hnum ; i++) {
        func(&header[i], arg);
    }

    return 0;
}

void foo(Elf32_Phdr* ptr, int i){
    printf("Program header number %d at address %p\n", i, ptr);
}

char* getType (Elf32_Word h_type){
    char* type="";
    switch (h_type) {
        case PT_NULL:
            type="NULL";
            break;
        case PT_LOAD:
            type = "LOAD";
            break;
        case PT_DYNAMIC:
            type = "DYNAMIC";
            break;
        case PT_INTERP:
            type = "INTERP";
            break;
        case PT_NOTE:
            type = "NOTE";
            break;
        case PT_SHLIB:
            type = "SHLIB";
            break;
        case PT_PHDR:
            type = "PHDR";
            break;
        default:
            break;
    }
    return type;
}

int getProtFlags(Elf32_Phdr* header) {
    int protFlags = 0;
    if (header->p_flags & PF_R) 
        protFlags |= PROT_READ;
    if (header->p_flags & PF_W) 
        protFlags |= PROT_WRITE;
    if (header->p_flags & PF_X) 
        protFlags |= PROT_EXEC;
    return protFlags;
}

void readelfl(Elf32_Phdr* header) {
    char* type = getType(header->p_type);
    Elf32_Off offset = header->p_offset;
    Elf32_Addr vaddr = header->p_vaddr;
    Elf32_Addr paddr = header->p_paddr;
    Elf32_Word files = header->p_filesz;
    Elf32_Word  mems = header->p_memsz;
    Elf32_Word  align = header->p_align;
    int protFlags = getProtFlags(header);
    char* mapFlags = "MAP_PRIVATE";
    

    printf("%s\t0x%x\t0x%x\t0x%x\t\t0x%x\t\t0x%x\t\t%c%c%c\t0x%x\t\t%d\t\t\t%s\n", 
        type, offset, vaddr, paddr, files, mems, 
        (header->p_flags & PF_R) ? 'R' : '-',
        (header->p_flags & PF_W) ? 'W' : '-',
        (header->p_flags & PF_X) ? 'E' : '-', 
        align,
        protFlags,
        mapFlags);
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    if (phdr->p_type == PT_LOAD) {
        if (mmap((void *)((phdr->p_vaddr)&0xfffff000), (phdr->p_memsz)+(phdr->p_vaddr&0xfff), getProtFlags(phdr), MAP_FIXED | MAP_PRIVATE, fd, (phdr->p_offset)&0xfffff000) == MAP_FAILED) {
            printf("Error mapping");
            exit(1);
        }
        readelfl(phdr);
    }
}

int main(int argc, char **argv){
    int fd = open(argv[1],O_RDWR);
    
    if (fd == -1) {
        printf("Error opening the file\n");
        return 1;
    }

    off_t fsize = lseek(fd, 0, SEEK_END);
    if (fsize == -1) {
        printf("Error seeking the end of file\n");
        return 1;
    }

    void* map_start = mmap(NULL, fsize, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (map_start == NULL) {
        printf("Error mapping the file\n");
        return 1;
    }

    foreach_phdr(map_start,load_phdr, fd);
    startup(argc-1, argv+1, (void *)(((Elf32_Ehdr *)map_start)->e_entry));
    return 0;
}