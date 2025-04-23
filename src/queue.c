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

        // if(q->size == 0){
        //         q->proc[0] = proc;
        //         q->size++;
        // }
        // else{
        // int put_prio = proc->priority;
        // int idx = 0;
        // int flag = 0;
        // for (int i = 0; i < q->size; i++){
        //         if(put_prio <= q->proc[i]->priority){
        //                 idx = i;
        //                 flag = 1;
        //                 break;
        //         }
        // }
        // q->size++;
        // if(flag == 1){
        //         for (int i = idx; i < q->size-1; i++){
        //                 q->proc[i + 1] = q->proc[i];
        //         }
        //         q->proc[idx] = proc;
        // }
        // else{
        //         q->proc[q->size - 1] = proc;
        // }
        // }

        
        q->proc[q->size] = proc;
        q->size++;
        printf("size: %d\n", q->size);
        for (int i = 0; i < q->size; i++){
                printf("PROC NAME: %s - \n", q->proc[i]->path);
        }
}

void enqueue_running(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (proc == NULL  || q == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) return;
        
        q->proc[q->size] = proc;
        q->size++;
        printf("enqueue--------------------IN RUNNING QUEUE----------------\n");
        for (int i = 0; i < q->size; i++){
                printf("PROC NAME: %s - \n", q->proc[i]->path);
        }
}

void dequeue_running(struct queue_t *running_list, struct pcb_t *proc){
        if (proc == NULL  || running_list == NULL) return;
        if (running_list->size >= MAX_QUEUE_SIZE) return;

        printf("dequeue--------------------IN RUNNING QUEUE----------------\n");
	for(int i = 0; i < running_list->size; i++){
		printf("RUNNing PROC NAME: %s - \n", running_list->proc[i]->path);
	}
	printf("-------------------IN RUNNING QUEUE----------------\n");

        for (int i = 0; i < running_list->size; i++){
                struct pcb_t * compr_proc = running_list->proc[i];
                if(proc->pid == compr_proc->pid){
                        for (int j = i; j < running_list->size - 1; j++){
                                running_list->proc[j] = running_list->proc[j + 1];
                        }
                        running_list->proc[running_list->size - 1] = NULL;
                        running_list->size--;
                        break;
                }
        }

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
                        break;
                }
        }
        struct pcb_t * proc = q->proc[index];
        for(int i = index; i < q->size - 1; i++){
                q->proc[i] = q->proc[i + 1];
        }
        q->proc[q->size-1] = NULL;
        q->size--;

  

        //################################################
        // struct pcb_t * proc = q->proc[0];
        // if(q->size != 0){
        //         for(int i = 0; i < q->size - 1; i++){
        //                 q->proc[i] = q->proc[i + 1];
        //         }
        //         q->proc[q->size - 1] = NULL;
        //         q->size--;
        // }
        return proc;
}

