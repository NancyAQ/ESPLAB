#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
//My global vars
char file_name[100]; //global file name
int debug_mode = 0;
void *mapping;
size_t size;
Elf32_Ehdr *elf_h;  //elf header
int file_count = 0; //to use in 3
int current_index = -1;
int valid_for_merge = -1;
//Struct for each mapped file
typedef struct loaded
{
    char file_name[100]; //global file name
    int debug_mode;
    int current_fd;
    void *mapping;
    size_t size;
    Elf32_Ehdr *elf_h;
    int sym_count;
    int sym_size;
    char *strtab;
    char *shstrtab;
} loaded;
//Array that holds 2 elements of struct
loaded *structArray;

typedef struct fun_desc
{
    char *name;
    void (*fun)();
} fun_desc;

//help functions start here
long f_size(int fd)
{
    off_t size;
    size = lseek(fd, SEEK_SET, SEEK_END); //getting the file size from start to finish
    lseek(fd, 0, SEEK_SET);               //going back to where we were(start of file)
    return size;
}
//help fucntions end here

//menu functions start here
void Toggle_Debug_Mode()
{
    if (structArray[current_index].debug_mode == 0)
    { //turn on the debug mode
        structArray[current_index].debug_mode = 1;
        fprintf(stdout, "Debug flag now On\n");
    }
    else if (structArray[current_index].debug_mode == 1)
    {
        structArray[current_index].debug_mode = 0;
        fprintf(stdout, "Debug flag now off\n");
    }
}
void data_encoding(Elf32_Ehdr *hdr)
{
    if (hdr->e_ident[5] == ELFDATA2MSB)
    {
        fprintf(stdout, "Data encoding:%s\n", "MSB");
    }
    else if (hdr->e_ident[5] == ELFDATA2LSB)
    {
        fprintf(stdout, "Data encoding:%s\n", "LSB");
    }
}
void print_examine_info(Elf32_Ehdr *hdr)
{
    Elf32_Addr e_point = structArray[current_index].elf_h->e_entry;
    fprintf(stdout, "%s entry's point is %x\n", file_name, e_point);
    fprintf(stdout, "Firt 3 bytes of file are %c %c %c\n", structArray[current_index].elf_h->e_ident[1], structArray[current_index].elf_h->e_ident[2], structArray[current_index].elf_h->e_ident[3]);
    data_encoding(hdr);
    fprintf(stdout, "Section Header File Offset : %x\n", hdr->e_shoff);
    fprintf(stdout, "Section Header Entries Number:%d\n", hdr->e_shnum);
    fprintf(stdout, "Section Header Entries Size:%d\n", hdr->e_shentsize);
    fprintf(stdout, "Section Header File Offset: %x\n", hdr->e_phoff);
    fprintf(stdout, "Program Header Entries Number:%d\n", hdr->e_phnum);
    fprintf(stdout, "Section Header Entries Size:%d\n", hdr->e_phentsize);
}
void Examine_ELF_File()
{
    current_index++;
    structArray[current_index].debug_mode = 1;
    if (current_index > 1)
    {
        fprintf(stderr, "More than 2 files are mapped!");
    }

    // if ((current_fd != 0) && (mapping != NULL))
    // {
    //     if (debug_mode == 1)
    //         fprintf(stderr, "closing file with file descriptor%d\n", current_fd);
    //     munmap(mapping, size);
    // }
    char input[100];
    fprintf(stdout, "Enter file name to examine:\n");
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    strcpy(structArray[current_index].file_name, input);
    if (structArray[current_index].debug_mode)
        fprintf(stderr, "File name received is: %s\n", file_name);
    int fd = open(structArray[current_index].file_name, O_RDONLY);
    if (fd < 0)
    {
        if (structArray[current_index].debug_mode == 1)
            fprintf(stderr, "opening file %s failed\n", structArray[current_index].file_name);
        return;
    }
    structArray[current_index].current_fd = fd;
    structArray[current_index].size = f_size(fd);
    structArray[current_index].mapping = mmap(NULL, structArray[current_index].size, PROT_READ, MAP_PRIVATE, structArray[current_index].current_fd, 0);
    if (structArray[current_index].mapping == MAP_FAILED)
    {
        if (structArray[current_index].debug_mode == 1)
        {
            fprintf(stderr, "mapping file %s failed", structArray[current_index].file_name);
        }
        return;
    }
    file_count++;

    structArray[current_index].elf_h = (Elf32_Ehdr *)structArray[current_index].mapping;
    if ((structArray[current_index].elf_h->e_ident[1] != 'E') || (structArray[current_index].elf_h->e_ident[2] != 'L') || (structArray[current_index].elf_h->e_ident[3] != 'F'))
    {
        if (structArray[current_index].debug_mode == 1)
        {
            fprintf(stderr, "File with file name %s isnt of elf format", structArray[current_index].file_name);
            return;
        }
    }

    print_examine_info(structArray[current_index].elf_h);
}
void Print_Section_Names()
{
    fprintf(stdout, "index\tsection_name\t\tsection_address\t\tsection_offset\t\tsection_size\t\tsection_type\n");

    if (structArray[current_index].current_fd == 0)
    {
        fprintf(stderr, "No available file mapped");
        return;
    }
    int s_num = structArray[current_index].elf_h->e_shnum;
    Elf32_Shdr *sect = (Elf32_Shdr *)(structArray[current_index].elf_h->e_shoff + (char *)structArray[current_index].mapping);
    Elf32_Shdr *shstrtab = &sect[structArray[current_index].elf_h->e_shstrndx];
    char *str_p = (char *)structArray[current_index].mapping + shstrtab->sh_offset;
    structArray[current_index].shstrtab=str_p;
    for (int i = 0; i < s_num; i++)
    {
        char *name = str_p + sect[i].sh_name;
        fprintf(stdout, "%d\t%s\t\t\t\t%x\t\t\t%x\t\t%d\t\t%d\n", i, name, sect[i].sh_addr, sect[i].sh_offset, sect[i].sh_size, sect[i].sh_type);
    }
}
void Print_Symbols()
{
    fprintf(stdout, "index\tvalue\tsection_index\tsection_name\tsymbol_name\n");
    if (structArray[current_index].current_fd == 0)
    {
        fprintf(stderr, "No available file mapped");
        return;
    }
    structArray[current_index].sym_count = 0;
    int s_num = structArray[current_index].elf_h->e_shnum;
    Elf32_Shdr *sect = (Elf32_Shdr *)(structArray[current_index].elf_h->e_shoff + (char *)structArray[current_index].mapping);
    Elf32_Shdr *shstrtab = &sect[structArray[current_index].elf_h->e_shstrndx];
    char *str_p = (char *)structArray[current_index].mapping + shstrtab->sh_offset;
    for (int i = 0; i < s_num; i++)
    {
        // char *section_name = str_p + sect[i].sh_name;
        if (sect[i].sh_type == SHT_SYMTAB)
        { //if section is a symbols table
            structArray[current_index].sym_count++;
            Elf32_Sym *sym = (Elf32_Sym *)((char *)structArray[current_index].mapping + sect[i].sh_offset);
            Elf32_Shdr *strtab_h = &sect[sect[i].sh_link];
            char *sym_p = (char *)structArray[current_index].mapping + strtab_h->sh_offset;
            int symbol_count = sect[i].sh_size / sizeof(Elf32_Sym); //calculating number of symbols
            for (int k = 0; k < symbol_count; k++)
            {
                char *name = sym[k].st_name + sym_p;
                if (sym[k].st_shndx == SHN_ABS)
                {
                    fprintf(stdout, "%d\t%x\t\tABS\t%s\t\t%s\n", k, sym[k].st_value, "", name);
                }
                else if (sym[k].st_shndx == SHN_UNDEF)
                {
                    fprintf(stdout, "%d\t%x\t\tUND\t%s\t\t%s\n", k, sym[k].st_value, "", name);
                }
                else
                {
                    fprintf(stdout, "%d\t%x\t\t%d\t%s\t\t%s\n", k, sym[k].st_value, sym[k].st_shndx, str_p + sect[sym[k].st_shndx].sh_name, name);
                }
            }
        }
    }
}
Elf32_Sym *get_sym_tab(int i) // help fucntion to get a pointer to the symbols table
{
    structArray[i].sym_size = 0;
    Elf32_Sym *sym = NULL;
    Elf32_Shdr *sect = (Elf32_Shdr *)(structArray[i].elf_h->e_shoff + (char *)structArray[i].mapping);
    for (int j = 0; j < structArray[i].elf_h->e_shnum; j++)
    {
        if (sect[j].sh_type == SHT_SYMTAB)
        {
            sym = (Elf32_Sym *)((char *)structArray[i].mapping + sect[j].sh_offset);
            structArray[i].sym_size = sect[j].sh_size / sizeof(Elf32_Sym);
            Elf32_Shdr *st_sect = &sect[sect[j].sh_link];
            structArray[i].strtab = (char *)(structArray[i].mapping + st_sect->sh_offset);
        }
    }
    return sym;
}
int equal_symbols(Elf32_Sym *sym1, Elf32_Sym *sym2)
{
    char *name1 = (char *)(sym1->st_name + structArray[0].strtab);
    char *name2 = (char *)(sym2->st_name + structArray[1].strtab);

    if (((strcmp(name1, name2) == 0) && (strcmp(name1, "") != 0)))
    {
        return 1;
    }
    return 0;
}
void Check_files_for_merge()
{
    if (file_count != 2)
    {
        fprintf(stderr, "You Need 2 Files To Merge!\n");
        valid_for_merge = 0;
        return;
    }
    if (structArray[0].sym_count != 1 || structArray[1].sym_count != 1)
    {
        fprintf(stderr, "Feature Not Supported!\n");
        valid_for_merge = 0;
        return;
    }
    valid_for_merge = 1;
    Elf32_Sym *SYMTAB1 = get_sym_tab(0);
    Elf32_Sym *SYMTAB2 = get_sym_tab(1);
    for (int i = 1; i < structArray[0].sym_size; i++)
    {
        if (SYMTAB1[i].st_shndx == SHN_UNDEF)
        {
            int found1 = 0;
            for (int j = 1; j < structArray[1].sym_size; j++)
            {
                if (equal_symbols(&SYMTAB1[i], &SYMTAB2[j]))
                {
                    found1 = j;
                }
            }
            if ((found1 == 0) || (found1 > 0 && SYMTAB2[found1].st_shndx == SHN_UNDEF))
            {
                fprintf(stderr, "Symbol Sym Undefined\n");
            }
        }
        else if (SYMTAB1[i].st_shndx != SHN_UNDEF)
        {
            int found2 = 0;
            for (int j = 1; j < structArray[1].sym_size; j++)
            {
                if (equal_symbols(&SYMTAB1[i], &SYMTAB2[j]))
                {

                    found2 = j;
                    break;
                }
            }
            if ((found2 > 0) && (SYMTAB2[found2].st_shndx != SHN_UNDEF))
            {
                fprintf(stderr, "Symbol Sym Multiply Defined\n");
            }
        }
    }
}

void Merge_elf_files()
{
    Check_files_for_merge();
    if (valid_for_merge != 1)
    {
        fprintf(stderr, "Merge isnt possible");
        return;
    }
    int fd = open("out.ro", O_RDWR | O_TRUNC | O_CREAT,0644);
    //prep vars
    //int pos = 0;
    Elf32_Ehdr *hdr1 = (Elf32_Ehdr *)structArray[0].mapping;
    Elf32_Ehdr *hdr2 = (Elf32_Ehdr *)structArray[1].mapping;
    Elf32_Shdr *section_hdr1 = (Elf32_Shdr *)((char *)structArray[0].mapping + hdr1->e_shoff);
    Elf32_Shdr *section_hdr2 = (Elf32_Shdr *)((char *)structArray[1].mapping + hdr2->e_shoff);
    Elf32_Ehdr new_hdr;
    Elf32_Shdr *new_section_hdr = (Elf32_Shdr *)malloc(hdr1->e_shnum * sizeof(Elf32_Shdr));
    memcpy(&new_hdr, hdr1, sizeof(Elf32_Ehdr));
    memcpy(new_section_hdr, section_hdr1, hdr1->e_shnum * sizeof(Elf32_Shdr));
    write(fd, &new_hdr, sizeof(Elf32_Ehdr));
    //pos = pos + sizeof(Elf32_Ehdr);
    for (int i = 1; i < hdr1->e_shnum; i++)
    {
        new_section_hdr[i].sh_offset = lseek(fd, 0, SEEK_CUR);
        char *sect_name1 = structArray[0].shstrtab+ section_hdr1[i].sh_name;
        //new_section_hdr[i].sh_offset = pos;
        //fprintf(stderr,"%d) %d , %s %s\n",i, section_hdr1[i].sh_name, structArray[0].shstrtab+1, sect_name1);
        if ((strcmp(sect_name1, ".rodata") == 0) || (strcmp(sect_name1, ".text") == 0) || (strcmp(sect_name1, ".data") == 0))
        {
            printf("found %s\n", sect_name1);
            write(fd, (char *)structArray[0].mapping + section_hdr1[i].sh_offset, section_hdr1[i].sh_size);
            //pos = pos + section_hdr1[i].sh_size;
            for (int j = 0; j < hdr2->e_shnum; j++)
            {
                char *sect_name2 = (char *)structArray[1].shstrtab + section_hdr2[j].sh_name;
                if (strcmp(sect_name1, sect_name2) == 0)
                {
                    write(fd, (char *)structArray[1].mapping + section_hdr2[j].sh_offset, section_hdr2[j].sh_size);
                    //pos = pos + section_hdr2[j].sh_size;
                    new_section_hdr[i].sh_size += section_hdr2[j].sh_size;
                    printf("New size = %d\n", new_section_hdr[i].sh_size);
                }
            }
        }
        else if (strcmp(sect_name1, ".symtab") == 0)
        {
            //taking care of the sym-tables
            Elf32_Sym *sym_tab1 = get_sym_tab(0);
            Elf32_Sym *sym_tab2 = get_sym_tab(1);
            int sym_count1 = structArray[0].sym_size;
            int sym_count2 = structArray[1].sym_size;
            Elf32_Sym *new_sym_tab = (Elf32_Sym *)malloc(sym_count1 * sizeof(Elf32_Sym));
            memcpy(new_sym_tab, sym_tab1, sym_count1 * sizeof(Elf32_Sym));
            for (int i = 1; i < sym_count1; i++)
            {
                for (int j = 1; j < sym_count2; j++)
                {
                    char *name1 = (char *)(structArray[0].strtab + sym_tab1[i].st_name);
                    char *name2 = (char *)(structArray[1].strtab + sym_tab2[j].st_name);
                    //fprintf(stderr, "%s %s\n", name1, name2);
                    if (strcmp(name1, name2) == 0)
                    {
                        if ((sym_tab1[i].st_shndx == SHN_UNDEF) && (j < sym_count1))
                        {
                            char *sect_name2 = (char *)structArray[1].shstrtab + section_hdr2[sym_tab2[j].st_shndx].sh_name;
                            printf("Searching for %s\n", sect_name2);
                            for (int k = 1; k < hdr1->e_shnum; k++)
                            {
                                char *sect_name11 = structArray[0].shstrtab+ section_hdr1[k].sh_name;
                                if (strcmp(sect_name11, sect_name2) == 0)
                                {
                                    printf("Found at %d\n", k);
                                    new_sym_tab[i].st_shndx = k;
                                }
                            }
                            
                            new_sym_tab[i].st_value = sym_tab2[j].st_value + section_hdr1[j].sh_size;
                            new_sym_tab[i].st_size = sym_tab2[j].st_size;
                            new_sym_tab[i].st_info = sym_tab2[j].st_info;
                            new_sym_tab[i].st_other = sym_tab2[j].st_other;
                            break;
                        }
                    }
                }
            }
            write(fd,(char*)new_sym_tab,sym_count1 * sizeof(Elf32_Sym));
        }
        else
        {
            write(fd, (char *)structArray[0].mapping + section_hdr1[i].sh_offset, section_hdr1[i].sh_size);
        }
    }

    new_hdr.e_shoff = lseek(fd, 0, SEEK_CUR);
    write(fd, new_section_hdr, hdr1->e_shnum * sizeof(Elf32_Shdr));
    lseek(fd, 0, SEEK_SET);
    write(fd, &new_hdr, sizeof(Elf32_Ehdr));
    fprintf(stderr, "closing the fd");
    close(fd);
}
void Quit()
{
    for (int i = 0; i < file_count; i++)
    {
        munmap(structArray[i].mapping, structArray[i].size);
        close(structArray[i].current_fd);
    }
    exit(0);
}

//menu function end here

struct fun_desc fun_menu[] = {{"Toggle Debug Mode", Toggle_Debug_Mode},
                              {"Examine ELF File", Examine_ELF_File},
                              {"Print Section Names", Print_Section_Names},
                              {"Print Symbols", Print_Symbols},
                              {"Check Files For Merge", Check_files_for_merge},
                              {"Merge ELF Files", Merge_elf_files},
                              {"Quit", Quit},
                              {NULL, NULL}};

int main(int argc, char **argv)
{
    structArray = malloc(2 * sizeof(loaded));
    char user_input[100];
    int bound = (sizeof(fun_menu) / sizeof(struct fun_desc) - 1);
    while (1)

    {
        fprintf(stdout, "Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < bound; i++)
        {
            printf("%d)%s\n", i, fun_menu[i].name);
        }
        printf("option: ");
        if (fgets(user_input, 100, stdin) == NULL)
            break;
        int func_num;
        sscanf(user_input, "%d\n", &func_num);
        func_num = func_num;
        if (func_num < 0 || func_num > bound - 1)
        {
            printf("Not within bounds\n");
            break;
        }
        printf("Within bounds\n");
        fun_menu[func_num].fun();
    }
}
