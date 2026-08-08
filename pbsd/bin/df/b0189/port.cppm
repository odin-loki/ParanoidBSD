module;

#include <sys/param.h>
#include <sys/stat.h>

#include <cinttypes>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <bsd/string.h>
#include <getopt.h>
#include <sysexits.h>

#ifndef MNAMELEN
#define MNAMELEN 1024
#endif
#ifndef MFSNAMELEN
#define MFSNAMELEN 16
#endif
#ifndef DEV_BSIZE
#define DEV_BSIZE 512
#endif
#ifndef MNT_WAIT
#define MNT_WAIT 0x0001
#endif
#ifndef MNT_NOWAIT
#define MNT_NOWAIT 0x0002
#endif
#ifndef MNT_IGNORE
#define MNT_IGNORE 0x800000ULL
#endif

export module pbsd.bin.df.b0189;

export namespace pbsd::bin_df::b0189 {

struct statfs {
	uint64_t f_flags;
	int64_t f_bsize;
	int64_t f_blocks;
	int64_t f_bfree;
	int64_t f_bavail;
	int64_t f_files;
	int64_t f_ffree;
	char f_fstypename[MFSNAMELEN];
	char f_mntonname[MNAMELEN];
	char f_mntfromname[MNAMELEN];
};

struct xvfsconf {
	uint32_t vfc_version;
	char vfc_name[MFSNAMELEN];
	int vfc_refcount;
	int vfc_flags;
};

#ifndef VFCF_NETWORK
#define VFCF_NETWORK 0x00000010
#endif

#ifndef HN_B
#define HN_B			0x01
#define HN_NOSPACE		0x02
#define HN_DECIMAL		0x04
#define HN_DIVISOR_1000		0x08
#define HN_AUTOSCALE		0x10
#endif

}

#define UNITS_SI	1
#define UNITS_2		2

struct maxwidths {
	int	mntfrom;
	int	fstype;
	int	total;
	int	used;
	int	avail;
	int	iused;
	int	ifree;
};

inline int aflag = 0;
inline int cflag = 0;
inline int hflag = 0;
inline int iflag = 0;
inline int kflag = 0;
inline int lflag = 0;
inline int nflag = 0;
inline int Tflag = 0;
inline int thousands = 0;
inline int skipvfs_l = 0;
inline int skipvfs_t = 0;
inline const char **vfslist_l = nullptr;
inline const char **vfslist_t = nullptr;

static const struct option long_options[] =
{
	{ "si", no_argument, NULL, 'H' },
	{ NULL, no_argument, NULL, 0 },
};

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1980, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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

int
imax(int a, int b)
{
	return (a > b ? a : b);
}

void addstat(struct statfs *, struct statfs *);
char *getmntpt(const char *);
const char **makevfslist(char *fslist, int *skip);
int checkvfsname(const char *vfsname, const char **vfslist, int skip);
int checkvfsselected(char *);
int int64width(int64_t);
char *makenetvfslist(void);
void prthuman(const struct statfs *, int64_t);
void prthumanval(const char *, int64_t);
void prthumanvalinode(const char *, int64_t);
intmax_t fsbtoblk(int64_t, uint64_t, u_long);
void prtstat(struct statfs *, struct maxwidths *);
size_t regetmntinfo(struct statfs **, long);
void update_maxwidths(struct maxwidths *, const struct statfs *);
void usage(void);

int
main(int argc, char *argv[])
{
	struct stat stbuf;
	struct statfs statfsbuf, totalbuf;
	struct maxwidths maxwidths;
	struct statfs *mntbuf;
	char *mntpt;
	int i, mntsize;
	int ch, rv;

	(void)setlocale(LC_ALL, "");
	memset(&maxwidths, 0, sizeof(maxwidths));
	memset(&totalbuf, 0, sizeof(totalbuf));
	totalbuf.f_bsize = DEV_BSIZE;
	strlcpy(totalbuf.f_mntfromname, "total", MNAMELEN);

	argc = xo_parse_args(argc, argv);
	if (argc < 0)
		exit(1);

	while ((ch = getopt_long(argc, argv, "+abcgHhiklmnPt:T,", long_options,
	    NULL)) != -1)
		switch (ch) {
		case 'a':
			aflag = 1;
			break;
		case 'b':
				/* FALLTHROUGH */
		case 'P':
			if (kflag)
				break;
			setenv("BLOCKSIZE", "512", 1);
			hflag = 0;
			break;
		case 'c':
			cflag = 1;
			break;
		case 'g':
			setenv("BLOCKSIZE", "1g", 1);
			hflag = 0;
			break;
		case 'H':
			hflag = UNITS_SI;
			break;
		case 'h':
			hflag = UNITS_2;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'k':
			kflag++;
			setenv("BLOCKSIZE", "1024", 1);
			hflag = 0;
			break;
		case 'l':
			if (lflag)
				break;
			vfslist_l = makevfslist(makenetvfslist(), &skipvfs_l);
			lflag = 1;
			break;
		case 'm':
			setenv("BLOCKSIZE", "1m", 1);
			hflag = 0;
			break;
		case 'n':
			nflag = 1;
			break;
		case 't':
			if (vfslist_t != NULL)
				xo_errx(1, "only one -t option may be specified");
			vfslist_t = makevfslist(optarg, &skipvfs_t);
			break;
		case 'T':
			Tflag = 1;
			break;
		case ',':
			thousands = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	rv = EXIT_SUCCESS;
	if (!*argv) {
		mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
		mntsize = regetmntinfo(&mntbuf, mntsize);
	} else {
		mntbuf = (struct statfs *)malloc(argc * sizeof(*mntbuf));
		if (mntbuf == NULL)
			xo_err(1, "malloc()");
		mntsize = 0;
	}

	xo_open_container("storage-system-information");
	xo_open_list("filesystem");

	for (; *argv; argv++) {
		if (stat(*argv, &stbuf) < 0) {
			if ((mntpt = getmntpt(*argv)) == NULL) {
				xo_warn("%s", *argv);
				rv = EXIT_FAILURE;
				continue;
			}
		} else if (S_ISCHR(stbuf.st_mode)) {
			mntpt = getmntpt(*argv);
			if (mntpt == NULL) {
				xo_warnx("%s: not mounted", *argv);
				rv = EXIT_FAILURE;
				continue;
			}
		} else {
			mntpt = *argv;
		}

		if (statfs(mntpt, &statfsbuf) < 0) {
			xo_warn("%s", mntpt);
			rv = EXIT_FAILURE;
			continue;
		}

		if (checkvfsselected(statfsbuf.f_fstypename) != 0) {
			rv = EXIT_FAILURE;
			continue;
		}

		statfsbuf.f_flags &= ~MNT_IGNORE;

		mntbuf[mntsize++] = statfsbuf;
	}

	memset(&maxwidths, 0, sizeof(maxwidths));
	for (i = 0; i < mntsize; i++) {
		if (aflag || (mntbuf[i].f_flags & MNT_IGNORE) == 0) {
			update_maxwidths(&maxwidths, &mntbuf[i]);
			if (cflag)
				addstat(&totalbuf, &mntbuf[i]);
		}
	}
	for (i = 0; i < mntsize; i++)
		if (aflag || (mntbuf[i].f_flags & MNT_IGNORE) == 0)
			prtstat(&mntbuf[i], &maxwidths);

	xo_close_list("filesystem");

	if (cflag)
		prtstat(&totalbuf, &maxwidths);

	xo_close_container("storage-system-information");
	if (xo_finish() < 0)
		rv = EXIT_FAILURE;
	exit(rv);
}

char *
getmntpt(const char *name)
{
	size_t mntsize, i;
	struct statfs *mntbuf;

	mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
	for (i = 0; i < mntsize; i++) {
		if (!strcmp(mntbuf[i].f_mntfromname, name))
			return (mntbuf[i].f_mntonname);
	}
	return (NULL);
}

const char **
makevfslist(char *fslist, int *skip)
{
	const char **av;
	int i;
	char *nextcp;

	if (fslist == NULL)
		return (NULL);
	*skip = 0;
	if (fslist[0] == 'n' && fslist[1] == 'o') {
		fslist += 2;
		*skip = 1;
	}
	for (i = 0, nextcp = fslist; *nextcp; nextcp++)
		if (*nextcp == ',')
			i++;
	if ((av = (const char **)malloc((size_t)(i + 2) * sizeof(char *))) == NULL) {
		xo_warnx("malloc failed");
		return (NULL);
	}
	nextcp = fslist;
	i = 0;
	av[i++] = nextcp;
	while ((nextcp = strchr(nextcp, ',')) != NULL) {
		*nextcp++ = '\0';
		av[i++] = nextcp;
	}
	av[i++] = NULL;
	return (av);
}

int
checkvfsname(const char *vfsname, const char **vfslist, int skip)
{

	if (vfslist == NULL)
		return (0);
	while (*vfslist != NULL) {
		if (strcmp(vfsname, *vfslist) == 0)
			return (skip);
		++vfslist;
	}
	return (!skip);
}

int
checkvfsselected(char *fstypename)
{
	int result;

	if (vfslist_t) {
		result = checkvfsname(fstypename, vfslist_t, skipvfs_t);
		if (vfslist_l) {
			if (checkvfsname(fstypename, vfslist_l, skipvfs_l) == skipvfs_t)
				result = skipvfs_t;
		}
	} else {
		result = checkvfsname(fstypename, vfslist_l, skipvfs_l);
	}
	return (result);
}

size_t
regetmntinfo(struct statfs **mntbufp, long mntsize)
{
	int error, i, j;
	struct statfs *mntbuf;

	if (vfslist_l == NULL && vfslist_t == NULL)
		return (nflag ? mntsize : getmntinfo(mntbufp, MNT_WAIT));

	mntbuf = *mntbufp;
	for (j = 0, i = 0; i < mntsize; i++) {
		if (checkvfsselected(mntbuf[i].f_fstypename) != 0)
			continue;
		error = statfs(mntbuf[i].f_mntonname, &mntbuf[j]);
		if (nflag || error < 0)
			if (i != j) {
				if (error < 0)
					xo_warnx("%s stats possibly stale",
					    mntbuf[i].f_mntonname);
				mntbuf[j] = mntbuf[i];
			}
		j++;
	}
	return (j);
}

void
prthuman(const struct statfs *sfsp, int64_t used)
{

	prthumanval("  {:blocks/%6s}", sfsp->f_blocks * sfsp->f_bsize);
	prthumanval("  {:used/%6s}", used * sfsp->f_bsize);
	prthumanval("  {:available/%6s}", sfsp->f_bavail * sfsp->f_bsize);
}

void
prthumanval(const char *fmt, int64_t bytes)
{
	char buf[6];
	int flags;

	flags = HN_B | HN_NOSPACE | HN_DECIMAL;
	if (hflag == UNITS_SI)
		flags |= HN_DIVISOR_1000;

	humanize_number(buf, sizeof(buf) - (bytes < 0 ? 0 : 1),
	    bytes, "", HN_AUTOSCALE, flags);

	xo_attr("value", "%lld", (long long) bytes);
	xo_emit(fmt, buf);
}

void
prthumanvalinode(const char *fmt, int64_t bytes)
{
	char buf[6];
	int flags;

	flags = HN_NOSPACE | HN_DECIMAL | HN_DIVISOR_1000;

	humanize_number(buf, sizeof(buf) - (bytes < 0 ? 0 : 1),
	    bytes, "", HN_AUTOSCALE, flags);

	xo_attr("value", "%lld", (long long) bytes);
	xo_emit(fmt, buf);
}

intmax_t
fsbtoblk(int64_t num, uint64_t fsbs, u_long bs)
{
	return (num * (intmax_t) fsbs / (int64_t) bs);
}

void
prtstat(struct statfs *sfsp, struct maxwidths *mwp)
{
	static long blocksize;
	static int headerlen, timesthrough = 0;
	static const char *header;
	int64_t used, availblks, inodes;
	const char *format;

	if (++timesthrough == 1) {
		mwp->mntfrom = imax(mwp->mntfrom, (int)strlen("Filesystem"));
		mwp->fstype = imax(mwp->fstype, (int)strlen("Type"));
		if (thousands) {
		    mwp->total += (mwp->total - 1) / 3;
		    mwp->used  += (mwp->used - 1) / 3;
		    mwp->avail += (mwp->avail - 1) / 3;
		    mwp->iused += (mwp->iused - 1) / 3;
		    mwp->ifree += (mwp->ifree - 1) / 3;
		}
		if (hflag) {
			header = "   Size";
			mwp->total = mwp->used = mwp->avail =
			    (int)strlen(header);
		} else {
			header = getbsize(&headerlen, &blocksize);
			mwp->total = imax(mwp->total, headerlen);
		}
		mwp->used = imax(mwp->used, (int)strlen("Used"));
		mwp->avail = imax(mwp->avail, (int)strlen("Avail"));

		xo_emit("{T:/%-*s}", mwp->mntfrom, "Filesystem");
		if (Tflag)
			xo_emit("  {T:/%-*s}", mwp->fstype, "Type");
		xo_emit(" {T:/%*s} {T:/%*s} {T:/%*s} {T:Capacity}",
			mwp->total, header,
			mwp->used, "Used", mwp->avail, "Avail");
		if (iflag) {
			mwp->iused = imax(hflag ? 0 : mwp->iused,
			    (int)strlen("  iused"));
			mwp->ifree = imax(hflag ? 0 : mwp->ifree,
			    (int)strlen("ifree"));
			xo_emit(" {T:/%*s} {T:/%*s} {T:\%iused}",
			    mwp->iused - 2, "iused", mwp->ifree, "ifree");
		}
		xo_emit("  {T:Mounted on}\n");
	}

	xo_open_instance("filesystem");
	if (sfsp->f_bsize == 0) {
		xo_warnx ("File system %s does not have a block size, assuming 512.",
		    sfsp->f_mntonname);
		sfsp->f_bsize = 512;
	}
	xo_emit("{tk:name/%-*s}", mwp->mntfrom, sfsp->f_mntfromname);
	if (Tflag)
		xo_emit("  {:type/%-*s}", mwp->fstype, sfsp->f_fstypename);
	used = sfsp->f_blocks - sfsp->f_bfree;
	availblks = sfsp->f_bavail + used;
	if (hflag) {
		prthuman(sfsp, used);
	} else {
		if (thousands)
		    format = " {t:total-blocks/%*j'd} {t:used-blocks/%*j'd} "
			"{t:available-blocks/%*j'd}";
		else
		    format = " {t:total-blocks/%*jd} {t:used-blocks/%*jd} "
			"{t:available-blocks/%*jd}";
		xo_emit(format,
		    mwp->total, fsbtoblk(sfsp->f_blocks,
		    sfsp->f_bsize, blocksize),
		    mwp->used, fsbtoblk(used, sfsp->f_bsize, blocksize),
		    mwp->avail, fsbtoblk(sfsp->f_bavail,
		    sfsp->f_bsize, blocksize));
	}
	xo_emit(" {:used-percent/%5.0f}{U:%%}",
	    availblks == 0 ? 100.0 : (double)used / (double)availblks * 100.0);
	if (iflag) {
		inodes = sfsp->f_files;
		used = inodes - sfsp->f_ffree;
		if (hflag) {
			xo_emit("  ");
			prthumanvalinode(" {:inodes-used/%5s}", used);
			prthumanvalinode(" {:inodes-free/%5s}", sfsp->f_ffree);
		} else {
			if (thousands)
			    format = " {:inodes-used/%*j'd} {:inodes-free/%*j'd}";
			else
			    format = " {:inodes-used/%*jd} {:inodes-free/%*jd}";
			xo_emit(format, mwp->iused, (intmax_t)used,
			    mwp->ifree, (intmax_t)sfsp->f_ffree);
		}
		if (inodes == 0)
			xo_emit(" {:inodes-used-percent/    -}{U:} ");
		else {
			xo_emit(" {:inodes-used-percent/%4.0f}{U:%%} ",
				(double)used / (double)inodes * 100.0);
		}
	} else
		xo_emit("  ");
	if (strncmp(sfsp->f_mntfromname, "total", MNAMELEN) != 0)
		xo_emit("  {:mounted-on}", sfsp->f_mntonname);
	xo_emit("\n");
	xo_close_instance("filesystem");
}

void
addstat(struct statfs *totalfsp, struct statfs *statfsp)
{
	uint64_t bsize;

	bsize = statfsp->f_bsize / totalfsp->f_bsize;
	totalfsp->f_blocks += statfsp->f_blocks * bsize;
	totalfsp->f_bfree += statfsp->f_bfree * bsize;
	totalfsp->f_bavail += statfsp->f_bavail * bsize;
	totalfsp->f_files += statfsp->f_files;
	totalfsp->f_ffree += statfsp->f_ffree;
}

void
update_maxwidths(struct maxwidths *mwp, const struct statfs *sfsp)
{
	static long blocksize = 0;
	int dummy;

	if (blocksize == 0)
		getbsize(&dummy, &blocksize);

	mwp->mntfrom = imax(mwp->mntfrom, (int)strlen(sfsp->f_mntfromname));
	mwp->fstype = imax(mwp->fstype, (int)strlen(sfsp->f_fstypename));
	mwp->total = imax(mwp->total, int64width(
	    fsbtoblk((int64_t)sfsp->f_blocks, sfsp->f_bsize, blocksize)));
	mwp->used = imax(mwp->used,
	    int64width(fsbtoblk((int64_t)sfsp->f_blocks -
	    (int64_t)sfsp->f_bfree, sfsp->f_bsize, blocksize)));
	mwp->avail = imax(mwp->avail, int64width(fsbtoblk(sfsp->f_bavail,
	    sfsp->f_bsize, blocksize)));
	mwp->iused = imax(mwp->iused, int64width((int64_t)sfsp->f_files -
	    sfsp->f_ffree));
	mwp->ifree = imax(mwp->ifree, int64width(sfsp->f_ffree));
}

int
int64width(int64_t val)
{
	int len;

	len = 0;
	if (val <= 0) {
		val = -val;
		len++;
	}
	while (val > 0) {
		len++;
		val /= 10;
	}

	return (len);
}

void
usage(void)
{

	xo_error(
"usage: df [-b | -g | -H | -h | -k | -m | -P] [-acilnT] [-t type] [-,]\n"
"          [file | filesystem ...]\n");
	exit(EX_USAGE);
}

char *
makenetvfslist(void)
{
	char *str, *strptr, **listptr;
	struct xvfsconf *xvfsp, *keep_xvfsp;
	size_t buflen;
	int cnt, i, maxvfsconf;

	if (sysctlbyname("vfs.conflist", NULL, &buflen, NULL, 0) < 0) {
		xo_warn("sysctl(vfs.conflist)");
		return (NULL);
	}
	xvfsp = (struct xvfsconf *)malloc(buflen);
	if (xvfsp == NULL) {
		xo_warnx("malloc failed");
		return (NULL);
	}
	keep_xvfsp = xvfsp;
	if (sysctlbyname("vfs.conflist", xvfsp, &buflen, NULL, 0) < 0) {
		xo_warn("sysctl(vfs.conflist)");
		free(keep_xvfsp);
		return (NULL);
	}
	maxvfsconf = buflen / sizeof(struct xvfsconf);

	if ((listptr = (char **)malloc(sizeof(char*) * maxvfsconf)) == NULL) {
		xo_warnx("malloc failed");
		free(keep_xvfsp);
		return (NULL);
	}

	for (cnt = 0, i = 0; i < maxvfsconf; i++) {
		if (xvfsp->vfc_flags & VFCF_NETWORK) {
			listptr[cnt++] = strdup(xvfsp->vfc_name);
			if (listptr[cnt-1] == NULL) {
				xo_warnx("malloc failed");
				free(listptr);
				free(keep_xvfsp);
				return (NULL);
			}
		}
		xvfsp++;
	}

	if (cnt == 0 ||
	    (str = (char *)malloc(sizeof(char) * (32 * cnt + cnt + 2))) == NULL) {
		if (cnt > 0)
			xo_warnx("malloc failed");
		free(listptr);
		free(keep_xvfsp);
		return (NULL);
	}

	*str = 'n'; *(str + 1) = 'o';
	for (i = 0, strptr = str + 2; i < cnt; i++, strptr++) {
		strlcpy(strptr, listptr[i], 32);
		strptr += strlen(listptr[i]);
		*strptr = ',';
		free(listptr[i]);
	}
	*(--strptr) = '\0';

	free(keep_xvfsp);
	free(listptr);
	return (str);
}

extern "C" {
int getmntinfo(struct statfs **mntbufp, int mode);
int statfs(const char *path, struct statfs *buf);
int sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen);
char *getbsize(int *headerlenp, long *blocksizep);
int humanize_number(char *buf, size_t len, int64_t bytes,
    const char *suffix, int scale, int flags);
int xo_parse_args(int argc, char *argv[]);
void xo_err(int eval, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void xo_errx(int eval, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void xo_warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void xo_warnx(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void xo_open_container(const char *name);
void xo_open_list(const char *name);
void xo_close_list(const char *name);
void xo_close_container(const char *name);
int xo_finish(void);
void xo_emit(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void xo_attr(const char *name, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
void xo_open_instance(const char *name);
void xo_close_instance(const char *name);
void xo_error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
} /* namespace pbsd::bin_df::b0189 */
