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
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function, but you may want to special case
 * "." and/or ".." here depnding on your implementation.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
    /*NOT_YET_IMPLEMENTED("VFS: lookup");*/
    KASSERT(dir != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.a\n");
    KASSERT(name != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.a\n");
    KASSERT(result != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.a\n");
    if ( dir->vn_ops->lookup == NULL ){
        dbg(DBG_PRINT, "GRADING2B\n");
        return -ENOTDIR;
    }

    if ( len == 0){/*kernel 3 change*/
        vget(dir->vn_fs, dir->vn_vno);/*kernel 3 change*/
        *result = dir;/*kernel 3 change*/
        return 0;/*kernel 3 change*/
    }/*kernel 3 change*/

    if ( len == 1 && name[0] == '.' ){
        dbg(DBG_PRINT, "GRADING2B\n");
        vget(dir->vn_fs, dir->vn_vno);
        *result = dir;
        return 0;
    }

    if ( len == 2 && name[0] == '.' && name[1] == '.' ){
        if( dir == vfs_root_vn ){
            dbg(DBG_PRINT, "GRADING2B\n");
            vget(dir->vn_fs, dir->vn_vno);
            *result = dir;
            return 0;
        }
        dbg(DBG_PRINT, "GRADING2B\n");
    }
    if ( dir->vn_ops->lookup(dir, name, len, result) == 0 ){
        dbg(DBG_PRINT, "GRADING2B\n");
        return 0;
    }
    else{
        dbg(DBG_PRINT, "GRADING2B\n");
        return -ENOENT;
    }
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
    /*NOT_YET_IMPLEMENTED("VFS: dir_namev");*/

    KASSERT(pathname != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.b\n");
    KASSERT(namelen != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.b\n");
    KASSERT(name != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.b\n");
    KASSERT(res_vnode != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.b\n");
    if(strlen(pathname)==0){
        dbg(DBG_PRINT, "GRADING2B\n");
        return -EINVAL;
    }

    const char * curr_pointer = pathname;
    if (curr_pointer[0] == '/'){
        dbg(DBG_PRINT, "GRADING2B\n");
        curr_pointer = &curr_pointer[1];
        base = vfs_root_vn;
    }else if (base == NULL){
        dbg(DBG_PRINT, "GRADING2B\n");
        base = curproc->p_cwd;
    }
    *res_vnode = base;
    KASSERT(res_vnode != NULL);
    dbg(DBG_PRINT, "GRADING2A 2.b\n");
    vget(base->vn_fs, base->vn_vno);
    const char *next_pointer = NULL;
    size_t lookup_length = 0;

    while(curr_pointer[0] == '/'){
        dbg(DBG_PRINT, "GRADING2B\n");
        curr_pointer = &curr_pointer[1];
    }
    *name = curr_pointer;
    *namelen = lookup_length;

    /*int itr = 0;*/
    vnode_t *child = NULL;
    while (strlen(curr_pointer) != 0){
        next_pointer = strchr(curr_pointer, '/');

        if (next_pointer == NULL){
            dbg(DBG_PRINT, "GRADING2B\n");
            lookup_length = strlen(curr_pointer);
            *name = curr_pointer;
            *namelen = lookup_length;
            break;
        }
        else{
            
            lookup_length = next_pointer - curr_pointer;
            while(next_pointer[0] == '/'){
                dbg(DBG_PRINT, "GRADING2B\n");
                next_pointer = &next_pointer[1];
            }
            if (strlen(next_pointer) == 0){
                dbg(DBG_PRINT, "GRADING2B\n");
                *name = curr_pointer;
                *namelen = lookup_length;
                break;
            }
            dbg(DBG_PRINT, "GRADING2B\n");
        }

        if( lookup_length >= NAME_LEN){
            dbg(DBG_PRINT, "GRADING2B\n");
            vput(*res_vnode); /*changes */
            return -ENAMETOOLONG;
        }

        if(lookup(*res_vnode, curr_pointer, lookup_length, &child) == -ENOENT){
            dbg(DBG_PRINT, "GRADING2B\n");
            vput(*res_vnode);
            return -ENOENT;
        }

        /*if(itr >= 1)*/
        vput(*res_vnode);
        if (!S_ISDIR(child->vn_mode)){
            dbg(DBG_PRINT, "GRADING2B\n");
            vput(child);
            return -ENOTDIR;
        }

        dbg(DBG_PRINT, "GRADING2B\n");
        curr_pointer = next_pointer;
        /*itr++;*/
        *res_vnode = child;
    }


    if( *namelen >= NAME_LEN){
        dbg(DBG_PRINT, "GRADING2B\n");
        vput(*res_vnode); /* Change */
        return -ENAMETOOLONG;
    }
    dbg(DBG_PRINT, "GRADING2B\n");
    return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified, and the file does
 * not exist call create() in the parent directory vnode.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
    /*NOT_YET_IMPLEMENTED("VFS: open_namev");*/
    size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode  = NULL;

    int retval_dir_namev = 0;
    if ((retval_dir_namev = dir_namev(pathname, &namelen, &name, base, &dir_vnode)) != 0){
        dbg(DBG_PRINT, "GRADING2B\n");
        return retval_dir_namev;
    }

    int retval_create = 0;
    if ( lookup(dir_vnode, name, namelen, res_vnode) == 0){
        dbg(DBG_PRINT, "GRADING2B\n");
        vput(dir_vnode);
        /*vput(*res_vnode);*/
        return retval_create;
    }else{
        if( (flag & O_CREAT) != 0 ){
            KASSERT(dir_vnode->vn_ops->create != NULL);
            dbg(DBG_PRINT, "GRADING2A 2.c\n");
            dbg(DBG_PRINT, "GRADING2B\n");
            retval_create = dir_vnode->vn_ops->create(dir_vnode, name, namelen, res_vnode);
            vput(dir_vnode);
            /*if(retval_create == 0)
                vput(*res_vnode);*/
            return retval_create;
        }
        if( (flag & O_CREAT) == 0 ){
            dbg(DBG_PRINT, "GRADING2B\n");
            vput(dir_vnode);
            return -ENOENT;
        }
        dbg(DBG_PRINT, "GRADING2B\n");
    }
    dbg(DBG_PRINT, "GRADING2B\n");
    return 0;

    /*if ( (flag & O_CREAT) != 0 ){

    }else{
        if (  )
            return 0;
    }*/
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
    NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
    return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
    NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

    return -ENOENT;
}
#endif /* __GETCWD__ */
