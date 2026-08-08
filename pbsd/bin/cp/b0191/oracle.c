/*-
 * Oracle for PBSD batch b0191 -- concatenated cp/utils.c and cp/cp.c
 */

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/stat.h>
#if defined(__linux__)
#define st_flags st_blksize
#endif
#include <sys/types.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#if defined(__linux__)
#define st_flags st_blksize
#include <bsd/string.h>
#include <bsd/err.h>
#else
#include <string.h>
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif
#ifndef O_SEARCH
#define O_SEARCH 0
#endif


#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MAXPHYS
#define MAXPHYS 65536
#endif

#ifndef AT_RESOLVE_BENEATH
#define AT_RESOLVE_BENEATH 0
#endif
#ifndef O_RESOLVE_BENEATH
#define O_RESOLVE_BENEATH 0
#endif
#ifndef ENOTCAPABLE
#define ENOTCAPABLE EACCES
#endif

typedef void *acl_t;
typedef unsigned int acl_type_t;
#ifndef ACL_TYPE_NFS4
#define ACL_TYPE_NFS4 0x00000004
#endif
#ifndef ACL_TYPE_ACCESS
#define ACL_TYPE_ACCESS 0x00000002
#endif
#ifndef _PC_ACL_NFS4
#define _PC_ACL_NFS4 64
#endif
#ifndef _PC_ACL_EXTENDED
#define _PC_ACL_EXTENDED 65
#endif

acl_t acl_get_fd_np(int, acl_type_t);
int acl_is_trivial_np(acl_t, int *);
int acl_set_fd_np(int, acl_t, acl_type_t);
int acl_free(acl_t);
int fchflags(int, unsigned long);
int chflagsat(int, const char *, unsigned long, int);

typedef struct {
	int		 dir;
	char		 base[PATH_MAX + 1];
	char		*end;
	char		 path[PATH_MAX];
} PATH_T;

enum ref_op { FILE_TO_FILE, FILE_TO_DIR, DIR_TO_DNE };

PATH_T ref_to = { .dir = -1, .end = ref_to.path };
bool ref_Nflag, ref_fflag, ref_iflag, ref_lflag, ref_nflag, ref_pflag, ref_sflag, ref_vflag;
static bool ref_Hflag, ref_Lflag, ref_Pflag, ref_Rflag, ref_rflag, ref_Sflag;
volatile sig_atomic_t ref_info;

int ref_setfile(struct stat *, int, bool);
int ref_preserve_fd_acls(int, int);
int ref_preserve_dir_acls(const char *, const char *);
void ref_usage(void);
int ref_copy(char *[], enum ref_op, int, struct stat *);
void ref_siginfo(int sig);
int ref_ftscmp(const FTSENT * const *, const FTSENT * const *);

#define to ref_to
#define Nflag ref_Nflag
#define fflag ref_fflag
#define iflag ref_iflag
#define lflag ref_lflag
#define nflag ref_nflag
#define pflag ref_pflag
#define sflag ref_sflag
#define vflag ref_vflag
#define Hflag ref_Hflag
#define Lflag ref_Lflag
#define Pflag ref_Pflag
#define Rflag ref_Rflag
#define rflag ref_rflag
#define Sflag ref_Sflag
#define info ref_info

#define copy_fallback ref_copy_fallback
#define copy_file ref_copy_file
#define copy_link ref_copy_link
#define copy_fifo ref_copy_fifo
#define copy_special ref_copy_special
#define setfile ref_setfile
#define preserve_fd_acls ref_preserve_fd_acls
#define preserve_dir_acls ref_preserve_dir_acls
#define usage ref_usage
#define main ref_main
#define ftscmp ref_ftscmp
#define copy ref_copy
#define siginfo ref_siginfo


/* --- utils.c --- */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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




#define	cp_pct(x, y)	((y == 0) ? 0 : (int)(100.0 * (x) / (y)))

/*
 * Memory strategy threshold, in pages: if physmem is larger then this, use a 
 * large buffer.
 */
#define PHYSPAGES_THRESHOLD (32*1024)

/* Maximum buffer size in bytes - do not allow it to grow larger than this. */
#define BUFSIZE_MAX (2*1024*1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define BUFSIZE_SMALL (MAXPHYS)

/*
 * Prompt used in -i case.
 */
#define YESNO "(y/n [n]) "

ssize_t
ref_copy_fallback(int from_fd, int to_fd)
{
	static char *buf = NULL;
	static size_t bufsize;
	ssize_t rcount, wresid, wcount = 0;
	char *bufp;

	if (buf == NULL) {
		if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
			bufsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
		else
			bufsize = BUFSIZE_SMALL;
		buf = malloc(bufsize);
		if (buf == NULL)
			err(1, "Not enough memory");
	}
	rcount = read(from_fd, buf, bufsize);
	if (rcount <= 0)
		return (rcount);
	for (bufp = buf, wresid = rcount; ; bufp += wcount, wresid -= wcount) {
		wcount = write(to_fd, bufp, wresid);
		if (wcount <= 0)
			break;
		if (wcount >= wresid)
			break;
	}
	return (wcount < 0 ? wcount : rcount);
}

int
ref_copy_file(const FTSENT *entp, bool dne, bool beneath)
{
	struct stat sb, *fs;
	ssize_t wcount;
	off_t wtotal;
	int ch, checkch, from_fd, rval, to_fd;
	bool use_copy_file_range = true;

	fs = entp->fts_statp;
	from_fd = to_fd = -1;
	if (!lflag && !sflag) {
		if ((from_fd = open(entp->fts_path, O_RDONLY, 0)) < 0 ||
		    fstat(from_fd, &sb) != 0) {
			warn("%s", entp->fts_path);
			if (from_fd >= 0)
				(void)close(from_fd);
			return (1);
		}
		/*
		 * Check that the file hasn't been replaced with one of a
		 * different type.  This can happen if we've been asked to
		 * copy something which is actively being modified and
		 * lost the race, or if we've been asked to copy something
		 * like /proc/X/fd/Y which stat(2) reports as S_IFREG but
		 * is actually something else once you open it.
		 */
		if ((sb.st_mode & S_IFMT) != (fs->st_mode & S_IFMT)) {
			warnx("%s: File changed", entp->fts_path);
			(void)close(from_fd);
			return (1);
		}
	}

	/*
	 * If the file exists and we're interactive, verify with the user.
	 * If the file DNE, set the mode to be the from file, minus setuid
	 * bits, modified by the umask; arguably wrong, but it makes copying
	 * executables work right and it's been that way forever.  (The
	 * other choice is 666 or'ed with the execute bits on the from file
	 * modified by the umask.)
	 */
	if (!dne) {
		if (nflag) {
			if (vflag)
				printf("%s%s not overwritten\n",
				    to.base, to.path);
			rval = 1;
			goto done;
		} else if (iflag) {
			(void)fprintf(stderr, "overwrite %s%s? %s",
			    to.base, to.path, YESNO);
			checkch = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (checkch != 'y' && checkch != 'Y') {
				(void)fprintf(stderr, "not overwritten\n");
				rval = 1;
				goto done;
			}
		}

		if (fflag) {
			/* remove existing destination file */
			(void)unlinkat(to.dir, to.path,
			    beneath ? AT_RESOLVE_BENEATH : 0);
			dne = 1;
		}
	}

	rval = 0;

	if (lflag) {
		if (linkat(AT_FDCWD, entp->fts_path, to.dir, to.path, 0) != 0) {
			warn("%s%s", to.base, to.path);
			rval = 1;
		}
		goto done;
	}

	if (sflag) {
		if (symlinkat(entp->fts_path, to.dir, to.path) != 0) {
			warn("%s%s", to.base, to.path);
			rval = 1;
		}
		goto done;
	}

	if (!dne) {
		/* overwrite existing destination file */
		to_fd = openat(to.dir, to.path,
		    O_WRONLY | O_TRUNC | (beneath ? O_RESOLVE_BENEATH : 0), 0);
	} else {
		/* create new destination file */
		to_fd = openat(to.dir, to.path,
		    O_WRONLY | O_TRUNC | O_CREAT |
		    (beneath ? O_RESOLVE_BENEATH : 0),
		    fs->st_mode & ~(S_ISUID | S_ISGID));
	}
	if (to_fd == -1) {
		warn("%s%s", to.base, to.path);
		rval = 1;
		goto done;
	}

	wtotal = 0;
	do {
		if (use_copy_file_range) {
			wcount = copy_file_range(from_fd, NULL,
			    to_fd, NULL, SSIZE_MAX, 0);
			if (wcount < 0 && errno == EINVAL) {
				/* probably a non-seekable descriptor */
				use_copy_file_range = false;
			}
		}
		if (!use_copy_file_range) {
			wcount = copy_fallback(from_fd, to_fd);
		}
		if (wcount >= 0)
			wtotal += wcount;
		else if (errno != EINTR)
			break;
		if (info) {
			info = 0;
			(void)fprintf(stderr,
			    "%s -> %s%s %3d%%\n",
			    entp->fts_path, to.base, to.path,
			    cp_pct(wtotal, fs->st_size));
		}
	} while (wcount != 0);
	if (wcount < 0) {
		warn("%s", entp->fts_path);
		rval = 1;
	}

	/*
	 * Don't remove the target even after an error.  The target might
	 * not be a regular file, or its attributes might be important,
	 * or its contents might be irreplaceable.  It would only be safe
	 * to remove it if we created it and its length is 0.
	 */
	if (pflag && setfile(fs, to_fd, beneath))
		rval = 1;
	if (pflag && preserve_fd_acls(from_fd, to_fd) != 0)
		rval = 1;
	if (close(to_fd)) {
		warn("%s%s", to.base, to.path);
		rval = 1;
	}

done:
	if (from_fd != -1)
		(void)close(from_fd);
	return (rval);
}

int
ref_copy_link(const FTSENT *p, bool dne, bool beneath)
{
	ssize_t len;
	int atflags = beneath ? AT_RESOLVE_BENEATH : 0;
	char llink[PATH_MAX];

	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if ((len = readlink(p->fts_path, llink, sizeof(llink) - 1)) == -1) {
		warn("readlink: %s", p->fts_path);
		return (1);
	}
	llink[len] = '\0';
	if (!dne && unlinkat(to.dir, to.path, atflags) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (symlinkat(llink, to.dir, to.path) != 0) {
		warn("symlink: %s", llink);
		return (1);
	}
	return (pflag ? setfile(p->fts_statp, -1, beneath) : 0);
}

int
ref_copy_fifo(struct stat *from_stat, bool dne, bool beneath)
{
	int atflags = beneath ? AT_RESOLVE_BENEATH : 0;

	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if (!dne && unlinkat(to.dir, to.path, atflags) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (mkfifoat(to.dir, to.path, from_stat->st_mode) != 0) {
		warn("mkfifo: %s%s", to.base, to.path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1, beneath) : 0);
}

int
ref_copy_special(struct stat *from_stat, bool dne, bool beneath)
{
	int atflags = beneath ? AT_RESOLVE_BENEATH : 0;

	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if (!dne && unlinkat(to.dir, to.path, atflags) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (mknodat(to.dir, to.path, from_stat->st_mode, from_stat->st_rdev) != 0) {
		warn("mknod: %s%s", to.base, to.path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1, beneath) : 0);
}

int
ref_setfile(struct stat *fs, int fd, bool beneath)
{
	static struct timespec tspec[2];
	struct stat ts;
	int atflags = beneath ? AT_RESOLVE_BENEATH : 0;
	int rval, gotstat, islink, fdval;

	rval = 0;
	fdval = fd != -1;
	islink = !fdval && S_ISLNK(fs->st_mode);
	if (islink)
		atflags |= AT_SYMLINK_NOFOLLOW;
	fs->st_mode &= S_ISUID | S_ISGID | S_ISVTX |
	    S_IRWXU | S_IRWXG | S_IRWXO;

	tspec[0] = fs->st_atim;
	tspec[1] = fs->st_mtim;
	if (fdval ? futimens(fd, tspec) :
	    utimensat(to.dir, to.path, tspec, atflags)) {
		warn("utimensat: %s%s", to.base, to.path);
		rval = 1;
	}
	if (fdval ? fstat(fd, &ts) :
	    fstatat(to.dir, to.path, &ts, atflags)) {
		gotstat = 0;
	} else {
		gotstat = 1;
		ts.st_mode &= S_ISUID | S_ISGID | S_ISVTX |
		    S_IRWXU | S_IRWXG | S_IRWXO;
	}
	/*
	 * Changing the ownership probably won't succeed, unless we're root
	 * or POSIX_CHOWN_RESTRICTED is not set.  Set uid/gid before setting
	 * the mode; current BSD behavior is to remove all setuid bits on
	 * chown.  If chown fails, lose setuid/setgid bits.
	 */
	if (!gotstat || fs->st_uid != ts.st_uid || fs->st_gid != ts.st_gid) {
		if (fdval ? fchown(fd, fs->st_uid, fs->st_gid) :
		    fchownat(to.dir, to.path, fs->st_uid, fs->st_gid, atflags)) {
			if (errno != EPERM) {
				warn("chown: %s%s", to.base, to.path);
				rval = 1;
			}
			fs->st_mode &= ~(S_ISUID | S_ISGID);
		}
	}

	if (!gotstat || fs->st_mode != ts.st_mode) {
		if (fdval ? fchmod(fd, fs->st_mode) :
		    fchmodat(to.dir, to.path, fs->st_mode, atflags)) {
			warn("chmod: %s%s", to.base, to.path);
			rval = 1;
		}
	}

	if (!Nflag && (!gotstat || fs->st_flags != ts.st_flags)) {
		if (fdval ? fchflags(fd, fs->st_flags) :
		    chflagsat(to.dir, to.path, fs->st_flags, atflags)) {
			/*
			 * NFS doesn't support chflags; ignore errors unless
			 * there's reason to believe we're losing bits.  (Note,
			 * this still won't be right if the server supports
			 * flags and we were trying to *remove* flags on a file
			 * that we copied, i.e., that we didn't create.)
			 */
			if (errno != EOPNOTSUPP || fs->st_flags != 0) {
				warn("chflags: %s%s", to.base, to.path);
				rval = 1;
			}
		}
	}

	return (rval);
}

int
ref_preserve_fd_acls(int source_fd, int dest_fd)
{
	acl_t acl;
	acl_type_t acl_type;
	int acl_supported = 0, ret, trivial;

	ret = fpathconf(source_fd, _PC_ACL_NFS4);
	if (ret > 0 ) {
		acl_supported = 1;
		acl_type = ACL_TYPE_NFS4;
	} else if (ret < 0 && errno != EINVAL) {
		warn("fpathconf(..., _PC_ACL_NFS4) failed for %s%s",
		    to.base, to.path);
		return (-1);
	}
	if (acl_supported == 0) {
		ret = fpathconf(source_fd, _PC_ACL_EXTENDED);
		if (ret > 0 ) {
			acl_supported = 1;
			acl_type = ACL_TYPE_ACCESS;
		} else if (ret < 0 && errno != EINVAL) {
			warn("fpathconf(..., _PC_ACL_EXTENDED) failed for %s%s",
			    to.base, to.path);
			return (-1);
		}
	}
	if (acl_supported == 0)
		return (0);

	acl = acl_get_fd_np(source_fd, acl_type);
	if (acl == NULL) {
		warn("failed to get acl entries while setting %s%s",
		    to.base, to.path);
		return (-1);
	}
	if (acl_is_trivial_np(acl, &trivial)) {
		warn("acl_is_trivial() failed for %s%s",
		    to.base, to.path);
		acl_free(acl);
		return (-1);
	}
	if (trivial) {
		acl_free(acl);
		return (0);
	}
	if (acl_set_fd_np(dest_fd, acl, acl_type) < 0) {
		warn("failed to set acl entries for %s%s",
		    to.base, to.path);
		acl_free(acl);
		return (-1);
	}
	acl_free(acl);
	return (0);
}

int
ref_preserve_dir_acls(const char *source_dir, const char *dest_dir)
{
	int source_fd = -1, dest_fd = -1, ret;

	if ((source_fd = open(source_dir, O_DIRECTORY | O_RDONLY)) < 0) {
		warn("%s: failed to copy ACLs", source_dir);
		return (-1);
	}
	dest_fd = (*dest_dir == '\0') ? to.dir :
	    openat(to.dir, dest_dir, O_DIRECTORY, AT_RESOLVE_BENEATH);
	if (dest_fd < 0) {
		warn("%s: failed to copy ACLs to %s%s", source_dir,
		    to.base, dest_dir);
		close(source_fd);
		return (-1);
	}
	if ((ret = preserve_fd_acls(source_fd, dest_fd)) != 0) {
		/* preserve_fd_acls() already printed a message */
	}
	if (dest_fd != to.dir)
		close(dest_fd);
	close(source_fd);
	return (ret);
}

void
ref_usage(void)
{

	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file target_file",
	    "       cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file ... "
	    "target_directory");
	exit(EX_USAGE);
}

/* --- cp.c --- */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * David Hitz of Auspex Systems Inc.
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
 * Cp copies source files to target files.
 *
 * The global PATH_T structure "to" always contains the path to the
 * current target file.  Since fts(3) does not change directories,
 * this path can be either absolute or dot-relative.
 *
 * The basic algorithm is to initialize "to" and use fts(3) to traverse
 * the file hierarchy rooted in the argument list.  A trivial case is the
 * case of 'cp file1 file2'.  The more interesting case is the case of
 * 'cp file1 file2 ... fileN dir' where the hierarchy is traversed and the
 * path (relative to the root of the traversal) is appended to dir (stored
 * in "to") to form the final target path.
 */




static char ref_dot[] = ".";
#define dot ref_dot

#define END(buf) (buf + sizeof(buf))



enum {
	SORT_OPT = CHAR_MAX,
};

static const struct option long_opts[] =
{
	{ "archive",		no_argument,		NULL,	'a' },
	{ "force",		no_argument,		NULL,	'f' },
	{ "interactive",	no_argument,		NULL,	'i' },
	{ "dereference",	no_argument,		NULL,	'L' },
	{ "link",		no_argument,		NULL,	'l' },
	{ "no-clobber",		no_argument,		NULL,	'n' },
	{ "no-dereference",	no_argument,		NULL,	'P' },
	{ "recursive",		no_argument,		NULL,	'R' },
	{ "symbolic-link",	no_argument,		NULL,	's' },
	{ "verbose",		no_argument,		NULL,	'v' },
	{ "one-file-system",	no_argument,		NULL,	'x' },
	{ "sort",		no_argument,		NULL,	SORT_OPT },
	{ 0 }
};

int
ref_main(int argc, char *argv[])
{
	struct stat to_stat, tmp_stat;
	enum ref_op type;
	int ch, fts_options, r;
	char *sep, *target;
	bool have_trailing_slash = false;

	fts_options = FTS_NOCHDIR | FTS_PHYSICAL;
	while ((ch = getopt_long(argc, argv, "+HLPRafilNnprsvx", long_opts,
	    NULL)) != -1)
		switch (ch) {
		case 'H':
			Hflag = true;
			Lflag = Pflag = false;
			break;
		case 'L':
			Lflag = true;
			Hflag = Pflag = false;
			break;
		case 'P':
			Pflag = true;
			Hflag = Lflag = false;
			break;
		case 'R':
			Rflag = true;
			break;
		case 'a':
			pflag = true;
			Rflag = true;
			Pflag = true;
			Hflag = Lflag = false;
			break;
		case 'f':
			fflag = true;
			iflag = nflag = false;
			break;
		case 'i':
			iflag = true;
			fflag = nflag = false;
			break;
		case 'l':
			lflag = true;
			break;
		case 'N':
			Nflag = true;
			break;
		case 'n':
			nflag = true;
			fflag = iflag = false;
			break;
		case 'p':
			pflag = true;
			break;
		case 'r':
			rflag = Lflag = true;
			Hflag = Pflag = false;
			break;
		case 's':
			sflag = true;
			break;
		case 'v':
			vflag = true;
			break;
		case 'x':
			fts_options |= FTS_XDEV;
			break;
		case SORT_OPT:
			Sflag = true;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	if (Rflag && rflag)
		errx(1, "the -R and -r options may not be specified together");
	if (lflag && sflag)
		errx(1, "the -l and -s options may not be specified together");
	if (rflag)
		Rflag = true;
	if (Rflag) {
		if (Hflag)
			fts_options |= FTS_COMFOLLOW;
		if (Lflag) {
			fts_options &= ~FTS_PHYSICAL;
			fts_options |= FTS_LOGICAL;
		}
	} else if (!Pflag) {
		fts_options &= ~FTS_PHYSICAL;
		fts_options |= FTS_LOGICAL | FTS_COMFOLLOW;
	}
	(void)signal(SIGINFO, siginfo);

	/* Save the target base in "to". */
	target = argv[--argc];
	if (*target == '\0') {
		target = dot;
	} else if ((sep = strrchr(target, '/')) != NULL && sep[1] == '\0') {
		have_trailing_slash = true;
		while (sep > target && *sep == '/')
			sep--;
		sep[1] = '\0';
	}
	/*
	 * Copy target into to.base, leaving room for a possible separator
	 * which will be appended later in the non-FILE_TO_FILE cases.
	 */
	if (strlcpy(to.base, target, sizeof(to.base) - 1) >=
	    sizeof(to.base) - 1)
		errc(1, ENAMETOOLONG, "%s", target);

	/* Set end of argument list for fts(3). */
	argv[argc] = NULL;

	/*
	 * Cp has two distinct cases:
	 *
	 * cp [-R] source target
	 * cp [-R] source1 ... sourceN directory
	 *
	 * In both cases, source can be either a file or a directory.
	 *
	 * In (1), the target becomes a copy of the source. That is, if the
	 * source is a file, the target will be a file, and likewise for
	 * directories.
	 *
	 * In (2), the real target is not directory, but "directory/source".
	 */
	r = stat(to.base, &to_stat);
	if (r == -1 && errno != ENOENT)
		err(1, "%s", target);
	if (r == -1 || !S_ISDIR(to_stat.st_mode)) {
		/*
		 * Case (1).  Target is not a directory.
		 */
		if (argc > 1)
			errc(1, ENOTDIR, "%s", target);

		/*
		 * Need to detect the case:
		 *	cp -R dir foo
		 * Where dir is a directory and foo does not exist, where
		 * we want pathname concatenations turned on but not for
		 * the initial mkdir().
		 */
		if (r == -1) {
			if (Rflag && (Lflag || Hflag))
				stat(*argv, &tmp_stat);
			else
				lstat(*argv, &tmp_stat);

			if (S_ISDIR(tmp_stat.st_mode) && Rflag)
				type = DIR_TO_DNE;
			else
				type = FILE_TO_FILE;
		} else
			type = FILE_TO_FILE;

		if (have_trailing_slash && type == FILE_TO_FILE) {
			if (r == -1)
				errc(1, ENOENT, "%s", target);
			else
				errc(1, ENOTDIR, "%s", target);
		}
	} else {
		/*
		 * Case (2).  Target is a directory.
		 */
		type = FILE_TO_DIR;
	}

	/*
	 * For DIR_TO_DNE, we could provide copy() with the to_stat we've
	 * already allocated on the stack here that isn't being used for
	 * anything.  Not doing so, though, simplifies later logic a little bit
	 * as we need to skip checking root_stat on the first iteration and
	 * ensure that we set it with the first mkdir().
	 */
	exit (copy(argv, type, fts_options, (type == DIR_TO_DNE ? NULL :
	    &to_stat)));
}

int
ref_ftscmp(const FTSENT * const *a, const FTSENT * const *b)
{
	return (strcmp((*a)->fts_name, (*b)->fts_name));
}

int
ref_ref_copy(char *argv[], enum ref_op type, int fts_options, struct stat *root_stat)
{
	char rootname[NAME_MAX];
	struct stat created_root_stat, to_stat, *curr_stat;
	FTS *ftsp;
	FTSENT *curr;
	char *recpath = NULL, *sep;
	int atflags, dne, badcp, len, level, rval;
	mode_t mask, mode;
	bool beneath = Rflag && type != FILE_TO_FILE;

	/*
	 * Keep an inverted copy of the umask, for use in correcting
	 * permissions on created directories when not using -p.
	 */
	mask = ~umask(0777);
	umask(~mask);

	if (type == FILE_TO_FILE) {
		to.dir = AT_FDCWD;
		to.end = to.path + strlcpy(to.path, to.base, sizeof(to.path));
		to.base[0] = '\0';
	} else if (type == FILE_TO_DIR) {
		to.dir = open(to.base, O_DIRECTORY | O_SEARCH);
		if (to.dir < 0)
			err(1, "%s", to.base);
		/*
		 * We have previously made sure there is room for this.
		 */
		if (strcmp(to.base, "/") != 0) {
			sep = strchr(to.base, '\0');
			sep[0] = '/';
			sep[1] = '\0';
		}
	} else {
		/*
		 * We will create the destination directory imminently.
		 */
		to.dir = -1;
	}

	level = FTS_ROOTLEVEL;
	if ((ftsp = fts_open(argv, fts_options, Sflag ? ftscmp : NULL)) == NULL)
		err(1, "fts_open");
	for (badcp = rval = 0;
	     (curr = fts_read(ftsp)) != NULL;
	     badcp = 0, *to.end = '\0') {
		curr_stat = curr->fts_statp;
		switch (curr->fts_info) {
		case FTS_NS:
		case FTS_DNR:
		case FTS_ERR:
			if (level > curr->fts_level) {
				/* leaving a directory; remove its name from to.path */
				if (type == DIR_TO_DNE &&
				    curr->fts_level == FTS_ROOTLEVEL) {
					/* this is actually our created root */
				} else {
					while (to.end > to.path && *to.end != '/')
						to.end--;
					assert(strcmp(to.end + (*to.end == '/'),
					    curr->fts_name) == 0);
					*to.end = '\0';
				}
				level--;
			}
			warnc(curr->fts_errno, "%s", curr->fts_path);
			badcp = rval = 1;
			continue;
		case FTS_DC:			/* Warn, continue. */
			warnx("%s: directory causes a cycle", curr->fts_path);
			badcp = rval = 1;
			continue;
		case FTS_D:
			/*
			 * Stash the root basename off for detecting
			 * recursion later.
			 *
			 * This will be essential if the root is a symlink
			 * and we're rolling with -L or -H.  The later
			 * bits will need this bit in particular.
			 */
			if (curr->fts_level == FTS_ROOTLEVEL) {
				strlcpy(rootname, curr->fts_name,
				    sizeof(rootname));
			}
			/* we must have a destination! */
			if (type == DIR_TO_DNE &&
			    curr->fts_level == FTS_ROOTLEVEL) {
				assert(to.dir < 0);
				assert(root_stat == NULL);
				mode = curr_stat->st_mode | S_IRWXU;
				/*
				 * Will our umask prevent us from entering
				 * the directory after we create it?
				 */
				if (~mask & S_IRWXU)
					umask(~mask & ~S_IRWXU);
				if (mkdir(to.base, mode) != 0) {
					warn("%s", to.base);
					fts_set(ftsp, curr, FTS_SKIP);
					badcp = rval = 1;
					if (~mask & S_IRWXU)
						umask(~mask);
					continue;
				}
				to.dir = open(to.base, O_DIRECTORY | O_SEARCH);
				if (to.dir < 0) {
					warn("%s", to.base);
					(void)rmdir(to.base);
					fts_set(ftsp, curr, FTS_SKIP);
					badcp = rval = 1;
					if (~mask & S_IRWXU)
						umask(~mask);
					continue;
				}
				if (fstat(to.dir, &created_root_stat) != 0) {
					warn("%s", to.base);
					(void)close(to.dir);
					(void)rmdir(to.base);
					fts_set(ftsp, curr, FTS_SKIP);
					to.dir = -1;
					badcp = rval = 1;
					if (~mask & S_IRWXU)
						umask(~mask);
					continue;
				}
				if (~mask & S_IRWXU)
					umask(~mask);
				root_stat = &created_root_stat;
				curr->fts_number = 1;
				/*
				 * We have previously made sure there is
				 * room for this.
				 */
				sep = strchr(to.base, '\0');
				sep[0] = '/';
				sep[1] = '\0';
			} else if (strcmp(curr->fts_name, "/") == 0) {
				/* special case when source is the root directory */
			} else {
				/* entering a directory; append its name to to.path */
				len = snprintf(to.end, END(to.path) - to.end, "%s%s",
				    to.end > to.path ? "/" : "", curr->fts_name);
				if (to.end + len >= END(to.path)) {
					*to.end = '\0';
					warnc(ENAMETOOLONG, "%s%s%s%s", to.base,
					    to.path, to.end > to.path ? "/" : "",
					    curr->fts_name);
					fts_set(ftsp, curr, FTS_SKIP);
					badcp = rval = 1;
					continue;
				}
				to.end += len;
			}
			level++;
			/*
			 * We're on the verge of recursing on ourselves.
			 * Either we need to stop right here (we knowingly
			 * just created it), or we will in an immediate
			 * descendant.  Record the path of the immediate
			 * descendant to make our lives a little less
			 * complicated looking.
			 */
			if (type != FILE_TO_FILE &&
			    root_stat->st_dev == curr_stat->st_dev &&
			    root_stat->st_ino == curr_stat->st_ino) {
				assert(recpath == NULL);
				if (root_stat == &created_root_stat) {
					/*
					 * This directory didn't exist
					 * when we started, we created it
					 * as part of traversal.  Stop
					 * right here before we do
					 * something silly.
					 */
					fts_set(ftsp, curr, FTS_SKIP);
					continue;
				}
				if (asprintf(&recpath, "%s/%s", to.path,
				    rootname) < 0) {
					warnc(ENOMEM, NULL);
					fts_set(ftsp, curr, FTS_SKIP);
					badcp = rval = 1;
					continue;
				}
			}
			if (recpath != NULL &&
			    strcmp(recpath, to.path) == 0) {
				fts_set(ftsp, curr, FTS_SKIP);
				continue;
			}
			break;
		case FTS_DP:
			/*
			 * We are nearly finished with this directory.  If we
			 * didn't actually copy it, or otherwise don't need to
			 * change its attributes, then we are done.
			 *
			 * If -p is in effect, set all the attributes.
			 * Otherwise, set the correct permissions, limited
			 * by the umask.  Optimise by avoiding a chmod()
			 * if possible (which is usually the case if we
			 * made the directory).  Note that mkdir() does not
			 * honour setuid, setgid and sticky bits, but we
			 * normally want to preserve them on directories.
			 */
			if (curr->fts_number && pflag) {
				int fd = *to.path ? -1 : to.dir;
				if (setfile(curr_stat, fd, true))
					rval = 1;
				if (preserve_dir_acls(curr->fts_accpath,
				    to.path) != 0)
					rval = 1;
			} else if (curr->fts_number) {
				const char *path = *to.path ? to.path : dot;
				mode = curr_stat->st_mode;
				if (fchmodat(to.dir, path, mode & mask, 0) != 0) {
					warn("chmod: %s%s", to.base, to.path);
					rval = 1;
				}
			}
			if (level > curr->fts_level) {
				/* leaving a directory; remove its name from to.path */
				if (type == DIR_TO_DNE &&
				    curr->fts_level == FTS_ROOTLEVEL) {
					/* this is actually our created root */
				} else if (strcmp(curr->fts_name, "/") == 0) {
					/* special case when source is the root directory */
				} else {
					while (to.end > to.path && *to.end != '/')
						to.end--;
					assert(strcmp(to.end + (*to.end == '/'),
					    curr->fts_name) == 0);
					*to.end = '\0';
				}
				level--;
			}
			continue;
		default:
			/* something else: append its name to to.path */
			if (type == FILE_TO_FILE)
				break;
			len = snprintf(to.end, END(to.path) - to.end, "%s%s",
			    to.end > to.path ? "/" : "", curr->fts_name);
			if (to.end + len >= END(to.path)) {
				*to.end = '\0';
				warnc(ENAMETOOLONG, "%s%s%s%s", to.base,
				    to.path, to.end > to.path ? "/" : "",
				    curr->fts_name);
				badcp = rval = 1;
				continue;
			}
			/* intentionally do not update to.end */
			break;
		}

		/* Not an error but need to remember it happened. */
		if (to.path[0] == '\0') {
			/*
			 * This can happen in three cases:
			 * - The source path is the root directory.
			 * - DIR_TO_DNE; we created the directory and
			 *   populated root_stat earlier.
			 * - FILE_TO_DIR if a source has a trailing slash;
			 *   the caller populated root_stat.
			 */
			dne = false;
			to_stat = *root_stat;
		} else {
			atflags = beneath ? AT_RESOLVE_BENEATH : 0;
			if (curr->fts_info == FTS_D || curr->fts_info == FTS_SL)
				atflags |= AT_SYMLINK_NOFOLLOW;
			dne = fstatat(to.dir, to.path, &to_stat, atflags) != 0;
		}

		/* Check if source and destination are identical. */
		if (!dne &&
		    to_stat.st_dev == curr_stat->st_dev &&
		    to_stat.st_ino == curr_stat->st_ino) {
			warnx("%s%s and %s are identical (not copied).",
			    to.base, to.path, curr->fts_path);
			badcp = rval = 1;
			if (S_ISDIR(curr_stat->st_mode))
				fts_set(ftsp, curr, FTS_SKIP);
			continue;
		}

		switch (curr_stat->st_mode & S_IFMT) {
		case S_IFLNK:
			if ((fts_options & FTS_LOGICAL) ||
			    ((fts_options & FTS_COMFOLLOW) &&
			    curr->fts_level == 0)) {
				/*
				 * We asked FTS to follow links but got
				 * here anyway, which means the target is
				 * nonexistent or inaccessible.  Let
				 * copy_file() deal with the error.
				 */
				if (copy_file(curr, dne, beneath))
					badcp = rval = 1;
			} else {
				/* Copy the link. */
				if (copy_link(curr, dne, beneath))
					badcp = rval = 1;
			}
			break;
		case S_IFDIR:
			if (!Rflag) {
				warnx("%s is a directory (not copied).",
				    curr->fts_path);
				fts_set(ftsp, curr, FTS_SKIP);
				badcp = rval = 1;
				break;
			}
			/*
			 * If the directory doesn't exist, create the new
			 * one with the from file mode plus owner RWX bits,
			 * modified by the umask.  Trade-off between being
			 * able to write the directory (if from directory is
			 * 555) and not causing a permissions race.  If the
			 * umask blocks owner writes, we fail.
			 */
			if (dne) {
				mode = curr_stat->st_mode | S_IRWXU;
				/*
				 * Will our umask prevent us from entering
				 * the directory after we create it?
				 */
				if (~mask & S_IRWXU)
					umask(~mask & ~S_IRWXU);
				if (mkdirat(to.dir, to.path, mode) != 0) {
					warn("%s%s", to.base, to.path);
					fts_set(ftsp, curr, FTS_SKIP);
					badcp = rval = 1;
					if (~mask & S_IRWXU)
						umask(~mask);
					break;
				}
				if (~mask & S_IRWXU)
					umask(~mask);
			} else if (!S_ISDIR(to_stat.st_mode)) {
				warnc(ENOTDIR, "%s%s", to.base, to.path);
				fts_set(ftsp, curr, FTS_SKIP);
				badcp = rval = 1;
				break;
			}
			/*
			 * Arrange to correct directory attributes later
			 * (in the post-order phase) if this is a new
			 * directory, or if the -p flag is in effect.
			 * Note that fts_number may already be set if this
			 * is the newly created destination directory.
			 */
			curr->fts_number |= pflag || dne;
			break;
		case S_IFBLK:
		case S_IFCHR:
			if (Rflag && !sflag) {
				if (copy_special(curr_stat, dne, beneath))
					badcp = rval = 1;
			} else {
				if (copy_file(curr, dne, beneath))
					badcp = rval = 1;
			}
			break;
		case S_IFSOCK:
			warnx("%s is a socket (not copied).",
			    curr->fts_path);
			break;
		case S_IFIFO:
			if (Rflag && !sflag) {
				if (copy_fifo(curr_stat, dne, beneath))
					badcp = rval = 1;
			} else {
				if (copy_file(curr, dne, beneath))
					badcp = rval = 1;
			}
			break;
		default:
			if (copy_file(curr, dne, beneath))
				badcp = rval = 1;
			break;
		}
		if (vflag && !badcp)
			(void)printf("%s -> %s%s\n", curr->fts_path, to.base, to.path);
	}
	assert(level == FTS_ROOTLEVEL);
	if (errno)
		err(1, "fts_read");
	(void)fts_close(ftsp);
	if (to.dir != AT_FDCWD && to.dir >= 0)
		(void)close(to.dir);
	free(recpath);
	return (rval);
}

void
ref_siginfo(int sig __unused)
{

	info = 1;
}
