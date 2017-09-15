#include<xinu.h>

int32 pollProcess[65];
sid32 semProcess[65];
int32 semProcessValue;
sid32 doneSemProcess[65];
int32 processCount;
int32 rnd;
int32 defaultRnd;
int32 pollingValue[6401];
int32 counter;


process pollingDec(volatile int32 *pId);
process semDec(volatile int32 pId);
void printHelp(void);
