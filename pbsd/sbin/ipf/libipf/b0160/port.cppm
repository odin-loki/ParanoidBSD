module;

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <strings.h>

export module pbsd.sbin.ipf.libipf.b0160;

export namespace pbsd::sbin_ipf_libipf::b0160 {

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#define AF_INET 2
#define AF_INET6 28

inline constexpr int MAX_ICMPCODE = 16;

typedef std::uint32_t u_32_t;

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

char *icmpcodes[MAX_ICMPCODE + 1] = {
	(char *)"net-unr", (char *)"host-unr", (char *)"proto-unr",
	(char *)"port-unr", (char *)"needfrag", (char *)"srcfail",
	(char *)"net-unk", (char *)"host-unk", (char *)"isolate",
	(char *)"net-prohib", (char *)"host-prohib", (char *)"net-tos",
	(char *)"host-tos", (char *)"filter-prohib", (char *)"host-preced",
	(char *)"preced-cutoff", nullptr
};

icmptype_t icmptypelist[] = {
	{ (char *)"echo", ICMP_ECHO, 0 },
	{ (char *)"echorep", ICMP_ECHOREPLY, 0 },
	{ (char *)"fqdnquery", -1, 0 },
	{ (char *)"fqdnreply", -1, 0 },
	{ (char *)"infoqry", -1, 0 },
	{ (char *)"inforeq", ICMP_IREQ, 0 },
	{ (char *)"inforep", ICMP_IREQREPLY, 0 },
	{ (char *)"listendone", -1, 0 },
	{ (char *)"listenqry", -1, 0 },
	{ (char *)"listenrep", -1, 0 },
	{ (char *)"maskrep", ICMP_MASKREPLY, -1 },
	{ (char *)"maskreq", ICMP_MASKREQ, -1 },
	{ (char *)"memberqry", -1, 0 },
	{ (char *)"memberred", -1, 0 },
	{ (char *)"memberreply", -1, 0 },
	{ (char *)"neighadvert", -1, 0 },
	{ (char *)"neighborsol", -1, 0 },
	{ (char *)"neighborsolicit", -1, 0 },
	{ (char *)"paramprob", ICMP_PARAMPROB, 0 },
	{ (char *)"redir", ICMP_REDIRECT, 0 },
	{ (char *)"renumber", -1, 0 },
	{ (char *)"routerad", ICMP_ROUTERADVERT, 0 },
	{ (char *)"routeradvert", ICMP_ROUTERADVERT, 0 },
	{ (char *)"routersol", ICMP_ROUTERSOLICIT, 0 },
	{ (char *)"routersolcit", ICMP_ROUTERSOLICIT, 0 },
	{ (char *)"squench", ICMP_SOURCEQUENCH, -1 },
	{ (char *)"timest", ICMP_TSTAMP, -1 },
	{ (char *)"timestrep", ICMP_TSTAMPREPLY, -1 },
	{ (char *)"timex", ICMP_TIMXCEED, 0 },
	{ (char *)"toobig", -1, 0 },
	{ (char *)"unreach", ICMP_UNREACH, 0 },
	{ (char *)"whorep", -1, 0 },
	{ (char *)"whoreq", -1, 0 },
	{ nullptr, -1, -1 }
};

struct ipopt_names ionames[] = {
	{ IPOPT_NOP, 0x000001, 1, (char *)"nop" },
	{ IPOPT_RR, 0x000002, 8, (char *)"rr" },
	{ IPOPT_ZSU, 0x000004, 4, (char *)"zsu" },
	{ IPOPT_MTUP, 0x000008, 4, (char *)"mtup" },
	{ IPOPT_MTUR, 0x000010, 4, (char *)"mtur" },
	{ IPOPT_ENCODE, 0x000020, 4, (char *)"encode" },
	{ IPOPT_TS, 0x000040, 8, (char *)"ts" },
	{ IPOPT_TR, 0x000080, 4, (char *)"tr" },
	{ IPOPT_SECURITY, 0x000100, 12, (char *)"sec" },
	{ IPOPT_SECURITY, 0x000100, 12, (char *)"sec-class" },
	{ IPOPT_LSRR, 0x000200, 8, (char *)"lsrr" },
	{ IPOPT_E_SEC, 0x000400, 8, (char *)"e-sec" },
	{ IPOPT_CIPSO, 0x000800, 8, (char *)"cipso" },
	{ IPOPT_SATID, 0x001000, 4, (char *)"satid" },
	{ IPOPT_SSRR, 0x002000, 8, (char *)"ssrr" },
	{ IPOPT_ADDEXT, 0x004000, 4, (char *)"addext" },
	{ IPOPT_VISA, 0x008000, 4, (char *)"visa" },
	{ IPOPT_IMITD, 0x010000, 4, (char *)"imitd" },
	{ IPOPT_EIP, 0x020000, 4, (char *)"eip" },
	{ IPOPT_FINN, 0x040000, 4, (char *)"finn" },
	{ IPOPT_DPS, 0x080000, 4, (char *)"dps" },
	{ IPOPT_SDB, 0x100000, 4, (char *)"sdb" },
	{ IPOPT_NSAPA, 0x200000, 4, (char *)"nsapa" },
	{ IPOPT_RTRALRT, 0x400000, 4, (char *)"rtralrt" },
	{ IPOPT_UMP, 0x800000, 4, (char *)"ump" },
	{ IPOPT_AH, 0x1000000, 0, (char *)"ah" },
	{ 0, 0, 0, (char *)nullptr }
};

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: dupmbt.c,v 1.3.2.2 2012/07/22 08:04:24 darren_r Exp $
 */

mb_t *
dupmbt(mb_t *orig)
{
	mb_t *m;

	m = (mb_t *)std::malloc(sizeof(mb_t));
	if (m == nullptr)
		return (nullptr);
	m->mb_len = orig->mb_len;
	m->mb_next = nullptr;
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
icmptypename(int family, int type)
{
	icmptype_t *i;

	if ((type < 0) || (type > 255))
		return (nullptr);

	for (i = icmptypelist; i->it_name != nullptr; i++) {
		if ((family == AF_INET) && (i->it_v4 == type))
			return (i->it_name);
#ifdef USE_INET6
		if ((family == AF_INET6) && (i->it_v6 == type))
			return (i->it_name);
#endif
	}

	return (nullptr);
}

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

u_32_t
getoptbyname(char *optname)
{
	struct ipopt_names *io;

	for (io = ionames; io->on_name; io++)
		if (!strcasecmp(optname, io->on_name))
			return (io->on_bit);
	return ((u_32_t)-1);
}

u_32_t
getoptbyvalue(int optval)
{
	struct ipopt_names *io;

	for (io = ionames; io->on_name; io++)
		if (io->on_value == optval)
			return (io->on_bit);
	return ((u_32_t)-1);
}

} // namespace pbsd::sbin_ipf_libipf::b0160
