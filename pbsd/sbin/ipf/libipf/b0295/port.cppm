module;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

export module pbsd.sbin.ipf.libipf.b0295;

export namespace pbsd::sbin_ipf_libipf::b0295 {

#define PRINTF (void)std::printf

typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef std::uint32_t u_32_t;

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

extern "C" char flagset[];
extern "C" std::uint16_t flags[];
extern "C" char *fac_toname(int);
extern "C" char *pri_toname(int);

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

std::uint16_t
tcpflags(char *flgs)
{
	std::uint16_t tcpf = 0;
	char *s, *t;

	for (s = flgs; *s; s++) {
		if (*s == 'W')
			tcpf |= TH_CWR;
		else {
			if (!(t = std::strchr(flagset, *s))) {
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
printtunable(ipftune_t *tup)
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
printlog(frentry_t *fp)
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
			s = (char *)"!!!";
		u = pri_toname(fp->fr_loglevel);
		if (u == NULL || *u == '\0')
			u = (char *)"!!!";
		PRINTF("%s.%s", s, u);
	}
}

} // namespace pbsd::sbin_ipf_libipf::b0295
