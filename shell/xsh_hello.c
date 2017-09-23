/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

void test();

shellcmd xsh_hello(int nargs, char *args[]) {


//	if(nargs > 2){
//		printf("Too many Arguments\n");   /*For more than 1 argument*/
//	}
//	else if(nargs < 2){
//		printf("Too few Arguments\n");	// For no arguments
//	}
//	else{
//		printf("Hello %s, Welcome to the world of Xinu!!\n",args[1]);
//	}
//	
//	return 0; 

	//create();
	
	//changes for Asignment4

	pri16 curPrio;
	//pid32 pid;
	
	curPrio=resume(create(test, 1024, 100, "test1", 1));
	//pid=getpid();
	printf("currPrio:%d",curPrio);
	

}

	void test(){
	struct  procent *prptr;         /* Ptr to process' table entry  */
        //pri16   prio; 
	prptr= &proctab[getpid()];
	prptr->prprio=50;

}
