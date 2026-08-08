#define _GNU_SOURCE

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE	1900
#endif

#ifndef nitems
#define nitems(a)	(sizeof(a) / sizeof((a)[0]))
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <utmpx.h>

jmp_buf oracle_err_jmp;
int oracle_err_armed;
int oracle_err_called;
int oracle_err_status;

void oracle_err_arm(void) { oracle_err_armed = 1; }
void oracle_err_disarm(void) { oracle_err_armed = 0; }

static void oracle_err_common(int eval, const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	oracle_err_called = 1;
	oracle_err_status = eval;
	if (oracle_err_armed)
		longjmp(oracle_err_jmp, 1);
	exit(eval);
}

static void oracle_err(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	oracle_err_common(eval, fmt, ap);
	va_end(ap);
}

static void oracle_errx(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	oracle_err_common(eval, fmt, ap);
	va_end(ap);
}

static void oracle_warnx(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

#define err oracle_err
#define errx oracle_errx
#define warnx oracle_warnx

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1985, 1987, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 */

struct iso8601_fmt {
	const char *refname;
	const char *format_string;
};

static const struct iso8601_fmt iso8601_fmts[] = {
	{ "date", "%Y-%m-%d" },
	{ "hours", "T%H" },
	{ "minutes", ":%M" },
	{ "seconds", ":%S" },
	{ "ns", ",%N" },
};
static const struct iso8601_fmt *iso8601_selected;
static const char *rfc2822_format = "%a, %d %b %Y %T %z";

size_t ref_strftime_ns(char * __restrict, size_t, const char * __restrict,
    const struct tm * __restrict, long, long);
void ref_badformat(void);
void ref_usage(void);

void ref_test_set_iso8601_selected(const struct iso8601_fmt *sel)
{ iso8601_selected = sel; }
const struct iso8601_fmt *ref_test_iso8601_fmts(void) { return iso8601_fmts; }
int ref_test_iso8601_fmts_count(void) { return (int)nitems(iso8601_fmts); }

void ref_printdate(const char *buf)
{
	(void)printf("%s\n", buf);
	if (fflush(stdout))
		err(1, "stdout");
	exit(EXIT_SUCCESS);
}

void ref_printisodate(struct tm *lt, long nsec, long res)
{
	const struct iso8601_fmt *it;
	char fmtbuf[64], buf[64], tzbuf[8];
	fmtbuf[0] = 0;
	for (it = iso8601_fmts; it <= iso8601_selected; it++)
		strlcat(fmtbuf, it->format_string, sizeof(fmtbuf));
	(void)ref_strftime_ns(buf, sizeof(buf), fmtbuf, lt, nsec, res);
	if (iso8601_selected > iso8601_fmts) {
		(void)ref_strftime_ns(tzbuf, sizeof(tzbuf), "%z", lt, nsec, res);
		memmove(&tzbuf[4], &tzbuf[3], 3);
		tzbuf[3] = ':';
		strlcat(buf, tzbuf, sizeof(buf));
	}
	ref_printdate(buf);
}

#define	ATOI2(s)	((s) += 2, ((s)[-2] - '0') * 10 + ((s)[-1] - '0'))

void ref_setthetime(const char *fmt, const char *p, int jflag, struct timespec *ts)
{
	struct utmpx utx;
	struct tm *lt;
	const char *dot, *t;
	int century;

	lt = localtime(&ts->tv_sec);
	if (lt == NULL)
		errx(1, "invalid time");
	lt->tm_isdst = -1;

	if (fmt != NULL) {
		t = strptime(p, fmt, lt);
		if (t == NULL) {
			fprintf(stderr, "Failed conversion of ``%s''"
				" using format ``%s''\n", p, fmt);
			ref_badformat();
		} else if (*t != '\0')
			fprintf(stderr, "Warning: Ignoring %ld extraneous"
				" characters in date string (%s)\n",
				(long) strlen(t), t);
	} else {
		for (t = p, dot = NULL; *t; ++t) {
			if (isdigit(*t))
				continue;
			if (*t == '.' && dot == NULL) {
				dot = t;
				continue;
			}
			ref_badformat();
		}
		if (dot != NULL) {
			dot++;
			if (strlen(dot) != 2)
				ref_badformat();
			lt->tm_sec = ATOI2(dot);
			if (lt->tm_sec > 61)
				ref_badformat();
		} else
			lt->tm_sec = 0;
		century = 0;
		switch (strlen(p) - ((dot != NULL) ? 3 : 0)) {
		case 12:
			lt->tm_year = ATOI2(p) * 100 - TM_YEAR_BASE;
			century = 1;
		case 10:
			if (century)
				lt->tm_year += ATOI2(p);
			else {
				lt->tm_year = ATOI2(p);
				if (lt->tm_year < 69)
					lt->tm_year += 2000 - TM_YEAR_BASE;
				else
					lt->tm_year += 1900 - TM_YEAR_BASE;
			}
		case 8:
			lt->tm_mon = ATOI2(p);
			if (lt->tm_mon > 12)
				ref_badformat();
			--lt->tm_mon;
		case 6:
			lt->tm_mday = ATOI2(p);
			if (lt->tm_mday > 31)
				ref_badformat();
		case 4:
			lt->tm_hour = ATOI2(p);
			if (lt->tm_hour > 23)
				ref_badformat();
		case 2:
			lt->tm_min = ATOI2(p);
			if (lt->tm_min > 59)
				ref_badformat();
			break;
		default:
			ref_badformat();
		}
	}
	lt->tm_yday = -1;
	ts->tv_sec = mktime(lt);
	if (lt->tm_yday == -1)
		errx(1, "nonexistent time");
	ts->tv_nsec = 0;
	if (!jflag) {
		utx.ut_type = OLD_TIME;
		memset(utx.ut_id, 0, sizeof(utx.ut_id));
		(void)gettimeofday(&utx.ut_tv, NULL);
		pututxline(&utx);
		if (clock_settime(CLOCK_REALTIME, ts) != 0)
			err(1, "clock_settime");
		utx.ut_type = NEW_TIME;
		(void)gettimeofday(&utx.ut_tv, NULL);
		pututxline(&utx);
		if ((p = getlogin()) == NULL)
			p = "???";
		syslog(LOG_AUTH | LOG_NOTICE, "date set by %s", p);
	}
}

size_t ref_strftime_ns(char * __restrict s, size_t maxsize, const char * __restrict format,
    const struct tm * __restrict t, long nsec, long res)
{
	size_t ret;
	char *newformat;
	char *oldformat;
	const char *prefix;
	const char *suffix;
	const char *tok;
	long number;
	int i, len, prefixlen, width, zeroes;
	bool seen_percent, seen_dash, seen_width;

	seen_percent = false;
	if ((newformat = strdup(format)) == NULL)
		err(1, "strdup");
	for (tok = newformat; *tok != '\0'; tok++) {
		switch (*tok) {
		case '%':
			if (seen_percent)
				seen_percent = false;
			else {
				seen_percent = true;
				seen_dash = seen_width = false;
				prefixlen = tok - newformat;
				width = 0;
			}
			break;
		case 'N':
			if (!seen_percent)
				break;
			oldformat = newformat;
			prefix = oldformat;
			suffix = tok + 1;
			if (seen_dash) {
				for (width = 9, number = res;
				     width > 0 && number > 0;
				     width--, number /= 10)
					;
			}
			number = nsec;
			zeroes = 0;
			if (width == 0)
				width = 9;
			else if (width > 9) {
				zeroes = width - 9;
				width = 9;
			} else {
				for (i = 0; i < 9 - width; i++)
					number /= 10;
			}
			asprintf(&newformat, "%.*s%.*ld%.*d%n%s", prefixlen,
			    prefix, width, number, zeroes, 0, &len, suffix);
			if (newformat == NULL)
				err(1, "asprintf");
			free(oldformat);
			tok = newformat + len - 1;
			seen_percent = false;
			break;
		case '-':
			if (seen_percent) {
				if (seen_dash || seen_width) {
					seen_percent = false;
					break;
				}
				seen_dash = true;
			}
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (seen_percent) {
				if (seen_dash) {
					seen_percent = false;
					break;
				}
				width = width * 10 + *tok - '0';
				seen_width = true;
			}
			break;
		default:
			seen_percent = false;
			break;
		}
	}
	ret = strftime(s, maxsize, newformat, t);
	free(newformat);
	return (ret);
}

void ref_badformat(void)
{
	warnx("illegal time format");
	ref_usage();
}

void ref_iso8601_usage(const char *badarg)
{
	errx(1, "invalid argument '%s' for -I", badarg);
}

void ref_multipleformats(void)
{
	errx(1, "multiple output formats specified");
}

void ref_usage(void)
{
	(void)fprintf(stderr, "%s\n%s\n%s\n",
	    "usage: date [-jnRu] [-I[date|hours|minutes|seconds|ns]] [-f input_fmt]",
	    "            "
	    "[ -z output_zone ] [-r filename|seconds] [-v[+|-]val[y|m|w|d|H|M|S]]",
	    "            "
	    "[[[[[[cc]yy]mm]dd]HH]MM[.SS] | new_date] [+output_fmt]"
	    );
	exit(1);
}
