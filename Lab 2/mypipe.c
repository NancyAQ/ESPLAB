#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char **argv)
{

    pid_t pid;
    int pipe_sides[2];
    if (pipe(pipe_sides) == -1)
    {
        fprintf(stderr, "creating pipe failed");
        _exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid < 0)
    { //fail
        fprintf(stderr, "fork failed");
        _exit(EXIT_FAILURE);
    }
    if (pid == 0)
    { //were in child
        char message[] = "hello";
        close(pipe_sides[0]);                               //read side
        write(pipe_sides[1], message,sizeof(message)); 
        close(pipe_sides[1]);                               //write side
    }
    if (pid > 0) //were in parent
    {
        close(pipe_sides[1]); //write side
        char message_received[100];
        read(pipe_sides[0], message_received, 2048); //use sizeof??
        printf("message recived from child is : %s\n", message_received);
        close(pipe_sides[0]); //read side
    }
}