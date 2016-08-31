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

#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"
#include "mm/tlb.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
    vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
    if (newvma) {
       newvma->vma_vmmap = NULL;
    }
    return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
    KASSERT(NULL != vma);
    slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
    vmmap_t *newvmap = (vmmap_t *) slab_obj_alloc(vmmap_allocator);
    if (newvmap) {
        dbg(DBG_PRINT, "GRADING3B 1\n");
        newvmap->vmm_proc = NULL;
        list_init(&newvmap->vmm_list);
    }
    dbg(DBG_PRINT, "GRADING3B 1\n");
        /*NOT_YET_IMPLEMENTED("VM: vmmap_create");*/
    return newvmap;
}
/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
    KASSERT(NULL != map);
    dbg(DBG_PRINT, "GRADING3A 3.a\n");

    vmarea_t * vmnewarea;
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        if (vmnewarea->vma_obj != NULL){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            vmnewarea->vma_obj->mmo_ops->put(vmnewarea->vma_obj);
        }

        if (list_link_is_linked(&vmnewarea->vma_plink)){
            dbg(DBG_PRINT, "GRADING3B 1\n");           
            list_remove(&vmnewarea->vma_plink);
        }
            
        if (list_link_is_linked(&vmnewarea->vma_olink)){
            dbg(DBG_PRINT, "GRADING3B 1\n");           
            list_remove(&vmnewarea->vma_olink);
        }
        dbg(DBG_PRINT, "GRADING3B 1\n");
        vmarea_free(vmnewarea);

    }list_iterate_end();
    slab_obj_free(vmmap_allocator, map);
    /*NOT_YET_IMPLEMENTED("VM: vmmap_destroy");*/
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_insert");*/
    /*offset?*/
    KASSERT(NULL != map && NULL != newvma);
    dbg(DBG_PRINT, "GRADING3A 3.b\n");
    KASSERT(NULL == newvma->vma_vmmap);
    dbg(DBG_PRINT, "GRADING3A 3.b\n");
    KASSERT(newvma->vma_start < newvma->vma_end);
    dbg(DBG_PRINT, "GRADING3A 3.b\n");
    KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
    dbg(DBG_PRINT, "GRADING3A 3.b\n");

    /*KASSERT(!(list_link_is_linked(&newvma->vma_plink)));*/

    newvma->vma_vmmap = map;

    vmarea_t * vmnewarea;
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        dbg(DBG_PRINT, "GRADING3B 1\n");       
        if (vmnewarea->vma_start >= newvma->vma_start){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            list_insert_before(&vmnewarea->vma_plink, &newvma->vma_plink);
            return;
        }
    }list_iterate_end();

    list_insert_tail(&map->vmm_list, &newvma->vma_plink);       

    
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        KASSERT(NULL != map);
        KASSERT(npages <= ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW));

        vmarea_t * vmnewarea;
        uint32_t start;
        /*if (dir == VMMAP_DIR_LOHI){
            start = ADDR_TO_PN(USER_MEM_LOW);
            dbg(DBG_PRINT, "GRADING MARCO 1074\n");
            list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
                dbg(DBG_PRINT, "GRADING MARCO 1075\n");
                if (vmnewarea->vma_start - start >= npages){
                    dbg(DBG_PRINT, "GRADING MARCO 1076\n");
                    return start;
                }
                start = vmnewarea->vma_end;
            }list_iterate_end();
            if (ADDR_TO_PN(USER_MEM_HIGH) - start >= npages){
                dbg(DBG_PRINT, "GRADING MARCO 1077\n");
                return start;
            }
        }*/
        if (dir == VMMAP_DIR_HILO){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            start = ADDR_TO_PN(USER_MEM_HIGH);
            list_iterate_reverse(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                if ((start - vmnewarea->vma_end) >= npages){
                    dbg(DBG_PRINT, "GRADING3B 1\n");
                    return (start-npages);
                }
                start = vmnewarea->vma_start;
            }list_iterate_end();
            if (start - ADDR_TO_PN(USER_MEM_LOW) >= npages){
                dbg(DBG_PRINT, "GRADING3D 2\n");               
                return (start-npages);
            }
        }        
        dbg(DBG_PRINT, "GRADING3D 2\n");
/*        NOT_YET_IMPLEMENTED("VM: vmmap_find_range");*/
        return -1;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
    KASSERT(map != NULL);
    dbg(DBG_PRINT, "GRADING3A 3.c\n");

    vmarea_t * vmnewarea;
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        if ((vmnewarea->vma_start <= vfn) && (vmnewarea->vma_end > vfn)){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            return vmnewarea;
        }
    }list_iterate_end();
    /*NOT_YET_IMPLEMENTED("VM: vmmap_lookup");*/
    return NULL;
}


/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
    vmmap_t * newmap = vmmap_create();
    /*if (newmap == NULL){
        dbg(DBG_PRINT, "GRADING MARCO 1085\n");
        return NULL;
    }*/
    
    dbg(DBG_PRINT, "GRADING3B 1\n");

    newmap->vmm_proc = map->vmm_proc; /*Not sure about this*/
    list_init(&newmap->vmm_list);

    vmarea_t * vmnewarea;
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        vmarea_t * newvmarea = vmarea_alloc();

        newvmarea->vma_start = vmnewarea->vma_start;
        newvmarea->vma_end = vmnewarea->vma_end;
        newvmarea->vma_off = vmnewarea->vma_off;
        newvmarea->vma_prot = vmnewarea->vma_prot;
        newvmarea->vma_flags = vmnewarea->vma_flags;
        newvmarea->vma_vmmap = newmap;
        newvmarea->vma_obj = NULL;
        list_link_init(&newvmarea->vma_plink);
        list_link_init(&newvmarea->vma_olink); /* should we copy from original?*/
        list_insert_before(&newmap->vmm_list,&newvmarea->vma_plink);

    }list_iterate_end();

    /*NOT_YET_IMPLEMENTED("VM: vmmap_clone");*/
    return newmap;
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_map");*/
    /*KASSERT(PAGE_ALIGNED(paddr));*/
    KASSERT(NULL != map);
    dbg(DBG_PRINT, "GRADING3A 3.d\n");
    KASSERT(0 < npages);
    dbg(DBG_PRINT, "GRADING3A 3.d\n");
    KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
    dbg(DBG_PRINT, "GRADING3A 3.d\n");
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
    dbg(DBG_PRINT, "GRADING3A 3.d\n");
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
    dbg(DBG_PRINT, "GRADING3A 3.d\n");
    KASSERT(PAGE_ALIGNED(off));
    dbg(DBG_PRINT, "GRADING3A 3.d\n");

    if (lopage == 0){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        int retval_vmmap_find_range = 0;
        retval_vmmap_find_range = vmmap_find_range(map, npages, dir);
        if (retval_vmmap_find_range < 0){
            dbg(DBG_PRINT, "GRADING3D 2\n");
            return -ENOMEM;
        }
        lopage = retval_vmmap_find_range;
    }else{
        dbg(DBG_PRINT, "GRADING3B 1\n");
        int t = vmmap_remove(map, lopage, npages);
        /*if (t<0){
            dbg(DBG_PRINT, "GRADING MARCO 1091\n");
            return t;
        }*/
    }
    vmarea_t *new_mmaping_area = NULL;
    mmobj_t *ret = NULL;
    new_mmaping_area = vmarea_alloc();
    /*if (new_mmaping_area == NULL){
        dbg(DBG_PRINT, "GRADING MARCO 1092\n");
        return -ENOMEM;
    }*/
    if (new != NULL){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        *new = new_mmaping_area;
    }
    (new_mmaping_area)->vma_start = lopage;
    (new_mmaping_area)->vma_end = (lopage+npages);
    (new_mmaping_area)->vma_off = ADDR_TO_PN(off);
    (new_mmaping_area)->vma_prot = prot;
    (new_mmaping_area)->vma_flags = flags;
    (new_mmaping_area)->vma_vmmap = NULL;

    list_link_init(&((new_mmaping_area)->vma_olink)); 
    list_link_init(&(new_mmaping_area->vma_plink)); 
    vmmap_insert(map, new_mmaping_area);

    if (file == NULL){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        (new_mmaping_area)->vma_obj = anon_create();
        ret = (new_mmaping_area)->vma_obj;
    }
    else{
        dbg(DBG_PRINT, "GRADING3B 1\n");
        
        int error = file->vn_ops->mmap(file, new_mmaping_area, &ret);
        
        new_mmaping_area->vma_obj = ret;
        /*MARCO: Might cause reference problems*/
        /*ret->mmo_ops->ref(ret);*/
    }
    list_insert_head(&(ret->mmo_un.mmo_vmas), &(new_mmaping_area->vma_olink));

    mmobj_t * shadow;
    if(MAP_PRIVATE & flags){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        shadow = shadow_create();

        shadow->mmo_shadowed = ret;
        shadow->mmo_un.mmo_bottom_obj = ret;
        new_mmaping_area -> vma_obj = shadow;
    }else{
        dbg(DBG_PRINT, "GRADING3D 1\n");
        new_mmaping_area -> vma_obj = ret;
    }
    
    
    /*If MAP_PRIVATE is specified set up a shadow object for the mmobj.*/
    /*NOT_YET_IMPLEMENTED("VM: vmmap_map");*/

    return 0;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_remove");*/
    /*if (npages == 0)
        return 0;*//*Sonny:need self check*/
    vmarea_t * vmnewarea;
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        /* * Case 1:  [   ******    ]
* The region to be unmapped lies completely inside the vmarea. We need to split the old vmarea into two vmareas. 
be sure to increment the reference count to the file associated with the vmarea.*/
        if ((vmnewarea->vma_start < lopage) && (vmnewarea->vma_end > (lopage+npages))){
            dbg(DBG_PRINT, "GRADING3D 2\n");

            uint32_t temp = vmnewarea->vma_end;
            vmnewarea->vma_end = lopage;

            vmarea_t * newvmarea = vmarea_alloc();
            /*if (newvmarea == NULL)
                return -ENOMEM;*//*Sonny:need self check*/
            newvmarea->vma_start = lopage+npages;
            newvmarea->vma_end = temp;
            newvmarea->vma_off = vmnewarea->vma_off - vmnewarea->vma_start + npages + lopage;
            newvmarea->vma_prot = vmnewarea->vma_prot;
            newvmarea->vma_flags = vmnewarea->vma_flags;
            newvmarea->vma_vmmap = NULL;
            newvmarea->vma_obj = vmnewarea->vma_obj;
                /*increase file reference: Confirm file or Object??? 
            if (vmnewarea->vma_obj != NULL)
                vmnewarea->vma_obj->mmo_ops->ref(vmnewarea->vma_obj);*/

            list_link_init(&newvmarea->vma_plink);
            list_link_init(&newvmarea->vma_olink); /* should we copy from original?*/
            /*list_insert_before(&vmnewarea->vma_plink, &newvmarea->vma_plink);*/

            vmmap_insert(map, newvmarea);

            if(newvmarea->vma_flags & MAP_PRIVATE)
            {
                dbg(DBG_PRINT, "GRADING3D 2\n");
                mmobj_t *new1=shadow_create();
                mmobj_t *new2=shadow_create();
                mmobj_t *old=vmnewarea->vma_obj;
                vmnewarea->vma_obj = new1;
                newvmarea->vma_obj = new2;
            
                new2->mmo_un.mmo_bottom_obj = old->mmo_un.mmo_bottom_obj;
                new1->mmo_un.mmo_bottom_obj = old->mmo_un.mmo_bottom_obj;
                new1->mmo_shadowed = old;
                new2->mmo_shadowed = old;
                old->mmo_ops->ref(old);
                list_insert_tail(&(old->mmo_un.mmo_bottom_obj->mmo_un.mmo_vmas), &(newvmarea -> vma_olink));
            }
            /*break;*/
        }
        else if ((vmnewarea->vma_start < lopage) && (vmnewarea->vma_end > lopage) && (lopage + npages >= vmnewarea -> vma_end)){
            /* * Case 2:  [      *******]**
            * The region overlaps the end of the vmarea. Just shorten the length of the mapping.*/             
            dbg(DBG_PRINT, "GRADING3D 1\n");
            vmnewarea->vma_end = lopage;
            /*break;*/
        }
        else if ((vmnewarea->vma_start >= lopage) && (vmnewarea->vma_end > (lopage+npages)) && (vmnewarea->vma_start < (lopage+npages))){
/* * Case 3: *[*****        ] * The region overlaps the beginning of the vmarea. Move the beginning of
* the mapping (remember to update vma_off), and shorten its length.*/
            dbg(DBG_PRINT, "GRADING3D 2\n");
            vmnewarea->vma_off = vmnewarea->vma_off + (lopage+npages-vmnewarea->vma_start); 
            vmnewarea->vma_start = lopage+npages;
            /*break;*/

        }
        else if ((vmnewarea->vma_start >= lopage) && (vmnewarea->vma_end <= (lopage+npages))){
/* * Case 4: *[*************]** The region completely contains the vmarea. Remove the vmarea from the list.*/
            dbg(DBG_PRINT, "GRADING3B 1\n");
            vmnewarea->vma_obj->mmo_ops->put(vmnewarea->vma_obj);
            
            if (list_link_is_linked(&vmnewarea->vma_plink)){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                list_remove(&vmnewarea->vma_plink);
            }
            
            if (list_link_is_linked(&vmnewarea->vma_olink)){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                list_remove(&vmnewarea->vma_olink);
            }
            vmarea_free(vmnewarea);
            /*break;*/
        }
    }list_iterate_end();
    dbg(DBG_PRINT, "GRADING3B 1\n");

    /*tlb_flush_range((uint32_t) PN_TO_ADDR(lopage), npages);*/
    tlb_flush_all();
    pt_unmap_range(curproc->p_pagedir, (uintptr_t) PN_TO_ADDR(lopage), (uintptr_t) PN_TO_ADDR(lopage+npages));

    return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_is_range_empty");*/
    vmarea_t * vmnewarea;
    uint32_t endvfn = startvfn+npages;
    KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
    dbg(DBG_PRINT, "GRADING3A 3.e\n");
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        if ((vmnewarea->vma_end > startvfn) && (vmnewarea->vma_end <= endvfn)){
            dbg(DBG_PRINT, "GRADING3D 2\n");
            return 0;
        }
        else if ((vmnewarea->vma_start >= startvfn) && (vmnewarea->vma_start < endvfn)){
            dbg(DBG_PRINT, "GRADING3D 2\n");
            return 0;
        }
        /*else if ((vmnewarea->vma_start <= startvfn) && (vmnewarea->vma_end >= endvfn)){
            dbg(DBG_PRINT, "GRADING MARCO 1108\n");
            return 0;
        }*/
    }list_iterate_end();
    return 1;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
    /*NOT_YET_IMPLEMENTED("VM: vmmap_read");*/
    vmarea_t *start_vmarea = vmmap_lookup(map, ADDR_TO_PN(vaddr));
    vmarea_t *end_vmarea = vmmap_lookup(map, ADDR_TO_PN((size_t)vaddr+count));

    vmarea_t * vmnewarea;
    size_t offset = PAGE_OFFSET(vaddr);
    size_t bytesread = 0;
    dbg(DBG_PRINT, "GRADING3B 1\n");
    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        if ((vmnewarea->vma_end >= start_vmarea->vma_end) && (vmnewarea->vma_start <= end_vmarea->vma_start)){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            struct pframe *curpf;
            uint32_t i = 0;
            for( i = vmnewarea->vma_start ; i < vmnewarea->vma_end; i++){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                if(ADDR_TO_PN((size_t)vaddr+bytesread) == i){
                    dbg(DBG_PRINT, "GRADING3B 1\n");
                    int retval = pframe_lookup(vmnewarea->vma_obj, i - vmnewarea->vma_start + vmnewarea->vma_off, 0, &curpf); /*Unsure about 0*/
                    /*if (retval < 0){
                        dbg(DBG_PRINT, "GRADING MARCO 1112\n");
                        return retval;
                    }*/
                    /*if (count > PAGE_SIZE-offset){
                        dbg(DBG_PRINT, "GRADING MARCO 1113\n");
                        memcpy((char *)buf+bytesread, (char *)curpf->pf_addr+offset, PAGE_SIZE-offset);
                    }*/
                    if (count <= PAGE_SIZE-offset){
                        dbg(DBG_PRINT, "GRADING3B 1\n");
                        memcpy((char *)buf+bytesread, (char *)curpf->pf_addr+offset, count);
                        return 0;                    
                    }
                    bytesread += PAGE_SIZE-offset;
                    count -= PAGE_SIZE-offset;
                    offset = 0;
                }
            }    
        }
    }list_iterate_end();
    return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_write");*/
    vmarea_t *start_vmarea = vmmap_lookup(map, ADDR_TO_PN(vaddr));
    vmarea_t *end_vmarea = vmmap_lookup(map, ADDR_TO_PN((size_t)vaddr+count-1));
    vmarea_t * vmnewarea;
    size_t offset = PAGE_OFFSET(vaddr); 
    size_t byteswrite = 0;
    dbg(DBG_PRINT, "GRADING3B 1\n");

    list_iterate_begin(&map->vmm_list, vmnewarea, vmarea_t, vma_plink){
        if ((vmnewarea->vma_end >= start_vmarea->vma_end) && (vmnewarea->vma_start <= end_vmarea->vma_start)){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            struct pframe *curpf;
            uint32_t i = 0;
            for( i = vmnewarea->vma_start ; i < vmnewarea->vma_end; i++){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                if(ADDR_TO_PN((size_t)vaddr+byteswrite) == i){
                    dbg(DBG_PRINT, "GRADING3B 1\n");
                    int retval = pframe_lookup(vmnewarea->vma_obj, i - vmnewarea->vma_start + vmnewarea->vma_off, 1, &curpf); /*Unsure about 0*/
                    /*if (retval < 0){
                        dbg(DBG_PRINT, "GRADING MARCO 1119\n");
                        return retval;
                    }*/
                    if (count > PAGE_SIZE-offset){
                        dbg(DBG_PRINT, "GRADING3B 6\n");
                        memcpy((char *)curpf->pf_addr+offset, (char *)buf+byteswrite, PAGE_SIZE-offset);
                    }
                    else{   
                        dbg(DBG_PRINT, "GRADING3B 1\n");
                        memcpy((char *)curpf->pf_addr+offset, (char *)buf+byteswrite, count);
                        retval = pframe_dirty(curpf);

                        return retval;
                    }
                    retval = pframe_dirty(curpf);
                    /*if (retval < 0){
                        dbg(DBG_PRINT, "GRADING MARCO 1123\n");
                        return retval;  
                    }*/
                    byteswrite += PAGE_SIZE-offset;
                    count -= PAGE_SIZE-offset;
                    offset = 0;
                }
            }    
        }

    }list_iterate_end();

    return 0;
}
