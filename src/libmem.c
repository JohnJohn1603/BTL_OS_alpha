/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{ 
  // struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;
  // if (rg_elmt->rg_start >= rg_elmt->rg_end)
  //   return -1;
  // if (rg_node != NULL){
  //   rg_elmt->rg_next = rg_node;
  //   mm->mmap->vm_freerg_list = rg_elmt;
  // }
  // else{
  //   mm->mmap->vm_freerg_list = rg_elmt;
  // }
  // return 0;
  // // free_start: 200
  // // free_end: 256
  // // free_start: 512
  // // free_end: 612
  // // free_start: 618
  // // free_end: 768
  // // free_start: 824
  // // free_end: 1024

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  struct vm_rg_struct **head = &mm->mmap->vm_freerg_list;
  struct vm_rg_struct *curr = *head;
  struct vm_rg_struct *prev = NULL;

  // Trường hợp danh sách rỗng
  if (curr == NULL) {
    rg_elmt->rg_next = NULL;
    *head = rg_elmt;
    return 0;
  }

  // Duyệt tìm vị trí để chèn sao cho giữ thứ tự tăng dần
  while (curr != NULL && curr->rg_start < rg_elmt->rg_start) {
    prev = curr;
    curr = curr->rg_next;
  }

  if (prev == NULL) {
    // Chèn vào đầu danh sách
    rg_elmt->rg_next = *head;
    *head = rg_elmt;
  } else {
    // Chèn vào giữa hoặc cuối
    prev->rg_next = rg_elmt;
    rg_elmt->rg_next = curr;
  }

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */

void alloc_nosyscall_treating(struct pcb_t *caller,
                            struct vm_rg_struct *arg_rg)
{
  struct vm_rg_struct *rgit = caller->mm->mmap->vm_freerg_list;
  struct vm_rg_struct *prev = NULL;
  struct vm_rg_struct *save_rg = malloc(sizeof(struct vm_rg_struct));
  while(rgit != NULL){
    if (arg_rg->rg_start == rgit->rg_start){
      save_rg->rg_start = rgit->rg_start;
      save_rg->rg_end = rgit->rg_end;
      defreelist(&caller->mm->mmap->vm_freerg_list, rgit);
      break;
    }
    rgit = rgit->rg_next;
  }
  save_rg->rg_start = arg_rg->rg_end;
}

int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{ 
  /*Allocate at the toproof */
  pthread_mutex_lock(&mmvm_lock);
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */
  // rgnode.vmaid
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].rg_next = NULL;

    struct vm_rg_struct * out_rg = malloc(sizeof(struct vm_rg_struct));
    out_rg->rg_start = rgnode.rg_start;
    out_rg->rg_end = rgnode.rg_end;

    alloc_nosyscall_treating(caller, out_rg);

    *alloc_addr = rgnode.rg_start;

    // struct vm_rg_struct *new_free_rg = malloc(sizeof(struct vm_rg_struct));
    // new_free_rg->rg_start = rgnode.rg_start + size;
    // new_free_rg->rg_end = rgnode.rg_end;
    // new_free_rg->rg_next = NULL;

    // enlist_vm_freerg_list(caller->mm, new_free_rg);

#ifdef DEBUG
  printf("=========== PHYSICAL MEMORY AFTER (NO-SYSCALL) ALLOCATION ===========\n");
  printf("PID=%d - Region=%d - Address=%08x - Size=%d byte\n", caller->pid, rgid, *alloc_addr, size);
  struct vm_rg_struct *list = caller->mm->mmap->vm_freerg_list;
  printf("free list in get_free_vmrg_area:\n");
  printf("-----------------------------------\n");
  while(list != NULL){
    printf("free_start: %d\n", list->rg_start);
    printf("free_end: %d\n", list->rg_end);
    list = list->rg_next;
  }
  printf("-----------------------------------\n");
#ifdef PAGETBL_DUMP
  print_pgtbl(caller, 0, -1); //print max TBL
#endif
#endif

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  int old_sbrk = cur_vma->sbrk;
  cur_vma->sbrk += inc_sz;

  /* TODO INCREASE THE LIMIT as inovking systemcall 
   * sys_memap with SYSMEM_INC_OP 
   */
  struct sc_regs regs;
  regs.a1 = SYSMEM_INC_OP;
  regs.a2 = vmaid;
  regs.a3 = inc_sz;


  if(syscall(caller, 17, &regs) == -1)
  {
    cur_vma->sbrk = old_sbrk;
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
  caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
  *alloc_addr = old_sbrk;
  struct vm_rg_struct * new_free_rg = malloc(sizeof(struct vm_rg_struct));
  new_free_rg->rg_start = old_sbrk + size;
  new_free_rg->rg_end = old_sbrk + inc_sz;
  new_free_rg->rg_next = NULL;

  enlist_vm_freerg_list(caller->mm, new_free_rg);
  
  

#ifdef DEBUG
    printf("=========== PHYSICAL MEMORY AFTER (SYSCALL) ALLOCATION ===========\n");
    printf("PID=%d - Region=%d - Address=%08x - Size=%d byte\n", caller->pid, rgid, *alloc_addr, size);

    struct vm_rg_struct *list = caller->mm->mmap->vm_freerg_list;
  printf("free list in get_free_vmrg_area:\n");
  printf("-----------------------------------\n");
  while(list != NULL){
    printf("free_start: %d\n", list->rg_start);
    printf("free_end: %d\n", list->rg_end);
    list = list->rg_next;
  }
  printf("-----------------------------------\n");
#ifdef PAGETBL_DUMP
    print_pgtbl(caller, 0, -1); //print max TBL
#endif
#endif
  pthread_mutex_unlock(&mmvm_lock);
  return 0;
  
  /* SYSCALL 17 sys_memmap */

  /* TODO: commit the limit increment */

  /* TODO: commit the allocation address 
  // *alloc_addr = ...
  */
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */

void defreelist(struct vm_rg_struct **rglist, struct vm_rg_struct *rgit)
{
  struct vm_rg_struct *tmp = *rglist;
  struct vm_rg_struct *pprev = NULL;
  
  while(tmp != NULL)
  {
    if (tmp->rg_start == rgit->rg_start && tmp->rg_end == rgit->rg_end)
    {
      if (pprev != NULL)
      {
        pprev->rg_next = tmp->rg_next;
      }
      else
      {
        *rglist = tmp->rg_next;
      }
      break;
    }
    pprev = tmp;
    tmp = tmp->rg_next;
  }
  return;
}

int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  // struct vm_rg_struct rgnode;
  pthread_mutex_lock(&mmvm_lock);

  // struct vm_rg_struct *freelist = caller->mm->mmap->vm_freerg_list;
  // while(freelist != NULL){
  //   printf("free list in dealloc:\n");
  //   printf("free_start: %d\n", freelist->rg_start);
  //   printf("free_end: %d\n", freelist->rg_end);
  //   freelist = freelist->rg_next;
  // }

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;
  
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) return -1; //khong ton tai vmaid
  struct vm_rg_struct deleteSymrgtbl = cur_vma->vm_mm->symrgtbl[rgid];
  printf("deleteSymrgtbl->rg_start = %d\n", deleteSymrgtbl.rg_start);
  printf("deleteSymrgtbl->rg_end = %d\n", deleteSymrgtbl.rg_end);
  //xu ly gop vung du lieu
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  struct vm_rg_struct *prev = NULL;

  struct vm_rg_struct *precheck = cur_vma->vm_freerg_list;
  printf("BEFORE defreelist:\n");
  while(precheck != NULL){
    printf("BEFORE free_start: %d\n", precheck->rg_start);
    printf("BEFORE free_end: %d\n", precheck->rg_end);
    precheck = precheck->rg_next;
  }

  struct vm_rg_struct *craft_rg = malloc(sizeof(struct vm_rg_struct));
  craft_rg->rg_start = deleteSymrgtbl.rg_start;
  craft_rg->rg_end = deleteSymrgtbl.rg_end;

  if(rgit != NULL){
    struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
    int merged;

    do {
        merged = 0;
        struct vm_rg_struct *prev = NULL;
        rgit = cur_vma->vm_freerg_list;

        while (rgit != NULL) {
            if (deleteSymrgtbl.rg_end == rgit->rg_start) {
                // Vùng cần sáp nhập bên dưới
                deleteSymrgtbl.rg_end = rgit->rg_end;

                if (prev == NULL) cur_vma->vm_freerg_list = rgit->rg_next;
                else prev->rg_next = rgit->rg_next;

                free(rgit); // giải phóng thủ công
                merged = 1;
                break;
            }
            else if (deleteSymrgtbl.rg_start == rgit->rg_end) {
                // Vùng cần sáp nhập bên trên
                deleteSymrgtbl.rg_start = rgit->rg_start;

                if (prev == NULL) cur_vma->vm_freerg_list = rgit->rg_next;
                else prev->rg_next = rgit->rg_next;

                free(rgit);
                merged = 1;
                break;
            }

            prev = rgit;
            rgit = rgit->rg_next;
        }
    } while (merged);

  // Sau khi gộp xong, thêm lại vùng đã gộp vào free list
  struct vm_rg_struct *putback = malloc(sizeof(struct vm_rg_struct));
  putback->rg_start = deleteSymrgtbl.rg_start;
  putback->rg_end = deleteSymrgtbl.rg_end;
  putback->rg_next = NULL;

  enlist_vm_freerg_list(caller->mm, putback);
  }
  else{
    struct vm_rg_struct *free_rg = malloc(sizeof(struct vm_rg_struct));

    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    if (cur_vma == NULL) return -1; //khong ton tai vmaid
    struct vm_rg_struct deleteSymrgtbl = cur_vma->vm_mm->symrgtbl[rgid];
    free_rg->rg_start = deleteSymrgtbl.rg_start;
    free_rg->rg_end = deleteSymrgtbl.rg_end;
    if(enlist_vm_freerg_list(caller->mm, free_rg) == -1) {
      pthread_mutex_unlock(&mmvm_lock);
      return -1;
  }
  }
  /* TODO: Manage the collect freed region to freerg_list */
  // struct vm_rg_struct * new_frrg = malloc(sizeof(struct vm_rg_struct));
  // new_frrg->rg_start = deleteSymrgtbl.rg_start;
  // new_frrg->rg_end = deleteSymrgtbl .rg_end;
  // printf("new_frrg->rg_start = %d\n", new_frrg->rg_start);
  // printf("new_frrg->rg_end = %d\n", new_frrg->rg_end);
  // /*enlist the obsoleted memory region */
  // if(enlist_vm_freerg_list(caller->mm, new_frrg) == -1) {
  //   // pthread_mutex_unlock(&mmvm_lock);
  //   return -1;
  // }
  // printf("--------------------\n");
  //     struct vm_rg_struct * tro = cur_vma->vm_freerg_list;
  //     while(tro != NULL){
  //       printf("ftro tro free list\n");
  //       printf("tro_start: %d\n", tro->rg_start);
  //       printf("tro_end: %d\n", tro->rg_end);
  //       tro = tro->rg_next;
  //     }
  // printf("--------------------\n");
  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  caller->mm->symrgtbl[rgid].rg_next = NULL;
  
#ifdef DEBUG
    printf("=========== PHYSICAL MEMORY AFTER DEALLOCATION ===========\n");
    printf("PID=%d - Region=%d\n", caller->pid, rgid);
    
    struct vm_rg_struct *list = caller->mm->mmap->vm_freerg_list;
  printf("free list in get_free_vmrg_area:\n");
  printf("-----------------------------------\n");
  while(list != NULL){
    printf("free_start: %d\n", list->rg_start);
    printf("free_end: %d\n", list->rg_end);
    list = list->rg_next;
  }
  printf("-----------------------------------\n");
#ifdef PAGETBL_DUMP
    print_pgtbl(caller, 0, -1); //print max TBL
#endif
#endif
  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* TODO Implement allocation on vm area 0 */
  int addr;

  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */

  /* By default using vmaid = 0 */
  return __free(proc, 0, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn, vicfpn;
    uint32_t vicpte;

    //int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);
    vicpte = mm->pgd[vicpgn];
    vicfpn = PAGING_PTE_FPN(vicpte);

    /* Get free frame in MEMSWP */
    if(MEMPHY_get_freefp(caller->active_mswp, &swpfpn) == -1) return -1;

    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

    /* TODO copy victim frame to swap 
     * SWP(vicfpn <--> swpfpn)
     * SYSCALL 17 sys_memmap 
     * with operation SYSMEM_SWP_OP
     */
    struct sc_regs regs;
    regs.a1 = SYSMEM_SWP_OP;
    regs.a2 = vicfpn;
    regs.a3 = swpfpn;

    if(syscall(caller, 17, &regs) == -1) return -1;

    pte_set_swap(&mm->pgd[vicpgn], caller->active_mswp_id, swpfpn);


    /* SYSCALL 17 sys_memmap */

    /* TODO copy target frame form swap to mem 
     * SWP(tgtfpn <--> vicfpn)
     * SYSCALL 17 sys_memmap
     * with operation SYSMEM_SWP_OP
     */

    int tgtfpn = PAGING_PTE_SWP(pte);
    /* TODO copy target frame form swap to mem 
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..
    */
    // regs.a1 = SYSMEM_SWP_OP;
    // regs.a2 = tgtfpn;
    // regs.a3 = vicfpn;

    if(__swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn) == -1) return -1;

    // if(syscall(caller, 17, &regs) == -1) return -1;

    pte_set_fpn(&mm->pgd[pgn], vicfpn);

    /* SYSCALL 17 sys_memmap */

    /* Update page table */
    // pte_set_swap() 
    // mm->pgd;

    /* Update its online status of the target page */
    //pte_set_fpn() &
    //mm->pgd[pgn];
    // pte_set_fpn();

    // enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }
  enlist_pgn_node(&caller->mm->fifo_pgn,pgn);

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int offs = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) == -1) return -1; /* invalid page access */

  /* TODO 
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  int phyaddr = fpn * PAGING_PAGESZ + offs;

  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_READ;
  regs.a2 = phyaddr;
  // regs.a3 = (int)data;

  if(syscall(caller, 17, &regs) == -1) return -1;

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE)
  *data = regs.a3;

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int offs = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0) return -1; /* invalid page access */

  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  int phyaddr = fpn * PAGING_PAGESZ + offs;
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_WRITE;
  regs.a2 = phyaddr;
  regs.a3 = value;
  if(syscall(caller, 17, &regs) == -1) return -1;

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE) 

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
  //destination
  *destination = data;
#ifdef DEBUG
  printf("=========== PHYSICAL MEMORY AFTER READING ===========\n");
#endif
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  int val = __write(proc, 0, destination, offset, data);
#ifdef DEBUG
  printf("=========== PHYSICAL MEMORY AFTER WRITING ===========\n");
#endif
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if(pg == NULL) return -1;

  if(pg->pg_next == NULL){
    *retpgn = pg->pgn;
    free(pg);
    mm->fifo_pgn = NULL;
    return 0;
  }

  struct pgn_t * prev_pg = NULL;
  while(pg->pg_next != NULL){
    prev_pg = pg;
    pg = pg->pg_next;
  }
  *retpgn = pg->pgn;
  prev_pg->pg_next = NULL;
  // free(pg->pg_next);
  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */

int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = 0;
  newrg->rg_end = 0;

  /* TODO Traverse on list of free vm region to find a fit space */
  //while (...)
  // ..
  while(rgit != NULL){
    if(rgit->rg_end - rgit->rg_start >= size){
      int res = rgit->rg_end - rgit->rg_start;
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;
      struct vm_rg_struct * temp_free_rg = malloc(sizeof(struct vm_rg_struct));
      temp_free_rg->rg_start = newrg->rg_start;
      temp_free_rg->rg_end = newrg->rg_end;

      if(rgit->rg_end - rgit->rg_start == size){
        defreelist(&cur_vma->vm_freerg_list, temp_free_rg);
      }
      else{
        struct vm_rg_struct * new_frrg = malloc(sizeof(struct vm_rg_struct));
        new_frrg->rg_start = rgit->rg_start + size;
        new_frrg->rg_end = rgit->rg_end;
        enlist_vm_freerg_list(caller->mm, new_frrg);
        defreelist(&cur_vma->vm_freerg_list, temp_free_rg);
      }
      return 0;
    } 
    rgit = rgit->rg_next;
    
  }

  return -1;
}

