module;

#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>

export module pbsd.sbin.ipf.libipf.b0296;

export namespace pbsd::sbin_ipf_libipf::b0296 {

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

#define OPT_NORESOLVE 0x8000000
#define OPT_DONTOPEN 0x10000000

#ifndef IPL_NAME
#define IPL_NAME "/dev/ipf"
#endif

#ifndef SIOCFUNCL
#define SIOCFUNCL _IOWR('r', 86, struct ipfunc_resolve)
#endif

typedef enum fr_ctypes_e {
	FR_NONE = 0,
	FR_EQUAL,
	FR_NEQUAL,
	FR_LESST,
	FR_GREATERT,
	FR_LESSTE,
	FR_GREATERTE,
	FR_OUTRANGE,
	FR_INRANGE,
	FR_INCRANGE
} fr_ctypes_t;

typedef struct frpcmp {
	fr_ctypes_t frp_cmp;
	u_32_t frp_port;
	u_32_t frp_top;
} frpcmp_t;

typedef struct frentry *(*ipfunc_t)(void *, u_32_t *);
typedef int (*ioctlfunc_t)(int, ioctlcmd_t, ...);

typedef struct ipfunc_resolve {
	char ipfu_name[32];
	ipfunc_t ipfu_addr;
	void *ipfu_init;
	void *ipfu_fini;
} ipfunc_resolve_t;

int opts;
int use_inet6;

void
fill6bits(int bits, u_int *msk)
{
	if (bits == 0) {
		msk[0] = 0;
		msk[1] = 0;
		msk[2] = 0;
		msk[3] = 0;
		return;
	}

	msk[0] = 0xffffffff;
	msk[1] = 0xffffffff;
	msk[2] = 0xffffffff;
	msk[3] = 0xffffffff;

	if (bits == 128)
		return;
	if (bits > 96) {
		msk[3] = htonl(msk[3] << (128 - bits));
	} else if (bits > 64) {
		msk[3] = 0;
		msk[2] = htonl(msk[2] << (96 - bits));
	} else if (bits > 32) {
		msk[3] = 0;
		msk[2] = 0;
		msk[1] = htonl(msk[1] << (64 - bits));
	} else {
		msk[3] = 0;
		msk[2] = 0;
		msk[1] = 0;
		msk[0] = htonl(msk[0] << (32 - bits));
	}
}

char *
portname(int pr, int port)
{
	static char buf[32];
	struct protoent *p = nullptr;
	struct servent *sv = nullptr;
	struct servent *sv1 = nullptr;

	if ((opts & OPT_NORESOLVE) == 0) {
		if (pr == -1) {
			if ((sv = getservbyport(htons(port), "tcp"))) {
				std::strncpy(buf, sv->s_name, sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';
				sv1 = getservbyport(htons(port), "udp");
				sv = strncasecmp(buf, sv->s_name, std::strlen(buf)) ?
				     nullptr : sv1;
			}
			if (sv)
				return (buf);
		} else if ((pr != -2) && (p = getprotobynumber(pr))) {
			if ((sv = getservbyport(htons(port), p->p_name))) {
				std::strncpy(buf, sv->s_name, sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';
				return (buf);
			}
		}
	}

	(void)std::snprintf(buf, sizeof(buf), "%d", port);
	return (buf);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

int
count4bits(u_int ip)
{
	int cnt = 0, i, j;
	u_int ipn;

	ip = ipn = ntohl(ip);
	for (i = 32; i; i--, ipn *= 2)
		if (ipn & 0x80000000)
			cnt++;
		else
			break;
	ipn = 0;
	for (i = 32, j = cnt; i; i--, j--) {
		ipn *= 2;
		if (j > 0)
			ipn++;
	}
	if (ipn == ip)
		return (cnt);
	return (-1);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

void
printportcmp(int pr, frpcmp_t *frp)
{
	static char *pcmp1[] = { "*", "=", "!=", "<", ">", "<=", ">=",
				 "<>", "><", ":" };

	if (frp->frp_cmp == FR_INRANGE || frp->frp_cmp == FR_OUTRANGE)
		PRINTF(" port %d %s %d", frp->frp_port,
			     pcmp1[frp->frp_cmp], frp->frp_top);
	else if (frp->frp_cmp == FR_INCRANGE)
		PRINTF(" port %d:%d", frp->frp_port, frp->frp_top);
	else
		PRINTF(" port %s %s", pcmp1[frp->frp_cmp],
			     portname(pr, frp->frp_port));
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

int
ntomask(int family, int nbits, u_32_t *ap)
{
	u_32_t mask;

	if (nbits < 0)
		return (-1);

	switch (family)
	{
	case AF_INET :
		if (nbits > 32 || use_inet6 == 1)
			return (-1);
		if (nbits == 0) {
			mask = 0;
		} else {
			mask = 0xffffffff;
			mask <<= (32 - nbits);
		}
		*ap = htonl(mask);
		break;

	case 0 :
	case AF_INET6 :
		if ((nbits > 128) || (use_inet6 == -1))
			return (-1);
		fill6bits(nbits, ap);
		break;

	default :
		return (-1);
	}
	return (0);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

ipfunc_t
nametokva(char *name, ioctlfunc_t iocfunc)
{
	ipfunc_resolve_t res;
	int fd;

	std::strncpy(res.ipfu_name, name, sizeof(res.ipfu_name));
	res.ipfu_addr = nullptr;
	fd = -1;

	if ((opts & OPT_DONTOPEN) == 0) {
		fd = open(IPL_NAME, O_RDONLY);
		if (fd == -1)
			return (nullptr);
	}
	(void) (*iocfunc)(fd, SIOCFUNCL, &res);
	if (fd >= 0)
		close(fd);
	if (res.ipfu_addr == nullptr)
		res.ipfu_addr = (ipfunc_t)-1;
	return (res.ipfu_addr);
}

} // namespace pbsd::sbin_ipf_libipf::b0296
