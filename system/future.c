#include <xinu.h>
#include "future.h"


future_t* future_alloc(future_mode_t mode){
	future_t* add;	//Address
	add = (future_t*)getmem(sizeof(future_t));
	add->state = FUTURE_EMPTY;
	add->mode = mode;
	return add;
}

syscall future_free(future_t *f){
	
	return freemem((char *)f,1);
	
}

syscall future_get(future_t *f, int *value) {

    intmask mask;            /* Saved interrupt mask		*/
    struct procent *prpntr;        /* Ptr to process' table entry	*/
    mask = disable();
    
	if (f->mode == FUTURE_EXCLUSIVE) {
        if (f->state == FUTURE_READY) {
            *value = f->value;
            f->state = FUTURE_EMPTY;
            restore(mask);
            return OK;
        } 
		else if (f->state == FUTURE_EMPTY) {
            f->state = FUTURE_WAITING;
            f->pid = getpid();
            suspend(f->pid);
            //*value = f->value;
            restore(mask);
            return OK;
        } 
		restore(mask);
        return SYSERR;
    } else if (f->mode == FUTURE_SHARED) {
        if (f->state == FUTURE_READY) {
            *value = f->value;
            restore(mask);
            return OK;
        } 
		else if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING) {
            f->state = FUTURE_WAITING;
            prpntr = &proctab[getpid()];
            prpntr->prstate = PR_WAIT;    /* Set state to waiting	*/
			//pid32 processID = getpid();
            //suspend(processID);
            enqueue(getpid(), f->get_queue);
            resched();
            *value = f->value;
            restore(mask);
            return OK;
        }
		
		restore(mask);
        return SYSERR;
    } else if (f->mode == FUTURE_QUEUE) {

        if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING) {
            //checking if set_queue is empty,if not retrieve the first pid in the queue
			if (!isempty(f->set_queue)) {
                ready(getfirst(f->set_queue));
            } else {
                f->state = FUTURE_WAITING;
                prpntr = &proctab[currpid];
                prpntr->prstate = PR_WAIT;    /* Set state to waiting	*/
				//pid32 processID = getpid();
				//suspend(processID);
                enqueue(getpid(), f->get_queue);
                resched();
            }
            *value = f->value;
            restore(mask);
            return OK;
        }
		restore(mask);
        return SYSERR;
	} 
	else{
		restore(mask);
        return SYSERR;
	}
	
}


syscall future_set(future_t *f, int value) {
    intmask mask;            /* Saved interrupt mask		*/
    struct procent *prpntr;        /* Ptr to process' table entry	*/

    mask = disable();
    if (f->mode == FUTURE_EXCLUSIVE) {
        if (f->state == FUTURE_EMPTY) {
            f->value = value;
            f->state = FUTURE_READY;
            restore(mask);
            return OK;
        } else if (f->state == FUTURE_WAITING) {
            f->value = value;
            f->state = FUTURE_READY;
            resume(f->pid);
			resched();
            restore(mask);
            return OK;
        } 
        restore(mask);
        return SYSERR;
        
    } else if (f->mode == FUTURE_SHARED) {
		//Checking if the future state is empty or waiting state
        if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING) {
            f->value = value;
            f->state = FUTURE_READY;
            resched_cntl(DEFER_START);
            while (!isempty(f->get_queue)) {
                ready(getfirst(f->get_queue));
            }
            resched_cntl(DEFER_STOP);
            restore(mask);
            return OK;
        }
		restore(mask);
        return OK;
		
    } else if (f->mode == FUTURE_QUEUE) {
        if (f->state == FUTURE_EMPTY) {
            prpntr = &proctab[getpid()];
            prpntr->prstate = PR_WAIT;    /* Set state to waiting	*/
            enqueue(getpid(), f->set_queue);
            resched();
            f->value = value;
            //f->state = FUTURE_READY;
            restore(mask);
            return OK;
        } else if (f->state == FUTURE_WAITING) {
            f->value = value;
            //f->state = FUTURE_READY;
            if (!isempty(f->get_queue)) {
                ready(getfirst(f->get_queue));
				resched();
            }
            restore(mask);
            return OK;
        }
    }
	
	restore(mask);
    return SYSERR;
	
}



