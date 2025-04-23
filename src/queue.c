#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (proc == NULL  || q == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) return;

        #ifdef MLQ_SCHED
        proc->priority = proc->prio;
        #endif

        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (empty(q)) return NULL;
        int index = 0;
        
        uint32_t min = q->proc[0]->priority;
        for(int i = 1; i < q->size; i++){
                if(q->proc[i]->priority < min){
                        min = q->proc[i]->priority;
                        index = i;
                }
        }

        struct pcb_t * proc = q->proc[index];
        for(int i = index; i < q->size - 1; i++){
                q->proc[i] = q->proc[i + 1];
        }
        q->proc[q->size-1] = NULL;
        q->size--;
        return proc;
}

