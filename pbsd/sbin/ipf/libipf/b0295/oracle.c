/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0295.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef uint32_t u_32_t;

#ifndef TH_FIN
#define TH_FIN 0x01
#endif
#ifndef TH_SYN
#define TH_SYN 0x02
#endif
#ifndef TH_RST
#define TH_RST 0x04
#endif
#ifndef TH_PUSH
#define TH_PUSH 0x08
#endif
#ifndef TH_ACK
#define TH_ACK 0x10
#endif
#ifndef TH_URG
#define TH_URG 0x20
#endif
#ifndef TH_ECN
#define TH_ECN 0x40
#endif
#ifndef TH_CWR
#define TH_CWR 0x80
#endif
#ifndef TH_AE
#define TH_AE 0x100
#endif

#define FR_LOGBODY 0x10000
#define FR_LOGFIRST 0x20000
#define FR_LOGORBLOCK 0x40000

#define PRINTF (void)printf

typedef union ipftuneval {
	u_long ipftu_long;
	u_int ipftu_int;
	u_short ipftu_short;
	u_char ipftu_char;
} ipftuneval_t;

typedef struct ipftune {
	void *ipft_cookie;
	ipftuneval_t ipft_un;
	u_long ipft_min;
	u_long ipft_max;
	int ipft_sz;
	int ipft_flags;
	char ipft_name[80];
} ipftune_t;

#define ipft_vlong ipft_un.ipftu_long
#define ipft_vint ipft_un.ipftu_int
#define ipft_vshort ipft_un.ipftu_short
#define ipft_vchar ipft_un.ipftu_char

typedef struct frentry {
	u_32_t fr_flags;
	u_int fr_loglevel;
} frentry_t;

char flagset[] = "FSRPAUEWe";
uint16_t flags[] = { TH_FIN, TH_SYN, TH_RST, TH_PUSH, TH_ACK, TH_URG,
    TH_ECN, TH_CWR, TH_AE };

typedef struct table {
	char *name;
	int value;
} table_t;

#if LOG_CRON == (9 << 3)
#define LOG_CRON1 LOG_CRON
#define LOG_CRON2 (15 << 3)
#elif LOG_CRON == (15 << 3)
#define LOG_CRON1 (9 << 3)
#define LOG_CRON2 LOG_CRON
#else
#define LOG_CRON1 LOG_CRON
#define LOG_CRON2 LOG_CRON
#endif

table_t facs[] = {
	{ "kern", LOG_KERN }, { "user", LOG_USER },
	{ "mail", LOG_MAIL }, { "daemon", LOG_DAEMON },
	{ "auth", LOG_AUTH }, { "syslog", LOG_SYSLOG },
	{ "lpr", LOG_LPR }, { "news", LOG_NEWS },
	{ "uucp", LOG_UUCP },
#if LOG_CRON == LOG_CRON2
	{ "cron2", LOG_CRON1 },
#else
	{ "cron", LOG_CRON1 },
#endif
#ifdef LOG_FTP
	{ "ftp", LOG_FTP },
#endif
#ifdef LOG_AUTHPRIV
	{ "authpriv", LOG_AUTHPRIV },
#endif
#ifdef LOG_AUDIT
	{ "audit", LOG_AUDIT },
#endif
#ifdef LOG_LFMT
	{ "logalert", LOG_LFMT },
#endif
#if LOG_CRON == LOG_CRON1
	{ "cron", LOG_CRON2 },
#else
	{ "cron2", LOG_CRON2 },
#endif
#ifdef LOG_SECURITY
	{ "security", LOG_SECURITY },
#endif
	{ "local0", LOG_LOCAL0 }, { "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 }, { "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 }, { "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 }, { "local7", LOG_LOCAL7 },
	{ NULL, 0 }
};

table_t pris[] = {
	{ "emerg", LOG_EMERG }, { "alert", LOG_ALERT },
	{ "crit", LOG_CRIT }, { "err", LOG_ERR },
	{ "warn", LOG_WARNING }, { "notice", LOG_NOTICE },
	{ "info", LOG_INFO }, { "debug", LOG_DEBUG },
	{ NULL, 0 }
};

char *
fac_toname(int facpri)
{
	int i, j, fac;

	fac = facpri & LOG_FACMASK;
	j = fac >> 3;
	if (j < (int)(sizeof(facs) / sizeof(facs[0]))) {
		if (facs[j].value == fac)
			return (facs[j].name);
	}
	for (i = 0; facs[i].name; i++)
		if (fac == facs[i].value)
			return (facs[i].name);

	return (NULL);
}

char *
pri_toname(int facpri)
{
	int i, pri;

	pri = facpri & LOG_PRIMASK;
	if (pris[pri].value == pri)
		return (pris[pri].name);
	for (i = 0; pris[i].name; i++)
		if (pri == pris[i].value)
			return (pris[i].name);
	return (NULL);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

uint16_t
ref_tcpflags(char *flgs)
{
	uint16_t tcpf = 0;
	char *s, *t;

	for (s = flgs; *s; s++) {
		if (*s == 'W')
			tcpf |= TH_CWR;
		else {
			if (!(t = strchr(flagset, *s))) {
				return (0);
			}
			tcpf |= flags[t - flagset];
		}
	}
	return (tcpf);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
ref_printtunable(ipftune_t *tup)
{
	PRINTF("%s\tmin %lu\tmax %lu\tcurrent ",
	    tup->ipft_name, tup->ipft_min, tup->ipft_max);
	if (tup->ipft_sz == sizeof(u_long))
		PRINTF("%lu\n", tup->ipft_vlong);
	else if (tup->ipft_sz == sizeof(u_int))
		PRINTF("%u\n", tup->ipft_vint);
	else if (tup->ipft_sz == sizeof(u_short))
		PRINTF("%hu\n", tup->ipft_vshort);
	else if (tup->ipft_sz == sizeof(u_char))
		PRINTF("%u\n", (u_int)tup->ipft_vchar);
	else {
		PRINTF("sz = %d\n", tup->ipft_sz);
	}
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
ref_printlog(frentry_t *fp)
{
	char *s, *u;

	PRINTF("log");
	if (fp->fr_flags & FR_LOGBODY)
		PRINTF(" body");
	if (fp->fr_flags & FR_LOGFIRST)
		PRINTF(" first");
	if (fp->fr_flags & FR_LOGORBLOCK)
		PRINTF(" or-block");
	if (fp->fr_loglevel != 0xffff) {
		PRINTF(" level ");
		s = fac_toname(fp->fr_loglevel);
		if (s == NULL || *s == '\0')
			s = "!!!";
		u = pri_toname(fp->fr_loglevel);
		if (u == NULL || *u == '\0')
			u = "!!!";
		PRINTF("%s.%s", s, u);
	}
}
