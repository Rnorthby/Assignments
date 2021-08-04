#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "processorSchedular.h"

// This function takes the pointer to a list of CPU's and the number of processors and inits the CPU list.
void makeCPU(CPU_type ** arr, unsigned int numProcesses){
    int i;
    for(i = 0 ; i < numProcesses; i++){
        arr[i] = (CPU_type *) malloc(sizeof(CPU_type));
        assert(arr[i] != NULL);
        arr[i]->time = 0;
        arr[i]->ID = i;
        arr[i]->newFastestProcess= 0;
        arr[i]->processFinished = NULL;
        arr[i]->processQueue = NULL;
    }
}

// This function sorts the CPU array list so that the smallest time is always first.
void sortCPUArray(CPU_type ** CPU_List, unsigned int numProcesses){
	int i;
	CPU_type * temp;
	for(i = 1; i < numProcesses; i++){
		if(CPU_List[i - 1]->time >= CPU_List[i]->time){
			if(CPU_List[i - 1]->time == CPU_List[i]->time){
			    // sort by cpu ID after time.
			    if(CPU_List[i - 1]->ID < CPU_List[i]->ID){
                    temp = CPU_List[i - 1];
                    CPU_List[i - 1] = CPU_List[i];
                    CPU_List[i] = temp;
                }
			}
            temp = CPU_List[i - 1];
            CPU_List[i - 1] = CPU_List[i];
            CPU_List[i] = temp;
		}
	}
}

// this function opens the file name and returns a sorted queue of the processes arriving
processQueue_type * initQueue(char * fileName){
	FILE * fprt = fopen(fileName, "r");
	assert(fprt != NULL);
	processQueue_type * head = (processQueue_type *) malloc(sizeof(processQueue_type));
	assert(head != NULL);
	processQueue_type * next = head;
	processQueue_type * previous = NULL;
	
	next->process = (process_type *) malloc(sizeof(process_type));
	assert(next->process != NULL);
	// 4294967296 has 10 char. so line cannot be over 50 chars.
	int bufferLength = 128;
	char* lineBuffer = (char *) malloc(sizeof(char) * bufferLength);
	assert(lineBuffer);

	int timeArrived;
	int processID;
	int executionTime;
	char parallelisable;
	unsigned int numProcesses = 0;
	// read each line.
	while(fgets(lineBuffer, bufferLength, fprt) != NULL){
	    // fill in the data points.
		sscanf(lineBuffer, "%d %d %d %c\n", &timeArrived, &processID, &executionTime, &parallelisable);
		next->process->timeArrived = timeArrived;
		sprintf(next->process->processID, "%d", processID);
		next->process->numericID = processID;
		next->process->executionTime = executionTime;
		next->process->totalExecutionTime = executionTime;
		next->process->parallelisable = parallelisable;
		next->process->parallelNumber = 0;
		next->previous = previous;
		next->next = (processQueue_type *) malloc(sizeof(processQueue_type));
		assert(next->next != NULL);
		previous = next;
		next = next->next;
		next->process = (process_type *) malloc(sizeof(process_type));
		assert(next->process != NULL);
		numProcesses++;
	}
	previous->next = NULL;
	free(next->process);
	free(next);

    processQueue_type * sortedHead = NULL;
	unsigned int t;
    processQueue_type * currentMin;
    processQueue_type * currentItem;
    processQueue_type * sortedCurrent;
    // sort by execution time when processes have the same arrival time.
    unsigned int i;
	for(i = 1; i < numProcesses; i++){
        t = head->process->timeArrived;
        //printf("time %d\n", t);
        currentMin = head;
        currentItem = head;
        while(currentItem->next != NULL && currentItem->next->process->timeArrived == t) {
            currentItem = currentItem->next;
            if (currentItem->process->executionTime < currentMin->process->executionTime) {
                currentMin = currentItem;
            }
            else if (currentItem->process->executionTime == currentMin->process->executionTime) {
                // sort by ID secondarily
                if (currentItem->process->processID < currentMin->process->processID) {
                    currentMin = currentItem;
                }
            }
        }
        // determian where to insert the new node
        if(currentMin->previous == NULL){
            // start
            currentMin->next->previous = NULL;
            head = currentMin->next;
        }
        else if(currentMin->next == NULL){
            // end
            currentMin->previous->next = NULL;
        }
        else {
            // middle
            currentMin->previous->next = currentMin->next;
            currentMin->next->previous = currentMin->previous;
        }

        if(sortedHead == NULL){
            sortedHead = currentMin;
            sortedHead->previous = NULL;
            sortedCurrent = sortedHead;
            sortedCurrent->next = NULL;
        }
        else{
            sortedCurrent->next = currentMin;
            currentMin->previous = sortedCurrent;
            sortedCurrent = sortedCurrent->next;
            sortedCurrent->next = NULL;
        }
    }
	sortedCurrent->next = head;
	head->previous = sortedCurrent;
	sortedCurrent = sortedCurrent->next;
	sortedCurrent->next = NULL;
    fclose(fprt);
    free(lineBuffer);
    // return the sorted queue.
	return sortedHead;
}


// pops the queue depending on the time t.
process_type * popWaitingQueue(processQueue_type ** queue, unsigned int time){

	if(*queue == NULL){ return NULL;}
	// return nothing if not the right time
	if((*queue)->process->timeArrived != time){ return NULL;}
	// if it is the right time. return the head.
	process_type * output = (*queue)->process;
	processQueue_type * temp = (*queue);
	*queue = (*queue)->next;
	free(temp);
	return output;
}

// this function adds a process to the active CPU queue.
int addToActiveQueue(process_type * toAdd, processQueue_type ** activeProcess){
	processQueue_type * current = * activeProcess;
	if(current == NULL){
	}
	processQueue_type * previous = NULL;
	// run until the current node is sorted in the right spot to insert the to add.
	while(1){
		if
		( 
			(current == NULL) || 
			(current->process->executionTime > toAdd->executionTime) || 
			(current->process->executionTime == toAdd->executionTime && current->process->numericID > toAdd->numericID) ||
			(current->process->numericID == toAdd->numericID && current->process->k > toAdd->k)
		) 
		{
			// Queue is empty
			if(!previous && !current){
				current = (processQueue_type *) malloc(sizeof(processQueue_type));
				assert(current != NULL);
				current->process = (process_type *) malloc(sizeof(process_type));
				assert(current->process != NULL);
				memcpy(current->process, toAdd, sizeof(process_type));
				*activeProcess = current;
                (*activeProcess)->next = NULL;
				return 1;
			}
			// end of the queue
			if(!current){
				previous->next = (processQueue_type *) malloc(sizeof(processQueue_type));
				assert(previous->next != NULL);
				previous->next->process = (process_type *) malloc(sizeof(process_type));
				assert(previous->next->process != NULL);
				memcpy(previous->next->process, toAdd, sizeof(process_type));
				previous->next->next = NULL;
				return 0;
			}
			// the start of the queue
			if(!previous){
				previous = (processQueue_type *) malloc(sizeof(processQueue_type));
				assert(previous != NULL);
				previous->process = (process_type *) malloc(sizeof(process_type));
				assert(previous->process != NULL);
				memcpy(previous->process, toAdd, sizeof(process_type));
				previous->next = current;
				*activeProcess = previous;
				return 1;
			}
			// somewhere in the middle
			processQueue_type * next = current;
			current = (processQueue_type *) malloc(sizeof(processQueue_type));
			assert(current != NULL);
			current->process = (process_type *) malloc(sizeof(process_type));
			assert(current->process != NULL);
			memcpy(current->process, toAdd, sizeof(process_type));
			previous->next = current;
			current->next = next;
			return 0;
		}
		previous = current;
		current = current->next;
	}
}

// removes time and returns the finished process.
process_type * removeTime(processQueue_type ** activeProcess){
	if((*activeProcess) == NULL){ return NULL;}
	// return the finished process.
	if((*activeProcess)->process->executionTime == 1){
		processQueue_type * temp = *activeProcess;
		*activeProcess = (*activeProcess)->next;
		process_type * output  = temp->process;
		free(temp);
		return output;
	}
	(*activeProcess)->process->executionTime--;
	return NULL;
}

// works out the total number of processes in a queue.
unsigned int procRemaining(processQueue_type * activeProcesses){
	unsigned int timeRemaining = 0;
	processQueue_type * current = activeProcesses;
	while(current){
		timeRemaining++;
		current = current->next;
	}
	return timeRemaining;
}

// returns 0 if the waitingQueue and CPU list are both empty meaning that the sim is over.
int isEmpty2(processQueue_type * waitingQueue, CPU_type ** CPU_List, unsigned int numProcesses){
    if(waitingQueue != NULL){
        return 0;
    }
    int i;
    for(i = 0; i < numProcesses; i++){
        if(CPU_List[i]->processQueue != NULL){ return 0; }
    }
    return 1;
}

// function used in debugging. leaving it here please ignore.
void printQueue(processQueue_type * queue){
	if(queue == NULL){ return;}
	processQueue_type * current = queue;
	printf("StarQueue\n");
	while(current != NULL){
		printf("%d %s %d %c ", current->process->timeArrived, current->process->processID, current->process->executionTime, current->process->parallelisable);
		if(current->previous != NULL){
		    printf("PREV %s ", current->previous->process->processID);
		}
		if(current->next != NULL){

		    printf("NEXT %s ", current->next->process->processID);
		}
		printf("\n");
		current = current->next;
	}
	printf("EndQueue\n\n");
}

// prints the new starting process.
void printNewStart(CPU_type ** CPU_List, unsigned int numCPU, unsigned int t){
    int i;
    for (i = 0; i < numCPU; i++) {
        // if a process is the new fastest or a process is finished.
        if (CPU_List[i]->newFastestProcess || CPU_List[i]->processFinished) {
            if (CPU_List[i]->processQueue) {
                printf("%d,RUNNING,pid=%s,remaining_time=%d,cpu=%d\n", t,
                       CPU_List[i]->processQueue->process->processID,
                       CPU_List[i]->processQueue->process->executionTime, CPU_List[i]->ID);
                CPU_List[i]->newFastestProcess = 0;
                CPU_List[i]->processFinished = NULL;
            }
        }
    }
}