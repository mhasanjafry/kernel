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
#include "kernel.h"
#include "errno.h"

#include "util/debug.h"
#include "api/access.h"

#include "proc/proc.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "vm/pagefault.h"
#include "vm/vmmap.h"

/*
 * This gets called by _pt_fault_handler in mm/pagetable.c The
 * calling function has already done a lot of error checking for
 * us. In particular it has checked that we are not page faulting
 * while in kernel mode. Make sure you understand why an
 * unexpected page fault in kernel mode is bad in Weenix. You
 * should probably read the _pt_fault_handler function to get a
 * sense of what it is doing.
 *
 * Before you can do anything you need to find the vmarea that
 * contains the address that was faulted on. Make sure to check
 * the permissions on the area to see if the process has
 * permission to do [cause]. If either of these checks does not
 * pass kill the offending process, setting its exit status to
 * EFAULT (normally we would send the SIGSEGV signal, however
 * Weenix does not support signals).
 *
 * Now it is time to find the correct page (don't forget
 * about shadow objects, especially copy-on-write magic!). Make
 * sure that if the user writes to the page it will be handled
 * correctly.
 *
 * Finally call pt_map to have the new mapping placed into the
 * appropriate page table.
 *
 * @param vaddr the address that was accessed to cause the fault
 *
 * @param cause this is the type of operation on the memory
 *              address which caused the fault, possible values
 *              can be found in pagefault.h
 */
void
handle_pagefault(uintptr_t vaddr, uint32_t cause)
{
    KASSERT(cause & FAULT_USER);

    vmarea_t * vma = vmmap_lookup(curproc->p_vmmap, ADDR_TO_PN(vaddr));
    if (vma == NULL){
        dbg(DBG_PRINT, "GRADING3C 5\n");
        do_exit(EFAULT);
    }
/*PROT_NONE   =   0x0  ->   No access. Defined in kernel/include/mm/mman.h */
    /*if (vma->vma_prot & PROT_NONE){ 
        dbg(DBG_PRINT, "GRADING MARCO 1037\n");
        do_exit(EFAULT);
    }*/

    if (!( ( (cause & FAULT_WRITE) || (cause & FAULT_EXEC) ) || (vma->vma_prot & PROT_READ) )){
        dbg(DBG_PRINT, "GRADING3D 2\n");
        do_exit(EFAULT);
    }

    if ((cause & FAULT_WRITE) && !(vma->vma_prot & PROT_WRITE)){
        dbg(DBG_PRINT, "GRADING3D 2\n");
        do_exit(EFAULT);
    }

    /*if ((cause & FAULT_EXEC) && !(vma->vma_prot & PROT_EXEC)){
        dbg(DBG_PRINT, "GRADING MARCO 1040\n");
        do_exit(EFAULT);
    }*/
    
    /*
    if(cause & FAULT_WRITE)
    {
        dbg(DBG_PRINT, "GRADING3B 1\n");
        if (addr_perm(curproc, (const void *)vaddr, (int)(PROT_READ | PROT_WRITE)) == 0){
            dbg(DBG_PRINT, "GRADING MARCO 1042\n");
            do_exit(EFAULT);
        }
    }else{
        dbg(DBG_PRINT, "GRADING3B 1\n");
        if (addr_perm(curproc, (const void *)vaddr, (int)PROT_READ) == 0){
            dbg(DBG_PRINT, "GRADING MARCO 1044\n");
            do_exit(EFAULT);
        }
    }
    */
    /**/
    int forwrite = (cause & FAULT_WRITE) ? 1 : 0; /*Not suitable for writing. Hence write error.*/

    pframe_t *result = NULL;
    int temp = pframe_lookup(vma->vma_obj, (ADDR_TO_PN(vaddr) - vma->vma_start + vma->vma_off), forwrite, &result);
    if (temp != 0){
        dbg(DBG_PRINT, "GRADING3D 2\n");
        do_exit(EFAULT);
    }

    KASSERT(result);
    dbg(DBG_PRINT, "GRADING3A 5.a\n");

    int pdflags = PD_PRESENT | PD_USER;
    int ptflags = PT_PRESENT | PT_USER;

    if (cause & FAULT_WRITE){
        dbg(DBG_PRINT, "GRADING3B 1\n");      
        int dirty_res = pframe_dirty(result);
        

        /*if (dirty_res < 0){
            dbg(DBG_PRINT, "GRADING MARCO 1047\n");
            do_exit(EFAULT);
        }*/

        pdflags |= PD_WRITE;
        ptflags |= PT_WRITE;
    }
    pframe_pin(result);

    KASSERT(result->pf_addr);
    dbg(DBG_PRINT, "GRADING3A 5.a\n");
    
    pt_map(curproc->p_pagedir, 
            (uintptr_t)PAGE_ALIGN_DOWN(vaddr), 
            (uintptr_t)PAGE_ALIGN_DOWN(pt_virt_to_phys((uintptr_t)result->pf_addr)), 
            pdflags, ptflags);
    pframe_unpin(result);

    tlb_flush_all();
/*2a) gets a free page frame*/

/*      int temp = 0;
        pframe_t *pf;
        if (NULL == (pf = slab_obj_alloc(pframe_allocator)))
            dbg(DBG_PFRAME, "WARNING: not enough kernel memory\n");
        else{
            if (NULL == (pf->pf_addr = page_alloc())) {
                dbg(DBG_PFRAME, "WARNING: not enough kernel memory\n");
                slab_obj_free(pframe_allocator, pf);
            }
            else{
                temp = 1;
                nallocated++;
                list_insert_tail(&alloc_list, &pf->pf_link);
            }
        }
Write page out if no free physical page ("swap" this physical page out to backing store)
    if (temp == 0){

    }
*/
    /*NOT_YET_IMPLEMENTED("VM: handle_pagefault");*/
}
