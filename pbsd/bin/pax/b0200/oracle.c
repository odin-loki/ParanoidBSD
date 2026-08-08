/*
 * b0200 oracle -- ftree.c, sel_subs.c, pax.c, and file_subs.c concatenated.
 * Every batch function is renamed with a ref_ prefix via the preprocessor;
 * function bodies are otherwise UNMODIFIED.
 */

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <fts.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <locale.h>
#include <paths.h>
#include <err.h>
#include <ctype.h>
#include <unistd.h>
#include <bsd/string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#define MAXBLK		64512
#define FILEBLK		10240
#define PAXPATHLEN	3072
#define LIST		0
#define EXTRACT		1
#define ARCHIVE		2
#define APPND		3
#define COPY		4
#define DEFOP		LIST
#define PAX_DIR		1
#define PAX_CHR		2
#define PAX_BLK		3
#define PAX_REG		4
#define PAX_SLK		5
#define PAX_SCK		6
#define PAX_FIF		7
#define PAX_HLK		8
#define PAX_HRG		9
#define PAX_CTG		10
#define NM_CPIO "cpio"
#define NM_PAX  "pax"
#define _TFILE_BASE	"paxXXXXXXXXXX"

#define USR_TB_SZ	317
#define GRP_TB_SZ	317
#define ATOI2(ar)	((ar)[0] - '0') * 10 + ((ar)[1] - '0'); (ar) += 2;
#define HASLOW		0x01
#define HASHIGH		0x02
#define CMPMTME		0x04
#define CMPCTME		0x08
#define CMPBOTH	(CMPMTME|CMPCTME)

typedef struct archd ARCHD;
typedef struct ftree FTREE;
typedef struct usrt USRT;
typedef struct grpt GRPT;
typedef struct time_rng TIME_RNG;
typedef struct fsub FSUB;

struct ftree {
	char		*fname;
	int		refcnt;
	int		chflg;
	struct ftree	*fow;
};

struct usrt {
	uid_t uid;
	struct usrt *fow;
};

struct grpt {
	gid_t gid;
	struct grpt *fow;
};

struct time_rng {
	time_t		low_time;
	time_t		high_time;
	int		flgs;
	struct time_rng	*fow;
};

struct archd {
	int nlen;
	char name[PAXPATHLEN+1];
	int ln_nlen;
	char ln_name[PAXPATHLEN+1];
	char *org_name;
	struct stat sb;
	off_t pad;
	off_t skip;
	unsigned long crc;
	int type;
};

int act = DEFOP;
FSUB *frmt = NULL;
int cflag;
int cwdfd;
int dflag;
int iflag;
int kflag;
int lflag;
int nflag;
int tflag;
int uflag;
int vflag;
int Dflag;
int Hflag;
int Lflag;
int Oflag;
int Xflag;
int Yflag;
int Zflag;
int vfpart;
int patime = 1;
int pmtime = 1;
int nodirs;
int pmode;
int pids;
int rmleadslash = 0;
int exit_val;
int docrc;
char *dirptr;
const char *argv0 = "pax";
sigset_t s_mask;
FILE *listf;
char *tempfile;
char *tempbase;
char *gzip_program = NULL;

static void ref_exit_hook(int) __attribute__((noreturn));
static void ref_exit_hook(int s) { _exit(s); }
#define exit ref_exit_hook

void ref_paxwarn(int, const char *, ...);
void ref_syswarn(int, int, const char *, ...);
int ref_l_strncpy(char *, const char *, int);

void ref_paxwarn(int set, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (set)
		exit_val = 1;
	if (vflag && vfpart) {
		(void)fflush(listf);
		(void)fputc('\n', stderr);
		vfpart = 0;
	}
	(void)fprintf(stderr, "%s: ", argv0);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fputc('\n', stderr);
}

void ref_syswarn(int set, int errnum, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (set)
		exit_val = 1;
	if (vflag && vfpart) {
		(void)fflush(listf);
		(void)fputc('\n', stderr);
		vfpart = 0;
	}
	(void)fprintf(stderr, "%s: ", argv0);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fputc('\n', stderr);
	if (errnum > 0 && errnum < 256)
		(void)fprintf(stderr, ": %s\n", strerror(errnum));
	else
		(void)fputc('\n', stderr);
}

int ref_l_strncpy(char *dest, const char *src, int len)
{
	char *stop;
	char *start;

	stop = dest + len;
	start = dest;
	while ((dest < stop) && (*src != '\0'))
		*dest++ = *src++;
	len = dest - start;
	while (dest < stop)
		*dest++ = '\0';
	return(len);
}

static int stub_atdir_start(void) { return 0; }
static void stub_atdir_end(void) {}
static void stub_add_atdir(char *p, dev_t d, ino_t i, time_t m, time_t a)
{
	(void)p; (void)d; (void)i; (void)m; (void)a;
}
static int stub_get_atdir(dev_t d, ino_t i, time_t *m, time_t *a)
{
	(void)d; (void)i;
	if (m) *m = 0;
	if (a) *a = 0;
	return -1;
}
static void stub_add_dir(char *n, int nl, struct stat *sb, int f)
{
	(void)n; (void)nl; (void)sb; (void)f;
}
static void stub_options(int ac, char **av) { (void)ac; (void)av; act = LIST; }
static int stub_tty_init(void) { return 0; }
static void stub_list(void) {}
static void stub_extract(void) {}
static void stub_archive(void) {}
static void stub_append(void) {}
static void stub_copy(void) {}
static void stub_ar_close(void) {}
static void stub_proc_dir(void) {}

#define atdir_start stub_atdir_start
#define atdir_end stub_atdir_end
#define add_atdir stub_add_atdir
#define get_atdir stub_get_atdir
#define add_dir stub_add_dir
#define options stub_options
#define tty_init stub_tty_init
#define list stub_list
#define extract stub_extract
#define archive stub_archive
#define append stub_append
#define copy stub_copy
#define ar_close stub_ar_close
#define proc_dir stub_proc_dir

int ref_ftree_start;
int ref_ftree_add;
int ref_ftree_sel;
int ref_ftree_notsel;
int ref_ftree_chk;
int ref_next_file;
int ref_sel_chk;
int ref_usr_add;
int ref_grp_add;
int ref_trng_add;
int ref_main;
int ref_sig_cleanup;
int ref_file_creat;
int ref_file_close;
int ref_lnk_creat;
int ref_cross_lnk;
int ref_chk_same;
int ref_node_creat;
int ref_unlnk_exist;
int ref_chk_path;
int ref_set_ftime;
int ref_set_ids;
int ref_set_pmode;
int ref_file_write;
int ref_file_flush;
int ref_rdfile_close;
int ref_set_crc;
#define paxwarn ref_paxwarn
#define syswarn ref_syswarn
#define l_strncpy ref_l_strncpy
#define ftree_start ref_ftree_start
#define ftree_add ref_ftree_add
#define ftree_sel ref_ftree_sel
#define ftree_notsel ref_ftree_notsel
#define ftree_chk ref_ftree_chk
#define next_file ref_next_file
#define sel_chk ref_sel_chk
#define usr_add ref_usr_add
#define grp_add ref_grp_add
#define trng_add ref_trng_add
#define main ref_main
#define sig_cleanup ref_sig_cleanup
#define file_creat ref_file_creat
#define file_close ref_file_close
#define lnk_creat ref_lnk_creat
#define cross_lnk ref_cross_lnk
#define chk_same ref_chk_same
#define node_creat ref_node_creat
#define unlnk_exist ref_unlnk_exist
#define chk_path ref_chk_path
#define set_ftime ref_set_ftime
#define set_ids ref_set_ids
#define set_pmode ref_set_pmode
#define file_write ref_file_write
#define file_flush ref_file_flush
#define rdfile_close ref_rdfile_close
#define set_crc ref_set_crc

/* ======================= ftree.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992 Keith Muller.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
 * routines to interface with the fts library function.
 *
 * file args supplied to pax are stored on a single linked list (of type FTREE)
 * and given to fts to be processed one at a time. pax "selects" files from
 * the expansion of each arg into the corresponding file tree (if the arg is a
 * directory, otherwise the node itself is just passed to pax). The selection
 * is modified by the -n and -u flags. The user is informed when a specific
 * file arg does not generate any selected files. -n keeps expanding the file
 * tree arg until one of its files is selected, then skips to the next file
 * arg. when the user does not supply the file trees as command line args to
 * pax, they are read from stdin
 */

static FTS *ftsp = NULL;		/* current FTS handle */
static int ftsopts;			/* options to be used on fts_open */
static char *farray[2];			/* array for passing each arg to fts */
static FTREE *fthead = NULL;		/* head of linked list of file args */
static FTREE *fttail = NULL;		/* tail of linked list of file args */
static FTREE *ftcur = NULL;		/* current file arg being processed */
static FTSENT *ftent = NULL;		/* current file tree entry */
static int ftree_skip;			/* when set skip to next file arg */

static int ftree_arg(void);

/*
 * ftree_start()
 *	initialize the options passed to fts_open() during this run of pax
 *	options are based on the selection of pax options by the user
 *	fts_start() also calls fts_arg() to open the first valid file arg. We
 *	also attempt to reset directory access times when -t (tflag) is set.
 * Return:
 *	0 if there is at least one valid file arg to process, -1 otherwise
 */

int
ftree_start(void)
{
	/*
	 * Set up the operation mode of fts, open the first file arg. We must
	 * use FTS_NOCHDIR, as the user may have to open multiple archives and
	 * if fts did a chdir off into the boondocks, we may create an archive
	 * volume in a place where the user did not expect to.
	 */
	ftsopts = FTS_NOCHDIR;

	/*
	 * optional user flags that effect file traversal
	 * -H command line symlink follow only (half follow)
	 * -L follow symlinks (logical)
	 * -P do not follow symlinks (physical). This is the default.
	 * -X do not cross over mount points
	 * -t preserve access times on files read.
	 * -n select only the first member of a file tree when a match is found
	 * -d do not extract subtrees rooted at a directory arg.
	 */
	if (Lflag)
		ftsopts |= FTS_LOGICAL;
	else
		ftsopts |= FTS_PHYSICAL;
	if (Hflag)
		ftsopts |= FTS_COMFOLLOW;
	if (Xflag)
		ftsopts |= FTS_XDEV;

	if ((fthead == NULL) && ((farray[0] = malloc(PAXPATHLEN+2)) == NULL)) {
		paxwarn(1, "Unable to allocate memory for file name buffer");
		return(-1);
	}

	if (ftree_arg() < 0)
		return(-1);
	if (tflag && (atdir_start() < 0))
		return(-1);
	return(0);
}

/*
 * ftree_add()
 *	add the arg to the linked list of files to process. Each will be
 *	processed by fts one at a time
 * Return:
 *	0 if added to the linked list, -1 if failed
 */

int
ftree_add(char *str, int chflg)
{
	FTREE *ft;
	int len;

	/*
	 * simple check for bad args
	 */
	if ((str == NULL) || (*str == '\0')) {
		paxwarn(0, "Invalid file name argument");
		return(-1);
	}

	/*
	 * allocate FTREE node and add to the end of the linked list (args are
	 * processed in the same order they were passed to pax). Get rid of any
	 * trailing / the user may pass us. (watch out for / by itself).
	 */
	if ((ft = (FTREE *)malloc(sizeof(FTREE))) == NULL) {
		paxwarn(0, "Unable to allocate memory for filename");
		return(-1);
	}

	if (((len = strlen(str) - 1) > 0) && (str[len] == '/'))
		str[len] = '\0';
	ft->fname = str;
	ft->refcnt = 0;
	ft->chflg = chflg;
	ft->fow = NULL;
	if (fthead == NULL) {
		fttail = fthead = ft;
		return(0);
	}
	fttail->fow = ft;
	fttail = ft;
	return(0);
}

/*
 * ftree_sel()
 *	this entry has been selected by pax. bump up reference count and handle
 *	-n and -d processing.
 */

void
ftree_sel(ARCHD *arcn)
{
	/*
	 * set reference bit for this pattern. This linked list is only used
	 * when file trees are supplied pax as args. The list is not used when
	 * the trees are read from stdin.
	 */
	if (ftcur != NULL)
		ftcur->refcnt = 1;

	/*
	 * if -n we are done with this arg, force a skip to the next arg when
	 * pax asks for the next file in next_file().
	 * if -d we tell fts only to match the directory (if the arg is a dir)
	 * and not the entire file tree rooted at that point.
	 */
	if (nflag)
		ftree_skip = 1;

	if (!dflag || (arcn->type != PAX_DIR))
		return;

	if (ftent != NULL)
		(void)fts_set(ftsp, ftent, FTS_SKIP);
}

/*
 * ftree_notsel()
 *	this entry has not been selected by pax.
 */

void
ftree_notsel(void)
{
	if (ftent != NULL)
		(void)fts_set(ftsp, ftent, FTS_SKIP);
}

/*
 * ftree_chk()
 *	called at end on pax execution. Prints all those file args that did not
 *	have a selected member (reference count still 0)
 */

void
ftree_chk(void)
{
	FTREE *ft;
	int wban = 0;

	/*
	 * make sure all dir access times were reset.
	 */
	if (tflag)
		atdir_end();

	/*
	 * walk down list and check reference count. Print out those members
	 * that never had a match
	 */
	for (ft = fthead; ft != NULL; ft = ft->fow) {
		if ((ft->refcnt > 0) || ft->chflg)
			continue;
		if (wban == 0) {
			paxwarn(1,"WARNING! These file names were not selected:");
			++wban;
		}
		(void)fprintf(stderr, "%s\n", ft->fname);
	}
}

/*
 * ftree_arg()
 *	Get the next file arg for fts to process. Can be from either the linked
 *	list or read from stdin when the user did not them as args to pax. Each
 *	arg is processed until the first successful fts_open().
 * Return:
 *	0 when the next arg is ready to go, -1 if out of file args (or EOF on
 *	stdin).
 */

static int
ftree_arg(void)
{
	char *pt;

	/*
	 * close off the current file tree
	 */
	if (ftsp != NULL) {
		(void)fts_close(ftsp);
		ftsp = NULL;
	}

	/*
	 * keep looping until we get a valid file tree to process. Stop when we
	 * reach the end of the list (or get an eof on stdin)
	 */
	for(;;) {
		if (fthead == NULL) {
			/*
			 * the user didn't supply any args, get the file trees
			 * to process from stdin;
			 */
			if (fgets(farray[0], PAXPATHLEN+1, stdin) == NULL)
				return(-1);
			if ((pt = strchr(farray[0], '\n')) != NULL)
				*pt = '\0';
		} else {
			/*
			 * the user supplied the file args as arguments to pax
			 */
			if (ftcur == NULL)
				ftcur = fthead;
			else if ((ftcur = ftcur->fow) == NULL)
				return(-1);
			if (ftcur->chflg) {
				/* First fchdir() back... */
				if (fchdir(cwdfd) < 0) {
					syswarn(1, errno,
					  "Can't fchdir to starting directory");
					return(-1);
				}
				if (chdir(ftcur->fname) < 0) {
					syswarn(1, errno, "Can't chdir to %s",
					    ftcur->fname);
					return(-1);
				}
				continue;
			} else
				farray[0] = ftcur->fname;
		}

		/*
		 * Watch it, fts wants the file arg stored in an array of char
		 * ptrs, with the last one a null. We use a two element array
		 * and set farray[0] to point at the buffer with the file name
		 * in it. We cannot pass all the file args to fts at one shot
		 * as we need to keep a handle on which file arg generates what
		 * files (the -n and -d flags need this). If the open is
		 * successful, return a 0.
		 */
		if ((ftsp = fts_open(farray, ftsopts, NULL)) != NULL)
			break;
	}
	return(0);
}

/*
 * next_file()
 *	supplies the next file to process in the supplied archd structure.
 * Return:
 *	0 when contents of arcn have been set with the next file, -1 when done.
 */

int
next_file(ARCHD *arcn)
{
	int cnt;
	time_t atime;
	time_t mtime;

	/*
	 * ftree_sel() might have set the ftree_skip flag if the user has the
	 * -n option and a file was selected from this file arg tree. (-n says
	 * only one member is matched for each pattern) ftree_skip being 1
	 * forces us to go to the next arg now.
	 */
	if (ftree_skip) {
		/*
		 * clear and go to next arg
		 */
		ftree_skip = 0;
		if (ftree_arg() < 0)
			return(-1);
	}

	/*
	 * loop until we get a valid file to process
	 */
	for(;;) {
		if ((ftent = fts_read(ftsp)) == NULL) {
			/*
			 * out of files in this tree, go to next arg, if none
			 * we are done
			 */
			if (ftree_arg() < 0)
				return(-1);
			continue;
		}

		/*
		 * handle each type of fts_read() flag
		 */
		switch(ftent->fts_info) {
		case FTS_D:
		case FTS_DEFAULT:
		case FTS_F:
		case FTS_SL:
		case FTS_SLNONE:
			/*
			 * these are all ok
			 */
			break;
		case FTS_DP:
			/*
			 * already saw this directory. If the user wants file
			 * access times reset, we use this to restore the
			 * access time for this directory since this is the
			 * last time we will see it in this file subtree
			 * remember to force the time (this is -t on a read
			 * directory, not a created directory).
			 */
			if (!tflag || (get_atdir(ftent->fts_statp->st_dev,
			    ftent->fts_statp->st_ino, &mtime, &atime) < 0))
				continue;
			set_ftime(ftent->fts_path, mtime, atime, 1);
			continue;
		case FTS_DC:
			/*
			 * fts claims a file system cycle
			 */
			paxwarn(1,"File system cycle found at %s",ftent->fts_path);
			continue;
		case FTS_DNR:
			syswarn(1, ftent->fts_errno,
			    "Unable to read directory %s", ftent->fts_path);
			continue;
		case FTS_ERR:
			syswarn(1, ftent->fts_errno,
			    "File system traversal error");
			continue;
		case FTS_NS:
		case FTS_NSOK:
			syswarn(1, ftent->fts_errno,
			    "Unable to access %s", ftent->fts_path);
			continue;
		}

		/*
		 * ok got a file tree node to process. copy info into arcn
		 * structure (initialize as required)
		 */
		arcn->skip = 0;
		arcn->pad = 0;
		arcn->ln_nlen = 0;
		arcn->ln_name[0] = '\0';
		arcn->sb = *(ftent->fts_statp);

		/*
		 * file type based set up and copy into the arcn struct
		 * SIDE NOTE:
		 * we try to reset the access time on all files and directories
		 * we may read when the -t flag is specified. files are reset
		 * when we close them after copying. we reset the directories
		 * when we are done with their file tree (we also clean up at
		 * end in case we cut short a file tree traversal). However
		 * there is no way to reset access times on symlinks.
		 */
		switch(S_IFMT & arcn->sb.st_mode) {
		case S_IFDIR:
			arcn->type = PAX_DIR;
			if (!tflag)
				break;
			add_atdir(ftent->fts_path, arcn->sb.st_dev,
			    arcn->sb.st_ino, arcn->sb.st_mtime,
			    arcn->sb.st_atime);
			break;
		case S_IFCHR:
			arcn->type = PAX_CHR;
			break;
		case S_IFBLK:
			arcn->type = PAX_BLK;
			break;
		case S_IFREG:
			/*
			 * only regular files with have data to store on the
			 * archive. all others will store a zero length skip.
			 * the skip field is used by pax for actual data it has
			 * to read (or skip over).
			 */
			arcn->type = PAX_REG;
			arcn->skip = arcn->sb.st_size;
			break;
		case S_IFLNK:
			arcn->type = PAX_SLK;
			/*
			 * have to read the symlink path from the file
			 */
			if ((cnt = readlink(ftent->fts_path, arcn->ln_name,
			    PAXPATHLEN - 1)) < 0) {
				syswarn(1, errno, "Unable to read symlink %s",
				    ftent->fts_path);
				continue;
			}
			/*
			 * set link name length, watch out readlink does not
			 * always NUL terminate the link path
			 */
			arcn->ln_name[cnt] = '\0';
			arcn->ln_nlen = cnt;
			break;
		case S_IFSOCK:
			/*
			 * under BSD storing a socket is senseless but we will
			 * let the format specific write function make the
			 * decision of what to do with it.
			 */
			arcn->type = PAX_SCK;
			break;
		case S_IFIFO:
			arcn->type = PAX_FIF;
			break;
		}
		break;
	}

	/*
	 * copy file name, set file name length
	 */
	arcn->nlen = l_strncpy(arcn->name, ftent->fts_path, sizeof(arcn->name) - 1);
	arcn->name[arcn->nlen] = '\0';
	arcn->org_name = ftent->fts_path;
	return(0);
}

/* ======================= sel_subs.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992 Keith Muller.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */




static int str_sec(const char *, time_t *);
static int usr_match(ARCHD *);
static int grp_match(ARCHD *);
static int trng_match(ARCHD *);

static TIME_RNG *trhead = NULL;		/* time range list head */
static TIME_RNG *trtail = NULL;		/* time range list tail */
static USRT **usrtb = NULL;		/* user selection table */
static GRPT **grptb = NULL;		/* group selection table */

/*
 * Routines for selection of archive members
 */

/*
 * sel_chk()
 *	check if this file matches a specified uid, gid or time range
 * Return:
 *	0 if this archive member should be processed, 1 if it should be skipped
 */

int
sel_chk(ARCHD *arcn)
{
	if (((usrtb != NULL) && usr_match(arcn)) ||
	    ((grptb != NULL) && grp_match(arcn)) ||
	    ((trhead != NULL) && trng_match(arcn)))
		return(1);
	return(0);
}

/*
 * User/group selection routines
 *
 * Routines to handle user selection of files based on the file uid/gid. To
 * add an entry, the user supplies either the name or the uid/gid starting with
 * a # on the command line. A \# will escape the #.
 */

/*
 * usr_add()
 *	add a user match to the user match hash table
 * Return:
 *	0 if added ok, -1 otherwise;
 */

int
usr_add(char *str)
{
	u_int indx;
	USRT *pt;
	struct passwd *pw;
	uid_t uid;

	/*
	 * create the table if it doesn't exist
	 */
	if ((str == NULL) || (*str == '\0'))
		return(-1);
	if ((usrtb == NULL) &&
 	    ((usrtb = (USRT **)calloc(USR_TB_SZ, sizeof(USRT *))) == NULL)) {
		paxwarn(1, "Unable to allocate memory for user selection table");
		return(-1);
	}

	/*
	 * figure out user spec
	 */
	if (str[0] != '#') {
		/*
		 * it is a user name, \# escapes # as first char in user name
		 */
		if ((str[0] == '\\') && (str[1] == '#'))
			++str;
		if ((pw = getpwnam(str)) == NULL) {
			paxwarn(1, "Unable to find uid for user: %s", str);
			return(-1);
		}
		uid = (uid_t)pw->pw_uid;
	} else
		uid = (uid_t)strtoul(str+1, NULL, 10);
	endpwent();

	/*
	 * hash it and go down the hash chain (if any) looking for it
	 */
	indx = ((unsigned)uid) % USR_TB_SZ;
	if ((pt = usrtb[indx]) != NULL) {
		while (pt != NULL) {
			if (pt->uid == uid)
				return(0);
			pt = pt->fow;
		}
	}

	/*
	 * uid is not yet in the table, add it to the front of the chain
	 */
	if ((pt = (USRT *)malloc(sizeof(USRT))) != NULL) {
		pt->uid = uid;
		pt->fow = usrtb[indx];
		usrtb[indx] = pt;
		return(0);
	}
	paxwarn(1, "User selection table out of memory");
	return(-1);
}

/*
 * usr_match()
 *	check if this files uid matches a selected uid.
 * Return:
 *	0 if this archive member should be processed, 1 if it should be skipped
 */

static int
usr_match(ARCHD *arcn)
{
	USRT *pt;

	/*
	 * hash and look for it in the table
	 */
	pt = usrtb[((unsigned)arcn->sb.st_uid) % USR_TB_SZ];
	while (pt != NULL) {
		if (pt->uid == arcn->sb.st_uid)
			return(0);
		pt = pt->fow;
	}

	/*
	 * not found
	 */
	return(1);
}

/*
 * grp_add()
 *	add a group match to the group match hash table
 * Return:
 *	0 if added ok, -1 otherwise;
 */

int
grp_add(char *str)
{
	u_int indx;
	GRPT *pt;
	struct group *gr;
	gid_t gid;

	/*
	 * create the table if it doesn't exist
	 */
	if ((str == NULL) || (*str == '\0'))
		return(-1);
	if ((grptb == NULL) &&
 	    ((grptb = (GRPT **)calloc(GRP_TB_SZ, sizeof(GRPT *))) == NULL)) {
		paxwarn(1, "Unable to allocate memory fo group selection table");
		return(-1);
	}

	/*
	 * figure out user spec
	 */
	if (str[0] != '#') {
		/*
		 * it is a group name, \# escapes # as first char in group name
		 */
		if ((str[0] == '\\') && (str[1] == '#'))
			++str;
		if ((gr = getgrnam(str)) == NULL) {
			paxwarn(1,"Cannot determine gid for group name: %s", str);
			return(-1);
		}
		gid = gr->gr_gid;
	} else
		gid = (gid_t)strtoul(str+1, NULL, 10);
	endgrent();

	/*
	 * hash it and go down the hash chain (if any) looking for it
	 */
	indx = ((unsigned)gid) % GRP_TB_SZ;
	if ((pt = grptb[indx]) != NULL) {
		while (pt != NULL) {
			if (pt->gid == gid)
				return(0);
			pt = pt->fow;
		}
	}

	/*
	 * gid not in the table, add it to the front of the chain
	 */
	if ((pt = (GRPT *)malloc(sizeof(GRPT))) != NULL) {
		pt->gid = gid;
		pt->fow = grptb[indx];
		grptb[indx] = pt;
		return(0);
	}
	paxwarn(1, "Group selection table out of memory");
	return(-1);
}

/*
 * grp_match()
 *	check if this files gid matches a selected gid.
 * Return:
 *	0 if this archive member should be processed, 1 if it should be skipped
 */

static int
grp_match(ARCHD *arcn)
{
	GRPT *pt;

	/*
	 * hash and look for it in the table
	 */
	pt = grptb[((unsigned)arcn->sb.st_gid) % GRP_TB_SZ];
	while (pt != NULL) {
		if (pt->gid == arcn->sb.st_gid)
			return(0);
		pt = pt->fow;
	}

	/*
	 * not found
	 */
	return(1);
}

/*
 * Time range selection routines
 *
 * Routines to handle user selection of files based on the modification and/or
 * inode change time falling within a specified time range (the non-standard
 * -T flag). The user may specify any number of different file time ranges.
 * Time ranges are checked one at a time until a match is found (if at all).
 * If the file has a mtime (and/or ctime) which lies within one of the time
 * ranges, the file is selected. Time ranges may have a lower and/or an upper
 * value. These ranges are inclusive. When no time ranges are supplied to pax
 * with the -T option, all members in the archive will be selected by the time
 * range routines. When only a lower range is supplied, only files with a
 * mtime (and/or ctime) equal to or younger are selected. When only an upper
 * range is supplied, only files with a mtime (and/or ctime) equal to or older
 * are selected. When the lower time range is equal to the upper time range,
 * only files with a mtime (or ctime) of exactly that time are selected.
 */

/*
 * trng_add()
 *	add a time range match to the time range list.
 *	This is a non-standard pax option. Lower and upper ranges are in the
 *	format: [[[[[cc]yy]mm]dd]HH]MM[.SS] and are comma separated.
 *	Time ranges are based on current time, so 1234 would specify a time of
 *	12:34 today.
 * Return:
 *	0 if the time range was added to the list, -1 otherwise
 */

int
trng_add(char *str)
{
	TIME_RNG *pt;
	char *up_pt = NULL;
	char *stpt;
	char *flgpt;
	int dot = 0;

	/*
	 * throw out the badly formed time ranges
	 */
	if ((str == NULL) || (*str == '\0')) {
		paxwarn(1, "Empty time range string");
		return(-1);
	}

	/*
	 * locate optional flags suffix /{cm}.
	 */
	if ((flgpt = strrchr(str, '/')) != NULL)
		*flgpt++ = '\0';

	for (stpt = str; *stpt != '\0'; ++stpt) {
		if ((*stpt >= '0') && (*stpt <= '9'))
			continue;
		if ((*stpt == ',') && (up_pt == NULL)) {
			*stpt = '\0';
			up_pt = stpt + 1;
			dot = 0;
			continue;
		}

		/*
		 * allow only one dot per range (secs)
		 */
		if ((*stpt == '.') && (!dot)) {
			++dot;
			continue;
		}
		paxwarn(1, "Improperly specified time range: %s", str);
		goto out;
	}

	/*
	 * allocate space for the time range and store the limits
	 */
	if ((pt = (TIME_RNG *)malloc(sizeof(TIME_RNG))) == NULL) {
		paxwarn(1, "Unable to allocate memory for time range");
		return(-1);
	}

	/*
	 * by default we only will check file mtime, but the user can specify
	 * mtime, ctime (inode change time) or both.
	 */
	if ((flgpt == NULL) || (*flgpt == '\0'))
		pt->flgs = CMPMTME;
	else {
		pt->flgs = 0;
		while (*flgpt != '\0') {
			switch(*flgpt) {
			case 'M':
			case 'm':
				pt->flgs |= CMPMTME;
				break;
			case 'C':
			case 'c':
				pt->flgs |= CMPCTME;
				break;
			default:
				paxwarn(1, "Bad option %c with time range %s",
				    *flgpt, str);
				free(pt);
				goto out;
			}
			++flgpt;
		}
	}

	/*
	 * start off with the current time
	 */
	pt->low_time = pt->high_time = time(NULL);
	if (*str != '\0') {
		/*
		 * add lower limit
		 */
		if (str_sec(str, &(pt->low_time)) < 0) {
			paxwarn(1, "Illegal lower time range %s", str);
			free(pt);
			goto out;
		}
		pt->flgs |= HASLOW;
	}

	if ((up_pt != NULL) && (*up_pt != '\0')) {
		/*
		 * add upper limit
		 */
		if (str_sec(up_pt, &(pt->high_time)) < 0) {
			paxwarn(1, "Illegal upper time range %s", up_pt);
			free(pt);
			goto out;
		}
		pt->flgs |= HASHIGH;

		/*
		 * check that the upper and lower do not overlap
		 */
		if (pt->flgs & HASLOW) {
			if (pt->low_time > pt->high_time) {
				paxwarn(1, "Upper %s and lower %s time overlap",
					up_pt, str);
				free(pt);
				return(-1);
			}
		}
	}

	pt->fow = NULL;
	if (trhead == NULL) {
		trtail = trhead = pt;
		return(0);
	}
	trtail->fow = pt;
	trtail = pt;
	return(0);

    out:
	paxwarn(1, "Time range format is: [[[[[cc]yy]mm]dd]HH]MM[.SS][/[c][m]]");
	return(-1);
}

/*
 * trng_match()
 *	check if this files mtime/ctime falls within any supplied time range.
 * Return:
 *	0 if this archive member should be processed, 1 if it should be skipped
 */

static int
trng_match(ARCHD *arcn)
{
	TIME_RNG *pt;

	/*
	 * have to search down the list one at a time looking for a match.
	 * remember time range limits are inclusive.
	 */
	pt = trhead;
	while (pt != NULL) {
		switch(pt->flgs & CMPBOTH) {
		case CMPBOTH:
			/*
			 * user wants both mtime and ctime checked for this
			 * time range
			 */
			if (((pt->flgs & HASLOW) &&
			    (arcn->sb.st_mtime < pt->low_time) &&
			    (arcn->sb.st_ctime < pt->low_time)) ||
			    ((pt->flgs & HASHIGH) &&
			    (arcn->sb.st_mtime > pt->high_time) &&
			    (arcn->sb.st_ctime > pt->high_time))) {
				pt = pt->fow;
				continue;
			}
			break;
		case CMPCTME:
			/*
			 * user wants only ctime checked for this time range
			 */
			if (((pt->flgs & HASLOW) &&
			    (arcn->sb.st_ctime < pt->low_time)) ||
			    ((pt->flgs & HASHIGH) &&
			    (arcn->sb.st_ctime > pt->high_time))) {
				pt = pt->fow;
				continue;
			}
			break;
		case CMPMTME:
		default:
			/*
			 * user wants only mtime checked for this time range
			 */
			if (((pt->flgs & HASLOW) &&
			    (arcn->sb.st_mtime < pt->low_time)) ||
			    ((pt->flgs & HASHIGH) &&
			    (arcn->sb.st_mtime > pt->high_time))) {
				pt = pt->fow;
				continue;
			}
			break;
		}
		break;
	}

	if (pt == NULL)
		return(1);
	return(0);
}

/*
 * str_sec()
 *	Convert a time string in the format of [[[[[cc]yy]mm]dd]HH]MM[.SS] to
 *	seconds UTC. Tval already has current time loaded into it at entry.
 * Return:
 *	0 if converted ok, -1 otherwise
 */

static int
str_sec(const char *p, time_t *tval)
{
	struct tm *lt;
	const char *dot, *t;
	size_t len;
	int bigyear;
	int yearset;

	yearset = 0;
	len = strlen(p);

	for (t = p, dot = NULL; *t; ++t) {
		if (isdigit((unsigned char)*t))
			continue;
		if (*t == '.' && dot == NULL) {
			dot = t;
			continue;
		}
		return(-1);
	}

	lt = localtime(tval);

	if (dot != NULL) {			/* .SS */
		if (strlen(++dot) != 2)
			return(-1);
		lt->tm_sec = ATOI2(dot);
		if (lt->tm_sec > 61)
			return(-1);
		len -= 3;
	} else
		lt->tm_sec = 0;

	switch (len) {
	case 12:				/* cc */
		bigyear = ATOI2(p);
		lt->tm_year = (bigyear * 100) - 1900;
		yearset = 1;
		/* FALLTHROUGH */
	case 10:				/* yy */
		if (yearset) {
			lt->tm_year += ATOI2(p);
		} else {
			lt->tm_year = ATOI2(p);
			if (lt->tm_year < 69)		/* hack for 2000 ;-} */
				lt->tm_year += (2000 - 1900);
		}
		/* FALLTHROUGH */
	case 8:					/* mm */
		lt->tm_mon = ATOI2(p);
		if ((lt->tm_mon > 12) || !lt->tm_mon)
			return(-1);
		--lt->tm_mon;			/* time struct is 0 - 11 */
		/* FALLTHROUGH */
	case 6:					/* dd */
		lt->tm_mday = ATOI2(p);
		if ((lt->tm_mday > 31) || !lt->tm_mday)
			return(-1);
		/* FALLTHROUGH */
	case 4:					/* HH */
		lt->tm_hour = ATOI2(p);
		if (lt->tm_hour > 23)
			return(-1);
		/* FALLTHROUGH */
	case 2:					/* MM */
		lt->tm_min = ATOI2(p);
		if (lt->tm_min > 59)
			return(-1);
		break;
	default:
		return(-1);
	}

	/* convert broken-down time to UTC clock time seconds */
	if ((*tval = mktime(lt)) == -1)
		return(-1);
	return(0);
}

/* ======================= pax.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992 Keith Muller.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static int gen_init(void);

/*
 * PAX main routines, general globals and some simple start up routines
 */

/*
 * Variables that can be accessed by any routine within pax
 */
int	act = DEFOP;		/* read/write/append/copy */
FSUB	*frmt = NULL;		/* archive format type */
int	cflag;			/* match all EXCEPT pattern/file */
int	cwdfd;			/* starting cwd */
int	dflag;			/* directory member match only  */
int	iflag;			/* interactive file/archive rename */
int	kflag;			/* do not overwrite existing files */
int	lflag;			/* use hard links when possible */
int	nflag;			/* select first archive member match */
int	tflag;			/* restore access time after read */
int	uflag;			/* ignore older modification time files */
int	vflag;			/* produce verbose output */
int	Dflag;			/* same as uflag except for inode change time */
int	Hflag;			/* follow command line symlinks (write only) */
int	Lflag;			/* follow symlinks when writing */
int	Oflag;			/* limit to single volume */
int	Xflag;			/* archive files with same device id only */
int	Yflag;			/* same as Dflg except after name mode */
int	Zflag;			/* same as uflg except after name mode */
int	vfpart;			/* is partial verbose output in progress */
int	patime = 1;		/* preserve file access time */
int	pmtime = 1;		/* preserve file modification times */
int	nodirs;			/* do not create directories as needed */
int	pmode;			/* preserve file mode bits */
int	pids;			/* preserve file uid/gid */
int	rmleadslash = 0;	/* remove leading '/' from pathnames */
int	exit_val;		/* exit value */
int	docrc;			/* check/create file crc */
char	*dirptr;		/* destination dir in a copy */
const	char *argv0;		/* root of argv[0] */
sigset_t s_mask;		/* signal mask for cleanup critical sect */
FILE	*listf;			/* file pointer to print file list to */
char	*tempfile;		/* tempfile to use for mkstemp(3) */
char	*tempbase;		/* basename of tempfile to use for mkstemp(3) */

/*
 *	PAX - Portable Archive Interchange
 *
 *	A utility to read, write, and write lists of the members of archive
 *	files and copy directory hierarchies. A variety of archive formats
 *	are supported (some are described in POSIX 1003.1 10.1):
 *
 *		ustar - 10.1.1 extended tar interchange format
 *		cpio  - 10.1.2 extended cpio interchange format
 *		tar - old BSD 4.3 tar format
 *		binary cpio - old cpio with binary header format
 *		sysVR4 cpio -  with and without CRC
 *
 * This version is a superset of IEEE Std 1003.2b-d3
 *
 * Summary of Extensions to the IEEE Standard:
 *
 * 1	READ ENHANCEMENTS
 * 1.1	Operations which read archives will continue to operate even when
 *	processing archives which may be damaged, truncated, or fail to meet
 *	format specs in several different ways. Damaged sections of archives
 *	are detected and avoided if possible. Attempts will be made to resync
 *	archive read operations even with badly damaged media.
 * 1.2	Blocksize requirements are not strictly enforced on archive read.
 *	Tapes which have variable sized records can be read without errors.
 * 1.3	The user can specify via the non-standard option flag -E if error
 *	resync operation should stop on a media error, try a specified number
 *	of times to correct, or try to correct forever.
 * 1.4	Sparse files (lseek holes) stored on the archive (but stored with blocks
 *	of all zeros will be restored with holes appropriate for the target
 *	file system
 * 1.5	The user is notified whenever something is found during archive
 *	read operations which violates spec (but the read will continue).
 * 1.6	Multiple archive volumes can be read and may span over different
 *	archive devices
 * 1.7	Rigidly restores all file attributes exactly as they are stored on the
 *	archive.
 * 1.8	Modification change time ranges can be specified via multiple -T
 *	options. These allow a user to select files whose modification time
 *	lies within a specific time range.
 * 1.9	Files can be selected based on owner (user name or uid) via one or more
 *	-U options.
 * 1.10	Files can be selected based on group (group name or gid) via one o
 *	more -G options.
 * 1.11	File modification time can be checked against existing file after
 *	name modification (-Z)
 *
 * 2	WRITE ENHANCEMENTS
 * 2.1	Write operation will stop instead of allowing a user to create a flawed
 *	flawed archive (due to any problem).
 * 2.2	Archives written by pax are forced to strictly conform to both the
 *	archive and pax the specific format specifications.
 * 2.3	Blocking size and format is rigidly enforced on writes.
 * 2.4	Formats which may exhibit header overflow problems (they have fields
 *	too small for large file systems, such as inode number storage), use
 *	routines designed to repair this problem. These techniques still
 *	conform to both pax and format specifications, but no longer truncate
 *	these fields. This removes any restrictions on using these archive
 *	formats on large file systems.
 * 2.5	Multiple archive volumes can be written and may span over different
 *	archive devices
 * 2.6	A archive volume record limit allows the user to specify the number
 *	of bytes stored on an archive volume. When reached the user is
 *	prompted for the next archive volume. This is specified with the
 *	non-standard -B flag. The limit is rounded up to the next blocksize.
 * 2.7	All archive padding during write use zero filled sections. This makes
 *	it much easier to pull data out of flawed archive during read
 *	operations.
 * 2.8	Access time reset with the -t applies to all file nodes (including
 *	directories).
 * 2.9	Symbolic links can be followed with -L (optional in the spec).
 * 2.10	Modification or inode change time ranges can be specified via
 *	multiple -T options. These allow a user to select files whose
 *	modification or inode change time lies within a specific time range.
 * 2.11	Files can be selected based on owner (user name or uid) via one or more
 *	-U options.
 * 2.12	Files can be selected based on group (group name or gid) via one o
 *	more -G options.
 * 2.13	Symlinks which appear on the command line can be followed (without
 *	following other symlinks; -H flag)
 *
 * 3	COPY ENHANCEMENTS
 * 3.1	Sparse files (lseek holes) can be copied without expanding the holes
 *	into zero filled blocks. The file copy is created with holes which are
 *	appropriate for the target file system
 * 3.2	Access time as well as modification time on copied file trees can be
 *	preserved with the appropriate -p options.
 * 3.3	Access time reset with the -t applies to all file nodes (including
 *	directories).
 * 3.4	Symbolic links can be followed with -L (optional in the spec).
 * 3.5	Modification or inode change time ranges can be specified via
 *	multiple -T options. These allow a user to select files whose
 *	modification or inode change time lies within a specific time range.
 * 3.6	Files can be selected based on owner (user name or uid) via one or more
 *	-U options.
 * 3.7	Files can be selected based on group (group name or gid) via one o
 *	more -G options.
 * 3.8	Symlinks which appear on the command line can be followed (without
 *	following other symlinks; -H flag)
 * 3.9  File inode change time can be checked against existing file before
 *	name modification (-D)
 * 3.10 File inode change time can be checked against existing file after
 *	name modification (-Y)
 * 3.11	File modification time can be checked against existing file after
 *	name modification (-Z)
 *
 * 4	GENERAL ENHANCEMENTS
 * 4.1	Internal structure is designed to isolate format dependent and
 *	independent functions. Formats are selected via a format driver table.
 *	This encourages the addition of new archive formats by only having to
 *	write those routines which id, read and write the archive header.
 */

/*
 * main()
 *	parse options, set up and operate as specified by the user.
 *	any operational flaw will set exit_val to non-zero
 * Return: 0 if ok, 1 otherwise
 */

int
main(int argc, char *argv[])
{
	const char *tmpdir;
	size_t tdlen;

	(void) setlocale(LC_ALL, "");
	listf = stderr;
	/*
	 * Keep a reference to cwd, so we can always come back home.
	 */
	cwdfd = open(".", O_RDONLY | O_CLOEXEC);
	if (cwdfd < 0) {
		syswarn(0, errno, "Can't open current working directory.");
		return(exit_val);
	}

	/*
	 * Where should we put temporary files?
	 */
	if ((tmpdir = getenv("TMPDIR")) == NULL || *tmpdir == '\0')
		tmpdir = _PATH_TMP;
	tdlen = strlen(tmpdir);
	while (tdlen > 0 && tmpdir[tdlen - 1] == '/')
		tdlen--;
	tempfile = malloc(tdlen + 1 + sizeof(_TFILE_BASE));
	if (tempfile == NULL) {
		paxwarn(1, "Cannot allocate memory for temp file name.");
		return(exit_val);
	}
	if (tdlen)
		memcpy(tempfile, tmpdir, tdlen);
	tempbase = tempfile + tdlen;
	*tempbase++ = '/';

	/*
	 * parse options, determine operational mode, general init
	 */
	options(argc, argv);
	if ((gen_init() < 0) || (tty_init() < 0))
		return(exit_val);

	/*
	 * select a primary operation mode
	 */
	switch (act) {
	case EXTRACT:
		extract();
		break;
	case ARCHIVE:
		archive();
		break;
	case APPND:
		if (gzip_program != NULL)
			err(1, "can not gzip while appending");
		append();
		break;
	case COPY:
		copy();
		break;
	default:
	case LIST:
		list();
		break;
	}
	return(exit_val);
}

/*
 * sig_cleanup()
 *	when interrupted we try to do whatever delayed processing we can.
 *	This is not critical, but we really ought to limit our damage when we
 *	are aborted by the user.
 * Return:
 *	never....
 */

void
sig_cleanup(int which_sig)
{
	/*
	 * restore modes and times for any dirs we may have created
	 * or any dirs we may have read. Set vflag and vfpart so the user
	 * will clearly see the message on a line by itself.
	 */
	vflag = vfpart = 1;
	if (which_sig == SIGXCPU)
		paxwarn(0, "Cpu time limit reached, cleaning up.");
	else
		paxwarn(0, "Signal caught, cleaning up.");

	ar_close();
	proc_dir();
	if (tflag)
		atdir_end();
	exit(1);
}

/*
 * setup_sig()
 *	set a signal to be caught, but only if it isn't being ignored already
 */

static int
setup_sig(int sig, const struct sigaction *n_hand)
{
	struct sigaction o_hand;

	if (sigaction(sig, NULL, &o_hand) < 0)
		return (-1);

	if (o_hand.sa_handler == SIG_IGN)
		return (0);

	return (sigaction(sig, n_hand, NULL));
}

/*
 * gen_init()
 *	general setup routines. Not all are required, but they really help
 *	when dealing with a medium to large sized archives.
 */

static int
gen_init(void)
{
	struct rlimit reslimit;
	struct sigaction n_hand;

	/*
	 * Really needed to handle large archives. We can run out of memory for
	 * internal tables really fast when we have a whole lot of files...
	 */
	if (getrlimit(RLIMIT_DATA , &reslimit) == 0){
		reslimit.rlim_cur = reslimit.rlim_max;
		(void)setrlimit(RLIMIT_DATA , &reslimit);
	}

	/*
	 * should file size limits be waived? if the os limits us, this is
	 * needed if we want to write a large archive
	 */
	if (getrlimit(RLIMIT_FSIZE , &reslimit) == 0){
		reslimit.rlim_cur = reslimit.rlim_max;
		(void)setrlimit(RLIMIT_FSIZE , &reslimit);
	}

	/*
	 * increase the size the stack can grow to
	 */
	if (getrlimit(RLIMIT_STACK , &reslimit) == 0){
		reslimit.rlim_cur = reslimit.rlim_max;
		(void)setrlimit(RLIMIT_STACK , &reslimit);
	}

	/*
	 * not really needed, but doesn't hurt
	 */
	if (getrlimit(RLIMIT_RSS , &reslimit) == 0){
		reslimit.rlim_cur = reslimit.rlim_max;
		(void)setrlimit(RLIMIT_RSS , &reslimit);
	}

	/*
	 * signal handling to reset stored directory times and modes. Since
	 * we deal with broken pipes via failed writes we ignore it. We also
	 * deal with any file size limit through failed writes. Cpu time
	 * limits are caught and a cleanup is forced.
	 */
	if ((sigemptyset(&s_mask) < 0) || (sigaddset(&s_mask, SIGTERM) < 0) ||
	    (sigaddset(&s_mask,SIGINT) < 0)||(sigaddset(&s_mask,SIGHUP) < 0) ||
	    (sigaddset(&s_mask,SIGPIPE) < 0)||(sigaddset(&s_mask,SIGQUIT)<0) ||
	    (sigaddset(&s_mask,SIGXCPU) < 0)||(sigaddset(&s_mask,SIGXFSZ)<0)) {
		paxwarn(1, "Unable to set up signal mask");
		return(-1);
	}
	memset(&n_hand, 0, sizeof n_hand);
	n_hand.sa_mask = s_mask;
	n_hand.sa_flags = 0;
	n_hand.sa_handler = sig_cleanup;

	if (setup_sig(SIGHUP,  &n_hand) ||
	   setup_sig(SIGTERM, &n_hand) ||
	   setup_sig(SIGINT,  &n_hand) ||
	   setup_sig(SIGQUIT, &n_hand) ||
	   setup_sig(SIGXCPU, &n_hand))
		goto out;

	n_hand.sa_handler = SIG_IGN;
	if ((sigaction(SIGPIPE, &n_hand, NULL) < 0) ||
	    (sigaction(SIGXFSZ, &n_hand, NULL) < 0))
		goto out;
	return(0);

    out:
	syswarn(1, errno, "Unable to set up signal handler");
	return(-1);
}

/* ======================= file_subs.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992 Keith Muller.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


static int
mk_link(char *,struct stat *,char *, int);

/*
 * routines that deal with file operations such as: creating, removing;
 * and setting access modes, uid/gid and times of files
 */

#define FILEBITS		(S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)
#define SETBITS			(S_ISUID | S_ISGID)
#define ABITS			(FILEBITS | SETBITS)

/*
 * file_creat()
 *	Create and open a file.
 * Return:
 *	file descriptor or -1 for failure
 */

int
file_creat(ARCHD *arcn)
{
	int fd = -1;
	mode_t file_mode;
	int oerrno;

	/*
	 * assume file doesn't exist, so just try to create it, most times this
	 * works. We have to take special handling when the file does exist. To
	 * detect this, we use O_EXCL. For example when trying to create a
	 * file and a character device or fifo exists with the same name, we
	 * can accidentally open the device by mistake (or block waiting to
	 * open). If we find that the open has failed, then spend the effort
	 * to figure out why. This strategy was found to have better average
	 * performance in common use than checking the file (and the path)
	 * first with lstat.
	 */
	file_mode = arcn->sb.st_mode & FILEBITS;
	if ((fd = open(arcn->name, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL,
	    file_mode)) >= 0)
		return(fd);

	/*
	 * the file seems to exist. First we try to get rid of it (found to be
	 * the second most common failure when traced). If this fails, only
	 * then we go to the expense to check and create the path to the file
	 */
	if (unlnk_exist(arcn->name, arcn->type) != 0)
		return(-1);

	for (;;) {
		/*
		 * try to open it again, if this fails, check all the nodes in
		 * the path and give it a final try. if chk_path() finds that
		 * it cannot fix anything, we will skip the last attempt
		 */
		if ((fd = open(arcn->name, O_WRONLY | O_CREAT | O_TRUNC,
		    file_mode)) >= 0)
			break;
		oerrno = errno;
		if (nodirs || chk_path(arcn->name,arcn->sb.st_uid,arcn->sb.st_gid) < 0) {
			syswarn(1, oerrno, "Unable to create %s", arcn->name);
			return(-1);
		}
	}
	return(fd);
}

/*
 * file_close()
 *	Close file descriptor to a file just created by pax. Sets modes,
 *	ownership and times as required.
 * Return:
 *	0 for success, -1 for failure
 */

void
file_close(ARCHD *arcn, int fd)
{
	int res = 0;

	if (fd < 0)
		return;
	if (close(fd) < 0)
		syswarn(0, errno, "Unable to close file descriptor on %s",
		    arcn->name);

	/*
	 * set owner/groups first as this may strip off mode bits we want
	 * then set file permission modes. Then set file access and
	 * modification times.
	 */
	if (pids)
		res = set_ids(arcn->name, arcn->sb.st_uid, arcn->sb.st_gid);

	/*
	 * IMPORTANT SECURITY NOTE:
	 * if not preserving mode or we cannot set uid/gid, then PROHIBIT
	 * set uid/gid bits
	 */
	if (!pmode || res)
		arcn->sb.st_mode &= ~(SETBITS);
	if (pmode)
		set_pmode(arcn->name, arcn->sb.st_mode);
	if (patime || pmtime)
		set_ftime(arcn->name, arcn->sb.st_mtime, arcn->sb.st_atime, 0);
}

/*
 * lnk_creat()
 *	Create a hard link to arcn->ln_name from arcn->name. arcn->ln_name
 *	must exist;
 * Return:
 *	0 if ok, -1 otherwise
 */

int
lnk_creat(ARCHD *arcn)
{
	struct stat sb;

	/*
	 * we may be running as root, so we have to be sure that link target
	 * is not a directory, so we lstat and check
	 */
	if (lstat(arcn->ln_name, &sb) < 0) {
		syswarn(1,errno,"Unable to link to %s from %s", arcn->ln_name,
		    arcn->name);
		return(-1);
	}

	if (S_ISDIR(sb.st_mode)) {
		paxwarn(1, "A hard link to the directory %s is not allowed",
		    arcn->ln_name);
		return(-1);
	}

	return(mk_link(arcn->ln_name, &sb, arcn->name, 0));
}

/*
 * cross_lnk()
 *	Create a hard link to arcn->org_name from arcn->name. Only used in copy
 *	with the -l flag. No warning or error if this does not succeed (we will
 *	then just create the file)
 * Return:
 *	1 if copy() should try to create this file node
 *	0 if cross_lnk() ok, -1 for fatal flaw (like linking to self).
 */

int
cross_lnk(ARCHD *arcn)
{
	/*
	 * try to make a link to original file (-l flag in copy mode). make sure
	 * we do not try to link to directories in case we are running as root
	 * (and it might succeed).
	 */
	if (arcn->type == PAX_DIR)
		return(1);
	return(mk_link(arcn->org_name, &(arcn->sb), arcn->name, 1));
}

/*
 * chk_same()
 *	In copy mode if we are not trying to make hard links between the src
 *	and destinations, make sure we are not going to overwrite ourselves by
 *	accident. This slows things down a little, but we have to protect all
 *	those people who make typing errors.
 * Return:
 *	1 the target does not exist, go ahead and copy
 *	0 skip it file exists (-k) or may be the same as source file
 */

int
chk_same(ARCHD *arcn)
{
	struct stat sb;

	/*
	 * if file does not exist, return. if file exists and -k, skip it
	 * quietly
	 */
	if (lstat(arcn->name, &sb) < 0)
		return(1);
	if (kflag)
		return(0);

	/*
	 * better make sure the user does not have src == dest by mistake
	 */
	if ((arcn->sb.st_dev == sb.st_dev) && (arcn->sb.st_ino == sb.st_ino)) {
		paxwarn(1, "Unable to copy %s, file would overwrite itself",
		    arcn->name);
		return(0);
	}
	return(1);
}

/*
 * mk_link()
 *	try to make a hard link between two files. if ign set, we do not
 *	complain.
 * Return:
 *	0 if successful (or we are done with this file but no error, such as
 *	finding the from file exists and the user has set -k).
 *	1 when ign was set to indicates we could not make the link but we
 *	should try to copy/extract the file as that might work (and is an
 *	allowed option). -1 an error occurred.
 */

static int
mk_link(char *to, struct stat *to_sb, char *from,
	int ign)
{
	struct stat sb;
	int oerrno;

	/*
	 * if from file exists, it has to be unlinked to make the link. If the
	 * file exists and -k is set, skip it quietly
	 */
	if (lstat(from, &sb) == 0) {
		if (kflag)
			return(0);

		/*
		 * make sure it is not the same file, protect the user
		 */
		if ((to_sb->st_dev==sb.st_dev)&&(to_sb->st_ino == sb.st_ino)) {
			paxwarn(1, "Unable to link file %s to itself", to);
			return(-1);
		}

		/*
		 * try to get rid of the file, based on the type
		 */
		if (S_ISDIR(sb.st_mode)) {
			if (rmdir(from) < 0) {
				syswarn(1, errno, "Unable to remove %s", from);
				return(-1);
			}
		} else if (unlink(from) < 0) {
			if (!ign) {
				syswarn(1, errno, "Unable to remove %s", from);
				return(-1);
			}
			return(1);
		}
	}

	/*
	 * from file is gone (or did not exist), try to make the hard link.
	 * if it fails, check the path and try it again (if chk_path() says to
	 * try again)
	 */
	for (;;) {
		if (link(to, from) == 0)
			break;
		oerrno = errno;
		if (!nodirs && chk_path(from, to_sb->st_uid, to_sb->st_gid) == 0)
			continue;
		if (!ign) {
			syswarn(1, oerrno, "Could not link to %s from %s", to,
			    from);
			return(-1);
		}
		return(1);
	}

	/*
	 * all right the link was made
	 */
	return(0);
}

/*
 * node_creat()
 *	create an entry in the file system (other than a file or hard link).
 *	If successful, sets uid/gid modes and times as required.
 * Return:
 *	0 if ok, -1 otherwise
 */

int
node_creat(ARCHD *arcn)
{
	int res;
	int ign = 0;
	int oerrno;
	int pass = 0;
	mode_t file_mode;
	struct stat sb;

	/*
	 * create node based on type, if that fails try to unlink the node and
	 * try again. finally check the path and try again. As noted in the
	 * file and link creation routines, this method seems to exhibit the
	 * best performance in general use workloads.
	 */
	file_mode = arcn->sb.st_mode & FILEBITS;

	for (;;) {
		switch(arcn->type) {
		case PAX_DIR:
			res = mkdir(arcn->name, file_mode);
			if (ign)
				res = 0;
			break;
		case PAX_CHR:
			file_mode |= S_IFCHR;
			res = mknod(arcn->name, file_mode, arcn->sb.st_rdev);
			break;
		case PAX_BLK:
			file_mode |= S_IFBLK;
			res = mknod(arcn->name, file_mode, arcn->sb.st_rdev);
			break;
		case PAX_FIF:
			res = mkfifo(arcn->name, file_mode);
			break;
		case PAX_SCK:
			/*
			 * Skip sockets, operation has no meaning under BSD
			 */
			paxwarn(0,
			    "%s skipped. Sockets cannot be copied or extracted",
			    arcn->name);
			return(-1);
		case PAX_SLK:
			res = symlink(arcn->ln_name, arcn->name);
			break;
		case PAX_CTG:
		case PAX_HLK:
		case PAX_HRG:
		case PAX_REG:
		default:
			/*
			 * we should never get here
			 */
			paxwarn(0, "%s has an unknown file type, skipping",
				arcn->name);
			return(-1);
		}

		/*
		 * if we were able to create the node break out of the loop,
		 * otherwise try to unlink the node and try again. if that
		 * fails check the full path and try a final time.
		 */
		if (res == 0)
			break;

		/*
		 * we failed to make the node
		 */
		oerrno = errno;
		if ((ign = unlnk_exist(arcn->name, arcn->type)) < 0)
			return(-1);

		if (++pass <= 1)
			continue;

		if (nodirs || chk_path(arcn->name,arcn->sb.st_uid,arcn->sb.st_gid) < 0) {
			syswarn(1, oerrno, "Could not create: %s", arcn->name);
			return(-1);
		}
	}

	/*
	 * we were able to create the node. set uid/gid, modes and times
	 */
	if (pids)
		res = set_ids(arcn->name, arcn->sb.st_uid, arcn->sb.st_gid);
	else
		res = 0;

	/*
	 * IMPORTANT SECURITY NOTE:
	 * if not preserving mode or we cannot set uid/gid, then PROHIBIT any
	 * set uid/gid bits
	 */
	if (!pmode || res)
		arcn->sb.st_mode &= ~(SETBITS);
	if (pmode)
		set_pmode(arcn->name, arcn->sb.st_mode);

	if (arcn->type == PAX_DIR && strcmp(NM_CPIO, argv0) != 0) {
		/*
		 * Dirs must be processed again at end of extract to set times
		 * and modes to agree with those stored in the archive. However
		 * to allow extract to continue, we may have to also set owner
		 * rights. This allows nodes in the archive that are children
		 * of this directory to be extracted without failure. Both time
		 * and modes will be fixed after the entire archive is read and
		 * before pax exits.
		 */
		if (access(arcn->name, R_OK | W_OK | X_OK) < 0) {
			if (lstat(arcn->name, &sb) < 0) {
				syswarn(0, errno,"Could not access %s (stat)",
				    arcn->name);
				set_pmode(arcn->name,file_mode | S_IRWXU);
			} else {
				/*
				 * We have to add rights to the dir, so we make
				 * sure to restore the mode. The mode must be
				 * restored AS CREATED and not as stored if
				 * pmode is not set.
				 */
				set_pmode(arcn->name,
				    ((sb.st_mode & FILEBITS) | S_IRWXU));
				if (!pmode)
					arcn->sb.st_mode = sb.st_mode;
			}

			/*
			 * we have to force the mode to what was set here,
			 * since we changed it from the default as created.
			 */
			add_dir(arcn->name, arcn->nlen, &(arcn->sb), 1);
		} else if (pmode || patime || pmtime)
			add_dir(arcn->name, arcn->nlen, &(arcn->sb), 0);
	}

	if (patime || pmtime)
		set_ftime(arcn->name, arcn->sb.st_mtime, arcn->sb.st_atime, 0);
	return(0);
}

/*
 * unlnk_exist()
 *	Remove node from file system with the specified name. We pass the type
 *	of the node that is going to replace it. When we try to create a
 *	directory and find that it already exists, we allow processing to
 *	continue as proper modes etc will always be set for it later on.
 * Return:
 *	0 is ok to proceed, no file with the specified name exists
 *	-1 we were unable to remove the node, or we should not remove it (-k)
 *	1 we found a directory and we were going to create a directory.
 */

int
unlnk_exist(char *name, int type)
{
	struct stat sb;

	/*
	 * the file does not exist, or -k we are done
	 */
	if (lstat(name, &sb) < 0)
		return(0);
	if (kflag)
		return(-1);

	if (S_ISDIR(sb.st_mode)) {
		/*
		 * try to remove a directory, if it fails and we were going to
		 * create a directory anyway, tell the caller (return a 1)
		 */
		if (rmdir(name) < 0) {
			if (type == PAX_DIR)
				return(1);
			syswarn(1,errno,"Unable to remove directory %s", name);
			return(-1);
		}
		return(0);
	}

	/*
	 * try to get rid of all non-directory type nodes
	 */
	if (unlink(name) < 0) {
		syswarn(1, errno, "Could not unlink %s", name);
		return(-1);
	}
	return(0);
}

/*
 * chk_path()
 *	We were trying to create some kind of node in the file system and it
 *	failed. chk_path() makes sure the path up to the node exists and is
 *	writeable. When we have to create a directory that is missing along the
 *	path somewhere, the directory we create will be set to the same
 *	uid/gid as the file has (when uid and gid are being preserved).
 *	NOTE: this routine is a real performance loss. It is only used as a
 *	last resort when trying to create entries in the file system.
 * Return:
 *	-1 when it could find nothing it is allowed to fix.
 *	0 otherwise
 */

int
chk_path( char *name, uid_t st_uid, gid_t st_gid)
{
	char *spt = name;
	struct stat sb;
	int retval = -1;

	/*
	 * watch out for paths with nodes stored directly in / (e.g. /bozo)
	 */
	if (*spt == '/')
		++spt;

	for(;;) {
		/*
		 * work forward from the first / and check each part of the path
		 */
		spt = strchr(spt, '/');

		/*
		 * skip creating a leaf dir (with an ending '/') as we only want
		 * to create parents here
		 */
		if ((spt == NULL) || (*(spt + 1) == '\0'))
			break;
		*spt = '\0';

		/*
		 * if it exists we assume it is a directory, it is not within
		 * the spec (at least it seems to read that way) to alter the
		 * file system for nodes NOT EXPLICITLY stored on the archive.
		 * If that assumption is changed, you would test the node here
		 * and figure out how to get rid of it (probably like some
		 * recursive unlink()) or fix up the directory permissions if
		 * required (do an access()).
		 */
		if (lstat(name, &sb) == 0) {
			*(spt++) = '/';
			continue;
		}

		/*
		 * the path fails at this point, see if we can create the
		 * needed directory and continue on
		 */
		if (mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
			*spt = '/';
			retval = -1;
			break;
		}

		/*
		 * we were able to create the directory. We will tell the
		 * caller that we found something to fix, and it is ok to try
		 * and create the node again.
		 */
		retval = 0;
		if (pids)
			(void)set_ids(name, st_uid, st_gid);

		/*
		 * make sure the user doesn't have some strange umask that
		 * causes this newly created directory to be unusable. We fix
		 * the modes and restore them back to the creation default at
		 * the end of pax
		 */
		if ((access(name, R_OK | W_OK | X_OK) < 0) &&
		    (lstat(name, &sb) == 0)) {
			set_pmode(name, ((sb.st_mode & FILEBITS) | S_IRWXU));
			add_dir(name, spt - name, &sb, 1);
		}
		*(spt++) = '/';
		continue;
	}
	return(retval);
}

/*
 * set_ftime()
 *	Set the access time and modification time for a named file. If frc is
 *	non-zero we force these times to be set even if the user did not
 *	request access and/or modification time preservation (this is also
 *	used by -t to reset access times).
 *	When frc is zero, only those times the user has asked for are set, the
 *	other ones are left alone. We do not assume the un-documented feature
 *	of many lutimes() implementations that consider a 0 time value as a do
 *	not set request.
 */

void
set_ftime(char *fnm, time_t mtime, time_t atime, int frc)
{
	static struct timeval tv[2] = {{0L, 0L}, {0L, 0L}};
	struct stat sb;

	tv[0].tv_sec = atime;
	tv[1].tv_sec = mtime;
	if (!frc && (!patime || !pmtime)) {
		/*
		 * if we are not forcing, only set those times the user wants
		 * set. We get the current values of the times if we need them.
		 */
		if (lstat(fnm, &sb) == 0) {
			if (!patime)
				tv[0].tv_sec = sb.st_atime;
			if (!pmtime)
				tv[1].tv_sec = sb.st_mtime;
		} else
			syswarn(0,errno,"Unable to obtain file stats %s", fnm);
	}

	/*
	 * set the times
	 */
	if (lutimes(fnm, tv) < 0)
		syswarn(1, errno, "Access/modification time set failed on: %s",
		    fnm);
	return;
}

/*
 * set_ids()
 *	set the uid and gid of a file system node
 * Return:
 *	0 when set, -1 on failure
 */

int
set_ids(char *fnm, uid_t uid, gid_t gid)
{
	if (lchown(fnm, uid, gid) < 0) {
		/*
		 * ignore EPERM unless in verbose mode or being run by root.
		 * if running as pax, POSIX requires a warning.
		 */
		if (strcmp(NM_PAX, argv0) == 0 || errno != EPERM || vflag ||
		    geteuid() == 0)
			syswarn(1, errno, "Unable to set file uid/gid of %s",
			    fnm);
		return(-1);
	}
	return(0);
}

/*
 * set_pmode()
 *	Set file access mode
 */

void
set_pmode(char *fnm, mode_t mode)
{
	mode &= ABITS;
	if (lchmod(fnm, mode) < 0)
		syswarn(1, errno, "Could not set permissions on %s", fnm);
	return;
}

/*
 * file_write()
 *	Write/copy a file (during copy or archive extract). This routine knows
 *	how to copy files with lseek holes in it. (Which are read as file
 *	blocks containing all 0's but do not have any file blocks associated
 *	with the data). Typical examples of these are files created by dbm
 *	variants (.pag files). While the file size of these files are huge, the
 *	actual storage is quite small (the files are sparse). The problem is
 *	the holes read as all zeros so are probably stored on the archive that
 *	way (there is no way to determine if the file block is really a hole,
 *	we only know that a file block of all zero's can be a hole).
 *	At this writing, no major archive format knows how to archive files
 *	with holes. However, on extraction (or during copy, -rw) we have to
 *	deal with these files. Without detecting the holes, the files can
 *	consume a lot of file space if just written to disk. This replacement
 *	for write when passed the basic allocation size of a file system block,
 *	uses lseek whenever it detects the input data is all 0 within that
 *	file block. In more detail, the strategy is as follows:
 *	While the input is all zero keep doing an lseek. Keep track of when we
 *	pass over file block boundaries. Only write when we hit a non zero
 *	input. once we have written a file block, we continue to write it to
 *	the end (we stop looking at the input). When we reach the start of the
 *	next file block, start checking for zero blocks again. Working on file
 *	block boundaries significantly reduces the overhead when copying files
 *	that are NOT very sparse. This overhead (when compared to a write) is
 *	almost below the measurement resolution on many systems. Without it,
 *	files with holes cannot be safely copied. It does have a side effect as
 *	it can put holes into files that did not have them before, but that is
 *	not a problem since the file contents are unchanged (in fact it saves
 *	file space). (Except on paging files for diskless clients. But since we
 *	cannot determine one of those file from here, we ignore them). If this
 *	ever ends up on a system where CTG files are supported and the holes
 *	are not desired, just do a conditional test in those routines that
 *	call file_write() and have it call write() instead. BEFORE CLOSING THE
 *	FILE, make sure to call file_flush() when the last write finishes with
 *	an empty block. A lot of file systems will not create an lseek hole at
 *	the end. In this case we drop a single 0 at the end to force the
 *	trailing 0's in the file.
 *	---Parameters---
 *	rem: how many bytes left in this file system block
 *	isempt: have we written to the file block yet (is it empty)
 *	sz: basic file block allocation size
 *	cnt: number of bytes on this write
 *	str: buffer to write
 * Return:
 *	number of bytes written, -1 on write (or lseek) error.
 */

int
file_write(int fd, char *str, int cnt, int *rem, int *isempt, int sz,
	char *name)
{
	char *pt;
	char *end;
	int wcnt;
	char *st = str;

	/*
	 * while we have data to process
	 */
	while (cnt) {
		if (!*rem) {
			/*
			 * We are now at the start of file system block again
			 * (or what we think one is...). start looking for
			 * empty blocks again
			 */
			*isempt = 1;
			*rem = sz;
		}

		/*
		 * only examine up to the end of the current file block or
		 * remaining characters to write, whatever is smaller
		 */
		wcnt = MIN(cnt, *rem);
		cnt -= wcnt;
		*rem -= wcnt;
		if (*isempt) {
			/*
			 * have not written to this block yet, so we keep
			 * looking for zero's
			 */
			pt = st;
			end = st + wcnt;

			/*
			 * look for a zero filled buffer
			 */
			while ((pt < end) && (*pt == '\0'))
				++pt;

			if (pt == end) {
				/*
				 * skip, buf is empty so far
				 */
				if (lseek(fd, (off_t)wcnt, SEEK_CUR) < 0) {
					syswarn(1,errno,"File seek on %s",
					    name);
					return(-1);
				}
				st = pt;
				continue;
			}
			/*
			 * drat, the buf is not zero filled
			 */
			*isempt = 0;
		}

		/*
		 * have non-zero data in this file system block, have to write
		 */
		if (write(fd, st, wcnt) != wcnt) {
			syswarn(1, errno, "Failed write to file %s", name);
			return(-1);
		}
		st += wcnt;
	}
	return(st - str);
}

/*
 * file_flush()
 *	when the last file block in a file is zero, many file systems will not
 *	let us create a hole at the end. To get the last block with zeros, we
 *	write the last BYTE with a zero (back up one byte and write a zero).
 */

void
file_flush(int fd, char *fname, int isempt)
{
	static char blnk[] = "\0";

	/*
	 * silly test, but make sure we are only called when the last block is
	 * filled with all zeros.
	 */
	if (!isempt)
		return;

	/*
	 * move back one byte and write a zero
	 */
	if (lseek(fd, (off_t)-1, SEEK_CUR) < 0) {
		syswarn(1, errno, "Failed seek on file %s", fname);
		return;
	}

	if (write(fd, blnk, 1) < 0)
		syswarn(1, errno, "Failed write to file %s", fname);
	return;
}

/*
 * rdfile_close()
 *	close a file we have beed reading (to copy or archive). If we have to
 *	reset access time (tflag) do so (the times are stored in arcn).
 */

void
rdfile_close(ARCHD *arcn, int *fd)
{
	/*
	 * make sure the file is open
	 */
	if (*fd < 0)
		return;

	(void)close(*fd);
	*fd = -1;
	if (!tflag)
		return;

	/*
	 * user wants last access time reset
	 */
	set_ftime(arcn->org_name, arcn->sb.st_mtime, arcn->sb.st_atime, 1);
	return;
}

/*
 * set_crc()
 *	read a file to calculate its crc. This is a real drag. Archive formats
 *	that have this, end up reading the file twice (we have to write the
 *	header WITH the crc before writing the file contents. Oh well...
 * Return:
 *	0 if was able to calculate the crc, -1 otherwise
 */

int
set_crc(ARCHD *arcn, int fd)
{
	int i;
	int res;
	off_t cpcnt = 0L;
	u_long size;
	unsigned long crc = 0L;
	char tbuf[FILEBLK];
	struct stat sb;

	if (fd < 0) {
		/*
		 * hmm, no fd, should never happen. well no crc then.
		 */
		arcn->crc = 0L;
		return(0);
	}

	if ((size = (u_long)arcn->sb.st_blksize) > (u_long)sizeof(tbuf))
		size = (u_long)sizeof(tbuf);

	/*
	 * read all the bytes we think that there are in the file. If the user
	 * is trying to archive an active file, forget this file.
	 */
	for(;;) {
		if ((res = read(fd, tbuf, size)) <= 0)
			break;
		cpcnt += res;
		for (i = 0; i < res; ++i)
			crc += (tbuf[i] & 0xff);
	}

	/*
	 * safety check. we want to avoid archiving files that are active as
	 * they can create inconsistent archive copies.
	 */
	if (cpcnt != arcn->sb.st_size)
		paxwarn(1, "File changed size %s", arcn->org_name);
	else if (fstat(fd, &sb) < 0)
		syswarn(1, errno, "Failed stat on %s", arcn->org_name);
	else if (arcn->sb.st_mtime != sb.st_mtime)
		paxwarn(1, "File %s was modified during read", arcn->org_name);
	else if (lseek(fd, (off_t)0L, SEEK_SET) < 0)
		syswarn(1, errno, "File rewind failed on: %s", arcn->org_name);
	else {
		arcn->crc = crc;
		return(0);
	}
	return(-1);
}
