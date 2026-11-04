/*
 * port.cppm -- PBSD batch b0294.
 *
 * C++23 module port of:
 *   hbsd/src/sbin/ipf/libipf/printactiveaddr.c
 *   hbsd/src/sbin/ipf/libipf/verbose.c
 *   hbsd/src/sbin/ipf/libipf/tcpoptnames.c
 *   hbsd/src/sbin/ipf/libipf/printsbuf.c
 *
 * printsbuf.c is built without IPFILTER_SCAN (the default libipf configuration).
 */

module;

#include <arpa/inet.h>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>

#define	USE_INET6	1

#undef	AF_INET
#define	AF_INET		2
#undef	AF_INET6
#define	AF_INET6	28

export module pbsd.sbin.ipf.libipf.b0294;

export namespace pbsd::sbin_ipf_libipf::b0294 {

typedef	std::uint32_t	u_32_t;

#define	PRINTF	(void)std::printf

#define	OPT_VERBOSE	0x000040

enum {
	FRI_NORMAL = 0
};

struct	ipopt_names	{
	int	on_value;
	int	on_bit;
	int	on_siz;
	char	*on_name;
};

typedef	union	i6addr	{
	u_32_t	i6[4];
	struct	in_addr	in4;
#ifdef	USE_INET6
	struct	in6_addr	in6;
#endif
} i6addr_t;

/* <netinet/tcp.h> */
#define	TCPOPT_NOP		1
#define	TCPOPT_MAXSEG		2
#define	TCPOPT_WINDOW		3
#define	TCPOPT_SACK_PERMITTED	4
#define	TCPOPT_SACK		5
#define	TCPOPT_TIMESTAMP	8

int	opts;

#define	ISPRINT(x)	std::isprint((unsigned char)(x))

void
printaddr(int family, int type, char *base, int ifidx, u_32_t *addr,
    u_32_t *mask)
{
	(void)mask;
	if (type == FRI_NORMAL) {
		PRINTF("<pa:%d:", family);
		if (family == AF_INET6) {
			PRINTF("%08x:%08x:%08x:%08x", addr[0], addr[1],
			    addr[2], addr[3]);
		} else {
			PRINTF("%08x", addr[0]);
		}
		if (base != nullptr)
			PRINTF(":%s", base);
		PRINTF(">");
	}
	(void)ifidx;
}

/* ====================================================================== */
/* printactiveaddr.c */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * Added redirect stuff and a variety of bug fixes. (mcn@EnGarde.com)
 */

void
printactiveaddress(int v, char *fmt, i6addr_t *addr, char *ifname)
{
	switch (v)
	{
	case 4 :
		PRINTF(fmt, inet_ntoa(addr->in4));
		break;
#ifdef USE_INET6
	case 6 :
		printaddr(AF_INET6, FRI_NORMAL, ifname, 0,
			  (u_32_t *)&addr->in6, NULL);
		break;
#endif
	default :
		break;
	}
}

/* ====================================================================== */
/* verbose.c */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void	verbose(int level, char *fmt, ...);

void
verbose(int level, char *fmt, ...)
{
	va_list pvar;

	va_start(pvar, fmt);

	if (opts & OPT_VERBOSE)
		vprintf(fmt, pvar);
	va_end(pvar);
}


void
ipfkverbose(char *fmt, ...)
{
	va_list pvar;

	va_start(pvar, fmt);

	if (opts & OPT_VERBOSE)
		verbose(0x1fffffff, fmt, pvar);
	va_end(pvar);
}

/* ====================================================================== */
/* tcpoptnames.c */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

struct	ipopt_names	tcpoptnames[] ={
	{ TCPOPT_NOP,			0x000001,	1,	(char *)"nop" },
	{ TCPOPT_MAXSEG,		0x000002,	4,	(char *)"maxseg" },
	{ TCPOPT_WINDOW,		0x000004,	3,	(char *)"wscale" },
	{ TCPOPT_SACK_PERMITTED,	0x000008,	2,	(char *)"sackok" },
	{ TCPOPT_SACK,			0x000010,	3,	(char *)"sack" },
	{ TCPOPT_TIMESTAMP,		0x000020,	10,	(char *)"tstamp" },
	{ 0, 		0,	0,	(char *)nullptr }     /* must be last */
};

/* ====================================================================== */
/* printsbuf.c (without IPFILTER_SCAN) */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void printsbuf(char *buf);

void
printsbuf(char *buf)
{
#if 0
	buf = buf;	/* gcc -Wextra */
#endif
}

} // namespace pbsd::sbin_ipf_libipf::b0294
