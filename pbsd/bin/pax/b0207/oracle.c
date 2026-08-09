/*
 * b0207 oracle: portable functions from
 *   hbsd/src/bin/pax/options.c
 *   hbsd/src/bin/pax/tables.c
 * concatenated with every function renamed with a "ref_" prefix.
 * Function bodies are otherwise unmodified.
 */

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif
#ifndef QUAD_MAX
#define QUAD_MAX	0x7fffffffffffffffLL
#endif
#define MAXKEYLEN	128		/* max number of chars for hash */

typedef struct oplist {
	char		*name;		/* option variable name e.g. name= */
	char		*value;		/* value for option variable */
	struct oplist	*fow;		/* next option */
} OPLIST;

static OPLIST *ophead = NULL;	/* head for format specific options -x */
static OPLIST *optail = NULL;	/* option tail */

static char	warnbuf[4096];
static int	warnlen;
static int	warncnt;

static void
warn_append(int set, const char *fmt, va_list ap)
{
	char tmp[1024];
	int n;

	(void)vsnprintf(tmp, sizeof(tmp), fmt, ap);
	++warncnt;
	n = snprintf(warnbuf + warnlen, sizeof(warnbuf) - (size_t)warnlen,
	    "[%d]%s\n", set, tmp);
	if (n < 0)
		return;
	warnlen += n;
	if (warnlen >= (int)sizeof(warnbuf))
		warnlen = (int)sizeof(warnbuf) - 1;
}

static void
paxwarn(int set, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	warn_append(set, fmt, ap);
	va_end(ap);
}

void
ref_warn_reset(void)
{
	warnbuf[0] = '\0';
	warnlen = 0;
	warncnt = 0;
}

int
ref_warn_count(void)
{
	return (warncnt);
}

const char *
ref_warn_text(void)
{
	return (warnbuf);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/bin/pax/options.c                                         */
/* ------------------------------------------------------------------ */

OPLIST *
ref_opt_next(void)
{
	OPLIST *opt;

	if ((opt = ophead) != NULL)
		ophead = ophead->fow;
	return(opt);
}

int
ref_opt_add(const char *str)
{
	OPLIST *opt;
	char *frpt;
	char *pt;
	char *endpt;
	char *lstr;

	if ((str == NULL) || (*str == '\0')) {
		paxwarn(0, "Invalid option name");
		return(-1);
	}
	if ((lstr = strdup(str)) == NULL) {
		paxwarn(0, "Unable to allocate space for option list");
		return(-1);
	}
	frpt = endpt = lstr;

	/*
	 * break into name and values pieces and stuff each one into a
	 * OPLIST structure. When we know the format, the format specific
	 * option function will go through this list
	 */
	while ((frpt != NULL) && (*frpt != '\0')) {
		if ((endpt = strchr(frpt, ',')) != NULL)
			*endpt = '\0';
		if ((pt = strchr(frpt, '=')) == NULL) {
			paxwarn(0, "Invalid options format");
			free(lstr);
			return(-1);
		}
		if ((opt = (OPLIST *)malloc(sizeof(OPLIST))) == NULL) {
			paxwarn(0, "Unable to allocate space for option list");
			free(lstr);
			return(-1);
		}
		lstr = NULL;	/* parts of string going onto the OPLIST */
		*pt++ = '\0';
		opt->name = frpt;
		opt->value = pt;
		opt->fow = NULL;
		if (endpt != NULL)
			frpt = endpt + 1;
		else
			frpt = NULL;
		if (ophead == NULL) {
			optail = ophead = opt;
			continue;
		}
		optail->fow = opt;
		optail = opt;
	}
	free(lstr);
	return(0);
}

off_t
ref_str_offt(char *val)
{
	char *expr;
	off_t num, t;

	num = strtoq(val, &expr, 0);
	if ((num == QUAD_MAX) || (num <= 0) || (expr == val))
		return(0);

	switch(*expr) {
	case 'b':
		t = num;
		num *= 512;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'k':
		t = num;
		num *= 1024;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'm':
		t = num;
		num *= 1048576;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'w':
		t = num;
		num *= sizeof(int);
		if (t > num)
			return(0);
		++expr;
		break;
	}

	switch(*expr) {
		case '\0':
			break;
		case '*':
		case 'x':
			t = num;
			num *= ref_str_offt(expr + 1);
			if (t > num)
				return(0);
			break;
		default:
			return(0);
	}
	return(num);
}

int
ref_no_op(void)
{
	return(0);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/bin/pax/tables.c                                          */
/* ------------------------------------------------------------------ */

u_int
ref_st_hash(char *name, int len, int tabsz)
{
	char *pt;
	char *dest;
	char *end;
	int i;
	u_int key = 0;
	int steps;
	int res;
	u_int val;

	/*
	 * only look at the tail up to MAXKEYLEN, we do not need to waste
	 * time here (remember these are pathnames, the tail is what will
	 * spread out the keys)
	 */
	if (len > MAXKEYLEN) {
		pt = &(name[len - MAXKEYLEN]);
		len = MAXKEYLEN;
	} else
		pt = name;

	/*
	 * calculate the number of u_int size steps in the string and if
	 * there is a runt to deal with
	 */
	steps = len/sizeof(u_int);
	res = len % sizeof(u_int);

	/*
	 * add up the value of the string in unsigned integer sized pieces
	 * too bad we cannot have unsigned int aligned strings, then we
	 * could avoid the expensive copy.
	 */
	for (i = 0; i < steps; ++i) {
		end = pt + sizeof(u_int);
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	/*
	 * add in the runt padded with zero to the right
	 */
	if (res) {
		val = 0;
		end = pt + res;
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	/*
	 * return the result mod the table size
	 */
	return(key % tabsz);
}
