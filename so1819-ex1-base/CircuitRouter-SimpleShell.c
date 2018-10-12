#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lib/commandlinereader.h"

#define SHELL_RL_BUFFSIZE 1024
#define MAZE_SOLVER "./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver"

long int global_max_children = 32;
long int global_processes_runnning = 0;

void free_tokens(char **tokens) {
    for( int i = 0; i < sizeof(tokens) - 1; i++ ) {
        if( tokens[i] != NULL ) free(tokens[i]);
    }
    free(tokens);
}

/**
 * executes input files
 */
int shell_execute(char *token) {
    int status;
    char* args[] = { MAZE_SOLVER, token, NULL };
    pid_t pid, wpid;

    if( execv(MAZE_SOLVER, args) == -1 ) {
        return -1;
    } else {
        return 0;
    }

    /*pid = fork();
    if(pid < 0) {
        //error handling
        puts("Failed to fork process");
        exit(1);
    }
    if( pid == 0 ) {
        puts("running child");
        sleep(2);
        execv(MAZE_SOLVER, args);
        exit(1);
    } else {
        // Parent process
        puts("waiting");
        pid = wait(&status);
        puts("child done, resuming");
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }*/
}

/**
 * reads arguments from shell
 * */
int shell_read(char **tokens) {
    int status = 0;

    if( (int)strcmp(tokens[0], "exit") == 0 ) {
        // exits
        return -1;
    }
    if( (int)strcmp(tokens[0], "MAXCHILDREN") == 0 ) {
        // sets maxchildren
        global_max_children = atol(tokens[1]);
    } else if( (int)strcmp(tokens[0], "run") == 0) {
        // reads
        if( tokens[1] != NULL ) {
            //status = shell_execute(tokens[1]);
            status = 1;
        } else {
            puts("No file passed to run");
        }
    } else if( (int)strcmp(tokens[0], "help") == 0 ) {
        // helps
        puts("  Available commands:");
        puts("    run <input_file> - runs input file");
        puts("    exit - exits shell");
    } else {
        // others
        puts("Invalid command");
    }

    return status;
}

/**
 * loops through shell until exit
 */
void shell_loop(void) {;
    int readStatus = 0,
        status,
        buffSize = SHELL_RL_BUFFSIZE;
    char **tokens = malloc(buffSize * sizeof(char*)),
         *stringBuffer = malloc(sizeof(char) * buffSize);
    pid_t pid, pids, wpid;
    
    //do {
    while(readStatus > -1) {
        readLineArguments(tokens, buffSize, stringBuffer, buffSize);
        readStatus = shell_read(tokens);

        if( readStatus == 1) {
            // there's an input file to run
            if( global_processes_runnning < global_max_children ) {
                // there're less processes running than total allowed
            }
        }

        if ( (pid = fork()) == 0) {
            printf("process: %d starting\n", pid);
            // Child process
            global_processes_runnning++;
            printf("processes running %d\n", global_processes_runnning);
            sleep(5);
            puts("ends child 1 sleep");
            if( readStatus == 1 ) {
                shell_execute(tokens[1]);
            }
            exit(EXIT_FAILURE);
        } else if( (pids = fork()) == 0 ) {
            // second
            printf("processes running %d\n", global_processes_runnning);
            sleep(2);
            puts("child 2 end");
        } else {
            // Parent process
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            printf("process: %d ending\n", pid);
        }
    }

    free_tokens(tokens);
}
/**
* main
*/
int main(int argc, char** argv){
    //parseArgs(argc, (char** const)argv);
    shell_loop();
    return 0;
}