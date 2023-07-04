#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// functions from task 2
char my_get(char c)
{

    char c1 = (fgetc(stdin));
    return c1;
}
char cprt(char c)
{
    if ((c >= 0x20) && (c <= 0x7E))
        printf("%c\n", c);
    else
        printf(".\n");

    return c;
}
char encrypt(char c)
{
    if ((c >= 0x20) && (c <= 0x7E))
        return c + 1;
    return c;
}
char decrypt(char c)
{
    if ((c >= 0x20) && (c <= 0x7E))
        return c - 1;
    return c;
}
char xprt(char c)
{
    if ((c >= 0x20) && (c <= 0x7E))
        printf("%x\n", c);
    else
        printf(".\n");
    return c;
}
char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = f(array[i]);
    }
    free(array);
    return mapped_array;
}
struct fun_desc
{
    char *name;
    char (*fun)(char);
};
struct fun_desc menu[] = {{"Get String", my_get}, {"Print String", cprt}, {"Print Hex", xprt}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {NULL, NULL}};

int main(int argc, char **argv)

{

    char user_input[100];
    int bound = (sizeof(menu) / sizeof(struct fun_desc) - 1);
    char *carray = calloc(5, sizeof(char));
    while (1)

    {
        fprintf(stdout, "Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < bound; i++)
        {
            printf("%d)%s\n", i, menu[i].name);
        }
        printf("option:\n");
        if (fgets(user_input, 100, stdin) == NULL)
            break;
        int func_num;
        sscanf(user_input, "%d\n", &func_num);
        if (func_num < 0 || func_num > bound - 1)
        {
            printf("Not within bounds\n");
            break; 
        }
        printf("Within bounds\n");
        carray = map(carray, 5, menu[func_num].fun);

        printf("carray = %s\n", carray);
        printf("DONE.\n\n");
    }
    free(carray);
}