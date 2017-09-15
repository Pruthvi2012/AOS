#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "process_ring.h"

//Process ring using semaphores

shellcmd xsh_process_ring(int nargs, char *args[])
{
volatile int32 index = 0;
int32 i;
defaultRnd = 0;
counter = 0;
int32 val1;
int32 val2;
uint32 timer;


processCount = 2;
rnd = 3;
int32 impNumber = 0;
int32 j;
for (j = 1; j < nargs; j++){
	if (strcmp("--help", args[j]) == 0){
		printHelp();
		return SHELL_OK;
	}	
	if (strcmp("-p", args[j]) == 0){
		/* parse numeric argument to -p */
		if (!(j + 1 < nargs)){
			/* No argument after flag */
			printHelp();
			printf("-p flag expected an argument\n");
			return SHELL_ERROR;
		}
		processCount = atoi(args[j + 1]);
		if (!(0 <= processCount && processCount <= 64)){
			printHelp();
			printf("-p flag expected a number between 0 - 64\n");
			return SHELL_ERROR;
		}		
	}
	if (strcmp("-r", args[j]) == 0){
		/* parse numeric argument to -r */
		if (!(j + 1 < nargs)){
			printHelp();
			printf("-r flag expected an argument\n");
			return SHELL_ERROR;
		}
		rnd = atoi(args[j + 1]);
		if (!(0 < rnd && rnd <= 100)){
			printHelp();
			printf("-r flag expected a number between 0 - 100\n");
			return SHELL_ERROR;
		}		
	}	
	if (strcmp("-i", args[j]) == 0){
		if (strcmp(args[j + 1], "poll") == 0){
			//printing the usage();
			impNumber = 0;
		}	
		else if (strcmp(args[j + 1], "sync") == 0){
			//printing usage();
			impNumber = 1;
		}
		else{
			printHelp();
		}
	}
}

printf("Number of Processes: %d\n",processCount);
printf("Number of Rings: %d\n",rnd);
val1 = gettime(&timer);

/*End of Argument Processing*/
if (impNumber == 0){
	/*Polling*/
	if (processCount > 0 && processCount < 64 && rnd > 0){
		pollingValue[0] = (processCount * rnd) - 1;
	}
	for (i = 0; i < processCount; i++){
		resume(create(pollingDec, 1024, 20, "process", 1, &index));
	}
	while (index < (processCount * rnd));
	// polling block loops ends
}
if (impNumber == 1){
	/*Semaphores created here */
	if (processCount > 0 && processCount < 64 && rnd > 0){
		for (i = 0; i < processCount; i++){
			if (i == 0){
				semProcess[i] = semcreate(1);
			}
			else{
				semProcess[i] = semcreate(0);
			}
			doneSemProcess[i] = semcreate(0);
		}
		semProcessValue = (processCount * rnd) - 1;
	}
	for (i = 0; i < processCount; i++){
		resume(create(semDec, 1024, 20, "process", 1, i));
	}
	for (i = 0; i < processCount; i++){
		wait(doneSemProcess[i]);
		semdelete(semProcess[i]);
		semdelete(doneSemProcess[i]);
	}
	//End of Semaphore Block
}
val2 = gettime(&timer);
printf("Elapsed Seconds: %d\n",val2-val1);
return SHELL_OK;
}
