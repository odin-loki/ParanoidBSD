/*-
 * PBSD batch b0187s2 oracle: hbsd/src/bin/stty/stty.c
 *
 * Scaffolding above the banner supplies interfaces stty.c depends on but
 * which are outside this batch.  ref_main and ref_usage are the original
 * function bodies with only the ref_ prefix added.
 */

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <termios.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif

#if defined(__linux__)
#include <bsd/stdlib.h>
#endif

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

enum FMT { NOTSET, GFLAG, BSD, POSIX };

struct pbsd_stty_hooks {
	unsigned print_calls;
	unsigned gprint_calls;
	unsigned gread_calls;
	unsigned checkredirect_calls;
	unsigned ksearch_hits;
	unsigned csearch_hits;
	unsigned msearch_hits;
	int last_print_fmt;
} pbsd_stty_hooks;

void
checkredirect(void)
{
	struct stat sb1, sb2;

	pbsd_stty_hooks.checkredirect_calls++;
	if (isatty(STDOUT_FILENO) && isatty(STDERR_FILENO) &&
	    !fstat(STDOUT_FILENO, &sb1) && !fstat(STDERR_FILENO, &sb2) &&
	    (sb1.st_rdev != sb2.st_rdev))
		warnx("stdout appears redirected, but stdin is the control descriptor");
}

int
ksearch(char ***argvp, struct info *ip)
{
	if (**argvp != NULL && strcmp(**argvp, "__ksearch__") == 0) {
		pbsd_stty_hooks.ksearch_hits++;
		++*argvp;
		ip->set = 1;
		return (1);
	}
	return (0);
}

int
csearch(char ***argvp, struct info *ip)
{
	if (**argvp != NULL && strcmp(**argvp, "__csearch__") == 0) {
		pbsd_stty_hooks.csearch_hits++;
		++*argvp;
		ip->set = 1;
		return (1);
	}
	return (0);
}

int
msearch(char ***argvp, struct info *ip)
{
	if (**argvp != NULL && strcmp(**argvp, "__msearch__") == 0) {
		pbsd_stty_hooks.msearch_hits++;
		++*argvp;
		ip->wset = 1;
		return (1);
	}
	return (0);
}

void
print(struct termios *tp, struct winsize *wp, int ldisc, enum FMT fmt)
{
	(void)tp;
	(void)wp;
	(void)ldisc;
	pbsd_stty_hooks.print_calls++;
	pbsd_stty_hooks.last_print_fmt = (int)fmt;
	putchar('P');
	fflush(stdout);
}

void
gprint(struct termios *tp, struct winsize *wp, int ldisc)
{
	(void)tp;
	(void)wp;
	(void)ldisc;
	pbsd_stty_hooks.gprint_calls++;
	putchar('G');
	fflush(stdout);
}

void
gread(struct termios *tp, char *s)
{
	(void)s;
	pbsd_stty_hooks.gread_calls++;
	tp->c_ospeed = B9600;
	tp->c_ispeed = B9600;
}

/* ------------------------- original sources ------------------------- */

#define usage ref_usage

void ref_usage(void) __dead2;

int
ref_main(int argc, char *argv[])
{
	struct info i;
	enum FMT fmt;
	int ch;
	const char *file, *errstr = NULL;

	fmt = NOTSET;
	i.fd = STDIN_FILENO;
	file = "stdin";

	opterr = 0;
	while (optind < argc &&
	    strspn(argv[optind], "-aefg") == strlen(argv[optind]) &&
	    (ch = getopt(argc, argv, "aef:g")) != -1)
		switch(ch) {
		case 'a':		/* undocumented: POSIX compatibility */
			fmt = POSIX;
			break;
		case 'e':
			fmt = BSD;
			break;
		case 'f':
			if ((i.fd = open(optarg, O_RDONLY | O_NONBLOCK)) < 0)
				err(1, "%s", optarg);
			file = optarg;
			break;
		case 'g':
			fmt = GFLAG;
			break;
		case '?':
		default:
			goto args;
		}

args:	argc -= optind;
	argv += optind;

	if (tcgetattr(i.fd, &i.t) < 0)
		errx(1, "%s isn't a terminal", file);
	if (ioctl(i.fd, TIOCGETD, &i.ldisc) < 0)
		err(1, "TIOCGETD");
	if (ioctl(i.fd, TIOCGWINSZ, &i.win) < 0)
		warn("TIOCGWINSZ");

	checkredirect();			/* conversion aid */

	switch(fmt) {
	case NOTSET:
		if (*argv)
			break;
		/* FALLTHROUGH */
	case BSD:
	case POSIX:
		print(&i.t, &i.win, i.ldisc, fmt);
		break;
	case GFLAG:
		gprint(&i.t, &i.win, i.ldisc);
		break;
	}

	for (i.set = i.wset = 0; *argv; ++argv) {
		if (ksearch(&argv, &i))
			continue;

		if (csearch(&argv, &i))
			continue;

		if (msearch(&argv, &i))
			continue;

		if (isdigit(**argv)) {
			speed_t speed;

			speed = strtonum(*argv, 0, UINT_MAX, &errstr);
			if (errstr)
				err(1, "speed");
			cfsetospeed(&i.t, speed);
			cfsetispeed(&i.t, speed);
			i.set = 1;
			continue;
		}

		if (!strncmp(*argv, "gfmt1", sizeof("gfmt1") - 1)) {
			gread(&i.t, *argv + sizeof("gfmt1") - 1);
			i.set = 1;
			continue;
		}

		warnx("illegal option -- %s", *argv);
		usage();
	}

	if (i.set && tcsetattr(i.fd, 0, &i.t) < 0)
		err(1, "tcsetattr");
	if (i.wset && ioctl(i.fd, TIOCSWINSZ, &i.win) < 0)
		warn("TIOCSWINSZ");
	exit(0);
}

void
ref_usage(void)
{

	(void)fprintf(stderr,
	    "usage: stty [-a | -e | -g] [-f file] [arguments]\n");
	exit (1);
}
