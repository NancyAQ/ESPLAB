#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc, char **argv)
{
    ///creating the first child process
    int child_id[2]; //for the wait part
    int file_descriptor[2];
    if (pipe(file_descriptor) < 0)
    {
        fprintf(stderr, "error ocured creating a pipe\n");
        _exit(EXIT_FAILURE);
    }
    fprintf(stderr, "parent_process>forking...\n");
    pid_t pid1 = fork();
    if (pid1 < 0)
    { //fail
        fprintf(stderr, "creating first child failed\n");
        _exit(EXIT_FAILURE);
    }
    if (pid1 == 0)
    { //child number 1
        // fprintf(stderr, "parent process >closing the write end of the pipe...\n");
        close(1); //closing the standard output
        fprintf(stderr, "child1>redirecting stdout to the write end of the pipe...\n");
        dup(file_descriptor[1]);
        if (close(file_descriptor[1]) < 0)
        {
            fprintf(stderr, "error closing file\n");
        }
        char *args1[] = {"ls", "-l", NULL};
        fprintf(stderr, "child1>going to execure command:%s...\n", args1[0]);
        execvp(args1[0], args1);
    }
    if (pid1 > 0)
    { // we are in parent process
        fprintf(stderr, "parent_process>created process with id : %d\n", pid1);
        child_id[0] = pid1;
         fprintf(stderr, "parent_process>closing the write end of the pipe...\n");
        if (close(file_descriptor[1]) < 0) //closing the write end
        {
            fprintf(stderr, "error closing file\n");
        }
    }
    ///creating the second child process
    fprintf(stderr, "parent_process>forking...\n");
    pid_t pid2 = fork();
    if (pid2 < 0)
    { //fail
        fprintf(stderr, "creating second child failed\n");
        _exit(EXIT_FAILURE);
    }
    if (pid2 == 0)
    {             // we afre in child process
        close(0); //closing the standard input
        fprintf(stderr, "child2>redirecting stdin to the read end of the pipe...\n");
        dup(file_descriptor[0]);
        if (close(file_descriptor[0]) < 0)
        {
            fprintf(stderr, "error closing file\n");
        }
        char *args2[] = {"tail", "-2", NULL};
        fprintf(stderr, "child2>going to execure command:%s...\n", args2[0]);
        execvp(args2[0], args2);
    }
    if (pid2 > 0)
    { //we are in parent processl
        fprintf(stderr, "parent_process>created process with id : %d\n", pid2);
        child_id[1] = pid2;
        fprintf(stderr, "parent_process>closing the read end of the pipe...\n");
        if (close(file_descriptor[0]) < 0) //closing the read end of the pipe
        {
            fprintf(stderr, "error closing file\n");
        }
    }
    int status;
    //waiting for child processes in the order they were created
    fprintf(stderr, "parent process> waiting for child procsses to terminate...\n");
    waitpid(child_id[0], &status, 0);
    waitpid(child_id[1], &status, 0);
    fprintf(stderr, "parent process>exiting...\n");
    _exit(EXIT_SUCCESS); //is this the correct way of exiting??
}