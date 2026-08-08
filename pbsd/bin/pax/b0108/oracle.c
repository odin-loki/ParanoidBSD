/*
 * b0108 oracle -- tty_subs.c, gen_subs.c, and cache.c concatenated.
 * Every batch function is renamed with a ref_ prefix via the preprocessor;
 * function bodies are otherwise UNMODIFIED.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <langinfo.h>
#include <limits.h>
#include <bsd/string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef u_quad_t
typedef unsigned long long u_quad_t;
#endif

#define MAXBLK		64512
#define PAXPATHLEN	3072
#define PAX_DIR		1
#define PAX_CHR		2
#define PAX_BLK		3
#define PAX_REG		4
#define PAX_SLK		5
#define PAX_HLK		8
#define PAX_HRG		9
#define HEX		16
#define OCT		8
#define MAJOR(x)	major(x)
#define MINOR(x)	minor(x)

#define UNMLEN		32
#define GNMLEN		32
#define UID_SZ		317
#define UNM_SZ		317
#define GID_SZ		251
#define GNM_SZ		317
#define VALID		1
#define INVALID		2
#define MAXKEYLEN	64

typedef struct archd ARCHD;
typedef struct uidc {
	int valid;
	char name[UNMLEN];
	uid_t uid;
} UIDC;
typedef struct gidc {
	int valid;
	char name[GNMLEN];
	gid_t gid;
} GIDC;

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

int iflag;
int vflag;
int vfpart;
int exit_val;
const char *argv0 = "pax";
FILE *listf;

u_int
st_hash(char *name, int len, int tabsz)
{
	char *pt;
	char *dest;
	char *end;
	int i;
	u_int key = 0;
	int steps;
	int res;
	u_int val;

	if (len > MAXKEYLEN) {
		pt = &(name[len - MAXKEYLEN]);
		len = MAXKEYLEN;
	} else
		pt = name;

	steps = len/sizeof(u_int);
	res = len % sizeof(u_int);

	for (i = 0; i < steps; ++i) {
		end = pt + sizeof(u_int);
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	if (res) {
		val = 0;
		end = pt + res;
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	return(key % tabsz);
}

#define tty_init ref_tty_init
#define tty_prnt ref_tty_prnt
#define tty_read ref_tty_read
#define paxwarn ref_paxwarn
#define syswarn ref_syswarn
#define ls_list ref_ls_list
#define ls_tty ref_ls_tty
#define l_strncpy ref_l_strncpy
#define asc_ul ref_asc_ul
#define ul_asc ref_ul_asc
#define asc_uqd ref_asc_uqd
#define uqd_asc ref_uqd_asc
#define uidtb_start ref_uidtb_start
#define gidtb_start ref_gidtb_start
#define usrtb_start ref_usrtb_start
#define grptb_start ref_grptb_start
#define name_uid ref_name_uid
#define name_gid ref_name_gid
#define uid_name ref_uid_name
#define gid_name ref_gid_name

/* ======================= tty_subs.c ======================= */

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
 * routines that deal with I/O to and from the user
 */

#define DEVTTY	  "/dev/tty"      /* device for interactive i/o */
static FILE *ttyoutf = NULL;		/* output pointing at control tty */
static FILE *ttyinf = NULL;		/* input pointing at control tty */

/*
 * tty_init()
 *	try to open the controlling terminal (if any) for this process. if the
 *	open fails, future ops that require user input will get an EOF
 */

int
tty_init(void)
{
	int ttyfd;

	if ((ttyfd = open(DEVTTY, O_RDWR)) >= 0) {
		if ((ttyoutf = fdopen(ttyfd, "w")) != NULL) {
			if ((ttyinf = fdopen(ttyfd, "r")) != NULL)
				return(0);
			(void)fclose(ttyoutf);
		}
		(void)close(ttyfd);
	}

	if (iflag) {
		paxwarn(1, "Fatal error, cannot open %s", DEVTTY);
		return(-1);
	}
	return(0);
}

/*
 * tty_prnt()
 *	print a message using the specified format to the controlling tty
 *	if there is no controlling terminal, just return.
 */

void
tty_prnt(const char *fmt, ...)
{
	va_list ap;
	if (ttyoutf == NULL)
		return;
	va_start(ap, fmt);
	(void)vfprintf(ttyoutf, fmt, ap);
	va_end(ap);
	(void)fflush(ttyoutf);
}

/*
 * tty_read()
 *	read a string from the controlling terminal if it is open into the
 *	supplied buffer
 * Return:
 *	0 if data was read, -1 otherwise.
 */

int
tty_read(char *str, int len)
{
	char *pt;

	if ((--len <= 0) || (ttyinf == NULL) || (fgets(str,len,ttyinf) == NULL))
		return(-1);
	*(str + len) = '\0';

	/*
	 * strip off that trailing newline
	 */
	if ((pt = strchr(str, '\n')) != NULL)
		*pt = '\0';
	return(0);
}

/*
 * paxwarn()
 *	write a warning message to stderr. if "set" the exit value of pax
 *	will be non-zero.
 */

void
paxwarn(int set, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (set)
		exit_val = 1;
	/*
	 * when vflag we better ship out an extra \n to get this message on a
	 * line by itself
	 */
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

/*
 * syswarn()
 *	write a warning message to stderr. if "set" the exit value of pax
 *	will be non-zero.
 */

void
syswarn(int set, int errnum, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (set)
		exit_val = 1;
	/*
	 * when vflag we better ship out an extra \n to get this message on a
	 * line by itself
	 */
	if (vflag && vfpart) {
		(void)fflush(listf);
		(void)fputc('\n', stderr);
		vfpart = 0;
	}
	(void)fprintf(stderr, "%s: ", argv0);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);

	/*
	 * format and print the errno
	 */
	if (errnum > 0)
		(void)fprintf(stderr, " <%s>", strerror(errnum));
	(void)fputc('\n', stderr);
}

/* ======================= gen_subs.c ======================= */

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
 * a collection of general purpose subroutines used by pax
 */

/*
 * constants used by ls_list() when printing out archive members
 */
#define MODELEN 20
#define DATELEN 64
#define SIXMONTHS	 ((365 / 2) * 86400)
#define CURFRMTM	"%b %e %H:%M"
#define OLDFRMTM	"%b %e  %Y"
#define CURFRMTD	"%e %b %H:%M"
#define OLDFRMTD	"%e %b  %Y"

static int d_first = -1;

/*
 * ls_list()
 *	list the members of an archive in ls format
 */

void
ls_list(ARCHD *arcn, time_t now, FILE *fp)
{
	struct stat *sbp;
	char f_mode[MODELEN];
	char f_date[DATELEN];
	const char *timefrmt;

	/*
	 * if not verbose, just print the file name
	 */
	if (!vflag) {
		(void)fprintf(fp, "%s\n", arcn->name);
		(void)fflush(fp);
		return;
	}

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');
	/*
	 * user wants long mode
	 */
	sbp = &(arcn->sb);
	strmode(sbp->st_mode, f_mode);

	/*
	 * time format based on age compared to the time pax was started.
	 */
	if ((sbp->st_mtime + SIXMONTHS) <= now)
		timefrmt = d_first ? OLDFRMTD : OLDFRMTM;
	else
		timefrmt = d_first ? CURFRMTD : CURFRMTM;

	/*
	 * print file mode, link count, uid, gid and time
	 */
	if (strftime(f_date,DATELEN,timefrmt,localtime(&(sbp->st_mtime))) == 0)
		f_date[0] = '\0';
	(void)fprintf(fp, "%s%2ju %-12s %-12s ", f_mode,
		(uintmax_t)sbp->st_nlink,
		name_uid(sbp->st_uid, 1), name_gid(sbp->st_gid, 1));

	/*
	 * print device id's for devices, or sizes for other nodes
	 */
	if ((arcn->type == PAX_CHR) || (arcn->type == PAX_BLK))
		(void)fprintf(fp, "%4lu,%4lu ", (unsigned long)MAJOR(sbp->st_rdev),
		    (unsigned long)MINOR(sbp->st_rdev));
	else {
		(void)fprintf(fp, "%9ju ", (uintmax_t)sbp->st_size);
	}

	/*
	 * print name and link info for hard and soft links
	 */
	(void)fprintf(fp, "%s %s", f_date, arcn->name);
	if ((arcn->type == PAX_HLK) || (arcn->type == PAX_HRG))
		(void)fprintf(fp, " == %s\n", arcn->ln_name);
	else if (arcn->type == PAX_SLK)
		(void)fprintf(fp, " => %s\n", arcn->ln_name);
	else
		(void)putc('\n', fp);
	(void)fflush(fp);
	return;
}

/*
 * tty_ls()
 * 	print a short summary of file to tty.
 */

void
ls_tty(ARCHD *arcn)
{
	char f_date[DATELEN];
	char f_mode[MODELEN];
	const char *timefrmt;

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');

	if ((arcn->sb.st_mtime + SIXMONTHS) <= time(NULL))
		timefrmt = d_first ? OLDFRMTD : OLDFRMTM;
	else
		timefrmt = d_first ? CURFRMTD : CURFRMTM;

	/*
	 * convert time to string, and print
	 */
	if (strftime(f_date, DATELEN, timefrmt,
	    localtime(&(arcn->sb.st_mtime))) == 0)
		f_date[0] = '\0';
	strmode(arcn->sb.st_mode, f_mode);
	tty_prnt("%s%s %s\n", f_mode, f_date, arcn->name);
	return;
}

/*
 * l_strncpy()
 *	copy src to dest up to len chars (stopping at first '\0').
 *	when src is shorter than len, pads to len with '\0'. 
 * Return:
 *	number of chars copied. (Note this is a real performance win over
 *	doing a strncpy(), a strlen(), and then a possible memset())
 */

int
l_strncpy(char *dest, const char *src, int len)
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

/*
 * asc_ul()
 *	convert hex/octal character string into a u_long. We do not have to
 *	check for overflow! (the headers in all supported formats are not large
 *	enough to create an overflow).
 *	NOTE: strings passed to us are NOT TERMINATED.
 * Return:
 *	unsigned long value
 */

u_long
asc_ul(char *str, int len, int base)
{
	char *stop;
	u_long tval = 0;

	stop = str + len;

	/*
	 * skip over leading blanks and zeros
	 */
	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;

	/*
	 * for each valid digit, shift running value (tval) over to next digit
	 * and add next digit
	 */
	if (base == HEX) {
		while (str < stop) {
			if ((*str >= '0') && (*str <= '9'))
				tval = (tval << 4) + (*str++ - '0');
			else if ((*str >= 'A') && (*str <= 'F'))
				tval = (tval << 4) + 10 + (*str++ - 'A');
			else if ((*str >= 'a') && (*str <= 'f'))
				tval = (tval << 4) + 10 + (*str++ - 'a');
			else
				break;
		}
	} else {
 		while ((str < stop) && (*str >= '0') && (*str <= '7'))
			tval = (tval << 3) + (*str++ - '0');
	}
	return(tval);
}

/*
 * ul_asc()
 *	convert an unsigned long into an hex/oct ascii string. pads with LEADING
 *	ascii 0's to fill string completely
 *	NOTE: the string created is NOT TERMINATED.
 */

int
ul_asc(u_long val, char *str, int len, int base)
{
	char *pt;
	u_long digit;

	/*
	 * WARNING str is not '\0' terminated by this routine
	 */
	pt = str + len - 1;

	/*
	 * do a tailwise conversion (start at right most end of string to place
	 * least significant digit). Keep shifting until conversion value goes
	 * to zero (all digits were converted)
	 */
	if (base == HEX) {
		while (pt >= str) {
			if ((digit = (val & 0xf)) < 10)
				*pt-- = '0' + (char)digit;
			else
				*pt-- = 'a' + (char)(digit - 10);
			if ((val = (val >> 4)) == (u_long)0)
				break;
		}
	} else {
		while (pt >= str) {
			*pt-- = '0' + (char)(val & 0x7);
			if ((val = (val >> 3)) == (u_long)0)
				break;
		}
	}

	/*
	 * pad with leading ascii ZEROS. We return -1 if we ran out of space.
	 */
	while (pt >= str)
		*pt-- = '0';
	if (val != (u_long)0)
		return(-1);
	return(0);
}

/*
 * asc_uqd()
 *	convert hex/octal character string into a u_quad_t. We do not have to
 *	check for overflow! (the headers in all supported formats are not large
 *	enough to create an overflow).
 *	NOTE: strings passed to us are NOT TERMINATED.
 * Return:
 *	u_quad_t value
 */

u_quad_t
asc_uqd(char *str, int len, int base)
{
	char *stop;
	u_quad_t tval = 0;

	stop = str + len;

	/*
	 * skip over leading blanks and zeros
	 */
	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;

	/*
	 * for each valid digit, shift running value (tval) over to next digit
	 * and add next digit
	 */
	if (base == HEX) {
		while (str < stop) {
			if ((*str >= '0') && (*str <= '9'))
				tval = (tval << 4) + (*str++ - '0');
			else if ((*str >= 'A') && (*str <= 'F'))
				tval = (tval << 4) + 10 + (*str++ - 'A');
			else if ((*str >= 'a') && (*str <= 'f'))
				tval = (tval << 4) + 10 + (*str++ - 'a');
			else
				break;
		}
	} else {
 		while ((str < stop) && (*str >= '0') && (*str <= '7'))
			tval = (tval << 3) + (*str++ - '0');
	}
	return(tval);
}

/*
 * uqd_asc()
 *	convert an u_quad_t into a hex/oct ascii string. pads with LEADING
 *	ascii 0's to fill string completely
 *	NOTE: the string created is NOT TERMINATED.
 */

int
uqd_asc(u_quad_t val, char *str, int len, int base)
{
	char *pt;
	u_quad_t digit;

	/*
	 * WARNING str is not '\0' terminated by this routine
	 */
	pt = str + len - 1;

	/*
	 * do a tailwise conversion (start at right most end of string to place
	 * least significant digit). Keep shifting until conversion value goes
	 * to zero (all digits were converted)
	 */
	if (base == HEX) {
		while (pt >= str) {
			if ((digit = (val & 0xf)) < 10)
				*pt-- = '0' + (char)digit;
			else
				*pt-- = 'a' + (char)(digit - 10);
			if ((val = (val >> 4)) == (u_quad_t)0)
				break;
		}
	} else {
		while (pt >= str) {
			*pt-- = '0' + (char)(val & 0x7);
			if ((val = (val >> 3)) == (u_quad_t)0)
				break;
		}
	}

	/*
	 * pad with leading ascii ZEROS. We return -1 if we ran out of space.
	 */
	while (pt >= str)
		*pt-- = '0';
	if (val != (u_quad_t)0)
		return(-1);
	return(0);
}

/* ======================= cache.c ======================= */

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
 * routines that control user, group, uid and gid caches (for the archive
 * member print routine).
 * IMPORTANT:
 * these routines cache BOTH hits and misses, a major performance improvement
 */

static	int pwopn = 0;		/* is password file open */
static	int gropn = 0;		/* is group file open */
static UIDC **uidtb = NULL;	/* uid to name cache */
static GIDC **gidtb = NULL;	/* gid to name cache */
static UIDC **usrtb = NULL;	/* user name to uid cache */
static GIDC **grptb = NULL;	/* group name to gid cache */

/*
 * uidtb_start
 *	creates an empty uidtb
 * Return:
 *	0 if ok, -1 otherwise
 */

int
uidtb_start(void)
{
	static int fail = 0;

	if (uidtb != NULL)
		return(0);
	if (fail)
		return(-1);
	if ((uidtb = (UIDC **)calloc(UID_SZ, sizeof(UIDC *))) == NULL) {
		++fail;
		paxwarn(1, "Unable to allocate memory for user id cache table");
		return(-1);
	}
	return(0);
}

/*
 * gidtb_start
 *	creates an empty gidtb
 * Return:
 *	0 if ok, -1 otherwise
 */

int
gidtb_start(void)
{
	static int fail = 0;

	if (gidtb != NULL)
		return(0);
	if (fail)
		return(-1);
	if ((gidtb = (GIDC **)calloc(GID_SZ, sizeof(GIDC *))) == NULL) {
		++fail;
		paxwarn(1, "Unable to allocate memory for group id cache table");
		return(-1);
	}
	return(0);
}

/*
 * usrtb_start
 *	creates an empty usrtb
 * Return:
 *	0 if ok, -1 otherwise
 */

int
usrtb_start(void)
{
	static int fail = 0;

	if (usrtb != NULL)
		return(0);
	if (fail)
		return(-1);
	if ((usrtb = (UIDC **)calloc(UNM_SZ, sizeof(UIDC *))) == NULL) {
		++fail;
		paxwarn(1, "Unable to allocate memory for user name cache table");
		return(-1);
	}
	return(0);
}

/*
 * grptb_start
 *	creates an empty grptb
 * Return:
 *	0 if ok, -1 otherwise
 */

int
grptb_start(void)
{
	static int fail = 0;

	if (grptb != NULL)
		return(0);
	if (fail)
		return(-1);
	if ((grptb = (GIDC **)calloc(GNM_SZ, sizeof(GIDC *))) == NULL) {
		++fail;
		paxwarn(1,"Unable to allocate memory for group name cache table");
		return(-1);
	}
	return(0);
}

/*
 * name_uid()
 *	caches the name (if any) for the uid. If frc set, we always return the
 *	the stored name (if valid or invalid match). We use a simple hash table.
 * Return
 *	Pointer to stored name (or an empty string).
 */

const char *
name_uid(uid_t uid, int frc)
{
	struct passwd *pw;
	UIDC *ptr;

	if ((uidtb == NULL) && (uidtb_start() < 0))
		return("");

	/*
	 * see if we have this uid cached
	 */
	ptr = uidtb[uid % UID_SZ];
	if ((ptr != NULL) && (ptr->valid > 0) && (ptr->uid == uid)) {
		/*
		 * have an entry for this uid
		 */
		if (frc || (ptr->valid == VALID))
			return(ptr->name);
		return("");
	}

	/*
	 * No entry for this uid, we will add it
	 */
	if (!pwopn) {
		setpassent(1);
		++pwopn;
	}
	if (ptr == NULL)
		ptr = uidtb[uid % UID_SZ] = (UIDC *)malloc(sizeof(UIDC));

	if ((pw = getpwuid(uid)) == NULL) {
		/*
		 * no match for this uid in the local password file
		 * a string that is the uid in numeric format
		 */
		if (ptr == NULL)
			return("");
		ptr->uid = uid;
		ptr->valid = INVALID;
		(void)snprintf(ptr->name, sizeof(ptr->name), "%lu",
			       (unsigned long)uid);
		if (frc == 0)
			return("");
	} else {
		/*
		 * there is an entry for this uid in the password file
		 */
		if (ptr == NULL)
			return(pw->pw_name);
		ptr->uid = uid;
		(void)strncpy(ptr->name, pw->pw_name, UNMLEN - 1);
		ptr->name[UNMLEN-1] = '\0';
		ptr->valid = VALID;
	}
	return(ptr->name);
}

/*
 * name_gid()
 *	caches the name (if any) for the gid. If frc set, we always return the
 *	the stored name (if valid or invalid match). We use a simple hash table.
 * Return
 *	Pointer to stored name (or an empty string).
 */

const char *
name_gid(gid_t gid, int frc)
{
	struct group *gr;
	GIDC *ptr;

	if ((gidtb == NULL) && (gidtb_start() < 0))
		return("");

	/*
	 * see if we have this gid cached
	 */
	ptr = gidtb[gid % GID_SZ];
	if ((ptr != NULL) && (ptr->valid > 0) && (ptr->gid == gid)) {
		/*
		 * have an entry for this gid
		 */
		if (frc || (ptr->valid == VALID))
			return(ptr->name);
		return("");
	}

	/*
	 * No entry for this gid, we will add it
	 */
	if (!gropn) {
		setgroupent(1);
		++gropn;
	}
	if (ptr == NULL)
		ptr = gidtb[gid % GID_SZ] = (GIDC *)malloc(sizeof(GIDC));

	if ((gr = getgrgid(gid)) == NULL) {
		/*
		 * no match for this gid in the local group file, put in
		 * a string that is the gid in numeric format
		 */
		if (ptr == NULL)
			return("");
		ptr->gid = gid;
		ptr->valid = INVALID;
		(void)snprintf(ptr->name, sizeof(ptr->name), "%lu",
			       (unsigned long)gid);
		if (frc == 0)
			return("");
	} else {
		/*
		 * there is an entry for this group in the group file
		 */
		if (ptr == NULL)
			return(gr->gr_name);
		ptr->gid = gid;
		(void)strncpy(ptr->name, gr->gr_name, GNMLEN - 1);
		ptr->name[GNMLEN-1] = '\0';
		ptr->valid = VALID;
	}
	return(ptr->name);
}

/*
 * uid_name()
 *	caches the uid for a given user name. We use a simple hash table.
 * Return
 *	the uid (if any) for a user name, or a -1 if no match can be found
 */

int
uid_name(char *name, uid_t *uid)
{
	struct passwd *pw;
	UIDC *ptr;
	int namelen;

	/*
	 * return -1 for mangled names
	 */
	if (((namelen = strlen(name)) == 0) || (name[0] == '\0'))
		return(-1);
	if ((usrtb == NULL) && (usrtb_start() < 0))
		return(-1);

	/*
	 * look up in hash table, if found and valid return the uid,
	 * if found and invalid, return a -1
	 */
	ptr = usrtb[st_hash(name, namelen, UNM_SZ)];
	if ((ptr != NULL) && (ptr->valid > 0) && !strcmp(name, ptr->name)) {
		if (ptr->valid == INVALID)
			return(-1);
		*uid = ptr->uid;
		return(0);
	}

	if (!pwopn) {
		setpassent(1);
		++pwopn;
	}

	if (ptr == NULL)
		ptr = usrtb[st_hash(name, namelen, UNM_SZ)] =
		  (UIDC *)malloc(sizeof(UIDC));

	/*
	 * no match, look it up, if no match store it as an invalid entry,
	 * or store the matching uid
	 */
	if (ptr == NULL) {
		if ((pw = getpwnam(name)) == NULL)
			return(-1);
		*uid = pw->pw_uid;
		return(0);
	}
	(void)strncpy(ptr->name, name, UNMLEN - 1);
	ptr->name[UNMLEN-1] = '\0';
	if ((pw = getpwnam(name)) == NULL) {
		ptr->valid = INVALID;
		return(-1);
	}
	ptr->valid = VALID;
	*uid = ptr->uid = pw->pw_uid;
	return(0);
}

/*
 * gid_name()
 *	caches the gid for a given group name. We use a simple hash table.
 * Return
 *	the gid (if any) for a group name, or a -1 if no match can be found
 */

int
gid_name(char *name, gid_t *gid)
{
	struct group *gr;
	GIDC *ptr;
	int namelen;

	/*
	 * return -1 for mangled names
	 */
	if (((namelen = strlen(name)) == 0) || (name[0] == '\0'))
		return(-1);
	if ((grptb == NULL) && (grptb_start() < 0))
		return(-1);

	/*
	 * look up in hash table, if found and valid return the uid,
	 * if found and invalid, return a -1
	 */
	ptr = grptb[st_hash(name, namelen, GID_SZ)];
	if ((ptr != NULL) && (ptr->valid > 0) && !strcmp(name, ptr->name)) {
		if (ptr->valid == INVALID)
			return(-1);
		*gid = ptr->gid;
		return(0);
	}

	if (!gropn) {
		setgroupent(1);
		++gropn;
	}
	if (ptr == NULL)
		ptr = grptb[st_hash(name, namelen, GID_SZ)] =
		  (GIDC *)malloc(sizeof(GIDC));

	/*
	 * no match, look it up, if no match store it as an invalid entry,
	 * or store the matching gid
	 */
	if (ptr == NULL) {
		if ((gr = getgrnam(name)) == NULL)
			return(-1);
		*gid = gr->gr_gid;
		return(0);
	}

	(void)strncpy(ptr->name, name, GNMLEN - 1);
	ptr->name[GNMLEN-1] = '\0';
	if ((gr = getgrnam(name)) == NULL) {
		ptr->valid = INVALID;
		return(-1);
	}
	ptr->valid = VALID;
	*gid = ptr->gid = gr->gr_gid;
	return(0);
}
