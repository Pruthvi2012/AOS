/* process_ring.c - process_ring */


#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include "process_ring.h"




//Polling value decrement function
//OUTPUT: OK
process pollingDec(volatile int32 *pId){
    int32 tmp = 0;
    while (tmp < rnd){   
        int32 pValue = pollingValue[*pId];
	int32 nextVal = *pId+1;
        pollingValue[nextVal] = pValue-1;
        *pId = nextVal;
        tmp++;
        printf("Ring %d : Round %d : Value : %d\n", counter, defaultRnd, pValue);
        if (counter == processCount - 1){
            defaultRnd++;
            counter = 0;
        }
        else{
            counter = counter + 1;
        }
    }
    return OK;
}

//Semaphore decrement value function
//output: OK
process semDec(int32 pId){
    int32 updatedId;
    while (defaultRnd < rnd){
        wait(semProcess[pId]);
        if (semProcessValue > -1){
            printf("Ring Element %d : Ring %d : Value : %d\n", pId, defaultRnd, semProcessValue);

            counter = counter + 1;
            semProcessValue = semProcessValue - 1;

            if (pId == processCount - 1){
                updatedId = 0;
                defaultRnd = defaultRnd + 1;
            }
            else{
                updatedId = pId + 1;
            }
        }
        signal(semProcess[updatedId]);
    }
    signal(doneSemProcess[pId]);
    return OK;
}

//function to print the value of rings and rounds
void printHelp(void)
{
    printf("Usage: Process Ring\n");
    printf("-p: <no. Of Process> <0-64>\n");
    printf("-r: no. Of Rounds \n");
    printf("-i: Choose Implementation <Poll / Sync>");
    printf("\tPoll : Polling\n");
    printf("\tSync : Semaphore\n");
}
