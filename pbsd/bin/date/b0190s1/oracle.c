#define _GNU_SOURCE

/*-
 * Copyright (c) 1997 Brian Somers <brian@Awfulhak.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

struct vary {
	char *arg;
	struct vary *next;
};

jmp_buf oracle_err_jmp;
int oracle_err_armed;
int oracle_err_called;
int oracle_err_status;

void
oracle_err_arm(void)
{
	oracle_err_armed = 1;
}

void
oracle_err_disarm(void)
{
	oracle_err_armed = 0;
}

static void
oracle_err(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	oracle_err_called = 1;
	oracle_err_status = eval;
	if (oracle_err_armed)
		longjmp(oracle_err_jmp, 1);
	exit(eval);
}

#define err oracle_err

struct trans {
	int64_t val;
	const char *str;
};

struct trans trans_mon[] = {
	{ 1, "january" }, { 2, "february" }, { 3, "march" }, { 4, "april" },
	{ 5, "may"}, { 6, "june" }, { 7, "july" }, { 8, "august" },
	{ 9, "september" }, { 10, "october" }, { 11, "november" }, { 12, "december" },
	{ -1, NULL }
};

struct trans trans_wday[] = {
	{ 0, "sunday" }, { 1, "monday" }, { 2, "tuesday" }, { 3, "wednesday" },
	{ 4, "thursday" }, { 5, "friday" }, { 6, "saturday" },
	{ -1, NULL }
};

char digits[] = "0123456789";
int ref_adjhour(struct tm *, char, int64_t, int);

int
ref_domktime(struct tm *t, char type)
{
	time_t ret;

	while ((ret = mktime(t)) == -1 && t->tm_year > 68 && t->tm_year < 138)
		/* While mktime() fails, adjust by an hour */
		ref_adjhour(t, type == '-' ? type : '+', 1, 0);

	return ret;
}

int
ref_trans(const struct trans t[], const char *arg)
{
	int f;

	for (f = 0; t[f].val != -1; f++)
		if (!strncasecmp(t[f].str, arg, 3) ||
		    !strncasecmp(t[f].str, arg, strlen(t[f].str)))
			return t[f].val;

	return -1;
}

struct vary *
ref_vary_append(struct vary *v, char *arg)
{
	struct vary *result, **nextp;

	if (v) {
		result = v;
		while (v->next)
			v = v->next;
		nextp = &v->next;
	} else
		nextp = &result;

	if ((*nextp = (struct vary *)malloc(sizeof(struct vary))) == NULL)
		err(1, "malloc");
	(*nextp)->arg = arg;
	(*nextp)->next = NULL;
	return result;
}

int mdays[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int
ref_daysinmonth(const struct tm *t)
{
	int year;

	year = t->tm_year + 1900;

	if (t->tm_mon == 1)
		if (!(year % 400))
			return 29;
		else if (!(year % 100))
			return 28;
		else if (!(year % 4))
			return 29;
		else
			return 28;
	else if (t->tm_mon >= 0 && t->tm_mon < 12)
		return mdays[t->tm_mon];

	return 0;
}


int
ref_adjyear(struct tm *t, char type, int64_t val, int mk)
{
	switch (type) {
		case '+':
			t->tm_year += val;
			break;
		case '-':
			t->tm_year -= val;
			break;
		default:
			t->tm_year = val;
			if (t->tm_year < 69)
				t->tm_year += 100;		/* as per date.c */
			else if (t->tm_year > 1900)
				t->tm_year -= 1900;             /* struct tm holds years since 1900 */
			break;
	}
	return !mk || ref_domktime(t, type) != -1;
}

int
ref_adjmon(struct tm *t, char type, int64_t val, int istext, int mk)
{
	int lmdays;

	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (istext) {
				if (val <= t->tm_mon)
					val += 11 - t->tm_mon;	/* early next year */
				else
					val -= t->tm_mon + 1;		/* later this year */
			}
			if (val) {
				if (!ref_adjyear(t, '+', (t->tm_mon + val) / 12, 0))
					return 0;
				val %= 12;
				t->tm_mon += val;
				if (t->tm_mon > 11)
					t->tm_mon -= 12;
			}
			break;

		case '-':
			if (istext) {
				if (val-1 > t->tm_mon)
					val = 13 - val + t->tm_mon;	/* later last year */
				else
					val = t->tm_mon - val + 1;	/* early this year */
			}
			if (val) {
				if (!ref_adjyear(t, '-', val / 12, 0))
					return 0;
				val %= 12;
				if (val > t->tm_mon) {
					if (!ref_adjyear(t, '-', 1, 0))
						return 0;
					val -= 12;
				}
				t->tm_mon -= val;
			}
			break;

		default:
			if (val > 12 || val < 1)
				return 0;
			t->tm_mon = --val;
	}

	/* e.g., -v-1m on March, 31 is the last day of February in common sense */
	lmdays = ref_daysinmonth(t);
	if (t->tm_mday > lmdays)
		t->tm_mday = lmdays;

	return !mk || ref_domktime(t, type) != -1;
}

int
ref_adjday(struct tm *t, char type, int64_t val, int mk)
{
	int lmdays;

	switch (type) {
		case '+':
			while (val) {
				lmdays = ref_daysinmonth(t);
				if (val > lmdays - t->tm_mday) {
					val -= lmdays - t->tm_mday + 1;
					t->tm_mday = 1;
					if (!ref_adjmon(t, '+', 1, 0, 0))
						return 0;
				} else {
					t->tm_mday += val;
					val = 0;
				}
			}
			break;
		case '-':
			while (val)
				if (val >= t->tm_mday) {
					val -= t->tm_mday;
					t->tm_mday = 1;
					if (!ref_adjmon(t, '-', 1, 0, 0))
						return 0;
					t->tm_mday = ref_daysinmonth(t);
				} else {
					t->tm_mday -= val;
					val = 0;
				}
			break;
		default:
			if (val > 0 && val <= ref_daysinmonth(t))
				t->tm_mday = val;
			else
				return 0;
			break;
	}

	return !mk || ref_domktime(t, type) != -1;
}

int
ref_adjwday(struct tm *t, char type, int64_t val, int istext, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (istext)
				if (val < t->tm_wday)
					val = 7 - t->tm_wday + val;  /* early next week */
				else
					val -= t->tm_wday;           /* later this week */
			else
				val *= 7;                      /* "-v+5w" == "5 weeks in the future" */
			return !val || ref_adjday(t, '+', val, mk);
		case '-':
			if (istext) {
				if (val > t->tm_wday)
					val = 7 - val + t->tm_wday;  /* later last week */
				else
					val = t->tm_wday - val;      /* early this week */
			} else
				val *= 7;                      /* "-v-5w" == "5 weeks ago" */
			return !val || ref_adjday(t, '-', val, mk);
		default:
			if (val < t->tm_wday)
				return ref_adjday(t, '-', t->tm_wday - val, mk);
			else if (val > 6)
				return 0;
			else if (val > t->tm_wday)
				return ref_adjday(t, '+', val - t->tm_wday, mk);
	}
	return 1;
}

int
ref_adjhour(struct tm *t, char type, int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				int days;

				days = (t->tm_hour + val) / 24;
				val %= 24;
				t->tm_hour += val;
				t->tm_hour %= 24;
				if (!ref_adjday(t, '+', days, 0))
					return 0;
			}
			break;

		case '-':
			if (val) {
				int days;

				days = val / 24;
				val %= 24;
				if (val > t->tm_hour) {
					days++;
					val -= 24;
				}
				t->tm_hour -= val;
				if (!ref_adjday(t, '-', days, 0))
					return 0;
			}
			break;

		default:
			if (val > 23)
				return 0;
			t->tm_hour = val;
	}

	return !mk || ref_domktime(t, type) != -1;
}

int
ref_adjmin(struct tm *t, char type, int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				if (!ref_adjhour(t, '+', (t->tm_min + val) / 60, 0))
					return 0;
				val %= 60;
				t->tm_min += val;
				if (t->tm_min > 59)
					t->tm_min -= 60;
			}
			break;

		case '-':
			if (val) {
				if (!ref_adjhour(t, '-', val / 60, 0))
					return 0;
				val %= 60;
				if (val > t->tm_min) {
					if (!ref_adjhour(t, '-', 1, 0))
						return 0;
					val -= 60;
				}
				t->tm_min -= val;
			}
			break;

		default:
			if (val > 59)
				return 0;
			t->tm_min = val;
	}

	return !mk || ref_domktime(t, type) != -1;
}

int
ref_adjsec(struct tm *t, char type, int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				if (!ref_adjmin(t, '+', (t->tm_sec + val) / 60, 0))
					return 0;
				val %= 60;
				t->tm_sec += val;
				if (t->tm_sec > 59)
					t->tm_sec -= 60;
			}
			break;

		case '-':
			if (val) {
				if (!ref_adjmin(t, '-', val / 60, 0))
					return 0;
				val %= 60;
				if (val > t->tm_sec) {
					if (!ref_adjmin(t, '-', 1, 0))
						return 0;
					val -= 60;
				}
				t->tm_sec -= val;
			}
			break;

		default:
			if (val > 59)
				return 0;
			t->tm_sec = val;
	}

	return !mk || ref_domktime(t, type) != -1;
}

const struct vary *
ref_vary_apply(const struct vary *v, struct tm *t)
{
	char type;
	char which;
	char *arg;
	size_t len;
	int64_t val;

	for (; v; v = v->next) {
		type = *v->arg;
		arg = v->arg;
		if (type == '+' || type == '-')
			arg++;
		else
			type = '\0';
		len = strlen(arg);
		if (len < 2)
			return v;

		if (type == '\0')
			t->tm_isdst = -1;

		if (strspn(arg, digits) != len-1) {
			val = ref_trans(trans_wday, arg);
			if (val != -1) {
				if (!ref_adjwday(t, type, val, 1, 1))
					return v;
			} else {
				val = ref_trans(trans_mon, arg);
				if (val != -1) {
					if (!ref_adjmon(t, type, val, 1, 1))
						return v;
				} else
					return v;
			}
		} else {
			val = atoi(arg);
			which = arg[len-1];

			switch (which) {
				case 'S':
					if (!ref_adjsec(t, type, val, 1))
						return v;
					break;
				case 'M':
					if (!ref_adjmin(t, type, val, 1))
						return v;
					break;
				case 'H':
					if (!ref_adjhour(t, type, val, 1))
						return v;
					break;
				case 'd':
					t->tm_isdst = -1;
					if (!ref_adjday(t, type, val, 1))
						return v;
					break;
				case 'w':
					t->tm_isdst = -1;
					if (!ref_adjwday(t, type, val, 0, 1))
						return v;
					break;
				case 'm':
					t->tm_isdst = -1;
					if (!ref_adjmon(t, type, val, 0, 1))
						return v;
					break;
				case 'y':
					t->tm_isdst = -1;
					if (!ref_adjyear(t, type, val, 1))
						return v;
					break;
				default:
					return v;
			}
		}
	}
	return 0;
}

void
ref_vary_destroy(struct vary *v)
{
	struct vary *n;

	while (v) {
		n = v->next;
		free(v);
		v = n;
	}
}
