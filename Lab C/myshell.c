#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "LineParser.h"
#include <sys/wait.h>
#include <fcntl.h>
int flag = 0;
int newest = 0;
int oldest = 0;
int history_size = 0;
int available = 0; // to keep track of available spot for the history method
#define HISTLEN 20  //for the history mechanism
//part 3a
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
typedef struct process
{
    cmdLine *cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;
process *singleton_process_list = NULL;
char *history[HISTLEN];

void free_archive(char *history[])
{
    for (int i = 0; i < HISTLEN; i++)
    {
        if (history[i] != NULL)
        {
            free(history[i]);
            history[i] = NULL;
        }
    }
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{

    process *new_process = malloc(sizeof(process)); //allocating memory for new process
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = 1; //is this correct?/?
    new_process->next = NULL;
    if (*process_list == NULL)
    {
        *process_list = new_process;
        return;
    }
    process *current = *process_list;
    while ((current != NULL) && (current->next != NULL))
    {
        current = current->next;
    }
    current->next = new_process;
}
//task 3b
void updateProcessList(process **process_list)
{
    int exit_status;
    process *curr = *process_list;
    while (curr != NULL)
    {
        pid_t cur_id = waitpid(curr->pid, &exit_status, WNOHANG | WUNTRACED | WCONTINUED);
        if (cur_id > 0)
        {                                //not running and it does exist
            if (WIFSTOPPED(exit_status)) //suspend
                curr->status = SUSPENDED;
            if (WIFSIGNALED(exit_status) | WIFEXITED(exit_status))
                curr->status = TERMINATED; //was terminated
            if (WIFCONTINUED(exit_status))
                curr->status = RUNNING; //running or woken up
        }
        curr = curr->next;
    }
}
void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;
    while ((curr != NULL) && (curr->pid != pid))
    {
        curr = curr->next;
    }
    if (curr != NULL)
        curr->status = status;
}
void freeProcess(process *process) //helper fucntion to clear process data
{
    freeCmdLines(process->cmd); //freeing dynamic allocations
    free(process);
}
void freeProcessList(process *process_list)
{
    process *cur = process_list;
    if (cur != NULL)
    {
        freeProcessList(process_list->next);
        freeProcess(cur);
    }
}
void printProcessList(process **process_list)
{
    updateProcessList(process_list); //updating the process list
    char *status[] = {"TERMINATED", "SUSPENDED", "RUNNING"};
    process *previous = NULL;
    process *curr = *process_list;
    fprintf(stdout, "INDEX\t\tPID\t\tCOMMAND\t\tSTATUS\n");
    if (curr != NULL)
    {
        int index = 0;
        while (curr != NULL)
        {
            fprintf(stdout, "%d\t\t%d\t\t%s\t\t%s\n", index, curr->pid, curr->cmd->arguments[0], status[curr->status + 1]);
            if (curr->status == -1)
            { //process was freshly terminated
                if (previous != NULL)
                {
                    previous->next = curr->next;
                    freeProcess(curr);
                    curr = previous->next;
                }
                else
                { //previous is null so list head becomes the curr next
                    *process_list = curr->next;
                    freeProcess(curr);
                    curr = *process_list;
                }
            }
            else
            {
                previous = curr;
                curr = curr->next;
            }
            index++;
        }
    }
}
void execute(cmdLine *pCmdLine)
{

    pid_t pid = fork(); //task 1a
    if (pid < 0)
    {
        perror("cant create child process");
        freeCmdLines(pCmdLine);
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
            //updating process status
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
            //updating process status
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
            //updating process status
            printf("kill is over\n");
        }
        else
        {

            if (pCmdLine->inputRedirect != NULL) //task 2 redirection,input
            {

                int input_file_descriptor = open(pCmdLine->inputRedirect, O_RDONLY); //added permission
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

                int output_file_descriptor = open(pCmdLine->outputRedirect,  O_WRONLY | O_CREAT, 0777);
                if (output_file_descriptor == -1)
                {
                    fprintf(stderr, "error opening file for output redirection");
                    perror("open1");
                    freeCmdLines(pCmdLine);
                    _exit(EXIT_FAILURE);
                }
                int output_redirect_status = dup2(output_file_descriptor, 1);
                if (output_redirect_status == -1)
                {
                    fprintf(stderr, "error in redirecting output");
                    freeCmdLines(pCmdLine);
                    _exit(EXIT_FAILURE);
                }
            }
            printf("executing %s\n", pCmdLine->arguments[0]);
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) < 0)
                perror("error happened whilst executing");
        }
        freeCmdLines(pCmdLine);
        freeProcessList(singleton_process_list);
        free_archive(history);
        _exit(EXIT_SUCCESS);
    }
    if (pid > 0) //were in parent process
    {

        addProcess(&singleton_process_list, pCmdLine, pid);
        if (flag == 1)
            fprintf(stderr, "the pid is %d and were in parent  process executing command :%s", pid, pCmdLine->arguments[0]);
        int exit_status;
        if (pCmdLine->blocking)
        {
            if (waitpid(pid, &exit_status, 0) < 0) //task 1b
            {
                perror("error happened while waiting");
            }
            updateProcessStatus(singleton_process_list, pid, TERMINATED); //this should be added here?
        }
    }
    // freeCmdLines(pCmdLine);
}
void handle_IO_pipe(cmdLine *pCmdLine, int son_num) //handles lagal IO redirection in pipes
{

    //first lets handle the legal cases
    if (pCmdLine->inputRedirect != NULL&&(son_num==1)) //redirecting input of left side
    {

        int input_file_descriptor = open(pCmdLine->inputRedirect, O_RDONLY);
        if (input_file_descriptor == -1)
        {
            fprintf(stderr, "error opening file for output redirection");
            freeCmdLines(pCmdLine);
            _exit(EXIT_FAILURE);
        }
        int input_redirect_status = dup2(input_file_descriptor, 0);
        if (input_redirect_status == -1)
        {
            fprintf(stderr, "error in redirecting input");
            freeCmdLines(pCmdLine);
            _exit(EXIT_FAILURE);
        }
    }
    if (pCmdLine->next->outputRedirect != NULL&&(son_num==2)) //redirectiong output in right side
    {
        int output_file_descriptor = open(pCmdLine->next->outputRedirect, O_WRONLY | O_CREAT, 0777);
        if (output_file_descriptor == -1)
        {
            fprintf(stderr, "error opening file for output redirection\n");
            perror("open2");
            freeCmdLines(pCmdLine);
            _exit(EXIT_FAILURE);
        }
        int output_redirect_status = dup2(output_file_descriptor, 1);
        if (output_redirect_status == -1)
        {
            fprintf(stderr, "error in redirecting output");
            freeCmdLines(pCmdLine);
            _exit(EXIT_FAILURE);
        }
    }
}
//////start of lab c task 2
void execute_pipe(cmdLine *pCmdLine)
{
   // handle_IO_pipe(pCmdLine); //legal io redirection
    ///creating the first child process
    int child_id[2]; //for the wait part
    int file_descriptor[2];
    if (pipe(file_descriptor) < 0)
    {
        fprintf(stderr, "error ocured creating a pipe\n");
        freeCmdLines(pCmdLine);
        _exit(EXIT_FAILURE);
    }
    if (flag == 1)
        fprintf(stderr, "parent_process>forking...\n");
    pid_t pid1 = fork();
    if (pid1 < 0)
    { //fail
        fprintf(stderr, "creating first child failed\n");
        freeCmdLines(pCmdLine);
        _exit(EXIT_FAILURE);
    }
    if (pid1 == 0)
    { //child number 1

        close(1); //closing the standard output
        if (flag == 1)
            fprintf(stderr, "child1>redirecting stdout to the write end of the pipe...\n");
        dup(file_descriptor[1]);
        if (close(file_descriptor[1]) < 0)
        {
            fprintf(stderr, "error closing file\n");
        }
        handle_IO_pipe(pCmdLine,1);
        if (flag == 1)
            fprintf(stderr, "child1>going to execure command:%s...\n", pCmdLine->arguments[0]);
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    }
    if (pid1 > 0)
    { // we are in parent process
        addProcess(&singleton_process_list, pCmdLine, pid1);
        if (flag == 1)
            fprintf(stderr, "parent_process>created process with id : %d\n", pid1);
        child_id[0] = pid1;
        if (flag == 1)
            fprintf(stderr, "parent_process>closing the write end of the pipe...\n");
        if (close(file_descriptor[1]) < 0) //closing the write end
        {
            fprintf(stderr, "error closing file\n");
        }
    }
    ///creating the second child process
    if (flag == 1)
        fprintf(stderr, "parent_process>forking...\n");
    pid_t pid2 = fork();
    if (pid2 < 0)
    { //fail
        fprintf(stderr, "creating second child failed\n");
        freeCmdLines(pCmdLine);
        _exit(EXIT_FAILURE);
    }
    if (pid2 == 0)
    {             // we afre in child process
        close(0); //closing the standard input
        if (flag == 1)
            fprintf(stderr, "child2>redirecting stdin to the read end of the pipe...\n");
        dup(file_descriptor[0]);
        if (close(file_descriptor[0]) < 0)
        {
            fprintf(stderr, "error closing file\n");
        }
        handle_IO_pipe(pCmdLine,2);
        if (flag == 1)
            fprintf(stderr, "child2>going to execure command:%s...\n", pCmdLine->next->arguments[0]);
        execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);
    }
    if (pid2 > 0)
    { //we are in parent processl
        addProcess(&singleton_process_list, pCmdLine->next, pid2);
        if (flag == 1)
            fprintf(stderr, "parent_process>created process with id : %d\n", pid2);
        child_id[1] = pid2;
        if (flag == 1)
            fprintf(stderr, "parent_process>closing the read end of the pipe...\n");
        if (close(file_descriptor[0]) < 0) //closing the read end of the pipe
        {
            fprintf(stderr, "error closing file\n");
        }
    }
    int status;
    //waiting for child processes in the order they were created
    if (flag == 1)
        fprintf(stderr, "parent process> waiting for child procsses to terminate...\n");
    waitpid(child_id[0], &status, 0);
    waitpid(child_id[1], &status, 0);                                 // do i need to exit after this?
    updateProcessStatus(singleton_process_list, child_id[0], TERMINATED); //should those 2 be added here?
    updateProcessStatus(singleton_process_list, child_id[1], TERMINATED);
    // freeCmdLines(pCmdLine);
}
/////end of pipe execute
//Support for History mechanism
void update_archieve(char *history[], char *command)
{
    printf("the index of newest is %d\n",newest);
    printf("the index of oldest is %d\n",oldest);
    printf("command is %s",command);
    strcpy(history[newest], command);
    newest = (newest + 1) % HISTLEN;
    if (history_size == HISTLEN)
    {
        oldest = (oldest + 1) % HISTLEN;   
    } else {
        history_size++;
    }
}
//functions names arent the best
void print_archive(char *history[]) //skips the first element
{
    fprintf(stdout, "printing history list\n");
    for (int index = 0, i = oldest; i != newest || index < history_size; i= (i + 1) %HISTLEN, index++) 
    {
        fprintf(stdout, "%d) %s", index, history[i]);
    }
}
void two_exclamation(char *history[])
{
    struct cmdLine *c_line = parseCmdLines(history[newest - 1]); //is this of use?
                                                                 // c_line = parseCmdLines(history[newest - 1]);
    if (c_line->next != NULL)
    {
        execute_pipe(c_line);
    }
    else
    {
        execute(c_line);
    }
}
void fact_n(int n, char *history[])
{
    if (n < 1 || n > 20)
    {
        fprintf(stdout, "n is out pf range \n");
    }
    else
    {
        struct cmdLine *c_line = parseCmdLines(history[n - 1]); //is this of use?
        //c_line = parseCmdLines(history[n - 1]);                  //check for safety!
        if (c_line->next != NULL)
        {
            execute_pipe(c_line);
        }
        else
        {
            execute(c_line);
        }
    }
}
int main(int argc, char **argv)
{

    if (argc > 1)
    {
        if (strncmp("-d", argv[1], 2) == 0)
            flag = 1;
    }
    
    for (int i = 0; i < HISTLEN; i++)
    {
        history[i] = malloc(2048 * sizeof(char));
    }

    while (1)
    {
        int pipe = 0;
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
        struct cmdLine *c_line = parseCmdLines(user_input);
        //c_line = parseCmdLines(user_input);
        if (c_line->next != NULL)
            pipe = 1;
        if (pipe == 0)
        { //command is regular ie not pipe
            if (strncmp(c_line->arguments[0], "quit", 4) == 0)
            {

                freeCmdLines(c_line);
                freeProcessList(singleton_process_list);
                free_archive(history);
                break;
            }
            if (strncmp(c_line->arguments[0], "history", 7) == 0)
            {
                print_archive(history);
                continue;
            }
            if (strncmp(c_line->arguments[0], "!!", 2) == 0)
            {
                two_exclamation(history);
                continue;
            }

            else if (strncmp(c_line->arguments[0], "!", 1) == 0)
            { //check this f regarding !n
                int n = atoi(&c_line->arguments[0][1]);
                fact_n(n, history);
                continue;
            }

            if (strncmp(c_line->arguments[0], "cd", 2) == 0) //task 1c
            {
                if (chdir(c_line->arguments[1]) == -1)
                    fprintf(stderr, "error while changing working directory");
            }
            if (strncmp(c_line->arguments[0], "procs", 5) == 0) //3a
            {
                printProcessList(&singleton_process_list); //is this correct use of the parameter
                freeCmdLines(c_line);
            }
            else
            {
                update_archieve(history, user_input);
                execute(c_line);
            }
        }
        if (pipe == 1) //command is a pipeline
        {
            if (strncmp(c_line->arguments[0], "cd", 2) == 0) //this doesnt seem to be needed
            {
                if (chdir(c_line->arguments[1]) == -1)
                    fprintf(stderr, "error while changing working directory");
            }
            //dealing with illegal io redirection before calling execute
            if (c_line->outputRedirect != NULL)
            {
                fprintf(stderr, "trying to redirect the output of left side of pipeline!");
                freeCmdLines(c_line);
            }
            else if (c_line->next->inputRedirect != NULL)
            {
                fprintf(stderr, "trying to redirect the input of right side of pipeline!");
                freeCmdLines(c_line);
            }
            else
            {
                update_archieve(history, user_input);
                execute_pipe(c_line);
            }
        }

    } //while ends here
}