#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> 

#include "processorSchedular.h"

// Robert Northby 1081270

int main(int argc, char ** argv){
	char * fileName;
	int numProcesses;
	int playChallange = 0;
	
	int c;
	// test what the file name is and the number of processes.
	while((c = getopt(argc, argv, "cf:p:")) != -1){
		switch (c) {
			case 'f':
				fileName = (char *) malloc(strlen(optarg) * sizeof(char) + 1);
				strcpy(fileName, optarg);
				break;
			case 'p':
				numProcesses = atoi(optarg);
				break;
			case 'c':
				playChallange = 1;
				break;
		}
	}
	// if c, play the challenge.
	if(playChallange){
        challengeScheduler(fileName, numProcesses);
	}
	else {
        scheduler(fileName, numProcesses);
    }
	
	free(fileName);
	
    return 0;
}
