#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int counter(char* input){
    int count=0;
    int i=0;
    while (*(input+i)!='\0')
    {
       if((*(input+i)>='0')&&(*(input+i)<='9'))
        count++;
       i++;
    }
    return count;
    
}
int main(int argc, char **argv)
{
   if(argc>1)
   printf( "Number of digits is %d \n",counter(argv[1]));
   else
    printf("theres no input");   
}
