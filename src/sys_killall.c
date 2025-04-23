/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "string.h"
#include "stdlib.h"
#include "queue.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    struct queue_t * run_list = caller->running_list;
    for(int i = 0; i < run_list->size; i++){
        struct pcb_t * curr_proc_r = run_list->proc[i];
        if(curr_proc_r == NULL) continue;
        if(strstr(curr_proc_r->path, proc_name)){
            free(curr_proc_r);
            for(int j = i; j < run_list->size - 1; j++){
                run_list->proc[j] = run_list->proc[j + 1];
            }
            run_list->size--;
            run_list->proc[run_list->size] = NULL;
        }
    }
    
    //caller->running_list
    //caller->mlq_ready_queue
    for(int i = 0; i < MAX_PRIO; i++){
        struct queue_t * ready_list = &caller->mlq_ready_queue[i];
        for(int j = 0; i < ready_list->size; i++){
            struct pcb_t * curr_proc_rdq = ready_list->proc[i];
            if(curr_proc_rdq == NULL) continue;
            if(strstr(curr_proc_rdq->path, proc_name)){
                free(curr_proc_rdq);
                for(int k = j; k < ready_list->size - 1; k++){
                    ready_list->proc[k] = ready_list->proc[k + 1];
                }
                ready_list->size--;
                ready_list->proc[ready_list->size] = NULL;
            }
        }
    }
    // struct queue_t * ready_list = caller->mlq_ready_queue;
    // for(int i = 0; i < ready_list->size; i++){
    //     struct pcb_t * curr_proc_rdq = ready_list->proc[i];
    //     if(curr_proc_rdq == NULL) continue;
    //     if(strstr(curr_proc_rdq->path, proc_name)){
    //         free(curr_proc_rdq);
    //         for(int j = i; j < ready_list->size - 1; j++){
    //             ready_list->proc[j] = ready_list->proc[j + 1];
    //         }
    //         ready_list->size--;
    //         ready_list->proc[ready_list->size] = NULL;
    //     }
    // }


    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    
    

    return 0; 
}
