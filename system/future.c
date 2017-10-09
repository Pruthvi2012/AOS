#include <xinu.h>
#include <future.h>
#include <stdlib.h>

future_t* future_alloc(future_mode_t mode){
	future_t * add; //creating a structure add.
	add = (future_t *)getmem(sizeof(future_t)); // get the starting address of future_t.
    	add->state = FUTURE_EMPTY; 
	add->mode = mode;
	add->set_queue = -1; // default value
	add->get_queue = -1; // default value
	if(add->mode==FUTURE_QUEUE)
	{
		add->set_queue=newqueue();
		add->get_queue=newqueue();
	}
	if(add->mode==FUTURE_SHARED)
	{
		add->get_queue=newqueue();
	}
	return add; 
}

syscall future_free(future_t* f){
	return freemem((char *)f,1);
}

syscall future_get(future_t* f, int* val){
    intmask mask;
    struct procent *prptr;
    mask = disable();

    if(f->mode == FUTURE_EXCLUSIVE){
        //FUTURE_READY
        if (f->state == FUTURE_READY) {
            *val = f->value;
            f->state = FUTURE_EMPTY;
            restore(mask);
            return OK;
        }
        //FUTURE_EMPTY
        else if (f->state == FUTURE_EMPTY) {
            f->state = FUTURE_WAITING;
            f->pid = getpid();
            suspend(f->pid);
            *val = f->value;
            restore(mask);
            return OK;
        } 
        restore(mask);
        return SYSERR;
    }
    else if(f->mode == FUTURE_SHARED){
        //FUTURE_READY
        if (f->state == FUTURE_READY) {
            *val = f->value;
            restore(mask);
            return OK;
        }
        //FUTURE_EMPTY
        else if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING ) {
            f->state = FUTURE_WAITING;
            prptr = &proctab[getpid()];
            prptr->prstate = PR_WAIT; // changing the status of the process to PR_WAIT. 
            enqueue(getpid(), f->get_queue); // Push current process id to get_queue.
            //Reconstructing the process table.
            resched();
            *val = f->value;
            restore(mask);
            return OK;
        }
        else{
            restore(mask);
            return SYSERR;
        }   
    }
    else if(f->mode == FUTURE_QUEUE){
        if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING){
            // checking whether if set_queue is empty or not
            if (!isempty(f->set_queue)) {
                //If it is not empty get the first pid in queue and change it to ready.
                ready(getfirst(f->set_queue)); 
            }
            else{
                f->state = FUTURE_WAITING;
                prptr = &proctab[currpid];   
                prptr->prstate = PR_WAIT;    // changing the status of the process to PR_WAIT. 	
                enqueue(getpid(), f->get_queue);   // Push current process id to get_queue.

                // Reconstructing the process table.
                resched();
            }
            *val = f->value;
            restore(mask);
            return OK;
        }
        else{
            restore(mask);
            return SYSERR;
        }
    }
    else{
        restore(mask);
        return SYSERR;
    }
}

syscall future_set(future_t* f, int val){
    intmask mask;
    mask = disable(); //Disable interupts.
    struct procent *prptr;

    // MODE: EXCLUSIVE
	if(f->mode == FUTURE_EXCLUSIVE){
        if (f->state == FUTURE_EMPTY){
            f->value = val; //Set the Value.
            f->state = FUTURE_READY; // Modify the State.
            restore(mask); //restore interupts.
            return OK;
        }
        else if (f->state == FUTURE_WAITING) {
            f->state = FUTURE_READY;
            f->value = val; // Set the Value.
            resume(f->pid); // Resuming the process.
            resched(); // Extra line.to ready the process table.
            restore(mask);
            return OK;
        }
        else{
            restore(mask);
            return SYSERR;
        }
    }
    // MODE: SHARED
	else if(f->mode == FUTURE_SHARED){
        // Check whether the given future is in empty or waiting state.
        if(f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING){
            f->value = val; //Set the Value.
            f->state = FUTURE_READY; // Modify the State.
            
            resched_cntl(DEFER_START);
            while(!isempty(f->get_queue)){
                ready(getfirst(f->get_queue));
            }
            resched_cntl(DEFER_STOP); //try with resched();

            restore(mask); //restore interupts.
            return OK;
        }else{
            restore(mask);
            return SYSERR;
        }         
    }
    // MODE: QUEUE
	else if(f->mode == FUTURE_QUEUE){
       if(f->state == FUTURE_EMPTY){
            f->value = val; //Set the Value.
            prptr = &proctab[getpid()]; 
            prptr -> prstate = PR_WAIT; 
            enqueue(getpid(), f->set_queue);
            resched();
            restore(mask);
            return OK;
       }
       else if(f->state == FUTURE_WAITING){
            f->value = val; //Set the Value.
            resched_cntl(DEFER_START);
            if (!isempty(f->get_queue)) {
                ready(getfirst(f->get_queue));
            }
            resched_cntl(DEFER_STOP);
            restore(mask);
            return OK;
       }
	   else{
            restore(mask);
            return SYSERR;
        }
	}
	else{
        restore(mask);
		return SYSERR;
    }
}