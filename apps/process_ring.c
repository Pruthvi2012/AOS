/* process_ring.c - process_ring */


#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <process_ring.h>




//Polling value decrement function
//OUTPUT: OK
process polling(volatile int32 *pIndex)
{
    int32 tmp = 0;
    while (tmp < rings)
    {   
        int32 pValue = pollingValueStore[*pIndex];
		int32 nextVal = *pIndex+1;
        pollingStore[nextVal] = pValue-1;
        *pIndex = nextVal;
        tmp++;
        printf("Ring %d : Round %d : Value : %d\n", liveCount, initRounds, pValue);
        if (liveCount == pCount - 1)
        {
            initRings++;
            liveCount = 0;
        }
        else
        {
            liveCount = liveCount + 1;
        }
    }
    return OK;
}

//Semaphore decrement value function
//output: OK
process semaphore(int32 pIndex)
{
    int32 updatedIndex;
    while (initRings < rings)
    {
        wait(process_semaphores[pIndex]);
        if (pValue_semaphore > -1)
        {
            printf("Ring Element %d : Ring %d : Value : %d\n", pIndex, initRings, pValue_semaphore);

            liveCount = liveCount + 1;
            pValue_semaphore = pValue_semaphore - 1;

            if (pIndex == pCount - 1)
            {
                updatedIndex = 0;
                initRings = initRings + 1;
            }
            else
            {
                updatedIndex = pIndex + 1;
            }
        }
        signal(process_semaphores[updatedIndex]);
    }
    signal(process_doneSemaphores[pIndex]);
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