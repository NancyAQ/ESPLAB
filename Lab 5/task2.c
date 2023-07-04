#include <elf.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
/*--------------------------Macros-------------------------*/
/*-----------------------End Of Macros----------------------*/
 extern int startup();
/*--------------------------Auxiliary Functions-------------------------*/
long file_size(int fd)
{
    long size;
    size = lseek(fd, SEEK_SET, SEEK_END);
    lseek(fd, SEEK_SET, SEEK_SET);
    return size;
}
char *type_match(Elf32_Phdr *hdr)
{
    switch (hdr->p_type)
    {
    case PT_NULL:
        return "NULL";

    case PT_LOAD:
        return "LOAD";

    case PT_DYNAMIC:
        return "DYNAMIC";

    case PT_INTERP:
        return "INTERP";

    case PT_NOTE:
        return "NOTE";

    case PT_SHLIB:
        return "SHLIB";

    case PT_PHDR:
        return "PHDR";

    case PT_GNU_STACK:
        return "GNU_ST";

    case PT_LOPROC:
        return "LOPROC";

    case PT_HIPROC:
        return "HIPROC";

    case PT_ARM_EXIDX:
        return "ARM_EX";

    case PT_GNU_RELRO:
        return "GNU_RE";

    case PT_TLS:
        return "TLS";
    case PT_GNU_EH_FRAME:
        return "GNU_FR";

    default:
        return "Unknown";
    }
}
void prot_flags(Elf32_Phdr *hdr){
    /*this part is for protection*/
    if(hdr->p_flags&PF_X)
     fprintf(stdout,"x");
    if(hdr->p_flags&PF_R)
     fprintf(stdout,"r");
    if(hdr->p_flags&PF_W)
     fprintf(stdout,"w");
    fprintf(stdout,"\t");

}
void map_flags(Elf32_Phdr *hdr){ 
   switch(hdr->p_type)
   {
       case PT_LOAD:
        fprintf(stdout,"Sh_Priv\n");
       default:
        fprintf(stdout,"\n");
       break;
   }
}

/*--------------------------End Of Auxiliary Functions-------------------------*/
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    fprintf(stdout, "Type\t\tOffset\t\tVirtAddr\t\tPhysAddr\t\tFileSize\tMemSize\t\tFLG\tAlign\tPF\tPM\n");
    Elf32_Ehdr *hdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *p_hdr = (Elf32_Phdr *)((char *)map_start + (hdr->e_phoff));
    int i;
    for (i = 1; i <= hdr->e_phnum; i++)
    {
        func(p_hdr, arg); 
        p_hdr = (Elf32_Phdr *)((hdr->e_phentsize) + (char *)p_hdr);
    }
    return 0;
}
void minus_l(Elf32_Phdr *hdr, int i)
{
    int counter=0;
    fprintf(stdout, "%s\t\t0x%08x\t0x%08x\t\t0x%08x\t\t0x%08x\t0x%08x\t", type_match(hdr), hdr->p_offset, hdr->p_vaddr, hdr->p_paddr,
            hdr->p_filesz, hdr->p_memsz);
    if(hdr->p_flags&PF_R){
     fprintf(stdout,"R");
     counter++;
     }
    if(hdr->p_flags&PF_W){
     fprintf(stdout,"W");
     counter++;
    }
    if(hdr->p_flags&PF_X){
     fprintf(stdout,"E");
     counter++;
     }
    if(counter==0)
     fprintf(stdout,"None");
    fprintf(stdout,"\t");
    fprintf(stdout,"0x%02x\t",hdr->p_align);
    prot_flags(hdr);
    map_flags(hdr);

    
}

void test(Elf32_Phdr *hdr, int i)
{
    fprintf(stdout, "Program Header Number %d at address %x \n", i, hdr->p_vaddr);
}

void load_phdr(Elf32_Phdr *phdr,int fd){
    int counter=0;
    if(phdr->p_type!=PT_LOAD){
        fprintf(stderr,"Provided Program Headers Type Is Not Load");
        exit(1); 
    }
   
    /*Printing the current program header*/
    minus_l(phdr,counter);
    /*Printing ends here*/

   int protection=0;
    if(phdr->p_flags&PF_X)
      protection|=PROT_EXEC; 
    if(phdr->p_flags&PF_R)
      protection|=PROT_READ;
    if(phdr->p_flags&PF_W)
      protection|=PROT_WRITE;
      /*fixing points according to the reading material*/
    void *addr=(void*)(uintptr_t)(phdr->p_vaddr&0xfffff000); 
    off_t offset=phdr->p_offset&0xfffff000;
    size_t padding=phdr->p_vaddr&0xfff;
    void *mapping=mmap(addr,phdr->p_memsz+padding,protection,MAP_PRIVATE|MAP_FIXED,fd,offset);
    if(mapping==NULL){
       fprintf(stderr,"Mapping failed");
        exit(1);
    }
     counter++;
    
     
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "No File Name Provided In Arguments\n");
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Couldn't Open File\n");
        return 1;
    }
    long size = file_size(fd);
    void *map_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0); /*what if I only want to use system_call :no null and no prot?*/
    if (map_start == MAP_FAILED)
    {
        munmap(map_start, size);
        fprintf(stdout, "Mapping file %s failed \n", argv[1]);
        return 1;
    }
    foreach_phdr(map_start, load_phdr, fd); /*arg is now the fd of the open file*/
    startup(argc-1,argv+1,(void*)((Elf32_Ehdr *)map_start)->e_entry);
    return 0;
}
