/*
 * b0200 oracle: the original HardenedBSD C sources for this batch,
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies are otherwise unmodified; only the supporting declarations that
 * normally live in pax.h / sel_subs.h / ftree.h have been inlined here so
 * that this file compiles stand alone with "cc -std=c11 -O2".
 *
 * File-scope variables that the original files declared "static" are left
 * with their original names but without "static", so the differential test
 * harness can set up and inspect them.  No function body was touched.
 */

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

/*
 * ---------------------------------------------------------------------------
 * from pax.h
 * ---------------------------------------------------------------------------
 */

#define	MAXBLK		64512	/* MAX blocksize supported (posix SPEC) */
#define	MAXBLK_POSIX	32256	/* MAX blocksize supported as per POSIX */
#define BLKMULT		512	/* blocksize must be even mult of 512 bytes */
#define DEVBLK		8192	/* default read blksize for devices */
#define FILEBLK		10240	/* default read blksize for files */
#define PAXPATHLEN	3072	/* maximum path length for pax. MUST be */
				/* longer than the system PATH_MAX */

typedef struct archd ARCHD;
typedef struct pattern PATTERN;

struct archd {
	int nlen;			/* file name length */
	char name[PAXPATHLEN+1];	/* file name */
	int ln_nlen;			/* link name length */
	char ln_name[PAXPATHLEN+1];	/* name to link to (if any) */
	char *org_name;			/* orig name in file system */
	PATTERN *pat;			/* ptr to pattern match (if any) */
	struct stat sb;			/* stat buffer see stat(2) */
	off_t pad;			/* bytes of padding after file xfer */
	off_t skip;			/* bytes of real data after header */
	u_long crc;			/* file crc */
	int type;			/* type of file node */
#define PAX_DIR		1		/* directory */
#define PAX_CHR		2		/* character device */
#define PAX_BLK		3		/* block device */
#define PAX_REG		4		/* regular file */
#define PAX_SLK		5		/* symbolic link */
#define PAX_SCK		6		/* socket */
#define PAX_FIF		7		/* fifo */
#define PAX_HLK		8		/* hard link */
#define PAX_HRG		9		/* hard link to a regular file */
#define PAX_CTG		10		/* high performance file */
};

#ifndef MIN
#define	       MIN(a,b) (((a)<(b))?(a):(b))
#endif

/*
 * ---------------------------------------------------------------------------
 * from sel_subs.h
 * ---------------------------------------------------------------------------
 */

#define USR_TB_SZ	317		/* user selection table size */
#define GRP_TB_SZ	317		/* user selection table size */

typedef struct usrt {
	uid_t uid;
	struct usrt *fow;		/* next uid */
} USRT;

typedef struct grpt {
	gid_t gid;
	struct grpt *fow;		/* next gid */
} GRPT;

#define ATOI2(ar)	((ar)[0] - '0') * 10 + ((ar)[1] - '0'); (ar) += 2;

typedef struct time_rng {
	time_t		low_time;	/* lower inclusive time limit */
	time_t		high_time;	/* higher inclusive time limit */
	int		flgs;		/* option flags */
#define	HASLOW		0x01		/* has lower time limit */
#define HASHIGH		0x02		/* has higher time limit */
#define CMPMTME		0x04		/* compare file modification time */
#define CMPCTME		0x08		/* compare inode change time */
#define CMPBOTH	(CMPMTME|CMPCTME)	/* compare inode and mod time */
	struct time_rng	*fow;		/* next pattern */
} TIME_RNG;

/*
 * ---------------------------------------------------------------------------
 * from ftree.h
 * ---------------------------------------------------------------------------
 */

typedef struct ftree {
	char		*fname;		/* file tree name */
	int		refcnt;		/* has tree had a selected file? */
	int		chflg;		/* change directory flag */
	struct ftree	*fow;		/* pointer to next entry on list */
} FTREE;

/*
 * ---------------------------------------------------------------------------
 * from extern.h (only what the ported functions reference)
 * ---------------------------------------------------------------------------
 */

extern void paxwarn(int, const char *, ...);
extern void syswarn(int, int, const char *, ...);

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

/* ========================= bin/pax/ftree.c ========================= */

FTREE *fthead = NULL;		/* head of linked list of file args */
FTREE *fttail = NULL;		/* tail of linked list of file args */

/*
 * ftree_add()
 *	add the arg to the linked list of files to process. Each will be
 *	processed by fts one at a time
 * Return:
 *	0 if added to the linked list, -1 if failed
 */

int
ref_ftree_add(char *str, int chflg)
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

/* ======================== bin/pax/sel_subs.c ======================== */

int ref_str_sec(const char *, time_t *);
int ref_usr_match(ARCHD *);
int ref_grp_match(ARCHD *);
int ref_trng_match(ARCHD *);

TIME_RNG *trhead = NULL;		/* time range list head */
TIME_RNG *trtail = NULL;		/* time range list tail */
USRT **usrtb = NULL;			/* user selection table */
GRPT **grptb = NULL;			/* group selection table */

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
ref_sel_chk(ARCHD *arcn)
{
	if (((usrtb != NULL) && ref_usr_match(arcn)) ||
	    ((grptb != NULL) && ref_grp_match(arcn)) ||
	    ((trhead != NULL) && ref_trng_match(arcn)))
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
ref_usr_add(char *str)
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

int
ref_usr_match(ARCHD *arcn)
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
ref_grp_add(char *str)
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

int
ref_grp_match(ARCHD *arcn)
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
ref_trng_add(char *str)
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
		if (ref_str_sec(str, &(pt->low_time)) < 0) {
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
		if (ref_str_sec(up_pt, &(pt->high_time)) < 0) {
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

int
ref_trng_match(ARCHD *arcn)
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

int
ref_str_sec(const char *p, time_t *tval)
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

/* ======================== bin/pax/file_subs.c ======================== */

#define FILEBITS		(S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)
#define SETBITS			(S_ISUID | S_ISGID)
#define ABITS			(FILEBITS | SETBITS)

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
ref_file_write(int fd, char *str, int cnt, int *rem, int *isempt, int sz,
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
ref_file_flush(int fd, char *fname, int isempt)
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
 * set_crc()
 *	read a file to calculate its crc. This is a real drag. Archive formats
 *	that have this, end up reading the file twice (we have to write the
 *	header WITH the crc before writing the file contents. Oh well...
 * Return:
 *	0 if was able to calculate the crc, -1 otherwise
 */

int
ref_set_crc(ARCHD *arcn, int fd)
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
