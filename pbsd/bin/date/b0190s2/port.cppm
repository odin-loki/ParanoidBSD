module;

#define _GNU_SOURCE
#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif
#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE 1900
#endif

#include <cctype>
#include <cerrno>
#include <cinttypes>
#include <clocale>
#include <csetjmp>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>
#include <utmpx.h>

export module pbsd.bin.date.b0190s2;

export namespace pbsd::bin_date::b0190s2 {

struct iso8601_fmt {
	const char *refname;
	const char *format_string;
};

inline std::jmp_buf port_err_jmp;
inline int port_err_armed;
inline int port_err_called;
inline int port_err_status;

inline void port_err_arm() { port_err_armed = 1; }
inline void port_err_disarm() { port_err_armed = 0; }

inline void port_err_common(int eval, const char *fmt, va_list ap)
{
	std::vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	port_err_called = 1;
	port_err_status = eval;
	if (port_err_armed)
		std::longjmp(port_err_jmp, 1);
	exit(eval);
}

inline void err(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	port_err_common(eval, fmt, ap);
	va_end(ap);
}

inline void errx(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	port_err_common(eval, fmt, ap);
	va_end(ap);
}

inline void warnx(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 1985, 1987, 1988, 1993 The Regents of the University of California.
 */

inline const iso8601_fmt *iso8601_fmts_table()
{
	static const iso8601_fmt t[] = {
		{ "date", "%Y-%m-%d" }, { "hours", "T%H" }, { "minutes", ":%M" },
		{ "seconds", ":%S" }, { "ns", ",%N" },
	};
	return t;
}

inline const iso8601_fmt *iso8601_selected;

inline void test_set_iso8601_selected(const iso8601_fmt *sel)
{ iso8601_selected = sel; }

inline const iso8601_fmt *test_iso8601_fmts() { return iso8601_fmts_table(); }

inline int test_iso8601_fmts_count()
{
	static const iso8601_fmt arr[] = {
		{ "date", "%Y-%m-%d" }, { "hours", "T%H" }, { "minutes", ":%M" },
		{ "seconds", ":%S" }, { "ns", ",%N" },
	};
	return (int)(sizeof(arr) / sizeof(arr[0]));
}

inline void badformat();
inline void usage();

inline std::size_t strftime_ns(char * __restrict s, std::size_t maxsize,
    const char * __restrict format, const std::tm * __restrict t, long nsec, long res)
{
	std::size_t ret;
	char *newformat, *oldformat;
	const char *prefix, *suffix, *tok;
	long number;
	int i, len, prefixlen, width, zeroes;
	bool seen_percent, seen_dash, seen_width;

	seen_percent = false;
	if ((newformat = strdup(format)) == nullptr)
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
				for (width = 9, number = res; width > 0 && number > 0;
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
			(void)asprintf(&newformat, "%.*s%.*ld%.*d%n%s", prefixlen,
			    prefix, width, number, zeroes, 0, &len, suffix);
			if (newformat == nullptr)
				err(1, "asprintf");
			std::free(oldformat);
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
	std::free(newformat);
	return (ret);
}

inline void printdate(const char *buf)
{
	(void)printf("%s\n", buf);
	if (fflush(stdout))
		err(1, "stdout");
	exit(EXIT_SUCCESS);
}

inline void printisodate(std::tm *lt, long nsec, long res)
{
	const iso8601_fmt *it;
	char fmtbuf[64], buf[64], tzbuf[8];
	const iso8601_fmt *fmts = iso8601_fmts_table();
	fmtbuf[0] = 0;
	for (it = fmts; it <= iso8601_selected; it++)
		strlcat(fmtbuf, it->format_string, sizeof(fmtbuf));
	(void)strftime_ns(buf, sizeof(buf), fmtbuf, lt, nsec, res);
	if (iso8601_selected > fmts) {
		(void)strftime_ns(tzbuf, sizeof(tzbuf), "%z", lt, nsec, res);
		memmove(&tzbuf[4], &tzbuf[3], 3);
		tzbuf[3] = ':';
		strlcat(buf, tzbuf, sizeof(buf));
	}
	printdate(buf);
}

#define ATOI2(s) ((s) += 2, ((s)[-2] - '0') * 10 + ((s)[-1] - '0'))

inline void setthetime(const char *fmt, const char *p, int jflag, struct timespec *ts)
{
	struct utmpx utx;
	std::tm *lt;
	const char *dot, *t;
	int century;

	lt = localtime(&ts->tv_sec);
	if (lt == nullptr)
		errx(1, "invalid time");
	lt->tm_isdst = -1;
	if (fmt != nullptr) {
		t = strptime(p, fmt, lt);
		if (t == nullptr) {
			fprintf(stderr, "Failed conversion of ``%s'' using format ``%s''\n", p, fmt);
			badformat();
		} else if (*t != '\0')
			fprintf(stderr, "Warning: Ignoring %ld extraneous characters in date string (%s)\n",
			    (long)strlen(t), t);
	} else {
		for (t = p, dot = nullptr; *t; ++t) {
			if (isdigit(*t))
				continue;
			if (*t == '.' && dot == nullptr) {
				dot = t;
				continue;
			}
			badformat();
		}
		if (dot != nullptr) {
			dot++;
			if (strlen(dot) != 2)
				badformat();
			lt->tm_sec = ATOI2(dot);
			if (lt->tm_sec > 61)
				badformat();
		} else
			lt->tm_sec = 0;
		century = 0;
		switch (strlen(p) - ((dot != nullptr) ? 3 : 0)) {
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
				badformat();
			--lt->tm_mon;
		case 6:
			lt->tm_mday = ATOI2(p);
			if (lt->tm_mday > 31)
				badformat();
		case 4:
			lt->tm_hour = ATOI2(p);
			if (lt->tm_hour > 23)
				badformat();
		case 2:
			lt->tm_min = ATOI2(p);
			if (lt->tm_min > 59)
				badformat();
			break;
		default:
			badformat();
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
		(void)gettimeofday(reinterpret_cast<struct timeval *>(&utx.ut_tv), nullptr);
		pututxline(&utx);
		if (clock_settime(CLOCK_REALTIME, ts) != 0)
			err(1, "clock_settime");
		utx.ut_type = NEW_TIME;
		(void)gettimeofday(reinterpret_cast<struct timeval *>(&utx.ut_tv), nullptr);
		pututxline(&utx);
		if ((p = getlogin()) == nullptr)
			p = "???";
		syslog(LOG_AUTH | LOG_NOTICE, "date set by %s", p);
	}
}

inline void badformat()
{
	warnx("illegal time format");
	usage();
}

inline void iso8601_usage(const char *badarg)
{
	errx(1, "invalid argument '%s' for -I", badarg);
}

inline void multipleformats()
{
	errx(1, "multiple output formats specified");
}

inline void usage()
{
	(void)fprintf(stderr, "%s\n%s\n%s\n",
	    "usage: date [-jnRu] [-I[date|hours|minutes|seconds|ns]] [-f input_fmt]",
	    "            [ -z output_zone ] [-r filename|seconds] [-v[+|-]val[y|m|w|d|H|M|S]]",
	    "            [[[[[[cc]yy]mm]dd]HH]MM[.SS] | new_date] [+output_fmt]");
	exit(1);
}

}
