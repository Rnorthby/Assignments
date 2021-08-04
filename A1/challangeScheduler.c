#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "processorSchedular.h"

// This function runs the challenge scheduler program.
void challengeScheduler(char * fileName, int numProcesses){
    // init the waiting queue.
    processQueue_type * waitingQueue = initQueue(fileName);


    // print log type records all the events that happen at time t to be printed later.
    // starting log records when each process starts as this scheduler looks into the future.
    printLog_type * startingLogHead = (printLog_type *) malloc(sizeof(printLog_type));
    assert(startingLogHead != NULL);
    startingLogHead->t = 0;
    startingLogHead->processQueue = NULL;
    startingLogHead->nextTime = NULL;
    printLog_type * lastStartPrintLog = startingLogHead;

    // likewise, finish log record taken note of when the processes finish.
    printLog_type * finishLogHead = (printLog_type *) malloc(sizeof(printLog_type));
    assert(finishLogHead != NULL);
    finishLogHead->t = 0;
    finishLogHead->processQueue = NULL;
    finishLogHead->nextTime = NULL;
    printLog_type * lastFinishPrintLog = finishLogHead;

    // unallocated list holds the processes which have arrived but not put into a CPU
    // this is the logic that makes my algorithm better than the standard.
    processQueue_type * unAllocatedQueue = NULL;
    int i;

    // Init the CPU list.
    challenge_CPU_type * CPU_List[numProcesses];
    for(i = 0; i <numProcesses; i++){
        CPU_List[i] = (challenge_CPU_type *) malloc(sizeof(challenge_CPU_type));
        assert(CPU_List[i] != NULL);
        CPU_List[i]->time = 0;
        CPU_List[i]->ID = i;
        CPU_List[i]->currentProcess = NULL;
    }
    // get the number of processes in total.
    unsigned int numProcessesRemaining = procRemaining(waitingQueue);
    unsigned int totalProcesses = numProcessesRemaining;

    // set up some variables which are used throughout the time loop.
    process_type * current;
    unsigned int t;

    processQueue_type * node;

    // the hash table keeps track of the split processes remaining.
    unsigned int subprocessHashCount = 0;
    unsigned int subprocessHashLookup[totalProcesses];
    for(i = 0; i < totalProcesses; i++) {
        subprocessHashLookup[i] = 0;
    }
    int k;

    // variables used in splitting the process into subprocesses when parallel.
    int CPU_has_room;
    unsigned int largestParallelProcess;
    unsigned int newExecutionTime;
    unsigned int oldSplitSize;
    int timeDifference;
    int toSplitCPU;
    int emptyCPU;
    int allEmpty;
    int timeNegative;
    processQueue_type * tempQueue;
    printLog_type * tempLog;


    // for every period of time till the processes are finished perform the following actions.
    for(t = 0;!isEmptyChallenge(waitingQueue, unAllocatedQueue, CPU_List, numProcesses) ; t++){
        // pop the processes which are arriving from the waiting Queue and put in unallocated Queue.
        while((current = popWaitingQueue(&waitingQueue, t)) != NULL) {
                addUnallocatedProcess(&unAllocatedQueue, current);
        }

        // In side each CPU see if its current process is finished. if so, add a new one if possible.
        k = 1;
        for(i = 0; i < numProcesses; i++){
            if(CPU_List[i]->currentProcess != NULL) {
                // decrease the time left
                CPU_List[i]->time--;
                // case when the CPU has finished a process.
                if (CPU_List[i]->time == 0) {
                    // when the finish print look up is empty it needs to be initialised. else is just placed in.
                    if(lastFinishPrintLog->t != t) {
                        lastFinishPrintLog->nextTime = (printLog_type *) malloc(sizeof(printLog_type));
                        assert(lastFinishPrintLog->nextTime != NULL);
                        lastFinishPrintLog = lastFinishPrintLog->nextTime;
                        lastFinishPrintLog->nextTime = NULL;
                        lastFinishPrintLog->t = t;
                        lastFinishPrintLog->processQueue = (processQueue_type *) malloc(sizeof(processQueue_type));
                        assert(lastFinishPrintLog->processQueue != NULL);
                        lastFinishPrintLog->processQueue->process = CPU_List[i]->currentProcess;
                        lastFinishPrintLog->processQueue->next = NULL;
                    }
                    else{
                        node = lastFinishPrintLog->processQueue;
                        while(node->next != NULL){
                            node = node->next;
                        }
                        node->next = (processQueue_type *) malloc(sizeof(processQueue_type));
                        assert(node->next != NULL);
                        node = node->next;
                        node->process = CPU_List[i]->currentProcess;
                        node->next = NULL;
                    }
                    // the CPU now has no active process.
                    CPU_List[i]->currentProcess = NULL;
                    CPU_List[i]->time = 0;
                }
            }

            // when the CPU has no current process it should add one if it can.
            if(CPU_List[i]->currentProcess == NULL){
                // When the unallocated Queue has a process in it.
                if (unAllocatedQueue != NULL) {
                    // create a new start print log node when new time period is reached.
                    if((lastStartPrintLog->t != t || t == 0) && k==1){
                        if(t != 0){
                            lastStartPrintLog->nextTime = (printLog_type *) malloc(sizeof(printLog_type));
                            assert(lastStartPrintLog->nextTime != NULL);
                            lastStartPrintLog = lastStartPrintLog->nextTime;
                        }
                        lastStartPrintLog->nextTime = NULL;
                        lastStartPrintLog->t = t;
                        lastStartPrintLog->processQueue = (processQueue_type *) malloc(sizeof(processQueue_type));
                        assert(lastStartPrintLog->processQueue != NULL);
                        lastStartPrintLog->processQueue->process = unAllocatedQueue->process;
                        lastStartPrintLog->processQueue->next = NULL;
                        k = 0;
                    }
                    else{
                        // allocate the newly active process into the existing print log.
                        node = lastStartPrintLog->processQueue;
                        while (node->next != NULL){
                            node = node->next;
                        }
                        node->next = (processQueue_type *) malloc(sizeof(processQueue_type));
                        assert(node->next != NULL);
                        node = node->next;
                        node->process = unAllocatedQueue->process;
                        node->next = NULL;
                    }
                    // allocate the unallocated making in active.
                    CPU_List[i]->currentProcess = unAllocatedQueue->process;
                    // if parallel add it in the parallel hash lookup.
                    if(CPU_List[i]->currentProcess->parallelisable == 'p'){
                        CPU_List[i]->currentProcess->subprocessHasher = subprocessHashCount;
                        subprocessHashLookup[subprocessHashCount++] += 1;
                    }
                    CPU_List[i]->currentProcess->k = i;
                    CPU_List[i]->time = unAllocatedQueue->process->executionTime;
                    tempQueue = unAllocatedQueue;
                    unAllocatedQueue = unAllocatedQueue->next;
                    free(tempQueue);
                }
            }
        }

        // When nothing in unallocated see if splitting a process is worth the time.
        if(unAllocatedQueue == NULL){
            CPU_has_room = 1;
            // while the cpu's can be split.
            while(CPU_has_room){
                emptyCPU = -1;
                CPU_has_room = 0;
                largestParallelProcess = 0;
                // see if a empty is around or else exit the loop.
                for(i = 0; i < numProcesses; i++){
                    if(CPU_List[i]->currentProcess == NULL){
                        emptyCPU = i;
                        CPU_has_room = 1;
                        break;
                    }
                }
                if(emptyCPU == -1){
                    break;
                }
                allEmpty = 0;
                oldSplitSize = 0;
                // see if all the CPU's are empty and exit loop, also get info on the largest active parallel process.
                for(i = 0; i < numProcesses; i++){
                    if(CPU_List[i]->currentProcess != NULL){
                        if(CPU_List[i]->currentProcess->parallelisable == 'p'){
                            allEmpty = 1;
                            if(CPU_List[i]->currentProcess->executionTime > largestParallelProcess){
                                toSplitCPU = i;
                                largestParallelProcess = CPU_List[i]->currentProcess->executionTime;
                                oldSplitSize = 0;
                            }
                            if(!strcmp(CPU_List[i]->currentProcess->processID, CPU_List[toSplitCPU]->currentProcess->processID)){
                                oldSplitSize++;
                            }
                        }
                    }
                }

                if(!allEmpty){
                    break;
                }
                if(oldSplitSize > numProcesses){
                    break;
                }
                // calculate the new split process time.
                newExecutionTime = ( CPU_List[toSplitCPU]->currentProcess->totalExecutionTime + oldSplitSize) / (oldSplitSize + 1) + 1;
                if(newExecutionTime >= CPU_List[toSplitCPU]->time){
                    break;
                }
                // see if the split makes new time below zero.
                timeNegative = 1;
                for(i = 0; i < numProcesses; i++){
                    if(CPU_List[i]->currentProcess == NULL){
                        continue;
                    }
                    if(CPU_List[i]->currentProcess->parallelisable == 'p'){
                        if(!strcmp(CPU_List[i]->currentProcess->processID, CPU_List[toSplitCPU]->currentProcess->processID)){
                            timeDifference = newExecutionTime + CPU_List[i]->time - CPU_List[i]->currentProcess->executionTime;
                            if(timeDifference < 0){
                                timeNegative = 0;
                                break;
                            }

                        }
                    }
                }
                // leave loop if new split makes time negative.
                if(timeNegative == 0){
                    break;
                }
                // apply the new process time to existing active split processes.
                for(i = 0; i < numProcesses; i++){
                    if(CPU_List[i]->currentProcess == NULL){
                        continue;
                    }
                    if(CPU_List[i]->currentProcess->parallelisable == 'p'){
                        if(!strcmp(CPU_List[i]->currentProcess->processID, CPU_List[toSplitCPU]->currentProcess->processID)){
                            CPU_List[i]->time = newExecutionTime + CPU_List[i]->time - CPU_List[i]->currentProcess->executionTime;
                            CPU_List[i]->currentProcess->executionTime = newExecutionTime;
                        }
                    }
                }
                // add in the one new process that comes up now that process is split.
                CPU_List[emptyCPU]->currentProcess = (process_type *) malloc(sizeof(process_type));
                assert(CPU_List[emptyCPU]->currentProcess != NULL);
                strcpy(CPU_List[emptyCPU]->currentProcess->processID, CPU_List[toSplitCPU]->currentProcess->processID);
                CPU_List[emptyCPU]->currentProcess->k = emptyCPU;
                CPU_List[emptyCPU]->currentProcess->subprocessHasher = CPU_List[toSplitCPU]->currentProcess->subprocessHasher;

                if(subprocessHashLookup[CPU_List[emptyCPU]->currentProcess->subprocessHasher] == 1){
                    CPU_List[toSplitCPU]->currentProcess->parallelNumber = 0;
                }
                // add one process to the subprocesses hash counter.
                CPU_List[emptyCPU]->currentProcess->parallelNumber = subprocessHashLookup[CPU_List[emptyCPU]->currentProcess->subprocessHasher]++;
                CPU_List[emptyCPU]->currentProcess->parallelisable = 'p';
                CPU_List[emptyCPU]->currentProcess->executionTime = newExecutionTime;
                CPU_List[emptyCPU]->currentProcess->timeArrived = t;
                CPU_List[emptyCPU]->currentProcess->totalExecutionTime = CPU_List[toSplitCPU]->currentProcess->totalExecutionTime;
                CPU_List[emptyCPU]->time = newExecutionTime;
            }
        }
    }

    // use the print logs to print out the starts and finished of each process.
    for(k = 0; finishLogHead != NULL; k++){
        // loop through to update the num Processes Remaining.
        if(finishLogHead != NULL){
            if(finishLogHead->t == k){
                node = finishLogHead->processQueue;
                while(node != NULL) {
                    // only update num processes remaining if all parallel sections are finished.
                    if (node->process->parallelisable == 'p') {
                        if (subprocessHashLookup[node->process->subprocessHasher] == 1) {
                            numProcessesRemaining--;
                        } else {
                            subprocessHashLookup[node->process->subprocessHasher]--;
                        }
                    } else {
                        numProcessesRemaining--;
                    }
                    node = node->next;
                }
            }
        }

        // print out the processes that are finished.
        if(finishLogHead != NULL) {
            if (finishLogHead->t == k) {
                node = finishLogHead->processQueue;
                while (node != NULL) {
                    // only print out the parallel if all sections are done.
                    if(node->process->parallelisable == 'p'){
                        if(subprocessHashLookup[node->process->subprocessHasher] == 1){
                            printf("%d,FINISHED,pid=%s,proc_remaining=%d\n", finishLogHead->t, node->process->processID, numProcessesRemaining);
                            subprocessHashLookup[node->process->subprocessHasher]--;

                        }
                    }
                    else{
                        printf("%d,FINISHED,pid=%s,proc_remaining=%d\n", finishLogHead->t, node->process->processID, numProcessesRemaining);
                    }
                    node = node->next;
                }
            }
        }
        // print out the processes Running.
        if(startingLogHead != NULL) {
            if (startingLogHead->t == k || startingLogHead->t == 0) {
                node = startingLogHead->processQueue;
                while (node != NULL) {
                    // print out each type of process.
                    if(node->process->parallelisable == 'p'){
                        printf("%d,RUNNING,pid=%s.%d,remaining_time=%d,cpu=%d\n", startingLogHead->t,
                               node->process->processID, node->process->parallelNumber, node->process->executionTime, node->process->k);

                    }
                    else {
                        printf("%d,RUNNING,pid=%s,remaining_time=%d,cpu=%d\n", startingLogHead->t,
                               node->process->processID, node->process->executionTime, node->process->k);
                    }
                    tempQueue = node;
                    node = node->next;
                    free(tempQueue);
                }
                tempLog = startingLogHead;
                startingLogHead = startingLogHead->nextTime;
                free(tempLog);
            }
        }

        // free the nodes from finished print log.
        if(finishLogHead != NULL) {
            if (finishLogHead->t == k) {
                node = finishLogHead->processQueue;
                while (node != NULL) {
                    free(node->process);
                    tempQueue = node;
                    node = node->next;
                    free(tempQueue);
                }
                free(node);
                tempLog = finishLogHead;
                finishLogHead = finishLogHead->nextTime;
                free(tempLog);
            }
        }

    }
    free(finishLogHead);
    free(startingLogHead);
    // return the makespan of the processes.
    printf("Makespan %d\n", t - 1);

    for(i = 0; i < numProcesses; i++){
        free(CPU_List[i]);
    }

    return;
}

// this function places a process in the unallocated list sorted by largest first.
void addUnallocatedProcess(processQueue_type ** unallocatedProcessQueue, process_type * toAdd){
    processQueue_type * current = *unallocatedProcessQueue;
    processQueue_type * previous = NULL;
    // when the unallocated queue is empty.
    if(current == NULL){
        current = (processQueue_type *) malloc(sizeof(processQueue_type));
        assert(current != NULL);
        current->process = (process_type *) malloc(sizeof(process_type));
        assert(current->process != NULL);
        memcpy(current->process, toAdd, sizeof(process_type));
        *unallocatedProcessQueue = current;
        (*unallocatedProcessQueue)->next = NULL;
        free(toAdd);
        return ;
    }
    // find the location of insertion.
    while(1){
        if( (current == NULL) || (current->process->executionTime < toAdd->executionTime)){
            if(!current){
                previous->next = (processQueue_type *) malloc(sizeof(processQueue_type));
                assert(previous->next != NULL);
                previous->next->process = (process_type *) malloc(sizeof(process_type));
                assert(previous->next->process != NULL);
                memcpy(previous->next->process, toAdd, sizeof(process_type));
                previous->next->next = NULL;
                free(toAdd);
                return ;
            }
            if(!previous){
                //printf("B\n");
                //printf("At the start!\n");
                previous = (processQueue_type *) malloc(sizeof(processQueue_type));
                assert(previous != NULL);
                previous->process = (process_type *) malloc(sizeof(process_type));
                assert(previous->process != NULL);
                memcpy(previous->process, toAdd, sizeof(process_type));
                previous->next = current;
                *unallocatedProcessQueue = previous;
                //printf("Finished\n");
                free(toAdd);
                return ;
            }
            // somewhere in the middle
            //printf("C\n");
            processQueue_type * next = current;
            current = (processQueue_type *) malloc(sizeof(processQueue_type));
            assert(current != NULL);
            current->process = (process_type *) malloc(sizeof(process_type));
            assert(current->process != NULL);
            memcpy(current->process, toAdd, sizeof(process_type));
            previous->next = current;
            current->next = next;
            free(toAdd);
            return ;
        }
        previous = current;
        current = current->next;
    }
}


// returns NULL when all queues are empty.
int isEmptyChallenge(processQueue_type * waitingQueue, processQueue_type * unAllocatedQueue,challenge_CPU_type ** CPU_List,unsigned int numProcesses){
    if(waitingQueue != NULL){
        return 0;
    }
    if(unAllocatedQueue != NULL){
        return 0;
    }
    int i;
    for(i = 0; i < numProcesses; i++){
        if(CPU_List[i]->currentProcess != NULL){
            return 0;
        }
    }
    return 1;
}