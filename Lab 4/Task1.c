#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct
{
    int debug_mode; //changed to int
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    int display_mode; //added
} state;
typedef struct fun_desc
{
    char *name;
    void (*fun)(state *s);
} fun_desc;

//menu functions sstart here
void Toggle_Debug_Mode(state *s)
{
    if (s->debug_mode == 0)
    { //turn on the debug mode
        s->debug_mode = 1;
        fprintf(stdout, "Debug flag now On");
    }
    else if (s->debug_mode == 1)
    {
        s->debug_mode = 0;
        fprintf(stdout, "Debug flag now off");
    }
}
void Set_File_Name(state *s)
{
    fprintf(stdout, "Enter File Name:\n"); //user prompt
    char input[100];
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';
    strcpy(s->file_name, input);
    if (s->debug_mode == 1)
        fprintf(stderr, "Debug:file name set to %s", input);
}
void Set_Unit_Size(state *s)
{
    fprintf(stdout, "Enter File Size:\n"); //user prompt
    char input[100];
    int size;
    fgets(input, 100, stdin);
    sscanf(input, "%d", &size);
    if ((size == 1) || (size == 2) || (size == 4))
    {
        s->unit_size = size;
        if (s->debug_mode == 1)
            fprintf(stderr, "Debug:set size to %d\n", size);
    }
    else
    {
        printf("Provided Size Is Not Valid");
    }
}
//Task 1.a
void Load_Into_Memory(state *s)
{
    if (strcmp("", s->file_name) == 0)
    {
        printf("There's No File To Load Into Memory From");
        return;
    }
    FILE *file = fopen(s->file_name, "rb"); //changed atm to abc change back to s->file_name
    if (file == NULL)
    {
        printf("Couldnt open file");
        return;
    }
    char loc[100];
    char len[100];
    unsigned int location;
    int length;
    printf("Enter Hex Location:\n");
    fgets(loc, 100, stdin);
    printf("Enter length:\n");
    fgets(len, 100, stdin);
    sscanf(loc, "%x", &location);
    sscanf(len, "%d", &length);
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "File Name:%s\n Location:%x\nLength:%d\n", s->file_name, location, length);
    }
    fseek(file, location, SEEK_SET); //pointing to the offset of location
    if (fread(s->mem_buf, s->unit_size, length, file) < 0)
    {
        printf("could not copy into struct");
        return;
    }
    fclose(file);
}
void Toggle_Display_Mode(state *s)
{

    if (s->display_mode == 0)
    { //turn on the debug mode
        s->display_mode = 1;
        fprintf(stdout, "Display flag now on,hexadecimal representation");
    }
    else if (s->display_mode == 1)
    {
        s->display_mode = 0;
        fprintf(stdout, "Display flag now off,decimal representation");
    }
}
void Memory_Diplay(state *s)
{
    static char *formats_hex[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"}; //for hex format
    static char *formats_dec[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"}; //for dec format
    char *offset;
    int u;
    unsigned int addr;
    char u_input[100];
    char address[100];
    printf("Enter u units :\n");
    fgets(u_input, 100, stdin);
    printf("Enter Addresss:\n");
    fgets(address, 100, stdin);
    sscanf(address, "%x", &addr);
    if (addr == 0)
        offset = (char *)s->mem_buf;
    else
        offset = (char *)address;
    sscanf(u_input, "%d", &u);
    if (s->display_mode)
        fprintf(stdout, "Hexadecimal\n");
    else
        fprintf(stdout, "Decimal\n");
    //similar impl to print units
    char *end = offset + s->unit_size * u;
    while (offset < end)
    {
        //print ints
        int var = *((int *)(offset));
        if (s->display_mode == 0)
            fprintf(stdout, formats_dec[s->unit_size - 1], var);
        else if (s->display_mode == 1)
            fprintf(stdout, formats_hex[s->unit_size - 1], var);
        offset += s->unit_size;
    }
}
void Save_Into_File(state *s)
{
    unsigned int source_address;
    unsigned int target_location;
    int length;
    char address[100];
    char target[100];
    char length_input[100];
    //input
    printf("Enter source address:\n");
    fgets(address, 100, stdin);
    sscanf(address, "%x", &source_address);
    printf("Enter target location:\n");
    fgets(target, 100, stdin);
    sscanf(target, "%x", &target_location);
    printf("Enter length:\n");
    fgets(length_input, 100, stdin);
    sscanf(length_input, "%d", &length);
    void *offset;
    if (source_address == 0)
        offset = &(s->mem_buf);
    else
        offset = &(source_address);
    //end of input

    FILE *file = fopen(s->file_name, "r+");
    if (file == NULL)
    {
        if (s->debug_mode == 1)
        {
            fprintf(stderr, "couldnt open file in the save into file func\n");
        }
        fprintf(stdout, "file name iss %sl", s->file_name);
        return;
    }
    ///CHECK IF TARGET LCOATION IS GREATER THAN FILE NAME!
    long size=ftell(file);
    if(target_location>size){
        fprintf(stderr,"error, target lcoation iss bigger than size of file");
        return;
    }
  //  end of check
    fseek(file, target_location, SEEK_SET); //starting pos at target location
    fwrite(offset, s->unit_size, length, file);
    fclose(file);
}

void Memory_Modify(state *s)
{
    //input
    char location[100];
    char value[100];
    unsigned int loc;
    unsigned int val;

    printf("Enter location:\n");
    fgets(location, 100, stdin);
    sscanf(location, "%x", &loc);
    printf("Enter value:\n");
    fgets(value, 100, stdin);
    sscanf(value, "%x", &val);
    //end of input
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "location:%x\nvalue:%x\n", loc, val);
    }
    s->mem_buf[loc] = val;
}
void Quit(state *s)
{
    //no need to free memory since there are no dynamic allocations
    if (s->debug_mode == 1)
        fprintf(stderr, "Quitting");
    exit(0);
}
void modify_multiple(state *s)//added new code
{
    //input
    char location[100];
    char value[100];
    char amount_input[100];
    unsigned int loc;
    unsigned int val;
    int amount;
    printf("Enter location:\n");
    fgets(location, 100, stdin);
    sscanf(location, "%x", &loc);
    printf("Enter value:\n");
    fgets(value, 100, stdin);
    sscanf(value, "%x", &val);
    printf("Enter amount of units to be modified:\n");
    fgets(amount_input, 100, stdin);
    sscanf(amount_input, "%d", &amount);
    //end of input
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "location:%x\nvalue:%x\n", loc, val);
    }
    for (int i = 0; i < amount; i++)
    {
        s->mem_buf[loc+i*(s->unit_size)]=val;
    }
    
}

//menu function end here
struct fun_desc fun_menu[] = {{"Toggle Debug Mode", Toggle_Debug_Mode},
                              {"Set File Name", Set_File_Name},
                              {"Set Unit Size", Set_Unit_Size},
                              {"Load Into Memory", Load_Into_Memory},
                              {"Toggle Display Mode", Toggle_Display_Mode},
                              {"Memory Display", Memory_Diplay},
                              {"Save Into File", Save_Into_File},
                              {"Memory Modify", Memory_Modify},
                              {"Quit", Quit},
                              {"Modify Multiple", modify_multiple}, //this ones extra and needs to be removed

                              {NULL, NULL}};

int main(int argc, char **argv)
{
    char user_input[100];
    int bound = (sizeof(fun_menu) / sizeof(struct fun_desc) - 1);
    state *s = malloc(sizeof(state));
    s->debug_mode = 1;

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
        fun_menu[func_num].fun(s);
    }
}
