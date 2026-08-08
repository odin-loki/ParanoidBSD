/*
 * oracle.c -- concatenated reference implementations for PBSD batch b0160.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#define AF_INET 2
#define AF_INET6 28

#define MAX_ICMPCODE 16

typedef uint32_t u_32_t;

typedef struct mb_s {
	struct mb_s *mb_next;
	char *mb_data;
	void *mb_ifp;
	int mb_len;
	int mb_flags;
	unsigned long mb_buf[2048];
} mb_t;

struct ipopt_names {
	int on_value;
	int on_bit;
	int on_siz;
	char *on_name;
};

typedef struct {
	char *it_name;
	int it_v4;
	int it_v6;
} icmptype_t;

#define ICMP_ECHOREPLY 0
#define ICMP_UNREACH 3
#define ICMP_SOURCEQUENCH 4
#define ICMP_REDIRECT 5
#define ICMP_ECHO 8
#define ICMP_ROUTERADVERT 9
#define ICMP_ROUTERSOLICIT 10
#define ICMP_TIMXCEED 11
#define ICMP_PARAMPROB 12
#define ICMP_TSTAMP 13
#define ICMP_TSTAMPREPLY 14
#define ICMP_IREQ 15
#define ICMP_IREQREPLY 16
#define ICMP_MASKREQ 17
#define ICMP_MASKREPLY 18

#define IPOPT_NOP 1
#define IPOPT_RR 7
#define IPOPT_ZSU 10
#define IPOPT_MTUP 11
#define IPOPT_MTUR 12
#define IPOPT_ENCODE 15
#define IPOPT_TS 68
#define IPOPT_TR 82
#define IPOPT_SECURITY 130
#define IPOPT_LSRR 131
#define IPOPT_E_SEC 133
#define IPOPT_CIPSO 134
#define IPOPT_SATID 136
#define IPOPT_SSRR 137
#define IPOPT_ADDEXT 147
#define IPOPT_VISA 142
#define IPOPT_IMITD 144
#define IPOPT_EIP 145
#define IPOPT_FINN 205
#define IPOPT_DPS 151
#define IPOPT_SDB 149
#define IPOPT_NSAPA 150
#define IPOPT_RTRALRT 148
#define IPOPT_UMP 152
#define IPOPT_AH 307

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

char *ref_icmpcodes[MAX_ICMPCODE + 1] = {
	"net-unr", "host-unr", "proto-unr", "port-unr", "needfrag", "srcfail",
	"net-unk", "host-unk", "isolate", "net-prohib", "host-prohib",
	"net-tos", "host-tos", "filter-prohib", "host-preced", "preced-cutoff",
	NULL };

icmptype_t ref_icmptypelist[] = {
	{ "echo", ICMP_ECHO, 0 },
	{ "echorep", ICMP_ECHOREPLY, 0 },
	{ "fqdnquery", -1, 0 },
	{ "fqdnreply", -1, 0 },
	{ "infoqry", -1, 0 },
	{ "inforeq", ICMP_IREQ, 0 },
	{ "inforep", ICMP_IREQREPLY, 0 },
	{ "listendone", -1, 0 },
	{ "listenqry", -1, 0 },
	{ "listenrep", -1, 0 },
	{ "maskrep", ICMP_MASKREPLY, -1 },
	{ "maskreq", ICMP_MASKREQ, -1 },
	{ "memberqry", -1, 0 },
	{ "memberred", -1, 0 },
	{ "memberreply", -1, 0 },
	{ "neighadvert", -1, 0 },
	{ "neighborsol", -1, 0 },
	{ "neighborsolicit", -1, 0 },
	{ "paramprob", ICMP_PARAMPROB, 0 },
	{ "redir", ICMP_REDIRECT, 0 },
	{ "renumber", -1, 0 },
	{ "routerad", ICMP_ROUTERADVERT, 0 },
	{ "routeradvert", ICMP_ROUTERADVERT, 0 },
	{ "routersol", ICMP_ROUTERSOLICIT, 0 },
	{ "routersolcit", ICMP_ROUTERSOLICIT, 0 },
	{ "squench", ICMP_SOURCEQUENCH, -1 },
	{ "timest", ICMP_TSTAMP, -1 },
	{ "timestrep", ICMP_TSTAMPREPLY, -1 },
	{ "timex", ICMP_TIMXCEED, 0 },
	{ "toobig", -1, 0 },
	{ "unreach", ICMP_UNREACH, 0 },
	{ "whorep", -1, 0 },
	{ "whoreq", -1, 0 },
	{ NULL, -1, -1 }
};

struct ipopt_names ref_ionames[] = {
	{ IPOPT_NOP, 0x000001, 1, "nop" },
	{ IPOPT_RR, 0x000002, 8, "rr" },
	{ IPOPT_ZSU, 0x000004, 4, "zsu" },
	{ IPOPT_MTUP, 0x000008, 4, "mtup" },
	{ IPOPT_MTUR, 0x000010, 4, "mtur" },
	{ IPOPT_ENCODE, 0x000020, 4, "encode" },
	{ IPOPT_TS, 0x000040, 8, "ts" },
	{ IPOPT_TR, 0x000080, 4, "tr" },
	{ IPOPT_SECURITY, 0x000100, 12, "sec" },
	{ IPOPT_SECURITY, 0x000100, 12, "sec-class" },
	{ IPOPT_LSRR, 0x000200, 8, "lsrr" },
	{ IPOPT_E_SEC, 0x000400, 8, "e-sec" },
	{ IPOPT_CIPSO, 0x000800, 8, "cipso" },
	{ IPOPT_SATID, 0x001000, 4, "satid" },
	{ IPOPT_SSRR, 0x002000, 8, "ssrr" },
	{ IPOPT_ADDEXT, 0x004000, 4, "addext" },
	{ IPOPT_VISA, 0x008000, 4, "visa" },
	{ IPOPT_IMITD, 0x010000, 4, "imitd" },
	{ IPOPT_EIP, 0x020000, 4, "eip" },
	{ IPOPT_FINN, 0x040000, 4, "finn" },
	{ IPOPT_DPS, 0x080000, 4, "dps" },
	{ IPOPT_SDB, 0x100000, 4, "sdb" },
	{ IPOPT_NSAPA, 0x200000, 4, "nsapa" },
	{ IPOPT_RTRALRT, 0x400000, 4, "rtralrt" },
	{ IPOPT_UMP, 0x800000, 4, "ump" },
	{ IPOPT_AH, 0x1000000, 0, "ah" },
	{ 0, 0, 0, (char *)NULL }
};

#define icmptypelist ref_icmptypelist
#define ionames ref_ionames

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: dupmbt.c,v 1.3.2.2 2012/07/22 08:04:24 darren_r Exp $
 */

mb_t *
ref_dupmbt(mb_t *orig)
{
	mb_t *m;

	m = (mb_t *)malloc(sizeof(mb_t));
	if (m == NULL)
		return (NULL);
	m->mb_len = orig->mb_len;
	m->mb_next = NULL;
	m->mb_data = (char *)m->mb_buf + (orig->mb_data - (char *)orig->mb_buf);
	bcopy(orig->mb_data, m->mb_data, m->mb_len);
	return (m);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */
char *
ref_icmptypename(int family, int type)
{
	icmptype_t *i;

	if ((type < 0) || (type > 255))
		return (NULL);

	for (i = icmptypelist; i->it_name != NULL; i++) {
		if ((family == AF_INET) && (i->it_v4 == type))
			return (i->it_name);
#ifdef USE_INET6
		if ((family == AF_INET6) && (i->it_v6 == type))
			return (i->it_name);
#endif
	}

	return (NULL);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

u_32_t
ref_getoptbyname(char *optname)
{
	struct ipopt_names *io;

	for (io = ionames; io->on_name; io++)
		if (!strcasecmp(optname, io->on_name))
			return (io->on_bit);
	return (-1);
}

u_32_t
ref_getoptbyvalue(int optval)
{
	struct ipopt_names *io;

	for (io = ionames; io->on_name; io++)
		if (io->on_value == optval)
			return (io->on_bit);
	return (-1);
}
