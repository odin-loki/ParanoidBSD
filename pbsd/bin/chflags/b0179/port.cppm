module;

#ifndef __dead2
#define __dead2	__attribute__((__noreturn__))
#endif

#ifndef __unused
#define __unused	__attribute__((__unused__))
#endif

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <sys/types.h>

#if defined(__linux__)
#define _SYS_STAT_H
struct pbsd_filestat {
	unsigned long st_flags;
};
#define stat pbsd_filestat
#endif

#include <sys/stat.h>

#ifndef UF_NODUMP
#define UF_NODUMP	0x00000001
#define UF_IMMUTABLE	0x00000002
#define UF_APPEND	0x00000004
#define UF_OPAQUE	0x00000008
#define UF_NOUNLINK	0x00000010
#define UF_COMPRESSED	0x00000020
#define UF_READONLY	0x00008000
#define UF_HIDDEN	0x00080000
#define UF_ARCHIVE	0x00020000
#define UF_OFFLINE	0x00100000
#define UF_REPARSE	0x00040000
#define UF_SPARSE	0x00010000
#define UF_SYSTEM	0x00000080
#define SF_APPEND	0x00800000
#define SF_ARCHIVED	0x00800000
#define SF_IMMUTABLE	0x01000000
#define SF_NOUNLINK	0x02000000
#define SF_SNAPSHOT	0x04000000
#endif

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <signal.h>
#include <unistd.h>

#if defined(__linux__)
int chflagsat(int, const char *, unsigned long, int);
#endif

export module pbsd.bin.chflags.b0179;

export namespace pbsd::bin_chflags::b0179 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993, 1994
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

namespace {

#define longestflaglen	12
struct mapping_entry {
	char name[longestflaglen + 1];
	char invert;
	u_long flag;
};

const mapping_entry strtofflags_mapping[] = {
	{ "nosappnd",		0, SF_APPEND	},
	{ "nosappend",		0, SF_APPEND	},
	{ "noarch",		0, SF_ARCHIVED	},
	{ "noarchived",		0, SF_ARCHIVED	},
	{ "noschg",		0, SF_IMMUTABLE	},
	{ "noschange",		0, SF_IMMUTABLE	},
	{ "nosimmutable",	0, SF_IMMUTABLE	},
	{ "nosunlnk",		0, SF_NOUNLINK	},
	{ "nosunlink",		0, SF_NOUNLINK	},
#ifdef SF_SNAPSHOT
	{ "nosnapshot",		0, SF_SNAPSHOT	},
#endif
	{ "nouappnd",		0, UF_APPEND	},
	{ "nouappend",		0, UF_APPEND	},
	{ "nouarch", 		0, UF_ARCHIVE	},
	{ "nouarchive",		0, UF_ARCHIVE	},
	{ "nohidden",		0, UF_HIDDEN	},
	{ "nouhidden",		0, UF_HIDDEN	},
	{ "nouchg",		0, UF_IMMUTABLE	},
	{ "nouchange",		0, UF_IMMUTABLE	},
	{ "nouimmutable",	0, UF_IMMUTABLE	},
	{ "nodump",		1, UF_NODUMP	},
	{ "nouunlnk",		0, UF_NOUNLINK	},
	{ "nouunlink",		0, UF_NOUNLINK	},
	{ "nooffline",		0, UF_OFFLINE	},
	{ "nouoffline",		0, UF_OFFLINE	},
	{ "noopaque",		0, UF_OPAQUE	},
	{ "nordonly",		0, UF_READONLY	},
	{ "nourdonly",		0, UF_READONLY	},
	{ "noreadonly",		0, UF_READONLY	},
	{ "noureadonly",	0, UF_READONLY	},
	{ "noreparse",		0, UF_REPARSE	},
	{ "noureparse",		0, UF_REPARSE	},
	{ "nosparse",		0, UF_SPARSE	},
	{ "nousparse",		0, UF_SPARSE	},
	{ "nosystem",		0, UF_SYSTEM	},
	{ "nousystem",		0, UF_SYSTEM	}
};
#define strtofflags_nmappings \
	(sizeof(strtofflags_mapping) / sizeof(strtofflags_mapping[0]))

int
strtofflags(char **stringp, u_long *setp, u_long *clrp)
{
	char *string, *p;
	int i;

	if (setp)
		*setp = 0;
	if (clrp)
		*clrp = 0;
	string = *stringp;
	while ((p = strsep(&string, "\t ,")) != NULL) {
		*stringp = p;
		if (*p == '\0')
			continue;
		for (i = 0; i < (int)strtofflags_nmappings; i++) {
			if (strcmp(p, strtofflags_mapping[i].name + 2) == 0) {
				if (strtofflags_mapping[i].invert) {
					if (clrp)
						*clrp |= strtofflags_mapping[i].flag;
				} else {
					if (setp)
						*setp |= strtofflags_mapping[i].flag;
				}
				break;
			} else if (strcmp(p, strtofflags_mapping[i].name) == 0) {
				if (strtofflags_mapping[i].invert) {
					if (setp)
						*setp |= strtofflags_mapping[i].flag;
				} else {
					if (clrp)
						*clrp |= strtofflags_mapping[i].flag;
				}
				break;
			}
		}
		if (i == (int)strtofflags_nmappings)
			return 1;
	}
	return 0;
}

} // namespace

volatile sig_atomic_t siginfo;

void usage(void) __dead2;

void
siginfo_handler(int sig __unused)
{

	siginfo = 1;
}

int
main(int argc, char *argv[])
{
	FTS *ftsp;
	FTSENT *p;
	u_long clear, newflags, set;
	long val;
	int Hflag, Lflag, Rflag, fflag, hflag, vflag, xflag;
	int ch, e, fts_options, oct, rval;
	char *flags, *ep;

	Hflag = Lflag = Rflag = fflag = hflag = vflag = xflag = 0;
	while ((ch = getopt(argc, argv, "HLPRfhvx")) != -1)
		switch (ch) {
		case 'H':
			Hflag = 1;
			Lflag = 0;
			break;
		case 'L':
			Lflag = 1;
			Hflag = 0;
			break;
		case 'P':
			Hflag = Lflag = 0;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'v':
			vflag++;
			break;
		case 'x':
			xflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	if (argc < 2)
		usage();

	(void)signal(SIGINFO, siginfo_handler);

	if (Rflag) {
		if (hflag)
			errx(1, "the -R and -h options may not be "
			    "specified together.");
		if (Lflag) {
			fts_options = FTS_LOGICAL;
		} else {
			fts_options = FTS_PHYSICAL;

			if (Hflag) {
				fts_options |= FTS_COMFOLLOW;
			}
		}
	} else if (hflag) {
		fts_options = FTS_PHYSICAL;
	} else {
		fts_options = FTS_LOGICAL;
	}
	if (xflag)
		fts_options |= FTS_XDEV;

	flags = *argv;
	if (*flags >= '0' && *flags <= '7') {
		errno = 0;
		val = strtol(flags, &ep, 8);
		if (val < 0)
			errno = ERANGE;
		if (errno)
                        err(1, "invalid flags: %s", flags);
                if (*ep)
                        errx(1, "invalid flags: %s", flags);
		set = val;
                oct = 1;
	} else {
		if (strtofflags(&flags, &set, &clear))
                        errx(1, "invalid flag: %s", flags);
		clear = ~clear;
		oct = 0;
	}

	if ((ftsp = fts_open(++argv, fts_options , 0)) == NULL)
		err(1, NULL);

	for (rval = 0; errno = 0, (p = fts_read(ftsp)) != NULL;) {
		int atflag;

		if ((fts_options & FTS_LOGICAL) ||
		    ((fts_options & FTS_COMFOLLOW) &&
		    p->fts_level == FTS_ROOTLEVEL))
			atflag = 0;
		else
			atflag = AT_SYMLINK_NOFOLLOW;

		switch (p->fts_info) {
		case FTS_D:	/* Change it at FTS_DP if we're recursive. */
			if (!Rflag)
				fts_set(ftsp, p, FTS_SKIP);
			continue;
		case FTS_DNR:			/* Warn, chflags. */
			warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
			rval = 1;
			break;
		case FTS_ERR:			/* Warn, continue. */
		case FTS_NS:
			warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
			rval = 1;
			continue;
		default:
			break;
		}
		if (oct)
			newflags = set;
		else
			newflags = (p->fts_statp->st_flags | set) & clear;
		if (newflags == p->fts_statp->st_flags)
			continue;
		if (chflagsat(AT_FDCWD, p->fts_accpath, newflags,
		    atflag) == -1) {
			e = errno;
			if (!fflag) {
				warnc(e, "%s", p->fts_path);
				rval = 1;
			}
			if (siginfo) {
				(void)printf("%s: %s\n", p->fts_path,
				    strerror(e));
				siginfo = 0;
			}
		} else if (vflag || siginfo) {
			(void)printf("%s", p->fts_path);
			if (vflag > 1 || siginfo)
				(void)printf(": 0%lo -> 0%lo",
				    (u_long)p->fts_statp->st_flags,
				    newflags);
			(void)printf("\n");
			siginfo = 0;
		}
	}
	if (errno)
		err(1, "fts_read");
	exit(rval);
}

void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: chflags [-fhvx] [-R [-H | -L | -P]] flags file ...\n");
	exit(1);
}

} // namespace pbsd::bin_chflags::b0179
