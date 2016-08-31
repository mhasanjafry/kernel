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

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_init");*/
    shadow_allocator = slab_allocator_create("shadow", sizeof(mmobj_t));
    KASSERT(NULL != shadow_allocator);
    dbg(DBG_PRINT, "GRADING3A 6.a\n");
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_create");*/
    mmobj_t *mmobject = slab_obj_alloc(shadow_allocator);
    /*if(NULL == mmobject){
        dbg(DBG_PRINT, "GRADING MARCO 1048\n");
        return NULL;
    }*/
    mmobj_init(mmobject, &shadow_mmobj_ops);
    mmobject->mmo_un.mmo_bottom_obj = NULL;
    mmobject->mmo_refcount = 1;
    dbg(DBG_PRINT, "GRADING3B 1\n");
    /*shadow_ref(mmobject);*/

    return mmobject;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
    KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
    dbg(DBG_PRINT, "GRADING3A 6.b\n");
    /*NOT_YET_IMPLEMENTED("VM: shadow_ref");*/
    o->mmo_refcount++;
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_put");*/
    KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
    dbg(DBG_PRINT, "GRADING3A 6.c\n");

    if(o->mmo_refcount-1 == o->mmo_nrespages){
        pframe_t *page_frame;
        dbg(DBG_PRINT, "GRADING3B 1\n");
        list_iterate_begin(&(o->mmo_respages), page_frame, pframe_t, pf_olink){
            pframe_unpin(page_frame);
            dbg(DBG_PRINT, "GRADING3B 1\n");
            if(pframe_is_dirty(page_frame)){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                pframe_clean(page_frame);
            }
            pframe_free(page_frame);/*I believe that this uncahes the pages*/

        }list_iterate_end();
        o->mmo_shadowed->mmo_ops->put(o->mmo_shadowed);

    }
    o->mmo_refcount--;
    if(o->mmo_refcount == 0){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        slab_obj_free(shadow_allocator, (void *) o);
    }
    
}
/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. It is important to
 * use iteration rather than recursion here as a recursive implementation
 * can overflow the kernel stack when looking down a long shadow chain */
static int
shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_lookuppage");*/
    int err = 0;
    if(!forwrite){
        dbg(DBG_PRINT, "GRADING3B 1\n");
        mmobj_t *new_obj = o;
        while(new_obj){
            dbg(DBG_PRINT, "GRADING3B 1\n");
            pframe_t *frame;
            list_iterate_begin(&new_obj->mmo_respages, frame, pframe_t, pf_olink){
                dbg(DBG_PRINT, "GRADING3B 1\n");
                if(frame->pf_pagenum == pagenum){
                    *pf = frame;
                    dbg(DBG_PRINT, "GRADING3A 6d\n");
            /*dbg(DBG_PRINT, "GRADING3A 6.d\n");*/
                    KASSERT(NULL != (*pf));
                    KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                    return 0;
                }

            }list_iterate_end();

            new_obj = new_obj->mmo_shadowed;
        }
        err = o->mmo_un.mmo_bottom_obj->mmo_ops->lookuppage(o, pagenum, forwrite, pf);
        if (err==0){
            dbg(DBG_PRINT, "GRADING3A 6d\n");            /*dbg(DBG_PRINT, "GRADING3A 6.d\n");*/
            KASSERT(NULL != (*pf));
            KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
        }
        return err;            
    }else{
        pframe_t *frame = pframe_get_resident(o, pagenum);
        /*dbg(DBG_PRINT, "GRADING 1057\n");*/

        if(frame!=NULL){
            *pf=frame;
            dbg(DBG_PRINT, "GRADING3A 6d\n");
            /*dbg(DBG_PRINT, "GRADING3A 6.d\n");*/
            KASSERT(NULL != (*pf));
            KASSERT(pagenum == (*pf)->pf_pagenum);
            return 0;
        }
        err = pframe_get(o, pagenum, pf);
        if (err==0){
            dbg(DBG_PRINT, "GRADING3A 6d\n");
            /*dbg(DBG_PRINT, "GRADING3A 6.d\n");*/
            KASSERT(NULL != (*pf));
            KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
        }
        else{
            dbg(DBG_PRINT, "GRADING3D 2\n");
            shadow_dirtypage(o, *pf);
        }
        return err;
    }
    /*MARCO: Possible warning for not returning*/
}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain).
 * It is important to use iteration rather than recursion here as a 
 * recursive implementation can overflow the kernel stack when 
 * looking down a long shadow chain */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_fillpage");*/
  /*  mmobj_t *new_obj = o;
    pframe_pin(pf);
    while(new_obj->mmo_shadowed){
        pframe_t *frame;
        list_iterate_begin(&new_obj->mmo_respages, frame, pframe_t, pf_olink){
            if(frame->pf_pagenum == pf->pf_pagenum){
                memcpy(pf->pf_addr, frame->pf_addr, PAGE_SIZE);
                return 0;
            }
        }list_iterate_end();
        new_obj = new_obj->mmo_shadowed;
    }
    pframe_t *new_frame = NULL;
    int error = pframe_get(o->mmo_un.mmo_bottom_obj, pf->pf_pagenum, &new_frame);
    if(error < 0){
        pframe_unpin(pf);
        return error;
    }
    memcpy(pf->pf_addr, new_frame->pf_addr, PAGE_SIZE);*/

    KASSERT(pframe_is_busy(pf));
    dbg(DBG_PRINT, "GRADING3A 6.e\n");
    KASSERT(!pframe_is_pinned(pf));
    dbg(DBG_PRINT, "GRADING3A 6.e\n");

    pframe_t *origin_pframe;
    if( pframe_lookup(o->mmo_shadowed, pf->pf_pagenum, 0, &origin_pframe) != 0 ){
        dbg(DBG_PRINT, "GRADING3D 2\n");
        return -1;
    }
    pframe_pin(pf);
    memcpy(pf->pf_addr, origin_pframe->pf_addr, PAGE_SIZE);
    return 0;
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_dirtypage");*/
    /*MARCO: return o->mmo_ops->dirtypage(o, pf);*/
    dbg(DBG_PRINT, "GRADING3B 1\n");
    pframe_set_dirty(pf);
    return 0;
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
    /*NOT_YET_IMPLEMENTED("VM: shadow_cleanpage");*/
    /*MARCO: return o->mmo_ops->cleanpage(o, pf);*/
    dbg(DBG_PRINT, "GRADING3B 1\n");
    pframe_t  *curr_page;
    pframe_lookup(o, pf->pf_pagenum, 1, &curr_page);
    memcpy(curr_page->pf_addr, pf->pf_addr, PAGE_SIZE);
    pframe_clear_dirty(curr_page);
    return 0;
}