/*
 * oracle.c -- reference implementation for PBSD batch b0160.
 *
 * The original HardenedBSD C sources for this batch, concatenated verbatim.
 * Every function is renamed with a "ref_" prefix; the bodies are unmodified.
 * The global tables the bodies reference are renamed via #define so that even
 * the identifiers inside the bodies are untouched source text.
 *
 * Sources:
 *   sbin/ipf/libipf/icmpcode.c
 *   sbin/ipf/libipf/dupmbt.c
 *   sbin/ipf/libipf/icmptypename.c
 *   sbin/ipf/libipf/optvalue.c
 *
 * The batch functions also need two tables that live in sibling files of
 * libipf (icmptypes.c, ionames.c) and the handful of types/constants that
 * "ipf.h", <netinet/ip.h>, <netinet/ip_icmp.h>, <netinet/icmp6.h> and
 * <netinet/ip_compat.h> supply.  Those are reproduced below with the exact
 * values the FreeBSD/HardenedBSD build uses.  sbin/ipf/Makefile.inc adds
 * -DUSE_INET6, so USE_INET6 is defined here.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define	USE_INET6	1

/* <sys/socket.h>, FreeBSD values. */
#undef	AF_INET
#define	AF_INET		2
#undef	AF_INET6
#define	AF_INET6	28

/* "ipf.h" */
typedef	uint32_t	u_32_t;

#define	MAX_ICMPCODE	16
#define	MAX_ICMPTYPE	19

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

/*
 * Rename the tables without touching the source text of any function body.
 */
#define	icmpcodes	ref_icmpcodes
#define	icmptypelist	ref_icmptypelist
#define	ionames		ref_ionames


/* ====================================================================== */
/* sbin/ipf/libipf/icmpcode.c						  */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

#ifndef	MIN
# define	MIN(a,b)	((a) > (b) ? (b) : (a))
#endif


char	*icmpcodes[MAX_ICMPCODE + 1] = {
	"net-unr", "host-unr", "proto-unr", "port-unr", "needfrag", "srcfail",
	"net-unk", "host-unk", "isolate", "net-prohib", "host-prohib",
	"net-tos", "host-tos", "filter-prohib", "host-preced", "preced-cutoff",
	NULL };


/* ====================================================================== */
/* sbin/ipf/libipf/icmptypes.c  (link dependency of icmptypename)	  */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

icmptype_t icmptypelist[] = {
	{ "echo",	ICMP_ECHO,		ICMP6_ECHO_REQUEST },
	{ "echorep",	ICMP_ECHOREPLY,		ICMP6_ECHO_REPLY },
	{ "fqdnquery",	-1,			ICMP6_FQDN_QUERY },
	{ "fqdnreply",	-1,			ICMP6_FQDN_REPLY },
	{ "infoqry",	-1,			ICMP6_NI_QUERY },
	{ "inforeq",	ICMP_IREQ,		ICMP6_NI_QUERY },
	{ "inforep",	ICMP_IREQREPLY,		ICMP6_NI_REPLY },
	{ "listendone",	-1,			MLD_LISTENER_DONE },
	{ "listenqry",	-1,			MLD_LISTENER_QUERY },
	{ "listenrep",	-1,			MLD_LISTENER_REPORT },
	{ "maskrep",	ICMP_MASKREPLY,		-1 },
	{ "maskreq",	ICMP_MASKREQ,		-1 },
	{ "memberqry",	-1,			ICMP6_MEMBERSHIP_QUERY },
	{ "memberred",	-1,			ICMP6_MEMBERSHIP_REDUCTION },
	{ "memberreply",-1,			ICMP6_MEMBERSHIP_REPORT },
	{ "neighadvert",	-1,		ND_NEIGHBOR_ADVERT },
	{ "neighborsol",	-1,		ND_NEIGHBOR_SOLICIT },
	{ "neighborsolicit",	-1,		ND_NEIGHBOR_SOLICIT },
	{ "paramprob",	ICMP_PARAMPROB,		ICMP6_PARAM_PROB },
	{ "redir",	ICMP_REDIRECT,		ND_REDIRECT },
	{ "renumber",	-1,			ICMP6_ROUTER_RENUMBERING },
	{ "routerad",	ICMP_ROUTERADVERT,	ND_ROUTER_ADVERT },
	{ "routeradvert",ICMP_ROUTERADVERT,	ND_ROUTER_ADVERT },
	{ "routersol",	ICMP_ROUTERSOLICIT,	ND_ROUTER_SOLICIT },
	{ "routersolcit",ICMP_ROUTERSOLICIT,	ND_ROUTER_SOLICIT },
	{ "squench",	ICMP_SOURCEQUENCH,	-1 },
	{ "timest",	ICMP_TSTAMP,		-1 },
	{ "timestrep",	ICMP_TSTAMPREPLY,	-1 },
	{ "timex",	ICMP_TIMXCEED,		ICMP6_TIME_EXCEEDED },
	{ "toobig",	-1,			ICMP6_PACKET_TOO_BIG },
	{ "unreach",	ICMP_UNREACH,		ICMP6_DST_UNREACH },
	{ "whorep",	-1,			ICMP6_WRUREPLY },
	{ "whoreq",	-1,			ICMP6_WRUREQUEST },
	{ NULL,		-1,			-1 }
};


/* ====================================================================== */
/* sbin/ipf/libipf/ionames.c  (link dependency of getoptby*)		  */
/* ====================================================================== */

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

struct	ipopt_names	ionames[] ={
	{ IPOPT_NOP,	0x000001,	1,	"nop" },	/* RFC791 */
	{ IPOPT_RR,	0x000002,	8,	"rr" },		/* 1 route */
	{ IPOPT_ZSU,	0x000004,	4,	"zsu" },	/* size ?? */
	{ IPOPT_MTUP,	0x000008,	4,	"mtup" },	/* RFC1191 */
	{ IPOPT_MTUR,	0x000010,	4,	"mtur" },	/* RFC1191 */
	{ IPOPT_ENCODE,	0x000020,	4,	"encode" },	/* size ?? */
	{ IPOPT_TS,	0x000040,	8,	"ts" },		/* 1 TS */
	{ IPOPT_TR,	0x000080,	4,	"tr" },		/* RFC1393 */
	{ IPOPT_SECURITY,0x000100,	12,	"sec" },	/* RFC1108 */
	{ IPOPT_SECURITY,0x000100,	12,	"sec-class" },	/* RFC1108 */
	{ IPOPT_LSRR,	0x000200,	8,	"lsrr" },	/* 1 route */
	{ IPOPT_E_SEC,	0x000400,	8,	"e-sec" },	/* RFC1108 */
	{ IPOPT_CIPSO,	0x000800,	8,	"cipso" },	/* size ?? */
	{ IPOPT_SATID,	0x001000,	4,	"satid" },	/* RFC791 */
	{ IPOPT_SSRR,	0x002000,	8,	"ssrr" },	/* 1 route */
	{ IPOPT_ADDEXT,	0x004000,	4,	"addext" },	/* IPv7 ?? */
	{ IPOPT_VISA,	0x008000,	4,	"visa" },	/* size ?? */
	{ IPOPT_IMITD,	0x010000,	4,	"imitd" },	/* size ?? */
	{ IPOPT_EIP,	0x020000,	4,	"eip" },	/* RFC1385 */
	{ IPOPT_FINN,	0x040000,	4,	"finn" },	/* size ?? */
	{ IPOPT_DPS,	0x080000,	4,	"dps" },	/* size ?? */
	{ IPOPT_SDB,	0x100000,	4,	"sdb" },	/* size ?? */
	{ IPOPT_NSAPA,	0x200000,	4,	"nsapa" },	/* size ?? */
	{ IPOPT_RTRALRT,0x400000,	4,	"rtralrt" },	/* RFC2113 */
	{ IPOPT_UMP,	0x800000,	4,	"ump" },	/* size ?? */
	{ IPOPT_AH,	0x1000000,	0,	"ah" },		/* IPPROTO_AH */
	{ 0, 		0,	0,	(char *)NULL }     /* must be last */
};


/* ====================================================================== */
/* sbin/ipf/libipf/dupmbt.c						  */
/* ====================================================================== */

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


/* ====================================================================== */
/* sbin/ipf/libipf/icmptypename.c					  */
/* ====================================================================== */

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


/* ====================================================================== */
/* sbin/ipf/libipf/optvalue.c						  */
/* ====================================================================== */

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
