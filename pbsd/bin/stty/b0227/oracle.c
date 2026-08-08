/* Oracle / port preamble: BSD termios layout for faithful stty batch */

#define _DEFAULT_SOURCE

#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>

extern int ioctl(int, unsigned long, ...);
void usage(void) __dead2;

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif

/* BSD termios (do not use host termios.h) */
#define VEOF		0
#define VEOL		1
#define VEOL2		2
#define VERASE		3
#define VWERASE 	4
#define VKILL		5
#define VREPRINT 	6
#define VERASE2 	7
#define VINTR		8
#define VQUIT		9
#define VSUSP		10
#define VDSUSP		11
#define VSTART		12
#define VSTOP		13
#define VLNEXT		14
#define VDISCARD	15
#define VMIN		16
#define VTIME		17
#define VSTATUS		18
#define NCCS		20
#define _POSIX_VDISABLE	0xff

#define	IGNBRK		0x00000001
#define	BRKINT		0x00000002
#define	IGNPAR		0x00000004
#define	PARMRK		0x00000008
#define	INPCK		0x00000010
#define	ISTRIP		0x00000020
#define	INLCR		0x00000040
#define	IGNCR		0x00000080
#define	ICRNL		0x00000100
#define	IXON		0x00000200
#define	IXOFF		0x00000400
#define	IXANY		0x00000800
#define	IMAXBEL		0x00002000
#define IUTF8		0x00004000

#define	OPOST		0x00000001
#define	ONLCR		0x00000002
#define	TABDLY		0x00000004
#define	    TAB0	    0x00000000
#define	    TAB3	    0x00000004
#define	OCRNL		0x00000010
#define	ONOCR		0x00000020
#define	ONLRET		0x00000040

#define	CSIZE		0x00000300
#define	    CS5		    0x00000000
#define	    CS6		    0x00000100
#define	    CS7		    0x00000200
#define	    CS8		    0x00000300
#define	CSTOPB		0x00000400
#define	CREAD		0x00000800
#define	PARENB		0x00001000
#define	PARODD		0x00002000
#define	HUPCL		0x00004000
#define	CLOCAL		0x00008000
#define	CCTS_OFLOW	0x00010000
#define	CRTSCTS		(CCTS_OFLOW | CRTS_IFLOW)
#define	CRTS_IFLOW	0x00020000
#define	CDTR_IFLOW	0x00040000
#define	CDSR_OFLOW	0x00080000
#define	CCAR_OFLOW	0x00100000
#define	CNO_RTSDTR	0x00200000
#define MDMBUF		0x00100000

#define	ECHOKE		0x00000001
#define	ECHOE		0x00000002
#define	ECHOK		0x00000004
#define	ECHO		0x00000008
#define	ECHONL		0x00000010
#define	ECHOPRT		0x00000020
#define	ECHOCTL  	0x00000040
#define	ISIG		0x00000080
#define	ICANON		0x00000100
#define	ALTWERASE	0x00000200
#define	IEXTEN		0x00000400
#define	EXTPROC         0x00000800
#define	TOSTOP		0x00400000
#define	FLUSHO		0x00800000
#define	NOKERNINFO	0x02000000
#define	PENDIN		0x20000000
#define	NOFLSH		0x80000000

#define	B0	0
#define	B9600	9600

typedef unsigned int	tcflag_t;
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned char	u_char;
typedef unsigned long	u_long;

struct termios {
	tcflag_t	c_iflag;
	tcflag_t	c_oflag;
	tcflag_t	c_lflag;
	tcflag_t	c_cflag;
	cc_t		c_cc[NCCS];
	speed_t		c_ispeed;
	speed_t		c_ospeed;
};

#define CTRL(x) ((x) >= 'a' && (x) <= 'z' ? \
	((x) - 'a' + 1) : (((x) - 'A' + 1) & 0x7f))
#define	CEOF		CTRL('D')
#define	CEOL		0xff
#define	CERASE		CTRL('?')
#define	CERASE2		CTRL('H')
#define	CINTR		CTRL('C')
#define	CSTATUS		CTRL('T')
#define	CKILL		CTRL('U')
#define	CMIN		1
#define	CQUIT		CTRL('\\')
#define	CSUSP		CTRL('Z')
#define	CTIME		0
#define	CDSUSP		CTRL('Y')
#define	CSTART		CTRL('Q')
#define	CSTOP		CTRL('S')
#define	CLNEXT		CTRL('V')
#define	CDISCARD	CTRL('O')
#define	CWERASE		CTRL('W')
#define	CREPRINT	CTRL('R')

#define	TTYDEF_IFLAG	(BRKINT | ICRNL | IMAXBEL | IXON | IXANY | IUTF8)
#define	TTYDEF_OFLAG	(OPOST | ONLCR)
#define	TTYDEF_LFLAG_ECHO (ICANON | ISIG | IEXTEN | ECHO | ECHOE | ECHOKE | ECHOCTL)
#define	TTYDEF_LFLAG TTYDEF_LFLAG_ECHO
#define	TTYDEF_CFLAG	(CREAD | CS8 | HUPCL)
#define	TTYDEF_SPEED	(B9600)

#define	TTYDISC		0
#define	SLIPDISC	4
#define	PPPDISC		5

#ifndef _IOC
struct winsize;
#define _IOC(dir,type,nr,size) \
	(((dir) << 30) | ((type) << 8) | ((nr) << 0) | ((size) << 16))
#define _IOW(type,nr,size) _IOC(1U,(type),(nr),sizeof(size))
#endif
#define	TIOCEXT		_IOW('t', 96, int)
#define	TIOCSETD	_IOW('t', 27, int)

enum FMT { NOTSET, GFLAG, BSD, POSIX };

#define	LINELENGTH	72

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

struct info {
	int fd;
	int ldisc;
	int off;
	int set;
	int wset;
	const char *arg;
	struct termios t;
	struct winsize win;
};

struct cchar {
	const char *name;
	int sub;
	u_char def;
};

struct cchar cchars1[] = {
	{ "discard",	VDISCARD, 	CDISCARD },
	{ "dsusp", 	VDSUSP,		CDSUSP },
	{ "eof",	VEOF,		CEOF },
	{ "eol",	VEOL,		CEOL },
	{ "eol2",	VEOL2,		CEOL },
	{ "erase",	VERASE,		CERASE },
	{ "erase2",	VERASE2,	CERASE2 },
	{ "intr",	VINTR,		CINTR },
	{ "kill",	VKILL,		CKILL },
	{ "lnext",	VLNEXT,		CLNEXT },
	{ "min",	VMIN,		CMIN },
	{ "quit",	VQUIT,		CQUIT },
	{ "reprint",	VREPRINT, 	CREPRINT },
	{ "start",	VSTART,		CSTART },
	{ "status",	VSTATUS, 	CSTATUS },
	{ "stop",	VSTOP,		CSTOP },
	{ "susp",	VSUSP,		CSUSP },
	{ "time",	VTIME,		CTIME },
	{ "werase",	VWERASE,	CWERASE },
	{ NULL,		0,		0},
};

static const cc_t ttydefchars[] = {
	CEOF, CEOL, CEOL, CERASE, CWERASE, CKILL, CREPRINT, CERASE2, CINTR,
	CQUIT, CSUSP, CDSUSP, CSTART, CSTOP, CLNEXT, CDISCARD, CMIN, CTIME,
	CSTATUS, _POSIX_VDISABLE
};

static speed_t cfgetispeed(const struct termios *t) { return t->c_ispeed; }
static speed_t cfgetospeed(const struct termios *t) { return t->c_ospeed; }
static int cfsetispeed(struct termios *t, speed_t s) { t->c_ispeed = s; return 0; }
static int cfsetospeed(struct termios *t, speed_t s) { t->c_ospeed = s; return 0; }

static void cfmakeraw(struct termios *t)
{
	t->c_iflag &= ~(IMAXBEL|IXOFF|INPCK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IGNPAR);
	t->c_iflag |= IGNBRK;
	t->c_oflag &= ~OPOST;
	t->c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|ISIG|IEXTEN|NOFLSH|TOSTOP|PENDIN);
	t->c_cflag &= ~(CSIZE|PARENB);
	t->c_cflag |= CS8|CREAD;
	t->c_cc[VMIN] = 1;
	t->c_cc[VTIME] = 0;
}

static void cfmakesane(struct termios *t)
{
	t->c_cflag = TTYDEF_CFLAG;
	t->c_iflag = TTYDEF_IFLAG;
	t->c_lflag = TTYDEF_LFLAG;
	t->c_oflag = TTYDEF_OFLAG;
	t->c_ispeed = TTYDEF_SPEED;
	t->c_ospeed = TTYDEF_SPEED;
	memcpy(&t->c_cc, ttydefchars, sizeof ttydefchars);
}

#define	LKEEP	(ECHOKE|ECHOE|ECHOK|ECHOPRT|ECHOCTL|ALTWERASE|TOSTOP|NOFLSH)

void ref_f_all(struct info *);
void ref_f_cbreak(struct info *);
void ref_f_columns(struct info *);
void ref_f_dec(struct info *);
void ref_f_ek(struct info *);
void ref_f_everything(struct info *);
void ref_f_extproc(struct info *);
void ref_f_ispeed(struct info *);
void ref_f_nl(struct info *);
void ref_f_ospeed(struct info *);
void ref_f_raw(struct info *);
void ref_f_rows(struct info *);
void ref_f_sane(struct info *);
void ref_f_size(struct info *);
void ref_f_speed(struct info *);
void ref_f_tty(struct info *);
int ref_ksearch(char ***, struct info *);
int ref_msearch(char ***, struct info *);
void ref_print(struct termios *, struct winsize *, int, enum FMT);


/* modes.c */
/*-
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



struct modes {
	const char *name;
	long set;
	long unset;
};

/*
 * The code in optlist() depends on minus options following regular
 * options, i.e. "foo" must immediately precede "-foo".
 */
static const struct modes cmodes[] = {
	{ "cs5",	CS5, CSIZE },
	{ "cs6",	CS6, CSIZE },
	{ "cs7",	CS7, CSIZE },
	{ "cs8",	CS8, CSIZE },
	{ "cstopb",	CSTOPB, 0 },
	{ "-cstopb",	0, CSTOPB },
	{ "cread",	CREAD, 0 },
	{ "-cread",	0, CREAD },
	{ "parenb",	PARENB, 0 },
	{ "-parenb",	0, PARENB },
	{ "parodd",	PARODD, 0 },
	{ "-parodd",	0, PARODD },
	{ "parity",	PARENB | CS7, PARODD | CSIZE },
	{ "-parity",	CS8, PARODD | PARENB | CSIZE },
	{ "evenp",	PARENB | CS7, PARODD | CSIZE },
	{ "-evenp",	CS8, PARODD | PARENB | CSIZE },
	{ "oddp",	PARENB | CS7 | PARODD, CSIZE },
	{ "-oddp",	CS8, PARODD | PARENB | CSIZE },
	{ "pass8",	CS8, PARODD | PARENB | CSIZE },
	{ "-pass8",	PARENB | CS7, PARODD | CSIZE },
	{ "hupcl",	HUPCL, 0 },
	{ "-hupcl",	0, HUPCL },
	{ "hup",	HUPCL, 0 },
	{ "-hup",	0, HUPCL },
	{ "clocal",	CLOCAL, 0 },
	{ "-clocal",	0, CLOCAL },
	{ "crtscts",	CRTSCTS, 0 },
	{ "-crtscts",	0, CRTSCTS },
	{ "ctsflow",	CCTS_OFLOW, 0 },
	{ "-ctsflow",	0, CCTS_OFLOW },
	{ "dsrflow",	CDSR_OFLOW, 0 },
	{ "-dsrflow",	0, CDSR_OFLOW },
	{ "dtrflow",	CDTR_IFLOW, 0 },
	{ "-dtrflow",	0, CDTR_IFLOW },
	{ "rtsflow",	CRTS_IFLOW, 0 },
	{ "-rtsflow",	0, CRTS_IFLOW },
	{ "mdmbuf",	MDMBUF, 0 },
	{ "-mdmbuf",	0, MDMBUF },
	{ "rtsdtr",	0, CNO_RTSDTR },
	{ "-rtsdtr",	CNO_RTSDTR, 0 },
	{ NULL,		0, 0 },
};

static const struct modes imodes[] = {
	{ "ignbrk",	IGNBRK, 0 },
	{ "-ignbrk",	0, IGNBRK },
	{ "brkint",	BRKINT, 0 },
	{ "-brkint",	0, BRKINT },
	{ "ignpar",	IGNPAR, 0 },
	{ "-ignpar",	0, IGNPAR },
	{ "parmrk",	PARMRK, 0 },
	{ "-parmrk",	0, PARMRK },
	{ "inpck",	INPCK, 0 },
	{ "-inpck",	0, INPCK },
	{ "istrip",	ISTRIP, 0 },
	{ "-istrip",	0, ISTRIP },
	{ "inlcr",	INLCR, 0 },
	{ "-inlcr",	0, INLCR },
	{ "igncr",	IGNCR, 0 },
	{ "-igncr",	0, IGNCR },
	{ "icrnl",	ICRNL, 0 },
	{ "-icrnl",	0, ICRNL },
	{ "ixon",	IXON, 0 },
	{ "-ixon",	0, IXON },
	{ "flow",	IXON, 0 },
	{ "-flow",	0, IXON },
	{ "ixoff",	IXOFF, 0 },
	{ "-ixoff",	0, IXOFF },
	{ "tandem",	IXOFF, 0 },
	{ "-tandem",	0, IXOFF },
	{ "ixany",	IXANY, 0 },
	{ "-ixany",	0, IXANY },
	{ "decctlq",	0, IXANY },
	{ "-decctlq",	IXANY, 0 },
	{ "imaxbel",	IMAXBEL, 0 },
	{ "-imaxbel",	0, IMAXBEL },
	{ "iutf8",	IUTF8, 0 },
	{ "-iutf8",	0, IUTF8 },
	{ NULL,		0, 0 },
};

static const struct modes lmodes[] = {
	{ "echo",	ECHO, 0 },
	{ "-echo",	0, ECHO },
	{ "echoe",	ECHOE, 0 },
	{ "-echoe",	0, ECHOE },
	{ "crterase",	ECHOE, 0 },
	{ "-crterase",	0, ECHOE },
	{ "crtbs",	ECHOE, 0 },	/* crtbs not supported, close enough */
	{ "-crtbs",	0, ECHOE },
	{ "echok",	ECHOK, 0 },
	{ "-echok",	0, ECHOK },
	{ "echoke",	ECHOKE, 0 },
	{ "-echoke",	0, ECHOKE },
	{ "crtkill",	ECHOKE, 0 },
	{ "-crtkill",	0, ECHOKE },
	{ "altwerase",	ALTWERASE, 0 },
	{ "-altwerase",	0, ALTWERASE },
	{ "iexten",	IEXTEN, 0 },
	{ "-iexten",	0, IEXTEN },
	{ "echonl",	ECHONL, 0 },
	{ "-echonl",	0, ECHONL },
	{ "echoctl",	ECHOCTL, 0 },
	{ "-echoctl",	0, ECHOCTL },
	{ "ctlecho",	ECHOCTL, 0 },
	{ "-ctlecho",	0, ECHOCTL },
	{ "echoprt",	ECHOPRT, 0 },
	{ "-echoprt",	0, ECHOPRT },
	{ "prterase",	ECHOPRT, 0 },
	{ "-prterase",	0, ECHOPRT },
	{ "isig",	ISIG, 0 },
	{ "-isig",	0, ISIG },
	{ "icanon",	ICANON, 0 },
	{ "-icanon",	0, ICANON },
	{ "noflsh",	NOFLSH, 0 },
	{ "-noflsh",	0, NOFLSH },
	{ "tostop",	TOSTOP, 0 },
	{ "-tostop",	0, TOSTOP },
	{ "flusho",	FLUSHO, 0 },
	{ "-flusho",	0, FLUSHO },
	{ "pendin",	PENDIN, 0 },
	{ "-pendin",	0, PENDIN },
	{ "crt",	ECHOE|ECHOKE|ECHOCTL, ECHOK|ECHOPRT },
	{ "-crt",	ECHOK, ECHOE|ECHOKE|ECHOCTL },
	{ "newcrt",	ECHOE|ECHOKE|ECHOCTL, ECHOK|ECHOPRT },
	{ "-newcrt",	ECHOK, ECHOE|ECHOKE|ECHOCTL },
	{ "nokerninfo",	NOKERNINFO, 0 },
	{ "-nokerninfo",0, NOKERNINFO },
	{ "kerninfo",	0, NOKERNINFO },
	{ "-kerninfo",	NOKERNINFO, 0 },
	{ NULL,		0, 0 },
};

static const struct modes omodes[] = {
	{ "opost",	OPOST, 0 },
	{ "-opost",	0, OPOST },
	{ "litout",	0, OPOST },
	{ "-litout",	OPOST, 0 },
	{ "onlcr",	ONLCR, 0 },
	{ "-onlcr",	0, ONLCR },
	{ "ocrnl",	OCRNL, 0 },
	{ "-ocrnl",	0, OCRNL },
	{ "tabs",	TAB0, TABDLY },		/* "preserve" tabs */
	{ "-tabs",	TAB3, TABDLY },
	{ "oxtabs",	TAB3, TABDLY },
	{ "-oxtabs",	TAB0, TABDLY },
	{ "tab0",	TAB0, TABDLY },
	{ "tab3",	TAB3, TABDLY },
	{ "onocr",	ONOCR, 0 },
	{ "-onocr",	0, ONOCR },
	{ "onlret",	ONLRET, 0 },
	{ "-onlret",	0, ONLRET },
	{ NULL,		0, 0 },
};

#define	CHK(s)	(*name == s[0] && !strcmp(name, s))

int
ref_msearch(char ***argvp, struct info *ip)
{
	const struct modes *mp;
	char *name;

	name = **argvp;

	for (mp = cmodes; mp->name; ++mp)
		if (CHK(mp->name)) {
			ip->t.c_cflag &= ~mp->unset;
			ip->t.c_cflag |= mp->set;
			ip->set = 1;
			return (1);
		}
	for (mp = imodes; mp->name; ++mp)
		if (CHK(mp->name)) {
			ip->t.c_iflag &= ~mp->unset;
			ip->t.c_iflag |= mp->set;
			ip->set = 1;
			return (1);
		}
	for (mp = lmodes; mp->name; ++mp)
		if (CHK(mp->name)) {
			ip->t.c_lflag &= ~mp->unset;
			ip->t.c_lflag |= mp->set;
			ip->set = 1;
			return (1);
		}
	for (mp = omodes; mp->name; ++mp)
		if (CHK(mp->name)) {
			ip->t.c_oflag &= ~mp->unset;
			ip->t.c_oflag |= mp->set;
			ip->set = 1;
			return (1);
		}
	return (0);
}

/* print.c */
/*-
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




static void  ref_binit(const char *);
static void  ref_bput(const char *);
static const char *ref_ccval(struct cchar *, int);

void
ref_print(struct termios *tp, struct winsize *wp, int ldisc, enum FMT fmt)
{
	struct cchar *p;
	long tmp;
	u_char *cc;
	int cnt, ispeed, ospeed;
	char buf1[100], buf2[100];

	cnt = 0;

	/* Line discipline. */
	if (ldisc != TTYDISC) {
		switch(ldisc) {
		case SLIPDISC:
			cnt += printf("slip disc; ");
			break;
		case PPPDISC:
			cnt += printf("ppp disc; ");
			break;
		default:
			cnt += printf("#%d disc; ", ldisc);
			break;
		}
	}

	/* Line speed. */
	ispeed = cfgetispeed(tp);
	ospeed = cfgetospeed(tp);
	if (ispeed != ospeed)
		cnt +=
		    printf("ispeed %d baud; ospeed %d baud;", ispeed, ospeed);
	else
		cnt += printf("speed %d baud;", ispeed);
	if (fmt >= BSD)
		cnt += printf(" %d rows; %d columns;", wp->ws_row, wp->ws_col);
	if (cnt)
		(void)printf("\n");

#define	on(f)	((tmp & (f)) != 0)
#define put(n, f, d) \
	if (fmt >= BSD || on(f) != (d)) \
		ref_bput((n) + on(f));

	/* "local" flags */
	tmp = tp->c_lflag;
	ref_binit("lflags");
	put("-icanon", ICANON, 1);
	put("-isig", ISIG, 1);
	put("-iexten", IEXTEN, 1);
	put("-echo", ECHO, 1);
	put("-echoe", ECHOE, 0);
	put("-echok", ECHOK, 0);
	put("-echoke", ECHOKE, 0);
	put("-echonl", ECHONL, 0);
	put("-echoctl", ECHOCTL, 0);
	put("-echoprt", ECHOPRT, 0);
	put("-altwerase", ALTWERASE, 0);
	put("-noflsh", NOFLSH, 0);
	put("-tostop", TOSTOP, 0);
	put("-flusho", FLUSHO, 0);
	put("-pendin", PENDIN, 0);
	put("-nokerninfo", NOKERNINFO, 0);
	put("-extproc", EXTPROC, 0);

	/* input flags */
	tmp = tp->c_iflag;
	ref_binit("iflags");
	put("-istrip", ISTRIP, 0);
	put("-icrnl", ICRNL, 1);
	put("-inlcr", INLCR, 0);
	put("-igncr", IGNCR, 0);
	put("-ixon", IXON, 1);
	put("-ixoff", IXOFF, 0);
	put("-ixany", IXANY, 1);
	put("-imaxbel", IMAXBEL, 1);
	put("-ignbrk", IGNBRK, 0);
	put("-brkint", BRKINT, 1);
	put("-inpck", INPCK, 0);
	put("-ignpar", IGNPAR, 0);
	put("-parmrk", PARMRK, 0);
	put("-iutf8", IUTF8, 1);

	/* output flags */
	tmp = tp->c_oflag;
	ref_binit("oflags");
	put("-opost", OPOST, 1);
	put("-onlcr", ONLCR, 1);
	put("-ocrnl", OCRNL, 0);
	switch(tmp&TABDLY) {
	case TAB0:
		ref_bput("tab0");
		break;
	case TAB3:
		ref_bput("tab3");
		break;
	}
	put("-onocr", ONOCR, 0);
	put("-onlret", ONLRET, 0);

	/* control flags (hardware state) */
	tmp = tp->c_cflag;
	ref_binit("cflags");
	put("-cread", CREAD, 1);
	switch(tmp&CSIZE) {
	case CS5:
		ref_bput("cs5");
		break;
	case CS6:
		ref_bput("cs6");
		break;
	case CS7:
		ref_bput("cs7");
		break;
	case CS8:
		ref_bput("cs8");
		break;
	}
	ref_bput("-parenb" + on(PARENB));
	put("-parodd", PARODD, 0);
	put("-hupcl", HUPCL, 1);
	put("-clocal", CLOCAL, 0);
	put("-cstopb", CSTOPB, 0);
	switch(tmp & (CCTS_OFLOW | CRTS_IFLOW)) {
	case CCTS_OFLOW:
		ref_bput("ctsflow");
		break;
	case CRTS_IFLOW:
		ref_bput("rtsflow");
		break;
	default:
		put("-crtscts", CCTS_OFLOW | CRTS_IFLOW, 0);
		break;
	}
	put("-dsrflow", CDSR_OFLOW, 0);
	put("-dtrflow", CDTR_IFLOW, 0);
	put("-mdmbuf", MDMBUF, 0);	/* XXX mdmbuf ==  dtrflow */
	if (on(CNO_RTSDTR))
		ref_bput("-rtsdtr");
	else {
		if (fmt >= BSD)
			ref_bput("rtsdtr");
	}

	/* special control characters */
	cc = tp->c_cc;
	if (fmt == POSIX) {
		ref_binit("cchars");
		for (p = cchars1; p->name; ++p) {
			(void)snprintf(buf1, sizeof(buf1), "%s = %s;",
			    p->name, ref_ccval(p, cc[p->sub]));
			ref_bput(buf1);
		}
		ref_binit(NULL);
	} else {
		ref_binit(NULL);
		for (p = cchars1, cnt = 0; p->name; ++p) {
			if (fmt != BSD && cc[p->sub] == p->def)
				continue;
#define	WD	"%-8s"
			(void)snprintf(buf1 + cnt * 8, sizeof(buf1) - cnt * 8,
			    WD, p->name);
			(void)snprintf(buf2 + cnt * 8, sizeof(buf2) - cnt * 8,
			    WD, ref_ccval(p, cc[p->sub]));
			if (++cnt == LINELENGTH / 8) {
				cnt = 0;
				(void)printf("%s\n", buf1);
				(void)printf("%s\n", buf2);
			}
		}
		if (cnt) {
			(void)printf("%s\n", buf1);
			(void)printf("%s\n", buf2);
		}
	}
}

static int col;
static const char *label;

static void
ref_binit(const char *lb)
{

	if (col) {
		(void)printf("\n");
		col = 0;
	}
	label = lb;
}

static void
ref_bput(const char *s)
{

	if (col == 0) {
		col = printf("%s: %s", label, s);
		return;
	}
	if ((col + strlen(s)) > LINELENGTH) {
		(void)printf("\n\t");
		col = printf("%s", s) + 8;
		return;
	}
	col += printf(" %s", s);
}

static const char *
ref_ccval(struct cchar *p, int c)
{
	static char buf[5];
	char *bp;

	if (p->sub == VMIN || p->sub == VTIME) {
		(void)snprintf(buf, sizeof(buf), "%d", c);
		return (buf);
	}
	if (c == _POSIX_VDISABLE)
		return ("<undef>");
	bp = buf;
	if (c & 0200) {
		*bp++ = 'M';
		*bp++ = '-';
		c &= 0177;
	}
	if (c == 0177) {
		*bp++ = '^';
		*bp++ = '?';
	}
	else if (c < 040) {
		*bp++ = '^';
		*bp++ = c + '@';
	}
	else
		*bp++ = c;
	*bp = '\0';
	return (buf);
}

/* key.c */
/*-
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





static struct key {
	const char *name;			/* name */
	void (*f)(struct info *);		/* function */
#define	F_NEEDARG	0x01			/* needs an argument */
#define	F_OFFOK		0x02			/* can turn off */
	int flags;
} keys[] = {
	{ "all",	ref_f_all,		0 },
	{ "cbreak",	ref_f_cbreak,	F_OFFOK },
	{ "cols",	ref_f_columns,	F_NEEDARG },
	{ "columns",	ref_f_columns,	F_NEEDARG },
	{ "cooked", 	ref_f_sane,		0 },
	{ "dec",	ref_f_dec,		0 },
	{ "ek",		ref_f_ek,		0 },
	{ "everything",	ref_f_everything,	0 },
	{ "extproc",	ref_f_extproc,	F_OFFOK },
	{ "ispeed",	ref_f_ispeed,	F_NEEDARG },
	{ "new",	ref_f_tty,		0 },
	{ "nl",		ref_f_nl,		F_OFFOK },
	{ "old",	ref_f_tty,		0 },
	{ "ospeed",	ref_f_ospeed,	F_NEEDARG },
	{ "raw",	ref_f_raw,		F_OFFOK },
	{ "rows",	ref_f_rows,		F_NEEDARG },
	{ "sane",	ref_f_sane,		0 },
	{ "size",	ref_f_size,		0 },
	{ "speed",	ref_f_speed,	0 },
	{ "tty",	ref_f_tty,		0 },
};

static int
ref_c_key(const void *a, const void *b)
{

        return (strcmp(((const struct key *)a)->name, ((const struct key *)b)->name));
}

int
ref_ksearch(char ***argvp, struct info *ip)
{
	char *name;
	struct key *kp, tmp;

	name = **argvp;
	if (*name == '-') {
		ip->off = 1;
		++name;
	} else
		ip->off = 0;

	tmp.name = name;
	if (!(kp = (struct key *)bsearch(&tmp, keys,
	    sizeof(keys)/sizeof(struct key), sizeof(struct key), ref_c_key)))
		return (0);
	if (!(kp->flags & F_OFFOK) && ip->off) {
		warnx("illegal option -- -%s", name);
		usage();
	}
	if (kp->flags & F_NEEDARG && !(ip->arg = *++*argvp)) {
		warnx("option requires an argument -- %s", name);
		usage();
	}
	kp->f(ip);
	return (1);
}

void
ref_f_all(struct info *ip)
{
	ref_print(&ip->t, &ip->win, ip->ldisc, BSD);
}

void
ref_f_cbreak(struct info *ip)
{

	if (ip->off)
		ref_f_sane(ip);
	else {
		ip->t.c_iflag |= BRKINT|IXON|IMAXBEL;
		ip->t.c_oflag |= OPOST;
		ip->t.c_lflag |= ISIG|IEXTEN;
		ip->t.c_lflag &= ~ICANON;
		ip->set = 1;
	}
}

void
ref_f_columns(struct info *ip)
{

	ip->win.ws_col = atoi(ip->arg);
	ip->wset = 1;
}

void
ref_f_dec(struct info *ip)
{

	ip->t.c_cc[VERASE] = (u_char)0177;
	ip->t.c_cc[VKILL] = CTRL('u');
	ip->t.c_cc[VINTR] = CTRL('c');
	ip->t.c_lflag &= ~ECHOPRT;
	ip->t.c_lflag |= ECHOE|ECHOKE|ECHOCTL;
	ip->t.c_iflag &= ~IXANY;
	ip->set = 1;
}

void
ref_f_ek(struct info *ip)
{

	ip->t.c_cc[VERASE] = CERASE;
	ip->t.c_cc[VKILL] = CKILL;
	ip->set = 1;
}

void
ref_f_everything(struct info *ip)
{

	ref_print(&ip->t, &ip->win, ip->ldisc, BSD);
}

void
ref_f_extproc(struct info *ip)
{

	if (ip->off) {
		int tmp = 0;
		(void)ioctl(ip->fd, TIOCEXT, &tmp);
	} else {
		int tmp = 1;
		(void)ioctl(ip->fd, TIOCEXT, &tmp);
	}
}

void
ref_f_ispeed(struct info *ip)
{

	cfsetispeed(&ip->t, (speed_t)atoi(ip->arg));
	ip->set = 1;
}

void
ref_f_nl(struct info *ip)
{

	if (ip->off) {
		ip->t.c_iflag |= ICRNL;
		ip->t.c_oflag |= ONLCR;
	} else {
		ip->t.c_iflag &= ~ICRNL;
		ip->t.c_oflag &= ~ONLCR;
	}
	ip->set = 1;
}

void
ref_f_ospeed(struct info *ip)
{

	cfsetospeed(&ip->t, (speed_t)atoi(ip->arg));
	ip->set = 1;
}

void
ref_f_raw(struct info *ip)
{

	if (ip->off)
		ref_f_sane(ip);
	else {
		cfmakeraw(&ip->t);
		ip->t.c_cflag &= ~(CSIZE|PARENB);
		ip->t.c_cflag |= CS8;
		ip->set = 1;
	}
}

void
ref_f_rows(struct info *ip)
{

	ip->win.ws_row = atoi(ip->arg);
	ip->wset = 1;
}

void
ref_f_sane(struct info *ip)
{
	struct termios def;

	cfmakesane(&def);
	ip->t.c_cflag = def.c_cflag | (ip->t.c_cflag & CLOCAL);
	ip->t.c_iflag = def.c_iflag;
	/* preserve user-preference flags in lflag */
#define	LKEEP	(ECHOKE|ECHOE|ECHOK|ECHOPRT|ECHOCTL|ALTWERASE|TOSTOP|NOFLSH)
	ip->t.c_lflag = def.c_lflag | (ip->t.c_lflag & LKEEP);
	ip->t.c_oflag = def.c_oflag;
	ip->set = 1;
}

void
ref_f_size(struct info *ip)
{

	(void)printf("%d %d\n", ip->win.ws_row, ip->win.ws_col);
}

void
ref_f_speed(struct info *ip)
{

	(void)printf("%lu\n", (u_long)cfgetospeed(&ip->t));
}

void
ref_f_tty(struct info *ip)
{
	int tmp;

	tmp = TTYDISC;
	if (ioctl(ip->fd, TIOCSETD, &tmp) < 0)
		err(1, "TIOCSETD");
}
