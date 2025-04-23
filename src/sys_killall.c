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

// khi o trong mlq_quqeue: chua duoc chay
// -> kill luon
//

// trong running list 
// ->finish ngay tai do || time slot ngay sau 1 donvi
// proc->pc == proc->code->size

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
    int idx = 0;
    int sz = run_list->size;
    while (idx < sz)
    {
        struct pcb_t * curr_proc_r = run_list->proc[idx];
        if(curr_proc_r == NULL){
            idx++;
            continue;
        }
        if(strstr(curr_proc_r->path, proc_name)){
            curr_proc_r->pc = curr_proc_r->code->size;
            for(int j = idx; j < sz - 1; j++){
                run_list->proc[j] = run_list->proc[j + 1];
            }
            run_list->size--;
            sz--;
            break;
        }
        idx++;
    }


    for (int i = 0; i < MAX_PRIO; i++){
        struct queue_t *ready_list = &caller->mlq_ready_queue[i];
        int j = 0;
        int size = ready_list->size;
        while(j < size){
            struct pcb_t * curr_proc_rdq = ready_list->proc[j];
            if(curr_proc_rdq == NULL){
                j++;
                continue;
            }
            if (strstr(curr_proc_rdq->path,proc_name)){
                free(curr_proc_rdq);
                for(int k = j; k < size - 1; k++){
                    ready_list->proc[k] = ready_list->proc[k + 1];
                }
                ready_list->size--;
                ready_list->proc[ready_list->size] = NULL;
                size--;
            }
            j++;
        }
    }

    //###########################################################################

    // int flag = 1;
    // // flag == 1: running_list
    // struct queue_t * run_list = caller->running_list;
    // struct pcb_t * take_from_running_list = NULL;
    // for(int i = 0; i < run_list->size; i++){
    //     struct pcb_t * curr_proc_r = run_list->proc[i];
    //     if(curr_proc_r == NULL) continue;
    //     if(strstr(curr_proc_r->path, proc_name)){
    //         take_from_running_list = curr_proc_r;
    //         flag = 1;
    //         break;
    //     }
    // }

    // // flag == 2: mlq_ready_queue
    // struct pcb_t * take_from_mlq_ready_queue = NULL;
    // for(int i = 0; i < MAX_PRIO; i++){
    //     struct queue_t * ready_list = &caller->mlq_ready_queue[i];
    //     for(int j = 0; j < ready_list->size; j++){
    //         struct pcb_t * curr_proc_rdq = ready_list->proc[j];
    //         if(curr_proc_rdq == NULL) continue;
    //         if(strstr(curr_proc_rdq->path, proc_name)){
    //             take_from_mlq_ready_queue = curr_proc_rdq;
    //             flag = 2;
    //             break;
    //         }
    //     }
    // }

    // if (flag == 1){
    //     // xử lý trong running_list -> finish (ngay tại đó || time slot ngay sau 1 đơn vị)

    //     // printf("\tCPU %d: Processed %2d has finished\n",
    //     //     id ,take_from_running_list->pid);
    //     // free(take_from_running_list);
    //     // take_from_running_list = NULL;
    //     take_from_running_list->pc = take_from_running_list->code->size;
    //     //lấp đầy còn lại

    // }   
    // else{
    //     // xử lý trong mlq_ready_queue, vì chưa được chạy nên sẽ bị kill ngay lập tức.
    //     // free(take_from_mlq_ready_queue);
    //     take_from_mlq_ready_queue->pc = take_from_mlq_ready_queue->code->size;
    // }


    //###########################################################################

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    
    

    return 0; 
}
