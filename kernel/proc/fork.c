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

#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
    /* Pointer argument and dummy return address, and userland dummy return
     * address */
    uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
    *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
    memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
    return esp;
}


/*
 * The implementation of fork(2). Once this works,
 * you're practically home free. This is what the
 * entirety of Weenix has been leading up to.
 * Go forth and conquer.
 */
int
do_fork(struct regs *regs)
{
        /*NOT_YET_IMPLEMENTED("VM: do_fork");*/
    /*Here I am unmapping to make sure that it doesn't mess with the mapping of the child, but not sure
    If this is the correct way of handling it*/
    /*Note: is vlow = USER_MEM_LOW? and high as well?*/
    KASSERT(regs != NULL);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");
    KASSERT(curproc != NULL);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");
    KASSERT(curproc->p_state == PROC_RUNNING);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");
    
    proc_t *child_proc = NULL;
    child_proc = proc_create("Child");

    KASSERT(child_proc != NULL);
    KASSERT(child_proc->p_state == PROC_RUNNING);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");
    KASSERT(child_proc->p_pagedir != NULL);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");

    child_proc->p_brk = curproc->p_brk;
    child_proc->p_start_brk = curproc->p_start_brk;

    kthread_t *child_thread = kthread_clone(curthr); /*This should be the thread that called do_fork*/
    KASSERT(child_thread != NULL);
    KASSERT(child_thread->kt_kstack != NULL);
    dbg(DBG_PRINT, "GRADING3A 7.a\n");

    child_thread->kt_proc = child_proc;
    list_insert_head(&child_proc->p_threads, &child_thread->kt_plink);

    /*Now we need to iterate through all the vmareas and set them equal to the child process*/

    child_proc->p_vmmap = vmmap_clone(curproc->p_vmmap); /*Need to take care of the mmobj object*/
    KASSERT(child_proc->p_vmmap != NULL);
    child_proc->p_vmmap->vmm_proc = child_proc;

    vmmap_t *child_map = child_proc->p_vmmap;
    list_link_t *c_area_link = (child_proc->p_vmmap->vmm_list).l_next;
    vmarea_t *child_area = NULL;
    vmarea_t *vmnewarea = NULL;
    list_iterate_begin(&curproc->p_vmmap->vmm_list, vmnewarea, vmarea_t, vma_plink){
        child_area = list_item(c_area_link, vmarea_t, vma_plink); KASSERT(child_area != NULL);

        child_area->vma_obj = vmnewarea->vma_obj;
        child_area->vma_obj->mmo_ops->ref(child_area->vma_obj);
        /*Need to insert in list the vma_olink, but shouldn't this be done only once? Well depending on the mapping*/
        
        if((vmnewarea->vma_flags & MAP_TYPE) == MAP_PRIVATE){
            /*For shadowed objects*/
            dbg(DBG_PRINT, "GRADING3B 1\n");
            mmobj_t *shadow1 = shadow_create();
            KASSERT(shadow1 != NULL);
            mmobj_t *shadow2 = shadow_create();
            KASSERT(shadow2 != NULL);
            /*MARCO: Assume that none of them will be NULL, so we don't have dereference*/
            shadow1->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(vmnewarea->vma_obj);
            KASSERT(shadow1->mmo_un.mmo_bottom_obj != NULL);
            shadow2->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(vmnewarea->vma_obj);
            KASSERT(shadow2->mmo_un.mmo_bottom_obj != NULL);

            /*MARCO: Not sure if this should be ref, Contrary to Shuo

            shadow1->mmo_un.mmo_bottom_obj->mmo_ops->ref(shadow1->mmo_un.mmo_bottom_obj);
            shadow2->mmo_un.mmo_bottom_obj->mmo_ops->ref(shadow2->mmo_un.mmo_bottom_obj); */

            shadow1->mmo_shadowed = vmnewarea->vma_obj;
            shadow2->mmo_shadowed = child_area->vma_obj;

            vmnewarea->vma_obj = shadow1;
            child_area->vma_obj = shadow2;
            list_insert_tail(&((mmobj_bottom_obj(shadow2))->mmo_un.mmo_vmas), &((child_area)->vma_olink));
        }
        else{
            dbg(DBG_PRINT, "GRADING3D 1\n");
            list_insert_tail(&(vmnewarea->vma_obj->mmo_un.mmo_vmas), &((child_area)->vma_olink));

        }
        dbg(DBG_PRINT, "GRADING3B 1\n");

        c_area_link = (c_area_link)->l_next;
    }list_iterate_end();
    /*Didn't include any of the unmapping because could this not create a problem with the page tables that we have?*/
    pt_unmap_range(curproc->p_pagedir, USER_MEM_LOW, USER_MEM_HIGH);
    pt_unmap_range(child_proc->p_pagedir,USER_MEM_LOW,USER_MEM_HIGH);

    regs->r_eax = 0; /*Return value for the child process*/
    child_thread->kt_ctx.c_eip = (uintptr_t) userland_entry;
    /*child_thread->kt_ctx.c_kstack*/

    child_thread->kt_ctx.c_ebp = child_thread->kt_ctx.c_esp;
    child_thread->kt_ctx.c_esp = (uintptr_t)fork_setup_stack(regs, (void *)child_thread->kt_ctx.c_kstack);
    child_thread->kt_ctx.c_pdptr = child_proc->p_pagedir;
   /* child_thread->kt_ctx.c_ebp = curthr->kt_ctx.c_ebp;*/

    /*MARCO: Not sure about unmapping page table*/
    sched_make_runnable(child_thread);
    regs->r_eax = child_proc->p_pid;
    tlb_flush_all();


    return child_proc->p_pid;
}