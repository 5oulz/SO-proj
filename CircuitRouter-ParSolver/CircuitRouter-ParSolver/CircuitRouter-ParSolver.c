/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "parhelper.h"
#include "lib/timer.h"
#include "lib/types.h"

enum param_types {
    PARAM_BENDCOST = (unsigned char)'b',
    PARAM_XCOST    = (unsigned char)'x',
    PARAM_YCOST    = (unsigned char)'y',
    PARAM_ZCOST    = (unsigned char)'z',
};

enum param_defaults {
    PARAM_DEFAULT_BENDCOST = 1,
    PARAM_DEFAULT_XCOST    = 1,
    PARAM_DEFAULT_YCOST    = 1,
    PARAM_DEFAULT_ZCOST    = 2,
};

bool_t global_doPrint = TRUE;
char* global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */
long int global_numthreads = 0;


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
    printf("Usage: %s [options] input_filename\n", appName);
    puts("\nOptions:                            (defaults)\n");
    printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
    printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
    printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
    printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
    printf("    h          [h]elp message       (false)\n");
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
    global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
    global_params[PARAM_XCOST]    = PARAM_DEFAULT_XCOST;
    global_params[PARAM_YCOST]    = PARAM_DEFAULT_YCOST;
    global_params[PARAM_ZCOST]    = PARAM_DEFAULT_ZCOST;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]){
    long opt;

    opterr = 0;

    setDefaultParams();

    while ((opt = getopt(argc, argv, "hb:x:y:z:t:")) != -1) {
        switch (opt) {
            case 'b':
            case 'x':
            case 'y':
            case 'z':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case '?':
            case 't':
                global_numthreads = atol(optarg);
                break;
            case 'h':
                displayUsage(argv[0]);
            default:
                break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Missing input file\n");
        displayUsage(argv[0]);
    }

    global_inputFile = argv[optind];
}

/* =============================================================================
 * outputFile
 * =============================================================================
 */
FILE * outputFile() {
    FILE *fp;

    char result_outputFile[strlen(global_inputFile) + strlen(".res") + 1];
    sprintf(result_outputFile, "%s.res", global_inputFile);

    if (access(result_outputFile, F_OK) == 0) {
        char old_outputFile[strlen(global_inputFile) + strlen(".res.old") + 1];
        sprintf(old_outputFile, "%s.res.old", global_inputFile);
        if (rename(result_outputFile, old_outputFile) == -1) {
            perror("Error renaming output file");
            exit(EXIT_FAILURE);;
        }
    }
    fp = fopen(result_outputFile, "wt");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void* contaThreads(void* number) {
    long int *numberthreads = (long int*)number;
    printf("Tenho %ld tarefas irmãs.\n", *number);
    return NULL;
}

void* print_threads(void* args) {
    long int *numthr = (long int*)args;
    printf("Tenho %ld ", *numthr);
    printf("tarefas irmãs.\n");
    return NULL;
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
    /*
     * Initialization
     */
    parseArgs(argc, argv);

    if( global_numthreads < 1 ) {
        puts("No threads");
        exit(1);
    }
    FILE* resultFp = outputFile();
    maze_t* mazePtr = maze_alloc();
    assert(mazePtr);
    long numPathToRoute = maze_read(mazePtr, global_inputFile, resultFp);
    router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
                                       global_params[PARAM_YCOST],
                                       global_params[PARAM_ZCOST],
                                       global_params[PARAM_BENDCOST]);
    assert(routerPtr);
    list_t* pathVectorListPtr = list_alloc(NULL);
    assert(pathVectorListPtr);

    router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr};
    TIMER_T startTime;
    TIMER_READ(startTime);

    /** queue lock init **/
    initLock(queueLock);
    solve_mutex_inits();

    /* initializing several threads */
    pthread_t tid[global_numthreads + 1];
    for( int i = 0; i < global_numthreads; i++ ) {
        if( pthread_create(&tid[i], NULL, router_solve, (void *)&routerArg) != 0 ) {
            puts("Thread creation failed");
            exit(1);
        }
    }

    if( pthread_create(&tid[global_numthreads], NULL, print_threads, (void *)&global_numthreads) != 0) {
        puts("Failed to create count thread thread");
        exit(1);
    }

    if( pthread_join(tid[global_numthreads], NULL) != 0 ) {
        puts("Count thread failed");
        exit(1);
    }

    for( int i = 0; i < global_numthreads; i++ ) {
        if( pthread_join(tid[i], NULL) != 0 ) {
            puts("Thread failed");
            exit(1);
        }
    }

    /** destroying queue lock **/
    destroyLock(queueLock);
    solve_mutex_destroys();

    TIMER_T stopTime;
    TIMER_READ(stopTime);

    long numPathRouted = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        numPathRouted += vector_getSize(pathVectorPtr);
    }
    fprintf(resultFp, "Paths routed    = %li\n", numPathRouted);
    fprintf(resultFp, "Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));


    /*
     * Check solution and clean up
     */
    assert(numPathRouted <= numPathToRoute);
    bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, resultFp, global_doPrint);
    assert(status == TRUE);
    fputs("Verification passed.\n",resultFp);

    maze_free(mazePtr);
    router_free(routerPtr);

    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        vector_t* v;
        while((v = vector_popBack(pathVectorPtr))) {
            // v stores pointers to longs stored elsewhere; no need to free them here
            vector_free(v);
        }
        vector_free(pathVectorPtr);
    }
    list_free(pathVectorListPtr);

    fclose(resultFp);
    exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
