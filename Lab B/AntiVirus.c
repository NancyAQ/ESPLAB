#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printHex(unsigned char *buffer, int length, FILE *output)
{
    const unsigned char *buff = (const unsigned char *)buffer;
    for (int i = 0; i < length; i++)
    {
        fprintf(output, "%02X ", buff[i]);
    }
    fprintf(output, "\n");
}
typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

virus *readVirus(FILE *file)
{
    virus *virus = malloc(sizeof(*virus));
    if (!virus)
    {
        return NULL;
    }

    int x = fread(virus, sizeof(virus->SigSize) + sizeof(virus->virusName), 1, file);
    if (x != 1)
    {
        free(virus);
        return NULL;
    }

    virus->sig = malloc(virus->SigSize);
    if (!virus->sig)
    {
        free(virus);
        return NULL;
    }

    int y = fread(virus->sig, sizeof(unsigned char), virus->SigSize, file);
    if (y != virus->SigSize)
    {
        free(virus->sig);
        free(virus);
        return NULL;
    }

    return virus;
}

void printVirus(virus *virus, FILE *output)
{

    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Signature size: %d\n", virus->SigSize);
    fprintf(output, "signature: ");
    printHex(virus->sig, virus->SigSize, output);
    fprintf(output, "\n\n");
}
///////////////1b////////////////
typedef struct link
{
    struct link *nextVirus;
    virus *vir;
} link;

void list_print(link *virus_list, FILE *output)
{
    while (virus_list != NULL)
    {
        printVirus(virus_list->vir, output);
        virus_list = virus_list->nextVirus;
    }
}

link *list_append(link *virus_list, virus *data)
{
    link *new_virus = malloc(sizeof(link)); //allocating memory for virus
    new_virus->vir = data;
    new_virus->nextVirus = NULL;

    if (virus_list == NULL)
    { //creating new list if og list is null
        return new_virus;
    }
    else
    {
        link *current = virus_list;
        while (current->nextVirus != NULL)
        {
            current = current->nextVirus;
        }
        current->nextVirus = new_virus;
        return virus_list;
    }
}
//helper function to free virus date
void freeVirus(virus *virus)
{
    free(virus->sig); //freeing dynamic allocations
    free(virus);
}
void list_free(link *virus_list)
{
    if (virus_list != NULL)
    {
        freeVirus(virus_list->vir);
        list_free(virus_list->nextVirus);
        free(virus_list);
    }
}
///the main menu part
typedef struct menuOP
{
    struct fun_desc *options;
    int loaded;
    link *virus_list;
    char *file;
} menuOP;
typedef struct fun_desc
{
    char *name;
    void (*fun)(menuOP *);
} fun_desc;

//wrapper functions
void load_sig_wrapper(menuOP *menu)
{
    char file_name[100];
    fprintf(stdout, "enter signatures file name \n");
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; //so it excluders the new line when opening the file
    FILE *signatures;
    signatures = fopen(file_name, "rb");
    if (signatures == NULL)
    {
        fprintf(stderr, "cant open given file\n");
        return;
    }
    char magic_num[4];
    int x = fread(magic_num, sizeof(char), 4, signatures); //fide pointer is now 4 steps ahead
    if (x != 4 || memcmp(magic_num, "VISL", 4) != 0)
    {
        
        fprintf(stderr, "file not compatible with correct magic number\n");
        return;
    }
    virus *virus;
    while ((virus = readVirus(signatures)) != NULL)
    {
        menu->virus_list = list_append(menu->virus_list, virus);
    }
    fclose(signatures);
    menu->loaded = 1;
    return;
}

void print_sig_wrapper(menuOP *menu)
{
    if (menu->loaded == 1)
    {
        list_print(menu->virus_list, stdout);
    }
}
void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    link *virus = virus_list;
    while (virus != NULL)
    {
        unsigned int location = 0;
        while (location <= (size - (virus->vir->SigSize)))
        {
            if (memcmp(buffer + location, virus->vir->sig, virus->vir->SigSize) == 0)
            {
                fprintf(stdout, "virus detected at: \n location:%d\n hex location is:0x%x\n Virus name is:%s\n signatures size is:%d\n", location,location, virus->vir->virusName, virus->vir->SigSize);
            }
            location++;
        }

        virus = virus->nextVirus;
    }
}
void detect_virus_wrapper(menuOP *menu)
{
    char buffer[10000];
    FILE *arg = fopen(menu->file, "rb");
    unsigned int size = fread(buffer, sizeof(unsigned char), sizeof(buffer), arg);
    fclose(arg);
    detect_virus(buffer, size, menu->virus_list);

}
void neutralize_virus(char *file_Name, int signatue_offset)
{
    fprintf(stdout,"location is %d\n",signatue_offset);
    FILE *sus;
    sus = fopen(file_Name, "rb+");
    if (sus == NULL)
    {
        fprintf(stderr, "cant open file");
        return;
    }
    fseek(sus, signatue_offset, SEEK_SET);
    unsigned char ret[] = {0xC3}; //pointer to the data we want to replace the first 2 bytes
    fwrite(ret, sizeof(ret), 2, sus);
    fclose(sus);
}
void fix_file_wrapper(menuOP *menu) //finding locations and sending to the neut function
{
    char buffer[10000];
    FILE *arg = fopen(menu->file, "rb");
    unsigned int size = fread(buffer, sizeof(unsigned char), sizeof(buffer), arg);
    fclose(arg);
    link *virus = menu->virus_list;
    while (virus != NULL)
    {
        unsigned int location = 0;
        while (location <= (size - (virus->vir->SigSize)))
        {
            if (memcmp(buffer + location, virus->vir->sig, virus->vir->SigSize) == 0)
            {
                neutralize_virus(menu->file, location);
            }
            location++;
        }

        virus = virus->nextVirus;
    }
}
void quit_wrapper(menuOP *menu)
{
    list_free(menu->virus_list);
    exit(0);
}
struct fun_desc fun_menu[] = {{"Load signatures", load_sig_wrapper}, {"Print signatures", print_sig_wrapper}, {"Detect viruses", detect_virus_wrapper}, {"Fix file", fix_file_wrapper}, {"Quit", quit_wrapper}, {NULL, NULL}};
struct menuOP menuOG = {fun_menu, 0, NULL, NULL};

int main(int argc, char **argv)
{
    char user_input[100];
    int bound = (sizeof(fun_menu) / sizeof(struct fun_desc) - 1);
    printf("bound is %d",bound);
    while (1)

    {
        menuOG.file = argv[1]; //is this how the file name should be loaded??
        fprintf(stdout, "Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < bound; i++)
        {
            printf("%d)%s\n", i+1, fun_menu[i].name);
        }
        printf("option: ");
        if (fgets(user_input, 100, stdin) == NULL)
            break;
        int func_num;
        sscanf(user_input, "%d\n", &func_num);
        func_num=func_num-1;
        if (func_num < 0 || func_num > bound-1)
        {
            printf("Not within bounds\n");
            break;
        }
        printf("Within bounds\n");
        fun_menu[func_num].fun(&menuOG);

        printf("DONE.\n\n");
    }
}