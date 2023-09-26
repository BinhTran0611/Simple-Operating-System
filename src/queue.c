#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define MAX_INT 100000

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    /* TODO: put a new process to queue [q] */
    q->size++;
    q->proc[q->size - 1] = proc;
    return;
}

#ifdef MLQ_SCHED
struct pcb_t * dequeue(struct queue_t * q) {
    /* TODO: return a pcb whose prioprity is the highest
        * in the queue [q] and remember to remove it from q
        * */
    struct pcb_t * target = NULL;
    if (empty(q)) return target;
    else{
        target = q->proc[0];
        for (int i = 0; i < q->size - 1; i++){
            q->proc[i] = q->proc[i + 1];
        }
        q->size--;
    }
    return target;
}
#else
struct pcb_t * dequeue(struct queue_t * q) {
    /* TODO: return a pcb whose prioprity is the highest
    * in the queue [q] and remember to remove it from q
    * */
    int highestPriority = MAX_INT;
    int ind = -1;
 
    for (int i = 0; i < q->size; i++) {
        // If priority is same choose
        // the element with the
        // highest value
        if (highestPriority > q->proc[i]->priority) {
            highestPriority = q->proc[i]->priority;
            ind = i;
        }
    }
    
    if (ind == -1) return NULL;
    else {
        struct pcb_t * target = q->proc[ind];
        for (int i = ind; i < q->size - 1; i++){
            q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return target;
    }
    return NULL;
}
#endif

