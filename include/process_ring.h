#include<xinu.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

initRings = 0;
liveCount = 0;
pCount = 2;
rings = 3;

process semaphore(int32 pIndex);
process polling(volatile int32 *pIndex);
void printHelp(void);
