
// struct that describes a particular process.
typedef struct {
	unsigned int timeArrived;
	char processID[10];
	unsigned int numericID;
	unsigned int executionTime;
	unsigned int parallelisable;
	unsigned int totalExecutionTime;
	unsigned int subprocessHasher;
	unsigned int k;
	unsigned int parallelNumber;
	unsigned int startTime;
} process_type;

// process node in a linked list.
typedef struct processQueue_type processQueue_type;

struct processQueue_type {
	process_type * process;
	processQueue_type * next;
	processQueue_type * previous;
};

// Struct that describes a CPU
typedef struct {
    processQueue_type * processQueue;
    unsigned int time;
    unsigned int ID;
    process_type * processFinished;
    int newFastestProcess;
} CPU_type;


// Used in the Challenge.
typedef struct {
	unsigned int time;
	unsigned int ID;
	process_type * currentProcess;
} challenge_CPU_type;
// used in the challenge to record the starts and finishes.
typedef struct printLog_type printLog_type;

struct printLog_type {
	unsigned int t;
	processQueue_type * processQueue;
	printLog_type * nextTime;
};


void makeCPU(CPU_type ** arr, unsigned int N);
void sortCPUArray(CPU_type ** CPU_List, unsigned int numProcesses);


int scheduler(char * fileName, unsigned int numProcesses);

// init queue returns a linked list of processes from a file name
processQueue_type * initQueue(char * fileName);
// gets the next process from a queue.
process_type * popWaitingQueue(processQueue_type ** queue, unsigned int time);

// adds process to CPU active list
int addToActiveQueue(process_type * toAdd, processQueue_type ** activeProcess);
// remove time and return if process finished.
process_type * removeTime(processQueue_type ** activeProcess);
// returns the number of proc remaining.
unsigned int procRemaining(processQueue_type * activeProcesses);

void printQueue(processQueue_type * queue);

// tests if the 2 fields are both empty meaning the simulation is finished.
int isEmpty2(processQueue_type * waitingQueue, CPU_type ** CPU_List, unsigned int numProcesses);

// prints the new starting processes.
void printNewStart(CPU_type ** CPU_List, unsigned int numCPU, unsigned int t);

// Challenge functions
void challengeScheduler(char * fileName,int numProcesses);
void addUnallocatedProcess(processQueue_type ** unallocatedProcessQueue, process_type * toAdd);
int isEmptyChallenge(processQueue_type * waitingQueue, processQueue_type * unAllocatedQueue,challenge_CPU_type ** CPU_List,unsigned int numProcesses);