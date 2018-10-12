#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lib/commandlinereader.h"

#define SHELL_RL_BUFFSIZE 1024
#define MAZE_SOLVER "./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver"

/**
 * executes input files
 */
int shell_execute(char *token) {
    int status;
    char* args[] = { MAZE_SOLVER, token, NULL };
    pid_t pid;

    pid = fork();
    if(pid < 0) {
        //error handling
        puts("Failed to fork process");
        exit(1);
    }
    if( pid == 0 ) {
        puts("running child");
        execv(MAZE_SOLVER, args);
        exit(1);
    } else {
        // Parent process
        pid = wait(&status);
        puts("child done, resuming");
        /*do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));*/
    }

    //printf( "        token %s\n", token );
    return 0;
}

/**
 * reads arguments from shell
 * */
int shell_read(char **tokens) {
    if( (int)strcmp(tokens[0], "exit") == 0 ) {
        // exits
        return -1;
    }
    if( (int)strcmp(tokens[0], "run") == 0) {
        // reads
        if( tokens[1] != NULL ) {
            shell_execute(tokens[1]);
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

    return 0;
}

/**
 * loops through shell until exit
 */
void shell_loop(void) {;
    int status,
        buffSize = SHELL_RL_BUFFSIZE;
    char **tokens = malloc(buffSize * sizeof(char*)),
         *stringBuffer = malloc(sizeof(char) * buffSize);

    do {
        printf("> ");
        readLineArguments(tokens, buffSize, stringBuffer, buffSize);
        status = shell_read(tokens);
    } while (status > -1);
}
/**
* main
*/
int main(int argc, char** argv){
    //parseArgs(argc, (char** const)argv);
    shell_loop();
    return 0;
}