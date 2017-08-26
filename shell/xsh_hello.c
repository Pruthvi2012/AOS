/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

shellcmd xsh_hello(int nargs, char *args[]) {


	if(nargs > 2){
		printf("Too many Arguments\n");   /*For more than 1 argument*/
	}
	else if(nargs < 2){
		printf("Too few Arguments\n");	// For no arguments
	}
	else{
		printf("Hello %s, Welcome to the world of Xinu!!\n",args[1]);
	}
	
	return 0;
}
