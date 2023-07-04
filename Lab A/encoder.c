#include <stdio.h>
#include <string.h> //from linux man page
int main(int argc, char **argv)
{
    int encoder = 0;
    int on = 0;
    char* key;
    //READ FROM FILE AND OUTPUT USING STDIN AS START(Part 3)+Part 1 the debugger
    FILE *input = stdin;
    FILE *output = stdout;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0)
        {
            on = 0;
        }
        if (on == 1)
        {
            fprintf(stderr, "%s\n", argv[i]);
        }
        if (strcmp(argv[i], "+D") == 0)
            on = 1;
        if (argv[i][0] == '-')
        {
            //case 1
            if (argv[i][1] == 'i')
            {
                input = fopen(argv[i] + 2, "r+");
                if (input == NULL){
                    fprintf(stderr, "Couldn't open file %s\n", argv[i]+2);
                    return 0;
                }
            }
            //case 2
            if (argv[i][1] == 'o')
            {
                    output = fopen(argv[i] + 2, "w");
                    if (output == NULL){
                        fprintf(stderr, "Couldn't open file %s\n", argv[i]+2);
                        return 0;
                    }
            
            }
        }
        if (strncmp("+e", argv[i], 2) == 0)
        { //case 1, encoder
            key = argv[i]+2;
            encoder = 1;
        }
        if (strncmp("-e", argv[i], 2) == 0)
        { //case 2, decoder
            key = argv[i]+2;
            encoder = -1;
        }
    }
    char ch = 0;
    int index = 0;
    if(encoder == 1){
    ////////////////////////////////part 2
        while ((ch = fgetc(input)) != EOF)
        {
            int notLegal = 0;
            if (key[index] == '\0')
            {
                index = 0;
            }
            if ((ch > 64) & (ch < 91))
            { //upper
                notLegal = 1;
                fputc(((ch + key[index] - 48 - 65) % 26) + 65, output);
            }
            if ((ch < 123) && (ch > 96))
            { //lower
                notLegal = 1;
                fputc(((ch + key[index] - 48 - 97) % 26) + 97, output);
            }
            else if ((ch > 47) && (ch < 58))
            { //numbers
                notLegal = 1;
                fputc((((ch + key[index] - 48) - 48) % 10) + 48, output);
            }
            else if (notLegal == 0) //ch is neither a number or a part of the alphabet
            {
                fputc(ch, output);
            }

            index = index + 1;
        }
    } else if(encoder == -1){
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (strncmp("-e", key, 2) == 0)
        { //case 2, decoder
            short ch = 0;
            int index = 2;
            while ((ch = fgetc(input)) != EOF)
            {
                int notLegal = 0;
                if (key[index] == '\0')
                {
                    index = 0;
                }
                if ((ch > 64) & (ch < 91))
                { //upper
                    notLegal = 1;
                    fputc(((ch + 26 - (key[index] - 48) - 65) % 26) + 65, output);
                }
                if ((ch < 123) && (ch > 96))
                { //lower
                    notLegal = 1;
                    fputc(((ch + 26 - (key[index] - 48) - 97) % 26) + 97, output);
                }
                else if ((ch > 47) && (ch < 58))
                { //numbers
                    notLegal = 1;
                    fputc((((ch + 10 - (key[index] - 48)) - 48) % 10) + 48, output);
                }
                else if (notLegal == 0) //ch is neither a number or a part of the alphabet
                {
                    fputc(ch, output);
                }

                index = index + 1;
            }
        }
    } else {
        while ((ch = fgetc(input)) != EOF)
            fputc(ch, output);
    }
    fclose(input);
    fclose(output);
}
