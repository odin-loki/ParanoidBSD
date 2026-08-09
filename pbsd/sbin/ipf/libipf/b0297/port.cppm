module;

#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

export module pbsd.sbin.ipf.libipf.b0297;

export namespace pbsd::sbin_ipf_libipf::b0297 {

typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef std::uint32_t u_32_t;
typedef u_long ioctlcmd_t;

#undef AF_INET
#define AF_INET 2
#undef AF_INET6
#define AF_INET6 28

#define PRINTF (void)std::printf

#define OPT_DONTOPEN 0x10000000

#ifndef IPL_NAME
#define IPL_NAME "/dev/ipf"
#endif

#ifndef SIOCFUNCL
#define SIOCFUNCL _IOWR('r', 86, struct ipfunc_resolve)
#endif

#ifndef SIOCIPFINTERROR
#define SIOCIPFINTERROR _IOR('r', 98, int)
#endif

#define IPL_LOGIPF 0
#define IPL_LOGNAT 1
#define IPL_LOGSTATE 2
#define IPL_LOGAUTH 3
#define IPL_LOGSYNC 4
#define IPL_LOGSCAN 5
#define IPL_LOGLOOKUP 6
#define IPL_LOGCOUNT 7
#define IPL_LOGALL (-1)

typedef union i6addr {
	u_32_t i6[4];
	struct in_addr in4;
} i6addr_t;

#define in4_addr in4.s_addr

typedef struct ipfr {
	struct ipfr *ipfr_hnext, **ipfr_hprev;
	struct ipfr *ipfr_next, **ipfr_prev;
	void *ipfr_data;
	void *ipfr_rule;
	u_long ipfr_ttl;
	u_int ipfr_pkts;
	u_int ipfr_bytes;
	u_int ipfr_badorder;
	int ipfr_ref;
	u_short ipfr_off;
	u_short ipfr_firstend;
	u_char ipfr_p;
	u_char ipfr_seen0;
	void *ipfr_ifp;
	i6addr_t ipfr_source;
	i6addr_t ipfr_dest;
	u_32_t ipfr_optmsk;
	u_short ipfr_secmsk;
	u_short ipfr_auth;
	u_32_t ipfr_id;
	u_32_t ipfr_pass;
	int ipfr_v;
} ipfr_t;

#define ipfr_src ipfr_source.in4
#define ipfr_dst ipfr_dest.in4

typedef struct frentry *(*ipfunc_t)(void *, u_32_t *);
typedef int (*ioctlfunc_t)(int, ioctlcmd_t, ...);

typedef struct ipfunc_resolve {
	char ipfu_name[32];
	ipfunc_t ipfu_addr;
	void *ipfu_init;
	void *ipfu_fini;
} ipfunc_resolve_t;

int opts;

char *
hostname(int family, void *ip)
{
	static char hostbuf[257];
	struct in_addr ipa;

	std::memset(&ipa, 0, sizeof(ipa));

	if (family == AF_INET) {
		ipa.s_addr = *(u_32_t *)ip;
		if (ipa.s_addr == htonl(0xfedcba98))
			return ("test.host.dots");
	}

	if (family != AF_INET) {
		return (inet_ntoa(ipa));
	}
	return ("IPv6");
}

char *
ipf_strerror(int errnum)
{
	static char text[80];

	std::snprintf(text, sizeof(text), "unknown error %d", errnum);
	return (text);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
printfraginfo(char *prefix, struct ipfr *ifr)
{
	int family;

	PRINTF("%s", prefix);
	if (ifr->ipfr_v == 6) {
		PRINTF("inet6");
		family = AF_INET6;
	} else {
		PRINTF("inet");
		family = AF_INET;
	}

	PRINTF(" %s -> ", hostname(family, &ifr->ipfr_src));
	PRINTF("%s id %x ttl %lu pr %d pkts %u bytes %u seen0 %d ref %d\n",
		hostname(family, &ifr->ipfr_dst), ifr->ipfr_id,
		ifr->ipfr_ttl, ifr->ipfr_p, ifr->ipfr_pkts, ifr->ipfr_bytes,
		ifr->ipfr_seen0, ifr->ipfr_ref);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

char *
kvatoname(ipfunc_t func, ioctlfunc_t iocfunc)
{
	static char funcname[40];
	ipfunc_resolve_t res;
	int fd;

	res.ipfu_addr = func;
	res.ipfu_name[0] = '\0';
	fd = -1;

	if ((opts & OPT_DONTOPEN) == 0) {
		fd = open(IPL_NAME, O_RDONLY);
		if (fd == -1)
			return (nullptr);
	}
	(void) (*iocfunc)(fd, SIOCFUNCL, &res);
	if (fd >= 0)
		close(fd);
	std::strncpy(funcname, res.ipfu_name, sizeof(funcname));
	funcname[sizeof(funcname) - 1] = '\0';
	return (funcname);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
printunit(int unit)
{

	switch (unit)
	{
	case IPL_LOGIPF :
		PRINTF("ipf");
		break;
	case IPL_LOGNAT :
		PRINTF("nat");
		break;
	case IPL_LOGSTATE :
		PRINTF("state");
		break;
	case IPL_LOGAUTH :
		PRINTF("auth");
		break;
	case IPL_LOGSYNC :
		PRINTF("sync");
		break;
	case IPL_LOGSCAN :
		PRINTF("scan");
		break;
	case IPL_LOGLOOKUP :
		PRINTF("lookup");
		break;
	case IPL_LOGCOUNT :
		PRINTF("count");
		break;
	case IPL_LOGALL :
		PRINTF("all");
		break;
	default :
		PRINTF("unknown(%d)", unit);
	}
}

void
ipf_perror(int err, char *string)
{
	if (err == 0)
		std::fprintf(stderr, "%s\n", string);
	else
		std::fprintf(stderr, "%s: %s\n", string, ipf_strerror(err));
}

int
ipf_perror_fd(int fd, ioctlfunc_t iocfunc, char *string)
{
	int save;
	int realerr;

	save = errno;
	if ((*iocfunc)(fd, SIOCIPFINTERROR, &realerr) == -1)
		realerr = 0;

	errno = save;
	std::fprintf(stderr, "%d:", realerr);
	ipf_perror(realerr, string);
	return (realerr ? realerr : save);

}

void
ipferror(int fd, char *msg)
{
	if (fd >= 0) {
		ipf_perror_fd(fd, ioctl, msg);
	} else {
		std::fprintf(stderr, "0:");
		perror(msg);
	}
}

} // namespace pbsd::sbin_ipf_libipf::b0297
