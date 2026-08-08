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

module;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" void paxwarn(int, const char *, ...);
extern "C" int l_strncpy(char *, const char *, int);
extern "C" unsigned long asc_ul(char *, int, int);
extern "C" unsigned long long asc_uqd(char *, int, int);
extern "C" int uid_name(char *, uid_t *);
extern "C" int gid_name(char *, gid_t *);
extern "C" int dflag;

export module pbsd.bin.pax.b0204;

#define MAXBLK 64512
#define BLKMULT 512
#define PAXPATHLEN 3072
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAJOR(x) major(x)
#define MINOR(x) minor(x)
#define TODEV(x,y) makedev((x),(y))
#define HEX 16
#define OCT 8
#define CHK_LEN 8
#define TNMSZ 100
#define NULLCNT 2
#define CHK_OFFSET 148
#define BLNKSUM 256L
#define REGTYPE '0'
#define AREGTYPE '\0'
#define LNKTYPE '1'
#define SYMTYPE '2'
#define CHRTYPE '3'
#define BLKTYPE '4'
#define DIRTYPE '5'
#define FIFOTYPE '6'
#define CONTTYPE '7'
#define TAR_PAD(x) ((512 - ((x) & 511)) & 511)
#define TPFSZ 155
#define TMAGIC "ustar"
#define TMAGLEN 6
#define TVERSION "00"
#define TVERSLEN 2
#define TRAILER "TRAILER!!!"
#define C_ISDIR 040000
#define C_ISFIFO 010000
#define C_ISREG 0100000
#define C_ISBLK 060000
#define C_ISCHR 020000
#define C_ISCTG 0110000
#define C_ISLNK 0120000
#define C_ISOCK 0140000
#define C_IFMT 0170000
#define MAGIC 070707
#define AMAGIC "070707"
#define SHRT_EXT(ch) ((((unsigned)(ch)[0])<<8) | (((unsigned)(ch)[1])&0xff))
#define RSHRT_EXT(ch) ((((unsigned)(ch)[1])<<8) | (((unsigned)(ch)[0])&0xff))
#define VCPIO_PAD(x) ((4 - ((x) & 3)) & 3)
#define BCPIO_PAD(x) ((2 - ((x) & 1)) & 1)
#define AVMAGIC "070701"
#define AVCMAGIC "070702"
#define MAXSUBEXP 10

export namespace pbsd::bin_pax::b0204 {

constexpr int PAX_DIR=1,PAX_CHR=2,PAX_BLK=3,PAX_REG=4,PAX_SLK=5,PAX_SCK=6,PAX_FIF=7,PAX_HLK=8,PAX_HRG=9,PAX_CTG=10;

struct pattern { char *pstr,*pend,*chdname; int plen,flgs; struct pattern *fow; };
typedef struct pattern PATTERN;
struct archd { int nlen; char name[PAXPATHLEN+1]; int ln_nlen; char ln_name[PAXPATHLEN+1]; char *org_name; PATTERN *pat; struct stat sb; off_t pad,skip; unsigned long crc; int type; };
typedef struct archd ARCHD;
typedef struct { char name[TNMSZ],mode[8],uid[8],gid[8],size[12],mtime[12],chksum[CHK_LEN],linkflag,linkname[TNMSZ]; } HD_TAR;
typedef struct { char name[TNMSZ],mode[8],uid[8],gid[8],size[12],mtime[12],chksum[CHK_LEN],typeflag,linkname[TNMSZ],magic[TMAGLEN],version[TVERSLEN],uname[32],gname[32],devmajor[8],devminor[8],prefix[TPFSZ]; } HD_USTAR;
typedef struct { char c_magic[6],c_dev[6],c_ino[6],c_mode[6],c_uid[6],c_gid[6],c_nlink[6],c_rdev[6],c_mtime[11],c_namesize[6],c_filesize[11]; } HD_CPIO;
typedef struct { unsigned char h_magic[2],h_dev[2],h_ino[2],h_mode[2],h_uid[2],h_gid[2],h_nlink[2],h_rdev[2],h_mtime_1[2],h_mtime_2[2],h_namesize[2],h_filesize_1[2],h_filesize_2[2]; } HD_BCPIO;
typedef struct { char c_magic[6],c_ino[8],c_mode[8],c_uid[8],c_gid[8],c_nlink[8],c_mtime[8],c_filesize[8],c_maj[8],c_min[8],c_rmaj[8],c_rmin[8],c_namesize[8],c_chksum[8]; } HD_VCPIO;

static int
fn_match(char *pattern, char *string, char **pend)
{
	char c;
	char test;

	*pend = NULL;
	for (;;) {
		switch (c = *pattern++) {
		case '\0':
			/*
			 * Ok we found an exact match
			 */
			if (*string == '\0')
				return(0);

			/*
			 * Check if it is a prefix match
			 */
			if ((dflag == 1) || (*string != '/'))
				return(-1);

			/*
			 * It is a prefix match, remember where the trailing
			 * / is located
			 */
			*pend = string;
			return(0);
		case '?':
			if ((test = *string++) == '\0')
				return (-1);
			break;
		case '*':
			c = *pattern;
			/*
			 * Collapse multiple *'s.
			 */
			while (c == '*')
				c = *++pattern;

			/*
			 * Optimized hack for pattern with a * at the end
			 */
			if (c == '\0')
				return (0);

			/*
			 * General case, use recursion.
			 */
			while ((test = *string) != '\0') {
				if (!fn_match(pattern, string, pend))
					return (0);
				++string;
			}
			return (-1);
		case '[':
			/*
			 * range match
			 */
			if (((test = *string++) == '\0') ||
			    ((pattern = range_match(pattern, test)) == NULL))
				return (-1);
			break;
		case '\\':
		default:
			if (c != *string++)
				return (-1);
			break;
		}
	}
	/* NOTREACHED */
}

static char *
range_match(char *pattern, int test)
{
	char c;
	char c2;
	int negate;
	int ok = 0;

	if ((negate = (*pattern == '!')) != 0)
		++pattern;

	while ((c = *pattern++) != ']') {
		/*
		 * Illegal pattern
		 */
		if (c == '\0')
			return (NULL);

		if ((*pattern == '-') && ((c2 = pattern[1]) != '\0') &&
		    (c2 != ']')) {
			if ((c <= test) && (test <= c2))
				ok = 1;
			pattern += 2;
		} else if (c == test)
			ok = 1;
	}
	return (ok == negate ? NULL : pattern);
}

/*
 * fix_path
 *	concatenate dir_name and or_name and store the result in or_name (if
 *	it fits). This is one ugly function.
 * Return:
 *	0 if ok, -1 if the final name is too long
 */

static int
fix_path( char *or_name, int *or_len, char *dir_name, int dir_len)
{
	char *src;
	char *dest;
	char *start;
	int len;

	/*
	 * we shift the or_name to the right enough to tack in the dir_name
	 * at the front. We make sure we have enough space for it all before
	 * we start. since dest always ends in a slash, we skip of or_name
	 * if it also starts with one.
	 */
	start = or_name;
	src = start + *or_len;
	dest = src + dir_len;
	if (*start == '/') {
		++start;
		--dest;
	}
	if ((len = dest - or_name) > PAXPATHLEN) {
		paxwarn(1, "File name %s/%s, too long", dir_name, start);
		return(-1);
	}
	*or_len = len;

	/*
	 * enough space, shift
	 */
	while (src >= start)
		*dest-- = *src--;
	src = dir_name + dir_len - 1;

	/*
	 * splice in the destination directory name
	 */
	while (src >= dir_name)
		*dest-- = *src--;

	*(or_name + len) = '\0';
	return(0);
}

/*
 * resub()
 *	apply the replacement to the matched expression. expand out the old
 * 	style ed(1) subexpression expansion.
 * Return:
 *	-1 if error, or the number of characters added to the destination.
 */

static int
resub(regex_t *rp, regmatch_t *pm, char *orig, char *src, char *dest,
	char *destend)
{
	char *spt;
	char *dpt;
	char c;
	regmatch_t *pmpt;
	int len;
	int subexcnt;

	spt =  src;
	dpt = dest;
	subexcnt = rp->re_nsub;
	while ((dpt < destend) && ((c = *spt++) != '\0')) {
		/*
		 * see if we just have an ordinary replacement character
		 * or we refer to a subexpression.
		 */
		if (c == '&') {
			pmpt = pm;
		} else if ((c == '\\') && (*spt >= '0') && (*spt <= '9')) {
			/*
			 * make sure there is a subexpression as specified
			 */
			if ((len = *spt++ - '0') > subexcnt)
				return(-1);
			pmpt = pm + len;
		} else {
 			/*
			 * Ordinary character, just copy it
			 */
 			if ((c == '\\') && ((*spt == '\\') || (*spt == '&')))
 				c = *spt++;
 			*dpt++ = c;
			continue;
		}

		/*
		 * continue if the subexpression is bogus
		 */
		if ((pmpt->rm_so < 0) || (pmpt->rm_eo < 0) ||
		    ((len = pmpt->rm_eo - pmpt->rm_so) <= 0))
			continue;

		/*
		 * copy the subexpression to the destination.
		 * fail if we run out of space or the match string is damaged
		 */
		if (len > (destend - dpt))
			len = destend - dpt;
		if (l_strncpy(dpt, orig + pmpt->rm_so, len) != len)
			return(-1);
		dpt += len;
	}
	return(dpt - dest);
}

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

/* ------------------------------------------------------------------ */
/* bin/pax/tar.c							      */
/* ------------------------------------------------------------------ */

/*
 * Routines for reading, writing and header identify of various versions of tar
 */

static unsigned long tar_chksm(char *, int);
static char *name_split(char *, int);
static int ul_oct(unsigned long, char *, int, int);
static int uqd_oct(unsigned long long, char *, int, int);

/*
 * tar_endrd()
 *	no cleanup needed here, just return size of trailer (for append)
 * Return:
 *	size of trailer (2 * BLKMULT)
 */

off_t
tar_endrd(void)
{
	return((off_t)(NULLCNT*BLKMULT));
}

/*
 * tar_trail()
 *	Called to determine if a header block is a valid trailer. We are passed
 *	the block, the in_sync flag (which tells us we are in resync mode;
 *	looking for a valid header), and cnt (which starts at zero) which is
 *	used to count the number of empty blocks we have seen so far.
 * Return:
 *	0 if a valid trailer, -1 if not a valid trailer, or 1 if the block
 *	could never contain a header.
 */

int
tar_trail(char *buf, int in_resync, int *cnt)
{
	int i;

	/*
	 * look for all zero, trailer is two consecutive blocks of zero
	 */
	for (i = 0; i < BLKMULT; ++i) {
		if (buf[i] != '\0')
			break;
	}

	/*
	 * if not all zero it is not a trailer, but MIGHT be a header.
	 */
	if (i != BLKMULT)
		return(-1);

	/*
	 * When given a zero block, we must be careful!
	 * If we are not in resync mode, check for the trailer. Have to watch
	 * out that we do not mis-identify file data as the trailer, so we do
	 * NOT try to id a trailer during resync mode. During resync mode we
	 * might as well throw this block out since a valid header can NEVER be
	 * a block of all 0 (we must have a valid file name).
	 */
	if (!in_resync && (++*cnt >= NULLCNT))
		return(0);
	return(1);
}

/*
 * ul_oct()
 *	convert an unsigned long to an octal string. many oddball field
 *	termination characters are used by the various versions of tar in the
 *	different fields. term selects which kind to use. str is '0' padded
 *	at the front to len. we are unable to use only one format as many old
 *	tar readers are very cranky about this.
 * Return:
 *	0 if the number fit into the string, -1 otherwise
 */

static int
ul_oct(unsigned long val, char *str, int len, int term)
{
	char *pt;

	/*
	 * term selects the appropriate character(s) for the end of the string
	 */
	pt = str + len - 1;
	switch(term) {
	case 3:
		*pt-- = '\0';
		break;
	case 2:
		*pt-- = ' ';
		*pt-- = '\0';
		break;
	case 1:
		*pt-- = ' ';
		break;
	case 0:
	default:
		*pt-- = '\0';
		*pt-- = ' ';
		break;
	}

	/*
	 * convert and blank pad if there is space
	 */
	while (pt >= str) {
		*pt-- = '0' + (char)(val & 0x7);
		if ((val = val >> 3) == (unsigned long)0)
			break;
	}

	while (pt >= str)
		*pt-- = '0';
	if (val != (unsigned long)0)
		return(-1);
	return(0);
}

/*
 * uqd_oct()
 *	convert an unsigned long long to an octal string. one of many oddball field
 *	termination characters are used by the various versions of tar in the
 *	different fields. term selects which kind to use. str is '0' padded
 *	at the front to len. we are unable to use only one format as many old
 *	tar readers are very cranky about this.
 * Return:
 *	0 if the number fit into the string, -1 otherwise
 */

static int
uqd_oct(unsigned long long val, char *str, int len, int term)
{
	char *pt;

	/*
	 * term selects the appropriate character(s) for the end of the string
	 */
	pt = str + len - 1;
	switch(term) {
	case 3:
		*pt-- = '\0';
		break;
	case 2:
		*pt-- = ' ';
		*pt-- = '\0';
		break;
	case 1:
		*pt-- = ' ';
		break;
	case 0:
	default:
		*pt-- = '\0';
		*pt-- = ' ';
		break;
	}

	/*
	 * convert and blank pad if there is space
	 */
	while (pt >= str) {
		*pt-- = '0' + (char)(val & 0x7);
		if ((val = val >> 3) == 0)
			break;
	}

	while (pt >= str)
		*pt-- = '0';
	if (val != (unsigned long long)0)
		return(-1);
	return(0);
}

/*
 * tar_chksm()
 *	calculate the checksum for a tar block counting the checksum field as
 *	all blanks (BLNKSUM is that value pre-calculated, the sum of 8 blanks).
 *	NOTE: we use len to short circuit summing 0's on write since we ALWAYS
 *	pad headers with 0.
 * Return:
 *	unsigned long checksum
 */

static unsigned long
tar_chksm(char *blk, int len)
{
	char *stop;
	char *pt;
	unsigned long chksm = BLNKSUM;	/* initial value is checksum field sum */

	/*
	 * add the part of the block before the checksum field
	 */
	pt = blk;
	stop = blk + CHK_OFFSET;
	while (pt < stop)
		chksm += (unsigned long)(*pt++ & 0xff);
	/*
	 * move past the checksum field and keep going, spec counts the
	 * checksum field as the sum of 8 blanks (which is pre-computed as
	 * BLNKSUM).
	 * ASSUMED: len is greater than CHK_OFFSET. (len is where our 0 padding
	 * starts, no point in summing zero's)
	 */
	pt += CHK_LEN;
	stop = blk + len;
	while (pt < stop)
		chksm += (unsigned long)(*pt++ & 0xff);
	return(chksm);
}

/*
 * tar_id()
 *	determine if a block given to us is a valid tar header (and not a USTAR
 *	header). We have to be on the lookout for those pesky blocks of all
 *	zero's.
 * Return:
 *	0 if a tar header, -1 otherwise
 */

int
tar_id(char *blk, int size)
{
	HD_TAR *hd;
	HD_USTAR *uhd;

	if (size < BLKMULT)
		return(-1);
	hd = (HD_TAR *)blk;
	uhd = (HD_USTAR *)blk;

	/*
	 * check for block of zero's first, a simple and fast test, then make
	 * sure this is not a ustar header by looking for the ustar magic
	 * cookie. We should use TMAGLEN, but some USTAR archive programs are
	 * wrong and create archives missing the \0. Last we check the
	 * checksum. If this is ok we have to assume it is a valid header.
	 */
	if (hd->name[0] == '\0')
		return(-1);
	if (strncmp(uhd->magic, TMAGIC, TMAGLEN - 1) == 0)
		return(-1);
	if (asc_ul(hd->chksum,sizeof(hd->chksum),OCT) != tar_chksm(blk,BLKMULT))
		return(-1);
	return(0);
}

/*
 * tar_rd()
 *	extract the values out of block already determined to be a tar header.
 *	store the values in the ARCHD parameter.
 * Return:
 *	0
 */

int
tar_rd(ARCHD *arcn, char *buf)
{
	HD_TAR *hd;
	char *pt;

	/*
	 * we only get proper sized buffers passed to us
	 */
	if (tar_id(buf, BLKMULT) < 0)
		return(-1);
	memset(arcn, 0, sizeof *arcn);
	arcn->org_name = arcn->name;
	arcn->sb.st_nlink = 1;

	/*
	 * copy out the name and values in the stat buffer
	 */
	hd = (HD_TAR *)buf;
	/*
	 * old tar format specifies the name always be null-terminated,
	 * but let's be robust to broken archives.
	 * the same applies to handling links below.
	 */
	arcn->nlen = l_strncpy(arcn->name, hd->name,
	    MIN(sizeof(hd->name), sizeof(arcn->name)) - 1);
	arcn->name[arcn->nlen] = '\0';
	arcn->sb.st_mode = (mode_t)(asc_ul(hd->mode,sizeof(hd->mode),OCT) &
	    0xfff);
	arcn->sb.st_uid = (uid_t)asc_ul(hd->uid, sizeof(hd->uid), OCT);
	arcn->sb.st_gid = (gid_t)asc_ul(hd->gid, sizeof(hd->gid), OCT);
	arcn->sb.st_size = (off_t)asc_uqd(hd->size, sizeof(hd->size), OCT);
	arcn->sb.st_mtime = (time_t)asc_uqd(hd->mtime, sizeof(hd->mtime), OCT);
	arcn->sb.st_ctime = arcn->sb.st_atime = arcn->sb.st_mtime;

	/*
	 * have to look at the last character, it may be a '/' and that is used
	 * to encode this as a directory
	 */
	pt = &(arcn->name[arcn->nlen - 1]);
	switch(hd->linkflag) {
	case SYMTYPE:
		/*
		 * symbolic link, need to get the link name and set the type in
		 * the st_mode so -v printing will look correct.
		 */
		arcn->type = PAX_SLK;
		arcn->ln_nlen = l_strncpy(arcn->ln_name, hd->linkname,
		    MIN(sizeof(hd->linkname), sizeof(arcn->ln_name)) - 1);
		arcn->ln_name[arcn->ln_nlen] = '\0';
		arcn->sb.st_mode |= S_IFLNK;
		break;
	case LNKTYPE:
		/*
		 * hard link, need to get the link name, set the type in the
		 * st_mode and st_nlink so -v printing will look better.
		 */
		arcn->type = PAX_HLK;
		arcn->sb.st_nlink = 2;
		arcn->ln_nlen = l_strncpy(arcn->ln_name, hd->linkname,
		    MIN(sizeof(hd->linkname), sizeof(arcn->ln_name)) - 1);
		arcn->ln_name[arcn->ln_nlen] = '\0';

		/*
		 * no idea of what type this thing really points at, but
		 * we set something for printing only.
		 */
		arcn->sb.st_mode |= S_IFREG;
		break;
	case DIRTYPE:
		/*
		 * It is a directory, set the mode for -v printing
		 */
		arcn->type = PAX_DIR;
		arcn->sb.st_mode |= S_IFDIR;
		arcn->sb.st_nlink = 2;
		break;
	case AREGTYPE:
	case REGTYPE:
	default:
		/*
		 * If we have a trailing / this is a directory and NOT a file.
		 */
		if (*pt == '/') {
			/*
			 * it is a directory, set the mode for -v printing
			 */
			arcn->type = PAX_DIR;
			arcn->sb.st_mode |= S_IFDIR;
			arcn->sb.st_nlink = 2;
		} else {
			/*
			 * have a file that will be followed by data. Set the
			 * skip value to the size field and calculate the size
			 * of the padding.
			 */
			arcn->type = PAX_REG;
			arcn->sb.st_mode |= S_IFREG;
			arcn->pad = TAR_PAD(arcn->sb.st_size);
			arcn->skip = arcn->sb.st_size;
		}
		break;
	}

	/*
	 * strip off any trailing slash.
	 */
	if (*pt == '/') {
		*pt = '\0';
		--arcn->nlen;
	}
	return(0);
}

/*
 * ustar_id()
 *	determine if a block given to us is a valid ustar header. We have to
 *	be on the lookout for those pesky blocks of all zero's
 * Return:
 *	0 if a ustar header, -1 otherwise
 */

int
ustar_id(char *blk, int size)
{
	HD_USTAR *hd;

	if (size < BLKMULT)
		return(-1);
	hd = (HD_USTAR *)blk;

	/*
	 * check for block of zero's first, a simple and fast test then check
	 * ustar magic cookie. We should use TMAGLEN, but some USTAR archive
	 * programs are fouled up and create archives missing the \0. Last we
	 * check the checksum. If ok we have to assume it is a valid header.
	 */
	if (hd->name[0] == '\0')
		return(-1);
	if (strncmp(hd->magic, TMAGIC, TMAGLEN - 1) != 0)
		return(-1);
	if (asc_ul(hd->chksum,sizeof(hd->chksum),OCT) != tar_chksm(blk,BLKMULT))
		return(-1);
	return(0);
}

/*
 * ustar_rd()
 *	extract the values out of block already determined to be a ustar header.
 *	store the values in the ARCHD parameter.
 * Return:
 *	0
 */

int
ustar_rd(ARCHD *arcn, char *buf)
{
	HD_USTAR *hd;
	char *dest;
	int cnt = 0;
	dev_t devmajor;
	dev_t devminor;

	/*
	 * we only get proper sized buffers
	 */
	if (ustar_id(buf, BLKMULT) < 0)
		return(-1);
	memset(arcn, 0, sizeof *arcn);
	arcn->org_name = arcn->name;
	arcn->sb.st_nlink = 1;
	hd = (HD_USTAR *)buf;

	/*
	 * see if the filename is split into two parts. if, so joint the parts.
	 * we copy the prefix first and add a / between the prefix and name.
	 */
	dest = arcn->name;
	if (*(hd->prefix) != '\0') {
		cnt = l_strncpy(dest, hd->prefix,
		    MIN(sizeof(hd->prefix), sizeof(arcn->name) - 2));
		dest += cnt;
		*dest++ = '/';
		cnt++;
	}
	/*
	 * ustar format specifies the name may be unterminated
	 * if it fills the entire field.  this also applies to
	 * the prefix and the linkname.
	 */
	arcn->nlen = cnt + l_strncpy(dest, hd->name,
	    MIN(sizeof(hd->name), sizeof(arcn->name) - cnt - 1));
	arcn->name[arcn->nlen] = '\0';

	/*
	 * follow the spec to the letter. we should only have mode bits, strip
	 * off all other crud we may be passed.
	 */
	arcn->sb.st_mode = (mode_t)(asc_ul(hd->mode, sizeof(hd->mode), OCT) &
	    0xfff);
	arcn->sb.st_size = (off_t)asc_uqd(hd->size, sizeof(hd->size), OCT);
	arcn->sb.st_mtime = (time_t)asc_uqd(hd->mtime, sizeof(hd->mtime), OCT);
	arcn->sb.st_ctime = arcn->sb.st_atime = arcn->sb.st_mtime;

	/*
	 * If we can find the ascii names for gname and uname in the password
	 * and group files we will use the uid's and gid they bind. Otherwise
	 * we use the uid and gid values stored in the header. (This is what
	 * the POSIX spec wants).
	 */
	hd->gname[sizeof(hd->gname) - 1] = '\0';
	if (gid_name(hd->gname, &(arcn->sb.st_gid)) < 0)
		arcn->sb.st_gid = (gid_t)asc_ul(hd->gid, sizeof(hd->gid), OCT);
	hd->uname[sizeof(hd->uname) - 1] = '\0';
	if (uid_name(hd->uname, &(arcn->sb.st_uid)) < 0)
		arcn->sb.st_uid = (uid_t)asc_ul(hd->uid, sizeof(hd->uid), OCT);

	/*
	 * set the mode and PAX type according to the typeflag in the header
	 */
	switch(hd->typeflag) {
	case FIFOTYPE:
		arcn->type = PAX_FIF;
		arcn->sb.st_mode |= S_IFIFO;
		break;
	case DIRTYPE:
		arcn->type = PAX_DIR;
		arcn->sb.st_mode |= S_IFDIR;
		arcn->sb.st_nlink = 2;

		/*
		 * Some programs that create ustar archives append a '/'
		 * to the pathname for directories. This clearly violates
		 * ustar specs, but we will silently strip it off anyway.
		 */
		if (arcn->name[arcn->nlen - 1] == '/')
			arcn->name[--arcn->nlen] = '\0';
		break;
	case BLKTYPE:
	case CHRTYPE:
		/*
		 * this type requires the rdev field to be set.
		 */
		if (hd->typeflag == BLKTYPE) {
			arcn->type = PAX_BLK;
			arcn->sb.st_mode |= S_IFBLK;
		} else {
			arcn->type = PAX_CHR;
			arcn->sb.st_mode |= S_IFCHR;
		}
		devmajor = (dev_t)asc_ul(hd->devmajor,sizeof(hd->devmajor),OCT);
		devminor = (dev_t)asc_ul(hd->devminor,sizeof(hd->devminor),OCT);
		arcn->sb.st_rdev = TODEV(devmajor, devminor);
		break;
	case SYMTYPE:
	case LNKTYPE:
		if (hd->typeflag == SYMTYPE) {
			arcn->type = PAX_SLK;
			arcn->sb.st_mode |= S_IFLNK;
		} else {
			arcn->type = PAX_HLK;
			/*
			 * so printing looks better
			 */
			arcn->sb.st_mode |= S_IFREG;
			arcn->sb.st_nlink = 2;
		}
		/*
		 * copy the link name
		 */
		arcn->ln_nlen = l_strncpy(arcn->ln_name, hd->linkname,
		    MIN(sizeof(hd->linkname), sizeof(arcn->ln_name) - 1));
		arcn->ln_name[arcn->ln_nlen] = '\0';
		break;
	case CONTTYPE:
	case AREGTYPE:
	case REGTYPE:
	default:
		/*
		 * these types have file data that follows. Set the skip and
		 * pad fields.
		 */
		arcn->type = PAX_REG;
		arcn->pad = TAR_PAD(arcn->sb.st_size);
		arcn->skip = arcn->sb.st_size;
		arcn->sb.st_mode |= S_IFREG;
		break;
	}
	return(0);
}

/*
 * name_split()
 *	see if the name has to be split for storage in a ustar header. We try
 *	to fit the entire name in the name field without splitting if we can.
 *	The split point is always at a /
 * Return
 *	character pointer to split point (always the / that is to be removed
 *	if the split is not needed, the points is set to the start of the file
 *	name (it would violate the spec to split there). A NULL is returned if
 *	the file name is too long
 */

static char *
name_split(char *name, int len)
{
	char *start;

	/*
	 * check to see if the file name is small enough to fit in the name
	 * field. if so just return a pointer to the name.
	 */
	if (len <= TNMSZ)
		return(name);
	if (len > TPFSZ + TNMSZ)
		return(NULL);

	/*
	 * we start looking at the biggest sized piece that fits in the name
	 * field. We walk forward looking for a slash to split at. The idea is
	 * to find the biggest piece to fit in the name field (or the smallest
	 * prefix we can find)
	 */
	start = name + len - TNMSZ;
	while ((*start != '\0') && (*start != '/'))
		++start;

	/*
	 * if we hit the end of the string, this name cannot be split, so we
	 * cannot store this file.
	 */
	if (*start == '\0')
		return(NULL);
	len = start - name;

	/*
	 * NOTE: /str where the length of str == TNMSZ can not be stored under
	 * the p1003.1-1990 spec for ustar. We could force a prefix of / and
	 * the file would then expand on extract to //str. The len == 0 below
	 * makes this special case follow the spec to the letter.
	 */
	if ((len > TPFSZ) || (len == 0))
		return(NULL);

	/*
	 * ok have a split point, return it to the caller
	 */
	return(start);
}

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

/* ------------------------------------------------------------------ */
/* bin/pax/cpio.c							      */
/* ------------------------------------------------------------------ */

/*
 * Routines which support the different cpio versions
 */

static int swp_head;		/* binary cpio header byte swap */

/*
 * cpio_trail()
 *	Called to determine if a header block is a valid trailer. We are
 *	passed the block, the in_sync flag (which tells us we are in resync
 *	mode; looking for a valid header), and cnt (which starts at zero)
 *	which is used to count the number of empty blocks we have seen so far.
 * Return:
 *	0 if a valid trailer, -1 if not a valid trailer,
 */

int
cpio_trail(ARCHD *arcn)
{
	/*
	 * look for trailer id in file we are about to process
	 */
	if ((strcmp(arcn->name, TRAILER) == 0) && (arcn->sb.st_size == 0))
		return(0);
	return(-1);
}

/*
 * cpio_id()
 *      determine if a block given to us is a valid extended byte oriented
 *	cpio header
 * Return:
 *      0 if a valid header, -1 otherwise
 */

int
cpio_id(char *blk, int size)
{
	if ((size < (int)sizeof(HD_CPIO)) ||
	    (strncmp(blk, AMAGIC, sizeof(AMAGIC) - 1) != 0))
		return(-1);
	return(0);
}

/*
 * cpio_endrd()
 *      no cleanup needed here, just return size of the trailer (for append)
 * Return:
 *      size of trailer header in this format
 */

off_t
cpio_endrd(void)
{
	return((off_t)(sizeof(HD_CPIO) + sizeof(TRAILER)));
}

/*
 * vcpio_id()
 *      determine if a block given to us is a valid system VR4 cpio header
 *	WITHOUT crc. WATCH it the magic cookies are in OCTAL, the header
 *	uses HEX
 * Return:
 *      0 if a valid header, -1 otherwise
 */

int
vcpio_id(char *blk, int size)
{
	if ((size < (int)sizeof(HD_VCPIO)) ||
	    (strncmp(blk, AVMAGIC, sizeof(AVMAGIC) - 1) != 0))
		return(-1);
	return(0);
}

/*
 * crc_id()
 *      determine if a block given to us is a valid system VR4 cpio header
 *	WITH crc. WATCH it the magic cookies are in OCTAL the header uses HEX
 * Return:
 *      0 if a valid header, -1 otherwise
 */

int
crc_id(char *blk, int size)
{
	if ((size < (int)sizeof(HD_VCPIO)) ||
	    (strncmp(blk, AVCMAGIC, (int)sizeof(AVCMAGIC) - 1) != 0))
		return(-1);
	return(0);
}

/*
 * vcpio_endrd()
 *      no cleanup needed here, just return size of the trailer (for append)
 * Return:
 *      size of trailer header in this format
 */

off_t
vcpio_endrd(void)
{
	return((off_t)(sizeof(HD_VCPIO) + sizeof(TRAILER) +
		(VCPIO_PAD(sizeof(HD_VCPIO) + sizeof(TRAILER)))));
}

/*
 * bcpio_id()
 *      determine if a block given to us is an old binary cpio header
 *	(with/without header byte swapping)
 * Return:
 *      0 if a valid header, -1 otherwise
 */

int
bcpio_id(char *blk, int size)
{
	if (size < (int)sizeof(HD_BCPIO))
		return(-1);

	/*
	 * check both normal and byte swapped magic cookies
	 */
	if (((u_short)SHRT_EXT(blk)) == MAGIC)
		return(0);
	if (((u_short)RSHRT_EXT(blk)) == MAGIC) {
		if (!swp_head)
			++swp_head;
		return(0);
	}
	return(-1);
}

/*
 * bcpio_endrd()
 *      no cleanup needed here, just return size of the trailer (for append)
 * Return:
 *      size of trailer header in this format
 */

off_t
bcpio_endrd(void)
{
	return((off_t)(sizeof(HD_BCPIO) + sizeof(TRAILER) +
		(BCPIO_PAD(sizeof(HD_BCPIO) + sizeof(TRAILER)))));
}


int fn_match_x(char *pattern, char *string, char **pend) { return fn_match(pattern,string,pend); }
char *range_match_x(char *pattern, int test) { return range_match(pattern,test); }
int fix_path_x(char *or_name, int *or_len, char *dir_name, int dir_len) { return fix_path(or_name,or_len,dir_name,dir_len); }
int resub_x(regex_t *rp, regmatch_t *pm, char *orig, char *src, char *dest, char *destend) { return resub(rp,pm,orig,src,dest,destend); }
int ul_oct_x(unsigned long val, char *str, int len, int term) { return ul_oct(val,str,len,term); }
int uqd_oct_x(unsigned long long val, char *str, int len, int term) { return uqd_oct(val,str,len,term); }
unsigned long tar_chksm_x(char *blk, int len) { return tar_chksm(blk,len); }
char *name_split_x(char *name, int len) { return name_split(name,len); }
int bcpio_swp_head(void) { return swp_head; }
int sizeof_HD_TAR(void) { return (int)sizeof(HD_TAR); }
int sizeof_HD_USTAR(void) { return (int)sizeof(HD_USTAR); }
int sizeof_HD_CPIO(void) { return (int)sizeof(HD_CPIO); }
int sizeof_HD_VCPIO(void) { return (int)sizeof(HD_VCPIO); }
int sizeof_HD_BCPIO(void) { return (int)sizeof(HD_BCPIO); }
int sizeof_ARCHD(void) { return (int)sizeof(ARCHD); }

}
