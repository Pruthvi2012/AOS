#include  <stdio.h>
#include  <sys/types.h>
#include <string.h>
#include <unistd.h>

int main(int nargs,char *args[])
{
    pid_t pid;                         /*pid for fork*/
    int p[2];				/*p for piping*/
    char buffer[100],cmd[20]={};
		
    if(nargs < 2)	{
	printf("give atleast one proper argument");
	return 0;
	}	
	pipe(p);
	
    pid=fork();
	    
    if (pid==0){				/*Child function*/
        printf("Child PID=%d\n",getpid()); 
		close(p[1]);
		read(p[0], buffer, sizeof(args[1]));
		strcat(cmd,"/bin/");
		strcat(cmd,buffer);
		if(strcmp(buffer,"echo")== 0){		/*echo functionality*/
		execl(cmd,buffer,"HELLO WORLD",NULL);}
		else{
		execl(cmd,buffer,NULL);}
    }else{					/*Parent function*/
        printf("Parent PID=%d\n",getpid());
		close(p[0]);
		write(p[1],args[1],(strlen(args[1])+1));
		wait(NULL);
    }
}

