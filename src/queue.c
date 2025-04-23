#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

int is_valid_proc(const char *name) {
        if (name == NULL) return 0;
    
        for (int i = 0; name[i] != '\0'; i++) {
            char c = name[i];
            if (!(isalnum((unsigned char)c) || c == '_' || c == '/')) {
                return 1; // chứa ký tự không hợp lệ
            }
        }
    
        return 0; // tất cả ký tự đều hợp lệ
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (is_valid_proc(proc->path) == 1){
                return;
        }

        if (proc == NULL  || q == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) return;

        #ifdef MLQ_SCHED
        proc->priority = proc->prio;
        #endif
        
        q->proc[q->size] = proc;
        q->size++;

        // dọn rác
        int size = q->size;
        int idx = 0;
        while(idx < size){
                struct pcb_t * curr_proc = q->proc[idx];
                if(is_valid_proc(curr_proc->path) == 1){
                        for(int j = idx; j < size - 1; j++){
                                q->proc[j] = q->proc[j + 1];
                        }
                        q->size--;
                        size--;
                }
                else{
                        idx++;
                }
        }
        //
}

void enqueue_running(struct queue_t * q, struct pcb_t * proc) {
        if (is_valid_proc(proc->path) == 1){
                return;
        }
        /* TODO: put a new process to queue [q] */
        if (proc == NULL  || q == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) return;

        q->proc[q->size] = proc;
        q->size++;

        // dọn rác
        int size = q->size;
        int idx = 0;
        while(idx < size){
                struct pcb_t * curr_proc = q->proc[idx];
                if(is_valid_proc(curr_proc->path) == 1){
                        for(int j = idx; j < size - 1; j++){
                                q->proc[j] = q->proc[j + 1];
                        }
                        q->size--;
                        size--;
                }
                else{
                        idx++;
                }
        }
}

void dequeue_running(struct queue_t *running_list, struct pcb_t *proc){
        if (is_valid_proc(proc->path) == 1){
                return;
        }
        if (proc == NULL  || running_list == NULL || running_list->size == 0) return;
        if (running_list->size >= MAX_QUEUE_SIZE) return;
        struct pcb_t * take_proc;
        for (int i = 0; i < running_list->size; i++){
                struct pcb_t * compr_proc = running_list->proc[i];
                if(proc->pid == compr_proc->pid){
                        take_proc = compr_proc;
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
        return proc;
}

