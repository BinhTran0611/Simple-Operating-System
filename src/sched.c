
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
pthread_mutex_t var_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++)
		mlq_ready_queue[i].size = 0;
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
    pthread_mutex_init(&var_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
    /*TODO: get a process from PRIORITY[ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
    static int curr_prio = MAX_PRIO;
    static int curr_slot = 0;
    struct pcb_t * proc = NULL;
    pthread_mutex_lock(&var_lock);
    pthread_mutex_lock(&queue_lock);
    int double_check = 0;
    while (double_check < 2) {
        if (!empty(&mlq_ready_queue[MAX_PRIO - curr_prio])) {
            proc = dequeue(&mlq_ready_queue[MAX_PRIO - curr_prio]); 
            curr_slot++;
            if (curr_slot >= curr_prio) { // Check curr_slot >= Thoi gian cho phep chay;
                curr_slot = 0;
                curr_prio--; // Ready to jump next queue
            }
            break;
        } else {
            // printf("%d\n", curr_prio);
            curr_slot = 0;
            curr_prio--; // Ready to jump to next queue
        }

        if (curr_prio < 1) {  
            double_check += 1;
            curr_prio = MAX_PRIO; 
        }
    }
    pthread_mutex_unlock(&queue_lock);
    pthread_mutex_unlock(&var_lock);
    return proc;
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
    pthread_mutex_lock(&queue_lock);
    if (!empty(&ready_queue)) {
        proc = dequeue(&ready_queue);
    }
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	// enqueue(&run_queue, proc);
    enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif