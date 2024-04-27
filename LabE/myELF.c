#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>


typedef struct {
  char debug_mode;
  char display_mode;
  char* file_name[2];
  int fd[2];
  void* map_start[2]; 
  struct stat stat[2];
} state;

struct fun_desc {
    char *name;
    void (*fun)(state *);
};

void toggleDebugMode(state* s) {
    if (s->debug_mode == '0'){
        s->debug_mode = '1'; 
        printf ("Debug flag now on\n");
    }
    else
    {
        s->debug_mode = '0'; 
        printf("Debug flag now off\n");
    }
} 

void mapELF (int file, state *s, char fileName[]){

    s->fd[file] = open(fileName, O_RDWR);
    if (s->fd[file] == -1){
        printf ("Error opening file\n");
        return;
    }
    if (fstat(s->fd[file],&(s->stat[file])) == -1){
        printf("Error getin stat\n");
        close(s->fd[file]);
        s->fd[file] = -1; 
        return;
    }
    if (((s->map_start[file]) = mmap(0, s->stat[file].st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, s->fd[file], 0)) == MAP_FAILED){
        printf("Error mmap fail\n");
        close(s->fd[file]);
        s->fd[file] = -1; 
        return; 
    }

    Elf32_Ehdr* start = (Elf32_Ehdr*) s->map_start[file]; 
    if (strncmp((char*)start->e_ident, (char*)ELFMAG, 4) != 0 ){
        printf("NOT an ELF file\n");
        munmap(s->map_start[file], s->stat[file].st_size); //UNMAP
        close(s->fd[file]);
        s->fd[file] = -1; 
    }
    else {
        printf("Magic number: %x %x %x %x\n", start->e_ident[EI_MAG0], start->e_ident[EI_MAG1], start->e_ident[EI_MAG2], start->e_ident[EI_MAG3]);
        printf("Data: ");
        if (start->e_ident[5] == ELFDATANONE) {
            printf("invalid data\n");
        } else if (start->e_ident[5] == ELFDATA2LSB) {
            printf("little endian\n");
        } else if (start->e_ident[5] == ELFDATA2MSB) {
            printf("big endian\n");
        }
        printf("Entry point address: 0x%x\n", start->e_entry);
        printf("File offset to section header table: %d (bytes into file)\n", start->e_shoff);
        printf("Number of section headers entries: %d\n", start->e_shnum);
        printf("Size of a section header: %d (bytes)\n", start->e_shentsize);
        printf("File offset to program headers table: %d (bytes into file)\n", start->e_phoff);
        printf("Number of program headers entries: %d\n", start->e_phnum);
        printf("Size of a program header: %d (bytes)\n", start->e_phentsize);

        s->file_name[file] = strdup(fileName); 
        if(s->debug_mode == '1') 
            fprintf(stderr, "Debug: file name set to %s\n", s->file_name[file]);
    }
    
} 

void ExamineFile(state* s) {
    char fileName[128]; 
    printf("Enter File Name\n");

    if (s->file_name[0] == NULL){
        scanf( "%s", fileName);
        getchar();
        mapELF (0, s, fileName);        
    }
    else if( s->file_name[1] == NULL){
        fscanf(stdin, "%s", fileName);
        getchar();
        mapELF (1, s, fileName);
    }
    else{
        printf("Too many files\n");
    }
}

char* sectionType(int t){
    char* type="";
    switch (t) {
        case PT_NULL:
            type="NULL";
            break;
        case SHT_DYNAMIC:
            type = "DYNAMIC";
            break;
        case SHT_DYNSYM:
            type = "DYNSYM";
            break;
        case SHT_FINI_ARRAY:
            type = "FINI_ARRAY";
            break;
        case SHT_HASH:
            type = "HASH";
            break;
        case SHT_HIPROC:
            type = "HIPROC";
            break;
        case SHT_HIUSER:
            type = "HIUSER";
            break;
        case SHT_INIT_ARRAY:
            type = "INIT_ARRAY";
            break;
        case SHT_LOPROC:
            type = "LOPROC";
            break;
        case SHT_LOUSER:
            type = "LOUSER";
            break;
        case SHT_NOBITS:
            type = "NOBITS";
            break;
        case SHT_NOTE:
            type = "NOTE";
            break;
        case SHT_PREINIT_ARRAY:
            type = "PREINIT_ARRAY";
            break;
        case SHT_PROGBITS:
            type = "PROGBITS";
            break;
        case SHT_REL:
            type = "REL";
            break;
        case SHT_RELA:
            type = "RELA";
            break;
        case SHT_SHLIB:
            type = "SHLIB";
            break;
        case SHT_STRTAB:
            type = "STRTAB";
            break;
        case SHT_SYMTAB:
            type = "SYMTAB";
            break;       
        default:
            break;
    }
    return type;
}

void printSN (state *s , int file){
    Elf32_Ehdr *start = (Elf32_Ehdr *) s->map_start[file]; 
    Elf32_Shdr* shstrtab_section_header = (Elf32_Shdr *)(s->map_start[file] + start->e_shoff + (start->e_shstrndx * start->e_shentsize));
    
    int snum = start->e_shnum;
    printf("File: %s\n", s->file_name[file]);
    printf("Index\tSection name\t\tSection address\t\tSection offset\tSection size\tSection type\n");

    for (int i=0 ; i < snum ; i++) {
        Elf32_Shdr* currSec = (Elf32_Shdr *)(s->map_start[file] + start->e_shoff + (i * start->e_shentsize)); //pointer to the current section
        char *section_name = (char *)(s->map_start[file] + shstrtab_section_header->sh_offset + currSec->sh_name);
        char* secType = sectionType(currSec->sh_type);
        printf("[%2d]\t%-10s\t\t0x%-8x\t\t0x%-8x\t%-8x\t%s\n", i, section_name, currSec->sh_addr, currSec->sh_offset, currSec->sh_size, secType);
        
    }
}

void PSectionName(state* s) {
    if (s->fd[0] != -1)
        printSN(s, 0);
    if (s->fd[1] != -1)
        printSN(s, 1);    
}

void printSymbol(state *s , int file) {

    Elf32_Ehdr *start = (Elf32_Ehdr *) s->map_start[file]; 
    Elf32_Shdr* sections = (Elf32_Shdr *)(s->map_start[file] + start->e_shoff);
    Elf32_Shdr* shstrtab_section_header = (Elf32_Shdr *)(s->map_start[file] + start->e_shoff + (start->e_shstrndx * start->e_shentsize));

    printf("File: %s\n", s->file_name[file]);
    printf("Index\tValue\t\tsection_index\tsection_name\tsymbol_name\n");
    // Find symbol table section
    for (int i = 0; i < start->e_shnum; i++) {
        if (sections[i].sh_type == SHT_SYMTAB || sections[i].sh_type == SHT_DYNSYM) {
            Elf32_Sym* symbol_table = (Elf32_Sym *)((char *)s->map_start[file] + sections[i].sh_offset);
            char *strtab = (char*) (s->map_start[file] + sections[sections[i].sh_link].sh_offset);
            int symbol_count = sections[i].sh_size / sections[i].sh_entsize;

            for (int j = 0; j < symbol_count; j++) {

                const char *symbol_name = strtab + symbol_table[j].st_name;  //  a pointer to its name in the string table
                Elf32_Shdr * section_header = &sections[symbol_table[j].st_shndx];
                const char *section_name;

                if (symbol_table[j].st_shndx == SHN_ABS) { // means the symbol has an absolute value that is not affected by relocation.
                    section_name = "ABS";  
                    // strcpy(section_k, "ABS");
                }
                else if (symbol_table[j].st_shndx == SHN_UNDEF) { // means the symbol is undefined.
                    section_name = "UND";
                    // strcpy(section_k, "UND");
                }
                else {
                    section_name = (char *)(s->map_start[file] + shstrtab_section_header->sh_offset + section_header->sh_name);
                    // sprintf(section_k, "%d", symbol_table[j].st_shndx);
                }                
                printf("[%2d]\t%-8x\t%d\t\t%-10s\t%-50s\n", j, symbol_table[j].st_value, symbol_table[j].st_shndx, section_name, symbol_name);
            }
        }
    }   
}

void PSymbols(state* s) {
    if (s->fd[0] != -1)
        printSymbol(s, 0);
    if (s->fd[1] != -1)
        printSymbol(s, 0);
}

// func returns 1 - if symbol is found in SYMTAB2 and not defined,
            //  2 - if symbol is found in SYMTAB2 and is defined,
            //  3 - symbol is not found in SYMTAB2
int symbolSearch(char *symbolName1, Elf32_Sym* SYMTAB2, int symCounter2, char *strtab2) {
    for (int i = 1 ; i < symCounter2 ; i++) {
        char* symbolName2 = strtab2 + SYMTAB2[i].st_name;
        if (strcmp(symbolName1, symbolName2) == 0) { //symbol found in SYMTAB2
            if (SYMTAB2[i].st_shndx == SHN_UNDEF) { 
                return 1;
            }
            else {
                return 2;
            }
        }
    }
    return 3;
}

void checkFileMerge(state* s) {
    if (s->fd[0] == -1 || s->fd[1] == -1) {
        printf("Didn't find 2 ELF files\n");
        return;
    }

    int symtabCounter1 = 0, symtabCounter2 = 0;
    Elf32_Ehdr *start1 = (Elf32_Ehdr *) s->map_start[0]; 
    Elf32_Ehdr *start2 = (Elf32_Ehdr *) s->map_start[1]; 
    Elf32_Shdr* sections1 = (Elf32_Shdr *)(s->map_start[0] + start1->e_shoff);
    Elf32_Shdr* sections2 = (Elf32_Shdr *)(s->map_start[1] + start2->e_shoff);
    
    Elf32_Sym *SYMTAB1, *SYMTAB2;
    int symCounter1 = 0,  symCounter2 = 0;
    char *strtab1, *strtab2;
    // count symbol table sections in file1
    for (int i = 0; i < start1->e_shnum; i++) {
        if (sections1[i].sh_type == SHT_SYMTAB || sections1[i].sh_type == SHT_DYNSYM) {
            SYMTAB1 = (Elf32_Sym *)((char *)s->map_start[0] + sections1[i].sh_offset);
            symCounter1 = sections1[i].sh_size / sections1[i].sh_entsize;
            strtab1 = s->map_start[0] + sections1[sections1[i].sh_link].sh_offset;
            symtabCounter1++;
        }
    }

    // count symbol table sections in file2 (only if file1 has exactly one symtab)
    for (int i = 0; symtabCounter1 == 1 && i < start2->e_shnum; i++) {
        if (sections2[i].sh_type == SHT_SYMTAB || sections2[i].sh_type == SHT_DYNSYM) {
            SYMTAB2 = (Elf32_Sym*)((char *)s->map_start[1] + sections2[i].sh_offset);
            symCounter2 = sections2[i].sh_size / sections2[i].sh_entsize;
            strtab2 = s->map_start[1] + sections2[sections2[i].sh_link].sh_offset;
            symtabCounter2++;
        }
    }

    if (symtabCounter1 != 1 || symtabCounter2 != 1){
        printf("feature not supported\n");
        return;
    }

    for (int i = 1 ; i < symCounter1 ; i++) {
        char *symbolName1 = strtab1 + SYMTAB1[i].st_name;
        if (strcmp(symbolName1, "") != 0) {
            int isfoundinSYMTAB2 = symbolSearch(symbolName1, SYMTAB2, symCounter2, strtab2);
            if (SYMTAB1[i].st_shndx == SHN_UNDEF) {
                if (isfoundinSYMTAB2 == 1 || isfoundinSYMTAB2 == 3)
                    printf("Symbol %s undefined\n", symbolName1);
            }
            else {
                if (isfoundinSYMTAB2 == 2) {
                    printf("Symbol %s multiply defined\n", symbolName1);
                }
            }   
        }
    }
}

void mergeFiles(state* s) {
    printf("Not Implemented");
}

void quit(state* s) {
    free(s);
    exit(0);
}

int main(int argc, char **argv){

    state *s = (state *)(malloc(sizeof(state)));
    s->fd[0] = -1;
    s->fd[1] = -1;
    s->debug_mode = '1';

    struct fun_desc menu[] = {{"Toggle Debug Mode", toggleDebugMode}, {"Examine ELF File", ExamineFile}, {"Print Section Names", PSectionName}, {"Print Symbols", PSymbols}, {"Check Files for Merge",checkFileMerge}, {"Merge ELF Files",mergeFiles},  {"Quit", quit}, {NULL, NULL}};
    int bound = sizeof(menu) / sizeof(menu[0]) - 1;
    while (1) {
        //if debug is on - print 
        if(s->debug_mode == '1'){
            printf ("Debug: file name is %s\n", s->file_name[0]);
            printf ("Debug: file name is %s\n", s->file_name[1]);
        }

        //print menu
        printf("%s", "Select operation from the following menu: (ctrl^D for exit)\n");
        for (int i = 0 ; i < bound && menu[i].name != NULL ; i++) {
            printf("%i) %s\n", i, menu[i].name);
        }
        printf("Option : ");
        
        char buffer[1024];
        char* pointer = fgets(buffer, sizeof(buffer), stdin);

        if (pointer == NULL) { 
            break;
        }

        int option = atoi(buffer);

        if (option < 0 || option > bound - 1) {
            printf("Not within bounds\n");
            break;    
        }
        else
        {
            printf("Within bounds\n");
            menu[option].fun(s);
            printf("DONE.\n\n");
        }
    }
    
    return 0;
}
