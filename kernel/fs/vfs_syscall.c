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

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.13 2015/12/15 14:38:24 william Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read fs_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_read");*/
	
	if (fd < 0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	file_t *curfile;
	if ((curfile = fget(fd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	if ( (curfile->f_mode & FMODE_READ) == 0 ){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curfile);
		return -EBADF;
	}

	if (S_ISDIR(curfile->f_vnode->vn_mode)){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curfile);
        return -EISDIR;
	}

    int bytes_read = curfile->f_vnode->vn_ops->read(curfile->f_vnode, curfile->f_pos, buf, nbytes);
    
	if (bytes_read >= 0 ){
		dbg(DBG_PRINT, "GRADING2B\n");
    	curfile->f_pos += bytes_read;
    }
    dbg(DBG_PRINT, "GRADING2B\n");

    fput(curfile);

    return bytes_read;
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * fs_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_write");*/
	if (fd < 0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}
	file_t *curfile;
	if ((curfile = fget(fd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	if ( (curfile->f_mode & FMODE_WRITE) == 0 ){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curfile);
		return -EBADF;
	}

	if (S_ISDIR(curfile->f_vnode->vn_mode)){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curfile);
        return -EISDIR;
	}

	if ((curfile->f_mode & FMODE_APPEND) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		do_lseek(fd, 0, SEEK_END);
	}

    int bytes_write = curfile->f_vnode->vn_ops->write(curfile->f_vnode, curfile->f_pos, buf, nbytes);
    if (bytes_write >= 0 ){
    	KASSERT((S_ISCHR(curfile->f_vnode->vn_mode)) || (S_ISBLK(curfile->f_vnode->vn_mode)) || ((S_ISREG(curfile->f_vnode->vn_mode)) && (curfile->f_pos <= curfile->f_vnode->vn_len)));
    	dbg(DBG_PRINT, "GRADING2A 3.a\n");
    	curfile->f_pos += bytes_write;
    }
    dbg(DBG_PRINT, "GRADING2B\n");
    fput(curfile);

    return bytes_write;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_close");*/
	if(fd <0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
        return -EBADF;
    }

    if ( curproc -> p_files[fd] == NULL){
    	dbg(DBG_PRINT, "GRADING2B\n");
    	return -EBADF;
    }
    dbg(DBG_PRINT, "GRADING2B\n");
    fput(curproc -> p_files[fd]);
    curproc -> p_files[fd] = NULL;

	return 0;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_dup");*/
	if (fd < 0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	int new_fd = get_empty_fd(curproc);
    if(new_fd == -EMFILE){
    	dbg(DBG_PRINT, "GRADING2B\n");
        return new_fd;
    }

    file_t *curfile;
	if ((curfile = fget(fd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}
	dbg(DBG_PRINT, "GRADING2B\n");
    curproc->p_files[new_fd] = curfile;
    
    return new_fd;

}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_dup");*/
	if (nfd < 0 || nfd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	if (ofd < 0 || ofd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

    file_t *curfile;
	if ((curfile = fget(ofd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}

	if (curproc->p_files[nfd] != NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		if(curproc -> p_files[ofd] != curproc -> p_files[nfd]){
			dbg(DBG_PRINT, "GRADING2B\n");
			do_close(nfd);
			curproc->p_files[nfd] = curproc->p_files[ofd];
		}
		else{
			dbg(DBG_PRINT, "GRADING2B\n");
			fput(curproc->p_files[nfd]); /*changes*/
		}
	}
	else{
		dbg(DBG_PRINT, "GRADING2B\n");
		curproc->p_files[nfd] = curproc->p_files[ofd];
	}
    dbg(DBG_PRINT, "GRADING2B\n");
    return nfd;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_mknod");*/
	if( !(mode == S_IFCHR || mode == S_IFBLK) ){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EINVAL;
	}

	size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode  = NULL;
    int retval_dir_namev = 0;
    if ((retval_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0){
    	dbg(DBG_PRINT, "GRADING2B\n");
        return retval_dir_namev;
    }

    vnode_t *res_vnode = NULL;
    if ( lookup(dir_vnode, name, namelen, &res_vnode) == 0){
    	dbg(DBG_PRINT, "GRADING2B\n");
    	vput(dir_vnode);
    	vput(res_vnode);
    	return -EEXIST;
    }
    KASSERT(dir_vnode->vn_ops->mknod != NULL);
    dbg(DBG_PRINT, "GRADING2A 3.b\n");

    int retval_mknod = 0;
	retval_mknod = dir_vnode->vn_ops->mknod(dir_vnode, name, namelen, mode, devid);
	vput(dir_vnode);
	return retval_mknod;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_mkdir");*/
	size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode = NULL;

    int retval_dir_namev = 0;
	if ( (retval_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		return retval_dir_namev;
	}

	dbg(DBG_PRINT, "GRADING2B\n");
	vnode_t *result = NULL;
	int retval_mkdir = 0;
	if ( lookup(dir_vnode, name, namelen, &result) != 0){
		KASSERT(dir_vnode->vn_ops->mkdir != NULL);
    	dbg(DBG_PRINT, "GRADING2A 3.c\n");
		retval_mkdir = dir_vnode->vn_ops->mkdir(dir_vnode, name, namelen);
		vput(dir_vnode);
		return retval_mkdir;
	}else{
		dbg(DBG_PRINT, "GRADING2B\n");
		vput(dir_vnode);
		vput(result);
		return -EEXIST;
	}
	
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_rmdir");*/
	size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode = NULL;

    int retval_dir_namev = 0;
	if ( (retval_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		return retval_dir_namev;
	}

	if(name[0] == '.'){
		if (name[1]== '.'){
			dbg(DBG_PRINT, "GRADING2B\n");
			vput(dir_vnode);/*changes*/
			return -ENOTEMPTY;
		}
		dbg(DBG_PRINT, "GRADING2B\n");
		vput(dir_vnode); /*changes*/
		return -EINVAL;
	}
	dbg(DBG_PRINT, "GRADING2B\n");

	vnode_t *result = NULL;
	if ( lookup(dir_vnode, name, namelen, &result) == 0){

		if(!S_ISDIR(result->vn_mode)){
			vput(dir_vnode);
	    	vput(result);
	    	dbg(DBG_PRINT, "GRADING2B\n");
	    	return -ENOTDIR;
    	}
    	KASSERT(dir_vnode->vn_ops->rmdir != NULL);
    	dbg(DBG_PRINT, "GRADING2A 3.d\n");
    	dbg(DBG_PRINT, "GRADING2B\n");
		int retval_rmdir = dir_vnode->vn_ops->rmdir(dir_vnode, name, namelen);
		vput(dir_vnode);
	    vput(result);
		return retval_rmdir;
	}else{
		dbg(DBG_PRINT, "GRADING2B\n");
		vput(dir_vnode);
		return -ENOENT;
	}
}

/*
 * Same as do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EISDIR
 *        path refers to a directory.
 *      o ENOENT
 *        A component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_unlink");*/
	size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode = NULL;

    int retval_dir_namev = 0;
	if ( (retval_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		return retval_dir_namev;
	}
	dbg(DBG_PRINT, "GRADING2B\n");
	vnode_t *result = NULL;
	if ( lookup(dir_vnode, name, namelen, &result) == 0){
		if(S_ISDIR(result->vn_mode)){
			dbg(DBG_PRINT, "GRADING2B\n");
	    	vput(dir_vnode);
	    	vput(result);
	    	return -EISDIR;
    	}
    	KASSERT(dir_vnode -> vn_ops->unlink != NULL);
    	dbg(DBG_PRINT, "GRADING2A 3.e\n");
    	dbg(DBG_PRINT, "GRADING2B\n");
		int retval_unlink = dir_vnode->vn_ops->unlink(dir_vnode, name, namelen);
		vput(dir_vnode);
	    vput(result);
	    return retval_unlink;
	}else{
		dbg(DBG_PRINT, "GRADING2B\n");
		vput(dir_vnode);
		return -ENOENT;
	}
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EISDIR
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
	/*No implementation because it is not tested in the grading guidelines tests!*/
	/*NOT_YET_IMPLEMENTED("VFS: do_link");*/
	
	size_t from_namelen = 0;
	const char *from_name = NULL;
	vnode_t *from_dir_vnode = NULL;
	vnode_t *from_result = NULL;

	int retval_dir_namev = 0;
	if ( (retval_dir_namev = dir_namev(from, &from_namelen, &from_name, NULL, &from_dir_vnode)) != 0){
		return retval_dir_namev;
	}
	if ( lookup(from_dir_vnode, from_name, from_namelen, &from_result) == 0){
		if(S_ISDIR(from_result->vn_mode)){
	    	vput(from_dir_vnode);
			vput(from_result);
	    	return -EISDIR;
		}

    	size_t namelen = 0;
    	const char *name = NULL;
    	vnode_t *dir_vnode = NULL;

		if ( (retval_dir_namev = dir_namev(to, &namelen, &name, NULL, &dir_vnode)) != 0){
			vput(from_dir_vnode);
			vput(from_result);
			return retval_dir_namev;
		}

		vnode_t *result = NULL;
		int retval_link = 0;
		if ( lookup(dir_vnode, name, namelen, &result) != 0){
			retval_link = dir_vnode->vn_ops->link(from_result, dir_vnode, name, namelen);
			vput(from_dir_vnode);
			vput(from_result);
			vput(dir_vnode);
			return retval_link;
		}else{
			vput(from_dir_vnode);
			vput(from_result);
			vput(dir_vnode);
			vput(result);
			return -EEXIST;
		}
	}else{
		vput(from_dir_vnode);
		return -EEXIST;
	}
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
	/*No implementation because it is not tested in the grading guidelines tests!*/
	NOT_YET_IMPLEMENTED("VFS: do_rename");
	return -1;
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_chdir");*/
	vnode_t *new_cwd = NULL;
	int retval_open_namev = open_namev(path, 0, &new_cwd, NULL);
    if (retval_open_namev != 0 ){
    	dbg(DBG_PRINT, "GRADING2B\n");
        return retval_open_namev;
    }
    if(!S_ISDIR(new_cwd->vn_mode)){
    	dbg(DBG_PRINT, "GRADING2B\n");
    	vput(new_cwd);
    	return -ENOTDIR;
    }
    if(curproc->p_cwd != NULL){
    	dbg(DBG_PRINT, "GRADING2B\n");
		vput(curproc->p_cwd);
	}
	dbg(DBG_PRINT, "GRADING2B\n");
	curproc->p_cwd = new_cwd;
	return 0;
}

/* Call the readdir fs_op on the given fd, filling in the given dirent_t*.
 * If the readdir fs_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_getdent");*/
	if(fd <0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
        return -EBADF;
    }

	file_t *curdir;
	if ((curdir = fget(fd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}
	if (!S_ISDIR(curdir->f_vnode->vn_mode)){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curdir);
        return -ENOTDIR;
	}

	if ( curdir->f_vnode->vn_ops->readdir == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		fput(curdir);
		return -ENOSYS;
	}
    int bytes_read = curdir->f_vnode->vn_ops->readdir(curdir->f_vnode, curdir->f_pos, dirp);

    if (bytes_read >0){
    	dbg(DBG_PRINT, "GRADING2B\n");
    	curdir->f_pos += bytes_read;
    	fput(curdir);
    	return sizeof(dirent_t);
    }else{
    	dbg(DBG_PRINT, "GRADING2B\n");
		fput(curdir);
    	return bytes_read;
    }
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_lseek");*/
	if (fd < 0 || fd >= NFILES){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}
	if(!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END)){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EINVAL;
	}
	file_t *curfile;
	if ((curfile = fget(fd)) == NULL){
		dbg(DBG_PRINT, "GRADING2B\n");
		return -EBADF;
	}
	int old_pos = curfile->f_pos;
	switch(whence){
		case SEEK_SET:
			dbg(DBG_PRINT, "GRADING2B\n");
			curfile->f_pos = 0 + offset;
			break;
		case SEEK_CUR:
			dbg(DBG_PRINT, "GRADING2B\n");
			curfile->f_pos += offset;
			break;
		case SEEK_END:
			dbg(DBG_PRINT, "GRADING2B\n");
			curfile->f_pos = curfile->f_vnode->vn_len + offset;
			break;
	}

	if(curfile->f_pos < 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		curfile->f_pos = old_pos;
		fput(curfile);
		return -EINVAL;
	}
	dbg(DBG_PRINT, "GRADING2B\n");
	fput(curfile);
	return curfile->f_pos;
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_stat(const char *path, struct stat *buf)
{
	/*NOT_YET_IMPLEMENTED("VFS: do_stat");*/
	size_t namelen = 0;
    const char *name = NULL;
    vnode_t *dir_vnode = NULL;

    int retval_dir_namev = 0;
	if ( (retval_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		return retval_dir_namev;
	}
	dbg(DBG_PRINT, "GRADING2B\n");
	vnode_t *result = NULL;
	if ( lookup(dir_vnode, name, namelen, &result) != 0){
		dbg(DBG_PRINT, "GRADING2B\n");
		vput(dir_vnode);
		return -ENOENT;
	}else{
		KASSERT(result->vn_ops->stat != NULL);
		dbg(DBG_PRINT, "GRADING2A 3.f\n");
		dbg(DBG_PRINT, "GRADING2B\n");
		result->vn_ops->stat(result, buf);
		vput(dir_vnode);
		vput(result);
		return 0;
	}
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
	NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
	return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
	NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
	return -EINVAL;
}
#endif
