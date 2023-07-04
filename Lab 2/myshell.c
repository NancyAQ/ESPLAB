#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "LineParser.h"
#include <sys/wait.h>
#include <fcntl.h>
int flag = 0;
void execute(cmdLine *pCmdLine)
{

    pid_t pid = fork(); //task 1a
    if (pid < 0)
    {
        perror("cant create child process");
        _exit(EXIT_FAILURE);
    }
    if (pid == 0)
    { //were in child process
        //task 1a
        if (flag == 1)
            fprintf(stderr, "the pid is %d and were in child process executing command :%s", pid, pCmdLine->arguments[0]);
        if (strncmp(pCmdLine->arguments[0], "suspend", 6) == 0) //task 3
        {
            int process_id;
            sscanf(pCmdLine->arguments[1], "%d", &process_id);
            if (kill(process_id, SIGTSTP) == -1)
            {
                fprintf(stderr, "cant suspend process");
            }
            printf("suspended\n");
        }
        else if (strncmp(pCmdLine->arguments[0], "wake", 4) == 0)
        {
            int process_id;
            sscanf(pCmdLine->arguments[1], "%d", &process_id);
            if (kill(process_id, SIGCONT) == -1)
            {
                fprintf(stderr, "cant wake up process");
            }
            printf("woke up\n");
        }
        else if (strncmp(pCmdLine->arguments[0], "kill", 4) == 0)
        {
            int process_id;
            sscanf(pCmdLine->arguments[1], "%d", &process_id);
            if (kill(process_id, SIGINT) == -1)
            {
                fprintf(stderr, "cant kill process");
            }
            printf("kill is over\n");
        }
        else
        {

            if (pCmdLine->inputRedirect != NULL) //task 2 redirection,input
            {

                int input_file_descriptor = open(pCmdLine->inputRedirect, O_RDONLY);
                if (input_file_descriptor == -1)
                {
                    fprintf(stderr, "error opening file for output redirection");
                    _exit(EXIT_FAILURE);
                }
                int input_redirect_status = dup2(input_file_descriptor, 0);
                if (input_redirect_status == -1)
                {
                    fprintf(stderr, "error in redirecting input");
                    _exit(EXIT_FAILURE);
                }
            }
            if (pCmdLine->outputRedirect != NULL) //task 2 redirection, output
            {

                int output_file_descriptor = open(pCmdLine->outputRedirect,O_RDONLY|O_WRONLY|O_CREAT,0666);
                if (output_file_descriptor == -1)
                {
                    fprintf(stderr, "error opening file for output redirection");
                    _exit(EXIT_FAILURE);
                }
                int output_redirect_status = dup2(output_file_descriptor, 1);
                if (output_redirect_status == -1)
                {
                    fprintf(stderr, "error in redirecting output");
                    _exit(EXIT_FAILURE);
                }
            }

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
                perror("error happened whilst executing");
        }
        _exit(EXIT_FAILURE);
    }
    if (pid > 0) //were in parent process
    {
        if(flag==1)
        fprintf(stderr, "the pid is %d and were in parent  process executing command :%s", pid, pCmdLine->arguments[0]);
        int exit_status;
        if (pCmdLine->blocking)
        {
            if (waitpid(pid, &exit_status, 0) < 0) //task 1b
            {
                perror("error ha[pened while waiting");
            }
        }
    }
    freeCmdLines(pCmdLine);
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        if (strncmp("-d", argv[1], 2) == 0)
            flag = 1;
    }
    while (1)
    {

        char current_path[PATH_MAX];
        if (getcwd(current_path, sizeof(current_path)) == NULL)
        {
            perror("cant get current path");
        }
        else
        {
            printf("%s $ ", current_path);
        }
        char user_input[2048];
        if (fgets(user_input, sizeof(user_input), stdin) == NULL)
        {
            fprintf(stdout, "\n");
            break;
        }
        if (strcmp(user_input, "\n") == 0)
        {
            continue;
        }
        struct cmdLine *c_line = malloc(sizeof(struct cmdLine));
        c_line = parseCmdLines(user_input);
        if (strncmp(c_line->arguments[0], "quit", 4) == 0)
        {
            freeCmdLines(c_line);
            break;
        }
        if (strncmp(c_line->arguments[0], "cd", 2) == 0) //task 1c
        {
            if (chdir(c_line->arguments[1]) == -1)
                fprintf(stderr, "error while changing working directory");
        } 
        execute(c_line);

    } //while ends here
}