/*
 * port.cppm -- PBSD batch b0160.
 *
 * C++23 module port of:
 *   sbin/ipf/libipf/icmpcode.c
 *   sbin/ipf/libipf/dupmbt.c
 *   sbin/ipf/libipf/icmptypename.c
 *   sbin/ipf/libipf/optvalue.c
 *
 * The behaviour of the originals is preserved exactly, including the signed
 * `int` table fields, the `u_32_t` functions that return a plain -1, and the
 * pointer arithmetic in dupmbt().  The only edits are the ones C++ forces:
 * string literals stored in `char *` table fields need an explicit cast, and
 * NULL is spelled nullptr.
 *
 * The two tables the batch functions index (icmptypelist from icmptypes.c and
 * ionames from ionames.c) are link dependencies rather than part of this
 * batch; they are reproduced here with the values the FreeBSD/HardenedBSD
 * build produces so the ported functions behave identically.  USE_INET6 is
 * defined because sbin/ipf/Makefile.inc adds -DUSE_INET6.
 */

module;

#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define	USE_INET6	1

/* <sys/socket.h>, FreeBSD values. */
#undef	AF_INET
#define	AF_INET		2
#undef	AF_INET6
#define	AF_INET6	28

#ifndef	MIN
# define	MIN(a,b)	((a) > (b) ? (b) : (a))
#endif

/* <netinet/ip_icmp.h> */
#define	ICMP_ECHOREPLY		0
#define	ICMP_UNREACH		3
#define	ICMP_SOURCEQUENCH	4
#define	ICMP_REDIRECT		5
#define	ICMP_ECHO		8
#define	ICMP_ROUTERADVERT	9
#define	ICMP_ROUTERSOLICIT	10
#define	ICMP_TIMXCEED		11
#define	ICMP_PARAMPROB		12
#define	ICMP_TSTAMP		13
#define	ICMP_TSTAMPREPLY	14
#define	ICMP_IREQ		15
#define	ICMP_IREQREPLY		16
#define	ICMP_MASKREQ		17
#define	ICMP_MASKREPLY		18

/* <netinet/icmp6.h> */
#define	ICMP6_DST_UNREACH		1
#define	ICMP6_PACKET_TOO_BIG		2
#define	ICMP6_TIME_EXCEEDED		3
#define	ICMP6_PARAM_PROB		4
#define	ICMP6_ECHO_REQUEST		128
#define	ICMP6_ECHO_REPLY		129
#define	MLD_LISTENER_QUERY		130
#define	MLD_LISTENER_REPORT		131
#define	MLD_LISTENER_DONE		132
#define	MLD_LISTENER_REDUCTION		MLD_LISTENER_DONE
#define	ICMP6_MEMBERSHIP_QUERY		130
#define	ICMP6_MEMBERSHIP_REPORT		131
#define	ICMP6_MEMBERSHIP_REDUCTION	132
#define	ND_ROUTER_SOLICIT		133
#define	ND_ROUTER_ADVERT		134
#define	ND_NEIGHBOR_SOLICIT		135
#define	ND_NEIGHBOR_ADVERT		136
#define	ND_REDIRECT			137
#define	ICMP6_ROUTER_RENUMBERING	138
#define	ICMP6_WRUREQUEST		139
#define	ICMP6_WRUREPLY			140
#define	ICMP6_FQDN_QUERY		139
#define	ICMP6_FQDN_REPLY		140
#define	ICMP6_NI_QUERY			139
#define	ICMP6_NI_REPLY			140

/* <netinet/in.h> */
#ifndef	IPPROTO_AH
# define	IPPROTO_AH	51
#endif

/* <netinet/ip.h> plus the "netinet/ip_compat.h" overrides */
#define	IPOPT_NOP	1
#define	IPOPT_RR	7
#define	IPOPT_ZSU	10	/* ZSU */
#define	IPOPT_MTUP	11	/* MTUP */
#define	IPOPT_MTUR	12	/* MTUR */
#define	IPOPT_ENCODE	15	/* ENCODE */
#define	IPOPT_TS	68
#define	IPOPT_TR	82	/* TR */
#define	IPOPT_SECURITY	130
#define	IPOPT_LSRR	131
#define	IPOPT_E_SEC	133	/* E-SEC */
#define	IPOPT_CIPSO	134	/* CIPSO */
#define	IPOPT_SATID	136
#define	IPOPT_SSRR	137
#define	IPOPT_ADDEXT	147	/* ADDEXT */
#define	IPOPT_VISA	142	/* VISA */
#define	IPOPT_IMITD	144	/* IMITD */
#define	IPOPT_EIP	145	/* EIP */
#define	IPOPT_RTRALRT	148	/* RTRALRT */
#define	IPOPT_SDB	149
#define	IPOPT_NSAPA	150
#define	IPOPT_DPS	151
#define	IPOPT_UMP	152
#define	IPOPT_FINN	205	/* FINN */
#define	IPOPT_AH	256+IPPROTO_AH

export module pbsd.sbin.ipf.libipf.b0160;

export namespace pbsd::sbin_ipf_libipf::b0160 {

/* "ipf.h" */
typedef	std::uint32_t	u_32_t;

inline constexpr int MAX_ICMPCODE = 16;
inline constexpr int MAX_ICMPTYPE = 19;

struct	ipopt_names	{
	int	on_value;
	int	on_bit;
	int	on_siz;
	char	*on_name;
};

typedef struct  {
	char	*it_name;
	int	it_v4;
	int	it_v6;
} icmptype_t;

/* "netinet/ip_compat.h" */
typedef	struct	mb_s	{
	struct	mb_s	*mb_next;
	char		*mb_data;
	void		*mb_ifp;
	int		mb_len;
	int		mb_flags;
	unsigned long	mb_buf[2048];
} mb_t;


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/icmpcode.c						  */
/* ---------------------------------------------------------------------- */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

char	*icmpcodes[MAX_ICMPCODE + 1] = {
	(char *)"net-unr", (char *)"host-unr", (char *)"proto-unr",
	(char *)"port-unr", (char *)"needfrag", (char *)"srcfail",
	(char *)"net-unk", (char *)"host-unk", (char *)"isolate",
	(char *)"net-prohib", (char *)"host-prohib",
	(char *)"net-tos", (char *)"host-tos", (char *)"filter-prohib",
	(char *)"host-preced", (char *)"preced-cutoff",
	nullptr };


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/icmptypes.c  (link dependency of icmptypename)	  */
/* ---------------------------------------------------------------------- */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

icmptype_t icmptypelist[] = {
	{ (char *)"echo",	ICMP_ECHO,		ICMP6_ECHO_REQUEST },
	{ (char *)"echorep",	ICMP_ECHOREPLY,		ICMP6_ECHO_REPLY },
	{ (char *)"fqdnquery",	-1,			ICMP6_FQDN_QUERY },
	{ (char *)"fqdnreply",	-1,			ICMP6_FQDN_REPLY },
	{ (char *)"infoqry",	-1,			ICMP6_NI_QUERY },
	{ (char *)"inforeq",	ICMP_IREQ,		ICMP6_NI_QUERY },
	{ (char *)"inforep",	ICMP_IREQREPLY,		ICMP6_NI_REPLY },
	{ (char *)"listendone",	-1,			MLD_LISTENER_DONE },
	{ (char *)"listenqry",	-1,			MLD_LISTENER_QUERY },
	{ (char *)"listenrep",	-1,			MLD_LISTENER_REPORT },
	{ (char *)"maskrep",	ICMP_MASKREPLY,		-1 },
	{ (char *)"maskreq",	ICMP_MASKREQ,		-1 },
	{ (char *)"memberqry",	-1,			ICMP6_MEMBERSHIP_QUERY },
	{ (char *)"memberred",	-1,		ICMP6_MEMBERSHIP_REDUCTION },
	{ (char *)"memberreply",-1,			ICMP6_MEMBERSHIP_REPORT },
	{ (char *)"neighadvert",	-1,		ND_NEIGHBOR_ADVERT },
	{ (char *)"neighborsol",	-1,		ND_NEIGHBOR_SOLICIT },
	{ (char *)"neighborsolicit",	-1,		ND_NEIGHBOR_SOLICIT },
	{ (char *)"paramprob",	ICMP_PARAMPROB,		ICMP6_PARAM_PROB },
	{ (char *)"redir",	ICMP_REDIRECT,		ND_REDIRECT },
	{ (char *)"renumber",	-1,			ICMP6_ROUTER_RENUMBERING },
	{ (char *)"routerad",	ICMP_ROUTERADVERT,	ND_ROUTER_ADVERT },
	{ (char *)"routeradvert",ICMP_ROUTERADVERT,	ND_ROUTER_ADVERT },
	{ (char *)"routersol",	ICMP_ROUTERSOLICIT,	ND_ROUTER_SOLICIT },
	{ (char *)"routersolcit",ICMP_ROUTERSOLICIT,	ND_ROUTER_SOLICIT },
	{ (char *)"squench",	ICMP_SOURCEQUENCH,	-1 },
	{ (char *)"timest",	ICMP_TSTAMP,		-1 },
	{ (char *)"timestrep",	ICMP_TSTAMPREPLY,	-1 },
	{ (char *)"timex",	ICMP_TIMXCEED,		ICMP6_TIME_EXCEEDED },
	{ (char *)"toobig",	-1,			ICMP6_PACKET_TOO_BIG },
	{ (char *)"unreach",	ICMP_UNREACH,		ICMP6_DST_UNREACH },
	{ (char *)"whorep",	-1,			ICMP6_WRUREPLY },
	{ (char *)"whoreq",	-1,			ICMP6_WRUREQUEST },
	{ nullptr,		-1,			-1 }
};


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/ionames.c  (link dependency of getoptby*)		  */
/* ---------------------------------------------------------------------- */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

struct	ipopt_names	ionames[] ={
	{ IPOPT_NOP,	0x000001,	1,	(char *)"nop" },
	{ IPOPT_RR,	0x000002,	8,	(char *)"rr" },
	{ IPOPT_ZSU,	0x000004,	4,	(char *)"zsu" },
	{ IPOPT_MTUP,	0x000008,	4,	(char *)"mtup" },
	{ IPOPT_MTUR,	0x000010,	4,	(char *)"mtur" },
	{ IPOPT_ENCODE,	0x000020,	4,	(char *)"encode" },
	{ IPOPT_TS,	0x000040,	8,	(char *)"ts" },
	{ IPOPT_TR,	0x000080,	4,	(char *)"tr" },
	{ IPOPT_SECURITY,0x000100,	12,	(char *)"sec" },
	{ IPOPT_SECURITY,0x000100,	12,	(char *)"sec-class" },
	{ IPOPT_LSRR,	0x000200,	8,	(char *)"lsrr" },
	{ IPOPT_E_SEC,	0x000400,	8,	(char *)"e-sec" },
	{ IPOPT_CIPSO,	0x000800,	8,	(char *)"cipso" },
	{ IPOPT_SATID,	0x001000,	4,	(char *)"satid" },
	{ IPOPT_SSRR,	0x002000,	8,	(char *)"ssrr" },
	{ IPOPT_ADDEXT,	0x004000,	4,	(char *)"addext" },
	{ IPOPT_VISA,	0x008000,	4,	(char *)"visa" },
	{ IPOPT_IMITD,	0x010000,	4,	(char *)"imitd" },
	{ IPOPT_EIP,	0x020000,	4,	(char *)"eip" },
	{ IPOPT_FINN,	0x040000,	4,	(char *)"finn" },
	{ IPOPT_DPS,	0x080000,	4,	(char *)"dps" },
	{ IPOPT_SDB,	0x100000,	4,	(char *)"sdb" },
	{ IPOPT_NSAPA,	0x200000,	4,	(char *)"nsapa" },
	{ IPOPT_RTRALRT,0x400000,	4,	(char *)"rtralrt" },
	{ IPOPT_UMP,	0x800000,	4,	(char *)"ump" },
	{ IPOPT_AH,	0x1000000,	0,	(char *)"ah" },
	{ 0, 		0,	0,	(char *)nullptr }     /* must be last */
};


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/dupmbt.c						  */
/* ---------------------------------------------------------------------- */

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

	m = (mb_t *)malloc(sizeof(mb_t));
	if (m == nullptr)
		return (nullptr);
	m->mb_len = orig->mb_len;
	m->mb_next = nullptr;
	m->mb_data = (char *)m->mb_buf + (orig->mb_data - (char *)orig->mb_buf);
	bcopy(orig->mb_data, m->mb_data, m->mb_len);
	return (m);
}


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/icmptypename.c					  */
/* ---------------------------------------------------------------------- */

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


/* ---------------------------------------------------------------------- */
/* sbin/ipf/libipf/optvalue.c						  */
/* ---------------------------------------------------------------------- */

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
	return (-1);
}


u_32_t
getoptbyvalue(int optval)
{
	struct ipopt_names *io;

	for (io = ionames; io->on_name; io++)
		if (io->on_value == optval)
			return (io->on_bit);
	return (-1);
}

} // namespace pbsd::sbin_ipf_libipf::b0160
