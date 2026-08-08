module;

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
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

export module pbsd.bin.pax.b0108;

export namespace pbsd::bin_pax::b0108 {

typedef unsigned long u_long;

#ifndef D_MD_ORDER
#define D_MD_ORDER ((nl_item)0x2000c)
#endif

static void
setpassent(int stayopen)
{
	(void)stayopen;
}

static void
setgroupent(int stayopen)
{
	(void)stayopen;
}

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

int iflag = 0;
int vflag = 0;
int vfpart = 0;
int exit_val = 0;
const char *argv0 = "pax";
FILE *listf = nullptr;

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

int tty_init(void);
void tty_prnt(const char *, ...);
int tty_read(char *, int);
void paxwarn(int, const char *, ...);
void syswarn(int, int, const char *, ...);
void ls_list(ARCHD *, time_t, FILE *);
void ls_tty(ARCHD *);
int l_strncpy(char *, const char *, int);
u_long asc_ul(char *, int, int);
int ul_asc(u_long, char *, int, int);
u_quad_t asc_uqd(char *, int, int);
int uqd_asc(u_quad_t, char *, int, int);
int uidtb_start(void);
int gidtb_start(void);
int usrtb_start(void);
int grptb_start(void);
const char *name_uid(uid_t, int);
const char *name_gid(gid_t, int);
int uid_name(char *, uid_t *);
int gid_name(char *, gid_t *);

#define DEVTTY	  "/dev/tty"      /* device for interactive i/o */
static FILE *ttyoutf = NULL;		/* output pointing at control tty */
static FILE *ttyinf = NULL;		/* input pointing at control tty */

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

int
tty_read(char *str, int len)
{
	char *pt;

	if ((--len <= 0) || (ttyinf == NULL) || (fgets(str,len,ttyinf) == NULL))
		return(-1);
	*(str + len) = '\0';

	if ((pt = strchr(str, '\n')) != NULL)
		*pt = '\0';
	return(0);
}

void
paxwarn(int set, const char *fmt, ...)
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

void
syswarn(int set, int errnum, const char *fmt, ...)
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

#define MODELEN 20
#define DATELEN 64
#define SIXMONTHS	 ((365 / 2) * 86400)
#define CURFRMTM	"%b %e %H:%M"
#define OLDFRMTM	"%b %e  %Y"
#define CURFRMTD	"%e %b %H:%M"
#define OLDFRMTD	"%e %b  %Y"

static int d_first = -1;

void
ls_list(ARCHD *arcn, time_t now, FILE *fp)
{
	struct stat *sbp;
	char f_mode[MODELEN];
	char f_date[DATELEN];
	const char *timefrmt;

	if (!vflag) {
		(void)fprintf(fp, "%s\n", arcn->name);
		(void)fflush(fp);
		return;
	}

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');
	sbp = &(arcn->sb);
	strmode(sbp->st_mode, f_mode);

	if ((sbp->st_mtime + SIXMONTHS) <= now)
		timefrmt = d_first ? OLDFRMTD : OLDFRMTM;
	else
		timefrmt = d_first ? CURFRMTD : CURFRMTM;

	if (strftime(f_date,DATELEN,timefrmt,localtime(&(sbp->st_mtime))) == 0)
		f_date[0] = '\0';
	(void)fprintf(fp, "%s%2ju %-12s %-12s ", f_mode,
		(uintmax_t)sbp->st_nlink,
		name_uid(sbp->st_uid, 1), name_gid(sbp->st_gid, 1));

	if ((arcn->type == PAX_CHR) || (arcn->type == PAX_BLK))
		(void)fprintf(fp, "%4lu,%4lu ", (unsigned long)MAJOR(sbp->st_rdev),
		    (unsigned long)MINOR(sbp->st_rdev));
	else {
		(void)fprintf(fp, "%9ju ", (uintmax_t)sbp->st_size);
	}

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

void
ls_tty(ARCHD *arcn)
{
	char f_date[DATELEN];
	char f_mode[MODELEN];
	const char *timefrmt;

	if (d_first >= 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');

	if ((arcn->sb.st_mtime + SIXMONTHS) <= time(NULL))
		timefrmt = d_first ? OLDFRMTD : OLDFRMTM;
	else
		timefrmt = d_first ? CURFRMTD : CURFRMTM;

	if (strftime(f_date, DATELEN, timefrmt,
	    localtime(&(arcn->sb.st_mtime))) == 0)
		f_date[0] = '\0';
	strmode(arcn->sb.st_mode, f_mode);
	tty_prnt("%s%s %s\n", f_mode, f_date, arcn->name);
	return;
}

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

u_long
asc_ul(char *str, int len, int base)
{
	char *stop;
	u_long tval = 0;

	stop = str + len;

	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;

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

int
ul_asc(u_long val, char *str, int len, int base)
{
	char *pt;
	u_long digit;

	pt = str + len - 1;

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

	while (pt >= str)
		*pt-- = '0';
	if (val != (u_long)0)
		return(-1);
	return(0);
}

u_quad_t
asc_uqd(char *str, int len, int base)
{
	char *stop;
	u_quad_t tval = 0;

	stop = str + len;

	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;

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

int
uqd_asc(u_quad_t val, char *str, int len, int base)
{
	char *pt;
	u_quad_t digit;

	pt = str + len - 1;

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

static	int pwopn = 0;
static	int gropn = 0;
static UIDC **uidtb = NULL;
static GIDC **gidtb = NULL;
static UIDC **usrtb = NULL;
static GIDC **grptb = NULL;

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

const char *
name_uid(uid_t uid, int frc)
{
	struct passwd *pw;
	UIDC *ptr;

	if ((uidtb == NULL) && (uidtb_start() < 0))
		return("");

	ptr = uidtb[uid % UID_SZ];
	if ((ptr != NULL) && (ptr->valid > 0) && (ptr->uid == uid)) {
		if (frc || (ptr->valid == VALID))
			return(ptr->name);
		return("");
	}

	if (!pwopn) {
		setpassent(1);
		++pwopn;
	}
	if (ptr == NULL)
		ptr = uidtb[uid % UID_SZ] = (UIDC *)malloc(sizeof(UIDC));

	if ((pw = getpwuid(uid)) == NULL) {
		if (ptr == NULL)
			return("");
		ptr->uid = uid;
		ptr->valid = INVALID;
		(void)snprintf(ptr->name, sizeof(ptr->name), "%lu",
			       (unsigned long)uid);
		if (frc == 0)
			return("");
	} else {
		if (ptr == NULL)
			return(pw->pw_name);
		ptr->uid = uid;
		(void)strncpy(ptr->name, pw->pw_name, UNMLEN - 1);
		ptr->name[UNMLEN-1] = '\0';
		ptr->valid = VALID;
	}
	return(ptr->name);
}

const char *
name_gid(gid_t gid, int frc)
{
	struct group *gr;
	GIDC *ptr;

	if ((gidtb == NULL) && (gidtb_start() < 0))
		return("");

	ptr = gidtb[gid % GID_SZ];
	if ((ptr != NULL) && (ptr->valid > 0) && (ptr->gid == gid)) {
		if (frc || (ptr->valid == VALID))
			return(ptr->name);
		return("");
	}

	if (!gropn) {
		setgroupent(1);
		++gropn;
	}
	if (ptr == NULL)
		ptr = gidtb[gid % GID_SZ] = (GIDC *)malloc(sizeof(GIDC));

	if ((gr = getgrgid(gid)) == NULL) {
		if (ptr == NULL)
			return("");
		ptr->gid = gid;
		ptr->valid = INVALID;
		(void)snprintf(ptr->name, sizeof(ptr->name), "%lu",
			       (unsigned long)gid);
		if (frc == 0)
			return("");
	} else {
		if (ptr == NULL)
			return(gr->gr_name);
		ptr->gid = gid;
		(void)strncpy(ptr->name, gr->gr_name, GNMLEN - 1);
		ptr->name[GNMLEN-1] = '\0';
		ptr->valid = VALID;
	}
	return(ptr->name);
}

int
uid_name(char *name, uid_t *uid)
{
	struct passwd *pw;
	UIDC *ptr;
	int namelen;

	if (((namelen = strlen(name)) == 0) || (name[0] == '\0'))
		return(-1);
	if ((usrtb == NULL) && (usrtb_start() < 0))
		return(-1);

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

int
gid_name(char *name, gid_t *gid)
{
	struct group *gr;
	GIDC *ptr;
	int namelen;

	if (((namelen = strlen(name)) == 0) || (name[0] == '\0'))
		return(-1);
	if ((grptb == NULL) && (grptb_start() < 0))
		return(-1);

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

} // namespace pbsd::bin_pax::b0108
