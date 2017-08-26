/* xsh_date.c - xsh_date */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_date - obtain and print the current month, day, year, and time
 *------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[]) {

	int32	retval;			/* return value			*/
	uint32	now;			/* current local time		*/
	
	if(nargs > 2){
		printf("Too many Arguments");
	}
	else if(nargs < 2){
		printf("Too few Arguments");
	}
	else{
		printf("Hello %s, Welcome to the world of Xinu!!",args[1]);
	}
	
	return 0;
}
