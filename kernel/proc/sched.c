/******************************************************************************/
/* Important Spring 2016 CSCI 402 usage information:                          */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5f2e8d450c0c5851acd538befe33744efca0f1c4f9fb5f       */
/*         3c8feabc561a99e53d4d21951738da923cd1c7bbd11b30a1afb11172f80b       */
/*         984b1acfbbf8fae6ea57e0583d2610a618379293cb1de8e1e9d07e6287e8       */
/*         de7e82f3d48866aa2009b599e92c852f7dbf7a6e573f1c7228ca34b9f368       */
/*         faaef0c0fcf294cb                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "globals.h"
#include "errno.h"

#include "main/interrupt.h"

#include "proc/sched.h"
#include "proc/kthread.h"

#include "util/init.h"
#include "util/debug.h"
#include "util/string.h"

static ktqueue_t kt_runq;

static __attribute__((unused)) void
sched_init(void)
{
	sched_queue_init(&kt_runq);
}
init_func(sched_init);



/*** PRIVATE KTQUEUE MANIPULATION FUNCTIONS ***/
/**
 * Enqueues a thread onto a queue.
 *
 * @param q the queue to enqueue the thread onto
 * @param thr the thread to enqueue onto the queue
 */
static void
ktqueue_enqueue(ktqueue_t *q, kthread_t *thr)
{
	KASSERT(!thr->kt_wchan);
	list_insert_head(&q->tq_list, &thr->kt_qlink);
	thr->kt_wchan = q;
	q->tq_size++;
}

/**
 * Dequeues a thread from the queue.
 *
 * @param q the queue to dequeue a thread from
 * @return the thread dequeued from the queue
 */
static kthread_t *
ktqueue_dequeue(ktqueue_t *q)
{
	kthread_t *thr;
	list_link_t *link;

	if (list_empty(&q->tq_list))
		return NULL;

	link = q->tq_list.l_prev;
	thr = list_item(link, kthread_t, kt_qlink);
	list_remove(link);
	thr->kt_wchan = NULL;

	q->tq_size--;

	return thr;
}

/**
 * Removes a given thread from a queue.
 *
 * @param q the queue to remove the thread from
 * @param thr the thread to remove from the queue
 */
static void
ktqueue_remove(ktqueue_t *q, kthread_t *thr)
{
	KASSERT(thr->kt_qlink.l_next && thr->kt_qlink.l_prev);
	list_remove(&thr->kt_qlink);
	thr->kt_wchan = NULL;
	q->tq_size--;
}

/*** PUBLIC KTQUEUE MANIPULATION FUNCTIONS ***/
void
sched_queue_init(ktqueue_t *q)
{
	list_init(&q->tq_list);
	q->tq_size = 0;
}

int
sched_queue_empty(ktqueue_t *q)
{
	return list_empty(&q->tq_list);
}

/*
 * Updates the thread's state and enqueues it on the given
 * queue. Returns when the thread has been woken up with wakeup_on or
 * broadcast_on.
 *
 * Use the private queue manipulation functions above.
 */
void
sched_sleep_on(ktqueue_t *q)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_sleep_on");*/
	dbg(DBG_PRINT, "GRADING1C\n");
	curthr->kt_state = KT_SLEEP;
	curproc->p_state = PROC_RUNNING;

	/*curthr->kt_cancelled = 0;*/

	/*ktqueue_remove(&kt_runq, curthr);*/
	ktqueue_enqueue(q, curthr);
	sched_switch();
}


/*
 * Similar to sleep on, but the sleep can be cancelled.
 *
 * Don't forget to check the kt_cancelled flag at the correct times.
 *
 * Use the private queue manipulation functions above.
 */
int
sched_cancellable_sleep_on(ktqueue_t *q)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_cancellable_sleep_on");*/
	if( curthr->kt_cancelled == 1 ){
		dbg(DBG_PRINT, "GRADING1C\n");
		return -EINTR;
	}
	curthr->kt_state = KT_SLEEP_CANCELLABLE;
	/*curthr->kt_cancelled = 0;*/

	/*ktqueue_remove(&kt_runq, curthr);*/

	ktqueue_enqueue(q, curthr);
	sched_switch();
	if( curthr->kt_cancelled == 1 ){
		dbg(DBG_PRINT, "GRADING1C\n");
		return -EINTR;
	}
	dbg(DBG_PRINT, "GRADING1C\n");
	/*kthread_exit(curthr->kt_retval);*/
	return 0;
}

kthread_t *
sched_wakeup_on(ktqueue_t *q)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_wakeup_on");*/
	if (sched_queue_empty(q)){
		dbg(DBG_PRINT, "GRADING1C\n");
		return NULL;
	}

	kthread_t *wakeup_thread = ktqueue_dequeue(q);
 
	KASSERT((wakeup_thread->kt_state == KT_SLEEP) || (wakeup_thread->kt_state == KT_SLEEP_CANCELLABLE));
	dbg(DBG_PRINT, "GRADING1A 4.a\n");
	wakeup_thread->kt_state = KT_RUN;

	sched_make_runnable(wakeup_thread);
	return wakeup_thread;
}

void
sched_broadcast_on(ktqueue_t *q)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_broadcast_on");*/

	while (!sched_queue_empty(q)){
		dbg(DBG_PRINT, "GRADING1C\n");
		sched_wakeup_on(q);
	}
	dbg(DBG_PRINT, "GRADING1C\n"); /*Would I need to do this, i.e. a different code path, if it all flows and exits at this point*/
}

/*
 * If the thread's sleep is cancellable, we set the kt_cancelled
 * flag and remove it from the queue. Otherwise, we just set the
 * kt_cancelled flag and leave the thread on the queue.
 *
 * Remember, unless the thread is in the KT_NO_STATE or KT_EXITED
 * state, it should be on some queue. Otherwise, it will never be run
 * again.
 */
void
sched_cancel(struct kthread *kthr)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_cancel");*/
	switch(kthr->kt_state){
		case KT_SLEEP:
			dbg(DBG_PRINT, "GRADING1C\n");
			kthr->kt_cancelled = 1;
			break;
		case KT_SLEEP_CANCELLABLE:
			dbg(DBG_PRINT, "GRADING1C\n");
			kthr->kt_cancelled = 1;
			/*ktqueue_remove(kthr->kt_wchan, kthr);*/
			kthread_cancel(kthr, 0);

		    ktqueue_remove(kthr->kt_wchan, kthr);
		    kthr->kt_state = KT_RUN;

		    sched_make_runnable(kthr);
			break;
		case KT_RUN:
			dbg(DBG_PRINT, "GRADING1C\n");
			kthr->kt_cancelled = 1;
			if(kthr->kt_wchan != NULL){/*kernel 3 change*/
				ktqueue_remove(kthr->kt_wchan, kthr);/*kernel 3 change*/
			    kthr->kt_state = KT_RUN;/*kernel 3 change*/
	            
	    		sched_make_runnable(kthr);/*kernel 3 change*/
    		}/*kernel 3 change*/
			break;
		default:
			dbg(DBG_PRINT, "GRADING1C\n");
			break;
	}
	dbg(DBG_PRINT, "GRADING1C\n");
}

/*
 * In this function, you will be modifying the run queue, which can
 * also be modified from an interrupt context. In order for thread
 * contexts and interrupt contexts to play nicely, you need to mask
 * all interrupts before reading or modifying the run queue and
 * re-enable interrupts when you are done. This is analagous to
 * locking a mutex before modifying a data structure shared between
 * threads. Masking interrupts is accomplished by setting the IPL to
 * high.
 *
 * Once you have masked interrupts, you need to remove a thread from
 * the run queue and switch into its context from the currently
 * executing context.
 *
 * If there are no threads on the run queue (assuming you do not have
 * any bugs), then all kernel threads are waiting for an interrupt
 * (for example, when reading from a block device, a kernel thread
 * will wait while the block device seeks). You will need to re-enable
 * interrupts and wait for one to occur in the hopes that a thread
 * gets put on the run queue from the interrupt context.
 *
 * The proper way to do this is with the intr_wait call. See
 * interrupt.h for more details on intr_wait.
 *
 * Note: When waiting for an interrupt, don't forget to modify the
 * IPL. If the IPL of the currently executing thread masks the
 * interrupt you are waiting for, the interrupt will never happen, and
 * your run queue will remain empty. This is very subtle, but
 * _EXTREMELY_ important.
 *
 * Note: Don't forget to set curproc and curthr. When sched_switch
 * returns, a different thread should be executing than the thread
 * which was executing when sched_switch was called.
 *
 * Note: The IPL is process specific.
 */
void
sched_switch(void)
{
/*NOT_YET_IMPLEMENTED("PROCS: sched_switch");*/
	uint8_t old_ipl = intr_getipl();
	intr_setipl(IPL_HIGH);

	while(sched_queue_empty(&kt_runq)){
		dbg(DBG_PRINT, "GRADING1C\n");	
		intr_disable();
		intr_setipl(0);
		intr_wait();
		intr_setipl(IPL_HIGH);
	}

	kthread_t *new_thr = ktqueue_dequeue(&kt_runq);
    
	/*if (!strcmp(new_thr->kt_proc->p_comm,"pageoutd")){
	dbg(DBG_PRINT, "GRADING1C\n");
	new_thr = ktqueue_dequeue(&kt_runq);
	}*/
	dbg(DBG_PRINT, "GRADING1C\n");
	kthread_t *old_thr = curthr;
	curthr = new_thr;
	curproc = new_thr->kt_proc;

    context_switch(&old_thr->kt_ctx, &curthr->kt_ctx);
	intr_setipl(old_ipl);
    /*if( curproc->p_state == PROC_DEAD )
    kthread_exit(curproc->p_status);*/
}

/*
 * Since we are modifying the run queue, we _MUST_ set the IPL to high
 * so that no interrupts happen at an inopportune moment.

 * Remember to restore the original IPL before you return from this
 * function. Otherwise, we will not get any interrupts after returning
 * from this function.
 *
 * Using intr_disable/intr_enable would be equally as effective as
 * modifying the IPL in this case. However, in some cases, we may want
 * more fine grained control, making modifying the IPL more
 * suitable. We modify the IPL here for consistency.
 */
void
sched_make_runnable(kthread_t *thr)
{
	/*NOT_YET_IMPLEMENTED("PROCS: sched_make_runnable");*/
	uint8_t old_ipl = intr_getipl();
	intr_setipl(IPL_HIGH);

 	KASSERT(&kt_runq != thr->kt_wchan);
    dbg(DBG_PRINT, "GRADING1A 4.b\n"); 	

	ktqueue_enqueue(&kt_runq, thr);

	intr_setipl(old_ipl);
	
}