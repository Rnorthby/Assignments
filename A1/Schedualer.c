#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "processorSchedular.h"

// This function runs the scheduling algorithm described in the spec. it is used for the first three tasks.
// It calls upon the processorScheduler.c functions.
int scheduler(char * fileName, unsigned int numProcesses){

    int i;
    // init Queue will  sort by execution time when coming at same time.
    processQueue_type * waitingQueue = initQueue(fileName);
    // init CPU list.
    CPU_type * CPU_List[numProcesses];
    makeCPU(CPU_List, numProcesses);
    CPU_type * InOrderCPU_List[numProcesses];
    // create an in order list and shortest first list.
    for(i = 0; i < numProcesses; i++){
		InOrderCPU_List[i] = CPU_List[i];
	}

    // the total number of processes coming in.
    unsigned long processesRemaining = procRemaining(waitingQueue);
    unsigned long totalProcesses = processesRemaining;
    processesRemaining = 0;

    // stats variables for use in main loop.
    process_type * current;
    unsigned long t;
    unsigned long sumOfTimeSpend = 0;
    double sumOfOverHeads = 0;
    double maxOverHead = 0;
    double overHead;

    
    unsigned long splitExecutionTime;
    process_type * splitProcess;
	
	// make the subprocess hash lookup.
	unsigned int subprocessHashCount = 0;
	unsigned int subprocessHashLookup[totalProcesses];
	char * subprocessIDHashLookup[totalProcesses];
	for(i = 0; i < totalProcesses; i++) {
        subprocessIDHashLookup[i] = NULL;
        subprocessHashLookup[i] = 0;
    }

	int k;

	// main loop goes over all time values until all processes are complete.
    for(t = 0; !isEmpty2(waitingQueue, CPU_List, numProcesses) ; t++){

        // work out the stats of each CPU at time t.
        for(i = 0; i < numProcesses; i++) {
            // take away the time period are get the finished process.
            InOrderCPU_List[i]->processFinished = removeTime(&(InOrderCPU_List[i]->processQueue));
            if (InOrderCPU_List[i]->time > 0) {
                InOrderCPU_List[i]->time = InOrderCPU_List[i]->time - 1;
            }
            // gather that stats for the finished process.
            if (InOrderCPU_List[i]->processFinished) {
                // parallel vs the n process have different logic
                if(InOrderCPU_List[i]->processFinished->parallelisable == 'p'){
                    if(subprocessHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher] == 1){
                        processesRemaining--;
                        sumOfTimeSpend += t - InOrderCPU_List[i]->processFinished->timeArrived;
                        overHead = (double) (t - InOrderCPU_List[i]->processFinished->timeArrived) / (double) InOrderCPU_List[i]->processFinished->totalExecutionTime;
                        sumOfOverHeads += overHead;
                        if (overHead > maxOverHead) { maxOverHead = overHead; }
                        // use subprocess hash to find amount remaining.
                        subprocessHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher]--;
                    }
                    else {
                        subprocessHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher]--;
                    }
                }
                else {
                    processesRemaining--;
                    sumOfTimeSpend += t - InOrderCPU_List[i]->processFinished->timeArrived;
                    overHead = (double) (t - InOrderCPU_List[i]->processFinished->timeArrived) / (double) InOrderCPU_List[i]->processFinished->totalExecutionTime;
                    sumOfOverHeads += overHead;
                    if (overHead > maxOverHead) { maxOverHead = overHead; }
                }
            }
        }
        // print the output for the finished process that was just found.
        for(i = 0; i < numProcesses; i++){
            if (InOrderCPU_List[i]->processFinished){
                // when the parallel is finished make sure all sections are also finished through subprocess hash.
				if(InOrderCPU_List[i]->processFinished->parallelisable == 'p'){
					if(subprocessHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher] == 0){
                        printf("%ld,FINISHED,pid=%s,proc_remaining=%ld\n", t, subprocessIDHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher], processesRemaining);
                        subprocessHashLookup[InOrderCPU_List[i]->processFinished->subprocessHasher]--;
					}
				}
				else{
					printf("%ld,FINISHED,pid=%s,proc_remaining=%ld\n", t, InOrderCPU_List[i]->processFinished->processID, processesRemaining);
				}
                free(InOrderCPU_List[i]->processFinished);
            }
        }

        // pop any arriving processes and allocate onto a CPU.
        while((current = popWaitingQueue(&waitingQueue, t)) != NULL) {
            // Add to emptiest CPU;
            // if it does allow parallelism.
            if (current->parallelisable == 'p' && numProcesses > 1) {
                if(numProcesses == 2){
                    k = 2;
                    splitExecutionTime = (current->executionTime + k- 1) / k + 1;
                }
                else if(current->executionTime == 1){
                    splitExecutionTime = 1;
                    k = 1;
                }
                else{
                    k = ((current->executionTime < numProcesses) ? current->executionTime : numProcesses);
                    splitExecutionTime = (current->executionTime + k - 1) / k + 1;
                }
                // perform the split and add to the CPU's.
                for (i = 0; i < k; i++) {
                    splitProcess = (process_type *) malloc(sizeof(process_type));
                    assert(splitProcess);
                    splitProcess->timeArrived = current->timeArrived;
                    sprintf(splitProcess->processID, "%s.%d", current->processID, i);
                    splitProcess->numericID = current->numericID;
                    splitProcess->k = i;
                    splitProcess->executionTime = splitExecutionTime;
                    splitProcess->parallelisable = current->parallelisable;
                    splitProcess->totalExecutionTime = current->executionTime;
                    splitProcess->subprocessHasher = subprocessHashCount;
                    // add differently for 2 processors.
                    if(numProcesses == 2){
                        InOrderCPU_List[i]->newFastestProcess = addToActiveQueue(splitProcess, &(InOrderCPU_List[i]->processQueue)) || InOrderCPU_List[i]->newFastestProcess;
                    }
                    else {
                        CPU_List[i]->newFastestProcess = addToActiveQueue(splitProcess, &(CPU_List[i]->processQueue)) ||
                                                         CPU_List[i]->newFastestProcess;
                    }
                    CPU_List[i]->time += splitProcess->executionTime;
                    free(splitProcess);
                    subprocessHashLookup[subprocessHashCount]++;
                }
                // add to hash look up.
                subprocessIDHashLookup[subprocessHashCount] = (char * ) malloc(strlen(current->processID) * sizeof(char) + 1);
                strcpy(subprocessIDHashLookup[subprocessHashCount], current->processID);
                subprocessHashCount++;
                processesRemaining++;
                free(current);
            }
            else if (current->parallelisable == 'n' || numProcesses == 1) {
                // add the n process to the smallest time CPU.
                unsigned int minTime = UINT_MAX;
                unsigned int minLocation = UINT_MAX;
                unsigned int minID = UINT_MAX;
                // find CPU.
                for (i = 0; i < numProcesses; i++) {
                    if (CPU_List[i]->time < minTime) {
                        minTime = CPU_List[i]->time;
                        minLocation = i;
                        minID = CPU_List[i]->ID;
                    } else if (CPU_List[i]->time == minTime && CPU_List[i]->ID < minID) {
                        minTime = CPU_List[i]->time;
                        minLocation = i;
                        minID = CPU_List[i]->ID;
                    }
                }
                // add to that smallest CPU.
                CPU_List[minLocation]->newFastestProcess = addToActiveQueue(current, &(CPU_List[minLocation]->processQueue)) || CPU_List[minLocation]->newFastestProcess;
                CPU_List[minLocation]->time += current->executionTime;
                free(current);
                sortCPUArray(CPU_List, numProcesses);
                processesRemaining++;
            }
        }
        // print new starting processors.
        printNewStart(InOrderCPU_List, numProcesses, t);

    }
    // print the final stats.
    maxOverHead = round(maxOverHead * 100) / 100;
    double averageOverHead = round(sumOfOverHeads * 100 / totalProcesses) / 100;
    printf("Turnaround time %ld\nTime overhead %g %g\nMakespan %ld\n", (sumOfTimeSpend + totalProcesses - 1)/(totalProcesses), maxOverHead, averageOverHead, t - 1);
    // free variables.
    for(i = 0; i < numProcesses; i++){
        free(CPU_List[i]);

    }
    for(i = 0 ; i < totalProcesses; i++){
        free(subprocessIDHashLookup[i]);
    }
    return 0;
}
