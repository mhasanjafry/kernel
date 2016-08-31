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

#include "globals.h"
#include "errno.h"
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
    /*NOT_YET_IMPLEMENTED("VM: do_mmap");*/
    KASSERT(NULL != curproc->p_pagedir);
    dbg(DBG_PRINT, "GRADING3A 2.a\n");

    /*What position should the TLB be cleared?*/
    if(!PAGE_ALIGNED(off)){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return -EINVAL;
    }
    if(len <= 0 || len >  USER_MEM_HIGH){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return (int)MAP_FAILED;
    }
    /*if((prot != PROT_NONE) && !(prot & (PROT_EXEC | PROT_READ | PROT_WRITE | PROT_NONE))){
        dbg(DBG_PRINT, "GRADING MARCO 1141\n");
        return -EINVAL;
    }*/

    if(!(((flags & MAP_TYPE) == MAP_SHARED) || ((flags & MAP_TYPE) == MAP_PRIVATE) )){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return -EINVAL;
    }

    if((flags & MAP_FIXED) && (!PAGE_ALIGNED(addr))){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return (int)MAP_FAILED;
    }

    if((fd < 0 || fd >= NFILES) && !(flags & MAP_ANON)){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return (int)MAP_FAILED;
    }

    /*if (addr != 0)
    {
        dbg(DBG_PRINT, "GRADING3D 1\n");
        if((((uint32_t)addr < USER_MEM_LOW) || ((uint32_t)addr+len > USER_MEM_HIGH)) && (flags & MAP_FIXED)){
            dbg(DBG_PRINT, "GRADING MARCO 1146\n");
            return (int)MAP_FAILED;
        }
    }*/
    if(addr == 0 && (flags & MAP_FIXED)){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return (int)MAP_FAILED;
    }
    vnode_t *vnode = NULL;
    file_t *f = NULL;
    if(!(flags & MAP_ANON)){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        if((fd < 0 || fd >= NFILES) || curproc->p_files[fd] == NULL){
            dbg(DBG_PRINT, "GRADING3D 1\n");
            return (int)MAP_FAILED;
        }
        /*if((flags & MAP_PRIVATE) && !(curproc->p_files[fd]->f_mode & FMODE_READ)){
            dbg(DBG_PRINT, "GRADING MARCO 1150\n");
            return (int)MAP_FAILED;
        }*/
        if((flags & MAP_SHARED) && (prot & PROT_WRITE) && !((curproc->p_files[fd]->f_mode & FMODE_READ) && (curproc->p_files[fd]->f_mode & FMODE_WRITE))){
            dbg(DBG_PRINT, "GRADING3D 1\n");
            return (int)MAP_FAILED;
        }
        f = fget(fd);
        vnode = f->f_vnode;
    }
    /*MARCO: MAP_ANON NOT IMPLEMENTED*/
    /* MARCO: map_fixed and addr = 0*/

    vmarea_t *v_area = NULL;
    int err = 0;
    err = vmmap_map(curproc->p_vmmap, vnode, ADDR_TO_PN(addr), (uint32_t)(PAGE_ALIGN_UP(len))/PAGE_SIZE, prot, flags, off, VMMAP_DIR_HILO, &v_area);
    if(f != NULL){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        fput(f);
    }

    if(err < 0){
        dbg(DBG_PRINT, "GRADING3D 2\n");
        curthr->kt_errno = (int)MAP_FAILED;
        return (int)MAP_FAILED;
    }
    else{
        dbg(DBG_PRINT, "GRADING3B 1\n");
        *ret = PN_TO_ADDR(v_area->vma_start);
    }

    tlb_flush_all();
    return err;
}


/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
    /*NOT_YET_IMPLEMENTED("VM: do_munmap");*/

    if(len <= 0 || len >  USER_MEM_HIGH){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return -EINVAL;
    }

    if(!PAGE_ALIGNED(addr)){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return -EINVAL;
    }

   if(((uint32_t)addr < USER_MEM_LOW) || ((uint32_t)addr+len > USER_MEM_HIGH)){
        dbg(DBG_PRINT, "GRADING3D 1\n");
        return -EINVAL;
    }


    int err = vmmap_remove(curproc->p_vmmap, ADDR_TO_PN(addr), (uint32_t)(PAGE_ALIGN_UP(len))/PAGE_SIZE);
    /*if(err < 0){
        dbg(DBG_PRINT, "GRADING MARCO 1158\n");
        curthr->kt_errno = EINVAL;
        return -1;
    }*/
    
    tlb_flush_all();
    return err;
}