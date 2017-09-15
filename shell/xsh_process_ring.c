#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process_ring.h>

//Process ring using semaphores

shellcmd xsh_process_ring(int nargs, char *args[])
{
volatile int32 index = 0;
int32 i;
initRings = 0;
liveCount = 0;
int32 afterVal;
int32 beforeVal;
uint32 present;


pCount = 2;
rings = 3;
int32 impNumber = 0;
int32 j;
for (j = 1; j < nargs; j++)
{
	if (strcmp("--help", args[j]) == 0)
{
printHelp();
return SHELL_OK;
}
if (strcmp("-p", args[j]) == 0)
{
/* parse numeric argument to -p */
if (!(j + 1 < nargs))
{
/* No argument after flag */
printHelp();
printf("-p flag expected an argument\n");
return SHELL_ERROR;
}
pCount = atoi(args[j + 1]);
if (!(0 <= pCount && pCount <= 64))
{
printHelp();
printf("-p flag expected a number between 0 - 64\n");
return SHELL_ERROR;
}
}
if (strcmp("-r", args[j]) == 0)
{
/* parse numeric argument to -r */
if (!(j + 1 < nargs))
{
printHelp();
printf("-r flag expected an argument\n");
return SHELL_ERROR;
}
rings = atoi(args[j + 1]);
if (!(0 < rings && rings <= 100))
{
printHelp();
printf("-r flag expected a number between 0 - 100\n");
return SHELL_ERROR;
}
}
if (strcmp("-i", args[j]) == 0)
{
if (strcmp(args[j + 1], "poll") == 0)
{//printing the usage();
impNumber = 0;
}
else if (strcmp(args[j + 1], "sync") == 0)
{
//printing usage();
impNumber = 1;
}
else
{
printHelp();
}
}
}

printf("Number of Processes: %d\n",pCount);
printf("Number of Rings: %d\n",rings);
afterVal = gettime(&present);

/*End of Argument Processing*/
if (impNumber == 0)
{
/*Polling*/
if (pCount > 0 && pCount < 64 && rings > 0)
{
pollingStore[0] = (pCount * rings) - 1;
}
for (i = 0; i < pCount; i++)
{
resume(create(polling, 1024, 20, "process", 1, &index));
}
while (index < (pCount * rings))
;
// polling block loops ends
}
if (impNumber == 1)
{
/*Semaphores created here */
if (pCount > 0 && pCount < 64 && rings > 0)
{
for (i = 0; i < pCount; i++)
{
if (i == 0)
{
process_semaphores[i] = semcreate(1);
}
else
{
process_semaphores[i] = semcreate(0);
}
process_doneSemaphores[i] = semcreate(0);
}
pValue_semaphore = (pCount * rings) - 1;
}
for (i = 0; i < pCount; i++)
{
resume(create(semaphore, 1024, 20, "process", 1, i));
}
for (i = 0; i < pCount; i++)
{
wait(process_doneSemaphores[i]);
semdelete(process_semaphores[i]);
semdelete(process_doneSemaphores[i]);
}
//End of Semaphore Block
}
beforeVal = gettime(&present);
printf("Elapsed Seconds: %d\n",beforeVal-afterVal);
return SHELL_OK;
}