/*
 * ntpdc-layout - print layout of NTP mode 7 request/response packets
 *
 * PBSD port of HardenedBSD hbsd/src/usr.sbin/ntp/ntpdc/nl.c (batch b0150).
 */

module;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>

export module pbsd.usr.sbin.ntp.ntpdc.b0150;

export namespace pbsd::usr_sbin_ntp_ntpdc::b0150 {


#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef uint32_t u_int32;
typedef int32_t int32;
typedef int32_t s_fp;
typedef u_int32 u_fp;
typedef uint16_t associd_t;
typedef u_int32 keyid_t;

typedef struct {
	union {
		u_int32 Xl_ui;
		int32 Xl_i;
	} Ul_i;
	u_int32 l_uf;
} l_fp;

#define NTP_SHIFT 8
#define NTP_HASH_SIZE 128
#define MODE7_PAYLOAD_LIM 176
#define RESP_DATA_SIZE 500
#define MAX_MDG_LEN (6 * (int)sizeof(u_int32) - (int)sizeof(u_int32))
#define NUMCBUGVALUES 16
#define NUMCBUGTIMES 32
#define NTP_MAXHOSTNAME (32 - (int)sizeof(u_int32) - (int)sizeof(unsigned short))

union addrun {
	struct in6_addr addr6;
	struct in_addr addr;
};

typedef union req_data_u_tag {
	u_int32 u32[MODE7_PAYLOAD_LIM / sizeof(u_int32)];
	char data[MODE7_PAYLOAD_LIM];
} req_data_u;

struct req_pkt {
	unsigned char rm_vn_mode;
	unsigned char auth_seq;
	unsigned char implementation;
	unsigned char request;
	unsigned short err_nitems;
	unsigned short mbz_itemsize;
	req_data_u u;
	l_fp tstamp;
	keyid_t keyid;
	char mac[MAX_MDG_LEN];
};

struct req_pkt_tail {
	l_fp tstamp;
	keyid_t keyid;
	char mac[MAX_MDG_LEN];
};

typedef union resp_pkt_u_tag {
	char data[RESP_DATA_SIZE];
	u_int32 u32[RESP_DATA_SIZE / sizeof(u_int32)];
} resp_pkt_u;

struct resp_pkt {
	unsigned char rm_vn_mode;
	unsigned char auth_seq;
	unsigned char implementation;
	unsigned char request;
	unsigned short err_nitems;
	unsigned short mbz_itemsize;
	resp_pkt_u u;
};

struct info_peer_list {
	u_int32 addr;
	unsigned short port;
	unsigned char hmode;
	unsigned char flags;
	unsigned int v6_flag;
	unsigned int unused1;
	struct in6_addr addr6;
};

struct info_peer_summary {
	u_int32 dstadr;
	u_int32 srcadr;
	unsigned short srcport;
	unsigned char stratum;
	signed char hpoll;
	signed char ppoll;
	unsigned char reach;
	unsigned char flags;
	unsigned char hmode;
	s_fp delay;
	l_fp offset;
	u_fp dispersion;
	unsigned int v6_flag;
	unsigned int unused1;
	struct in6_addr dstadr6;
	struct in6_addr srcadr6;
};

struct info_peer {
	u_int32 dstadr;
	u_int32 srcadr;
	unsigned short srcport;
	unsigned char flags;
	unsigned char leap;
	unsigned char hmode;
	unsigned char pmode;
	unsigned char stratum;
	unsigned char ppoll;
	unsigned char hpoll;
	signed char precision;
	unsigned char version;
	unsigned char unused8;
	unsigned char reach;
	unsigned char unreach;
	unsigned char flash;
	unsigned char ttl;
	unsigned short flash2;
	associd_t associd;
	keyid_t keyid;
	u_int32 pkeyid;
	u_int32 refid;
	u_int32 timer;
	s_fp rootdelay;
	u_fp rootdispersion;
	l_fp reftime;
	l_fp org;
	l_fp rec;
	l_fp xmt;
	s_fp filtdelay[NTP_SHIFT];
	l_fp filtoffset[NTP_SHIFT];
	unsigned char order[NTP_SHIFT];
	s_fp delay;
	u_fp dispersion;
	l_fp offset;
	u_fp selectdisp;
	int32 unused1;
	int32 unused2;
	int32 unused3;
	int32 unused4;
	int32 unused5;
	int32 unused6;
	int32 unused7;
	s_fp estbdelay;
	unsigned int v6_flag;
	unsigned int unused9;
	struct in6_addr dstadr6;
	struct in6_addr srcadr6;
};

struct info_peer_stats {
	u_int32 dstadr;
	u_int32 srcadr;
	unsigned short srcport;
	unsigned short flags;
	u_int32 timereset;
	u_int32 timereceived;
	u_int32 timetosend;
	u_int32 timereachable;
	u_int32 sent;
	u_int32 unused1;
	u_int32 processed;
	u_int32 unused2;
	u_int32 badauth;
	u_int32 bogusorg;
	u_int32 oldpkt;
	u_int32 unused3;
	u_int32 unused4;
	u_int32 seldisp;
	u_int32 selbroken;
	u_int32 unused5;
	unsigned char candidate;
	unsigned char unused6;
	unsigned char unused7;
	unsigned char unused8;
	unsigned int v6_flag;
	unsigned int unused9;
	struct in6_addr dstadr6;
	struct in6_addr srcadr6;
};

struct info_loop {
	l_fp last_offset;
	l_fp drift_comp;
	u_int32 compliance;
	u_int32 watchdog_timer;
};

struct info_sys {
	u_int32 peer;
	unsigned char peer_mode;
	unsigned char leap;
	unsigned char stratum;
	signed char precision;
	s_fp rootdelay;
	u_fp rootdispersion;
	u_int32 refid;
	l_fp reftime;
	u_int32 poll;
	unsigned char flags;
	unsigned char unused1;
	unsigned char unused2;
	unsigned char unused3;
	s_fp bdelay;
	s_fp frequency;
	l_fp authdelay;
	u_fp stability;
	unsigned int v6_flag;
	unsigned int unused4;
	struct in6_addr peer6;
};

struct info_sys_stats {
	u_int32 timeup;
	u_int32 timereset;
	u_int32 denied;
	u_int32 oldversionpkt;
	u_int32 newversionpkt;
	u_int32 unknownversion;
	u_int32 badlength;
	u_int32 processed;
	u_int32 badauth;
	u_int32 received;
	u_int32 limitrejected;
	u_int32 lamport;
	u_int32 tsrounding;
};

struct old_info_sys_stats {
	u_int32 timeup;
	u_int32 timereset;
	u_int32 denied;
	u_int32 oldversionpkt;
	u_int32 newversionpkt;
	u_int32 unknownversion;
	u_int32 badlength;
	u_int32 processed;
	u_int32 badauth;
	u_int32 wanderhold;
};

struct info_mem_stats {
	u_int32 timereset;
	unsigned short totalpeermem;
	unsigned short freepeermem;
	u_int32 findpeer_calls;
	u_int32 allocations;
	u_int32 demobilizations;
	unsigned char hashcount[NTP_HASH_SIZE];
};

struct info_io_stats {
	u_int32 timereset;
	unsigned short totalrecvbufs;
	unsigned short freerecvbufs;
	unsigned short fullrecvbufs;
	unsigned short lowwater;
	u_int32 dropped;
	u_int32 ignored;
	u_int32 received;
	u_int32 sent;
	u_int32 notsent;
	u_int32 interrupts;
	u_int32 int_received;
};

struct info_timer_stats {
	u_int32 timereset;
	u_int32 alarms;
	u_int32 overflows;
	u_int32 xmtcalls;
};

struct old_conf_peer {
	u_int32 peeraddr;
	unsigned char hmode;
	unsigned char version;
	unsigned char minpoll;
	unsigned char maxpoll;
	unsigned char flags;
	unsigned char ttl;
	unsigned short unused;
	keyid_t keyid;
};

struct conf_peer {
	u_int32 peeraddr;
	unsigned char hmode;
	unsigned char version;
	unsigned char minpoll;
	unsigned char maxpoll;
	unsigned char flags;
	unsigned char ttl;
	unsigned short unused1;
	keyid_t keyid;
	char keystr[128];
	unsigned int v6_flag;
	unsigned int unused2;
	struct in6_addr peeraddr6;
};

struct conf_unpeer {
	u_int32 peeraddr;
	unsigned int v6_flag;
	struct in6_addr peeraddr6;
};

struct conf_sys_flags {
	u_int32 flags;
};

struct info_restrict {
	u_int32 addr;
	u_int32 mask;
	u_int32 count;
	unsigned short flags;
	unsigned short mflags;
	unsigned int v6_flag;
	unsigned int unused1;
	struct in6_addr addr6;
	struct in6_addr mask6;
};

struct conf_restrict {
	u_int32 addr;
	u_int32 mask;
	short ippeerlimit;
	int srvfuzrft;
	unsigned short flags;
	unsigned short mflags;
	short unused1;
	unsigned int v6_flag;
	struct in6_addr addr6;
	struct in6_addr mask6;
};

struct info_monitor_1 {
	u_int32 avg_int;
	u_int32 last_int;
	u_int32 restr;
	u_int32 count;
	u_int32 addr;
	u_int32 daddr;
	u_int32 flags;
	unsigned short port;
	unsigned char mode;
	unsigned char version;
	unsigned int v6_flag;
	unsigned int unused1;
	struct in6_addr addr6;
	struct in6_addr daddr6;
};

struct info_monitor {
	u_int32 avg_int;
	u_int32 last_int;
	u_int32 restr;
	u_int32 count;
	u_int32 addr;
	unsigned short port;
	unsigned char mode;
	unsigned char version;
	unsigned int v6_flag;
	unsigned int unused1;
	struct in6_addr addr6;
};

struct old_info_monitor {
	u_int32 lasttime;
	u_int32 firsttime;
	u_int32 count;
	u_int32 addr;
	unsigned short port;
	unsigned char mode;
	unsigned char version;
	unsigned int v6_flag;
	struct in6_addr addr6;
};

struct reset_flags {
	u_int32 flags;
};

struct info_auth {
	u_int32 timereset;
	u_int32 numkeys;
	u_int32 numfreekeys;
	u_int32 keylookups;
	u_int32 keynotfound;
	u_int32 encryptions;
	u_int32 decryptions;
	u_int32 expired;
	u_int32 keyuncached;
};

struct info_trap {
	u_int32 local_address;
	u_int32 trap_address;
	unsigned short trap_port;
	unsigned short sequence;
	u_int32 settime;
	u_int32 origtime;
	u_int32 resets;
	u_int32 flags;
	unsigned int v6_flag;
	struct in6_addr local_address6;
	struct in6_addr trap_address6;
};

struct conf_trap {
	u_int32 local_address;
	u_int32 trap_address;
	unsigned short trap_port;
	unsigned short unused;
	unsigned int v6_flag;
	struct in6_addr local_address6;
	struct in6_addr trap_address6;
};

struct info_control {
	u_int32 ctltimereset;
	u_int32 numctlreq;
	u_int32 numctlbadpkts;
	u_int32 numctlresponses;
	u_int32 numctlfrags;
	u_int32 numctlerrors;
	u_int32 numctltooshort;
	u_int32 numctlinputresp;
	u_int32 numctlinputfrag;
	u_int32 numctlinputerr;
	u_int32 numctlbadoffset;
	u_int32 numctlbadversion;
	u_int32 numctldatatooshort;
	u_int32 numctlbadop;
	u_int32 numasyncmsgs;
};

struct info_clock {
	u_int32 clockadr;
	unsigned char type;
	unsigned char flags;
	unsigned char lastevent;
	unsigned char currentstatus;
	u_int32 polls;
	u_int32 noresponse;
	u_int32 badformat;
	u_int32 baddata;
	u_int32 timestarted;
	l_fp fudgetime1;
	l_fp fudgetime2;
	int32 fudgeval1;
	u_int32 fudgeval2;
};

struct conf_fudge {
	u_int32 clockadr;
	u_int32 which;
	l_fp fudgetime;
	u_int32 fudgeval_flags;
};

struct info_clkbug {
	u_int32 clockadr;
	unsigned char nvalues;
	unsigned char ntimes;
	unsigned short svalues;
	u_int32 stimes;
	u_int32 values[NUMCBUGVALUES];
	l_fp times[NUMCBUGTIMES];
};

struct info_kernel {
	int32 offset;
	int32 freq;
	int32 maxerror;
	int32 esterror;
	unsigned short status;
	unsigned short shift;
	int32 constant;
	int32 precision;
	int32 tolerance;
	int32 ppsfreq;
	int32 jitter;
	int32 stabil;
	int32 jitcnt;
	int32 calcnt;
	int32 errcnt;
	int32 stbcnt;
};

struct info_if_stats {
	union addrun unaddr;
	union addrun unbcast;
	union addrun unmask;
	u_int32 v6_flag;
	char name[32];
	int32 flags;
	int32 last_ttl;
	int32 num_mcast;
	int32 received;
	int32 sent;
	int32 notsent;
	int32 uptime;
	u_int32 scopeid;
	u_int32 ifindex;
	u_int32 ifnum;
	u_int32 peercnt;
	unsigned short family;
	unsigned char ignore_packets;
	unsigned char action;
	int32 _filler0;
};

struct info_dns_assoc {
	u_int32 peeraddr;
	associd_t associd;
	char hostname[NTP_MAXHOSTNAME];
};


int
nl(void)
{
  printf("sizeof(union req_data_u_tag) = %d\n", 
	 (int) sizeof(union req_data_u_tag));
  printf("offsetof(u32) = %d\n", 
	 (int) offsetof(union req_data_u_tag, u32));
  printf("offsetof(data) = %d\n", 
	 (int) offsetof(union req_data_u_tag, data));
  printf("\n");

  printf("sizeof(struct req_pkt) = %d\n", 
	 (int) sizeof(struct req_pkt));
  printf("offsetof(rm_vn_mode) = %d\n", 
	 (int) offsetof(struct req_pkt, rm_vn_mode));
  printf("offsetof(auth_seq) = %d\n", 
	 (int) offsetof(struct req_pkt, auth_seq));
  printf("offsetof(implementation) = %d\n", 
	 (int) offsetof(struct req_pkt, implementation));
  printf("offsetof(request) = %d\n", 
	 (int) offsetof(struct req_pkt, request));
  printf("offsetof(err_nitems) = %d\n", 
	 (int) offsetof(struct req_pkt, err_nitems));
  printf("offsetof(mbz_itemsize) = %d\n", 
	 (int) offsetof(struct req_pkt, mbz_itemsize));
  printf("offsetof(u) = %d\n", 
	 (int) offsetof(struct req_pkt, u));
  printf("offsetof(tstamp) = %d\n", 
	 (int) offsetof(struct req_pkt, tstamp));
  printf("offsetof(keyid) = %d\n", 
	 (int) offsetof(struct req_pkt, keyid));
  printf("offsetof(mac) = %d\n", 
	 (int) offsetof(struct req_pkt, mac));
  printf("\n");

  printf("sizeof(struct req_pkt_tail) = %d\n", 
	 (int) sizeof(struct req_pkt_tail));
  printf("offsetof(tstamp) = %d\n", 
	 (int) offsetof(struct req_pkt_tail, tstamp));
  printf("offsetof(keyid) = %d\n", 
	 (int) offsetof(struct req_pkt_tail, keyid));
  printf("offsetof(mac) = %d\n", 
	 (int) offsetof(struct req_pkt_tail, mac));
  printf("\n");

  printf("sizeof(union resp_pkt_u_tag) = %d\n", 
	 (int) sizeof(union resp_pkt_u_tag));
  printf("offsetof(data) = %d\n", 
	 (int) offsetof(union resp_pkt_u_tag, data));
  printf("offsetof(u32) = %d\n", 
	 (int) offsetof(union resp_pkt_u_tag, u32));
  printf("\n");

  printf("sizeof(struct resp_pkt) = %d\n", 
	 (int) sizeof(struct resp_pkt));
  printf("offsetof(rm_vn_mode) = %d\n", 
	 (int) offsetof(struct resp_pkt, rm_vn_mode));
  printf("offsetof(auth_seq) = %d\n", 
	 (int) offsetof(struct resp_pkt, auth_seq));
  printf("offsetof(implementation) = %d\n", 
	 (int) offsetof(struct resp_pkt, implementation));
  printf("offsetof(request) = %d\n", 
	 (int) offsetof(struct resp_pkt, request));
  printf("offsetof(err_nitems) = %d\n", 
	 (int) offsetof(struct resp_pkt, err_nitems));
  printf("offsetof(mbz_itemsize) = %d\n", 
	 (int) offsetof(struct resp_pkt, mbz_itemsize));
  printf("offsetof(u) = %d\n", 
	 (int) offsetof(struct resp_pkt, u));
  printf("\n");

  printf("sizeof(struct info_peer_list) = %d\n", 
	 (int) sizeof(struct info_peer_list));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct info_peer_list, addr));
  printf("offsetof(port) = %d\n", 
	 (int) offsetof(struct info_peer_list, port));
  printf("offsetof(hmode) = %d\n", 
	 (int) offsetof(struct info_peer_list, hmode));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_peer_list, flags));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_peer_list, v6_flag));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_peer_list, unused1));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct info_peer_list, addr6));
  printf("\n");

  printf("sizeof(struct info_peer_summary) = %d\n", 
	 (int) sizeof(struct info_peer_summary));
  printf("offsetof(dstadr) = %d\n", 
	 (int) offsetof(struct info_peer_summary, dstadr));
  printf("offsetof(srcadr) = %d\n", 
	 (int) offsetof(struct info_peer_summary, srcadr));
  printf("offsetof(srcport) = %d\n", 
	 (int) offsetof(struct info_peer_summary, srcport));
  printf("offsetof(stratum) = %d\n", 
	 (int) offsetof(struct info_peer_summary, stratum));
  printf("offsetof(hpoll) = %d\n", 
	 (int) offsetof(struct info_peer_summary, hpoll));
  printf("offsetof(ppoll) = %d\n", 
	 (int) offsetof(struct info_peer_summary, ppoll));
  printf("offsetof(reach) = %d\n", 
	 (int) offsetof(struct info_peer_summary, reach));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_peer_summary, flags));
  printf("offsetof(hmode) = %d\n", 
	 (int) offsetof(struct info_peer_summary, hmode));
  printf("offsetof(delay) = %d\n", 
	 (int) offsetof(struct info_peer_summary, delay));
  printf("offsetof(offset) = %d\n", 
	 (int) offsetof(struct info_peer_summary, offset));
  printf("offsetof(dispersion) = %d\n", 
	 (int) offsetof(struct info_peer_summary, dispersion));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_peer_summary, v6_flag));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_peer_summary, unused1));
  printf("offsetof(dstadr6) = %d\n", 
	 (int) offsetof(struct info_peer_summary, dstadr6));
  printf("offsetof(srcadr6) = %d\n", 
	 (int) offsetof(struct info_peer_summary, srcadr6));
  printf("\n");

  printf("sizeof(struct info_peer) = %d\n", 
	 (int) sizeof(struct info_peer));
  printf("offsetof(dstadr) = %d\n", 
	 (int) offsetof(struct info_peer, dstadr));
  printf("offsetof(srcadr) = %d\n", 
	 (int) offsetof(struct info_peer, srcadr));
  printf("offsetof(srcport) = %d\n", 
	 (int) offsetof(struct info_peer, srcport));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_peer, flags));
  printf("offsetof(leap) = %d\n", 
	 (int) offsetof(struct info_peer, leap));
  printf("offsetof(hmode) = %d\n", 
	 (int) offsetof(struct info_peer, hmode));
  printf("offsetof(pmode) = %d\n", 
	 (int) offsetof(struct info_peer, pmode));
  printf("offsetof(stratum) = %d\n", 
	 (int) offsetof(struct info_peer, stratum));
  printf("offsetof(ppoll) = %d\n", 
	 (int) offsetof(struct info_peer, ppoll));
  printf("offsetof(hpoll) = %d\n", 
	 (int) offsetof(struct info_peer, hpoll));
  printf("offsetof(precision) = %d\n", 
	 (int) offsetof(struct info_peer, precision));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct info_peer, version));
  printf("offsetof(unused8) = %d\n", 
	 (int) offsetof(struct info_peer, unused8));
  printf("offsetof(reach) = %d\n", 
	 (int) offsetof(struct info_peer, reach));
  printf("offsetof(unreach) = %d\n", 
	 (int) offsetof(struct info_peer, unreach));
  printf("offsetof(flash) = %d\n", 
	 (int) offsetof(struct info_peer, flash));
  printf("offsetof(ttl) = %d\n", 
	 (int) offsetof(struct info_peer, ttl));
  printf("offsetof(flash2) = %d\n", 
	 (int) offsetof(struct info_peer, flash2));
  printf("offsetof(associd) = %d\n", 
	 (int) offsetof(struct info_peer, associd));
  printf("offsetof(keyid) = %d\n", 
	 (int) offsetof(struct info_peer, keyid));
  printf("offsetof(pkeyid) = %d\n", 
	 (int) offsetof(struct info_peer, pkeyid));
  printf("offsetof(refid) = %d\n", 
	 (int) offsetof(struct info_peer, refid));
  printf("offsetof(timer) = %d\n", 
	 (int) offsetof(struct info_peer, timer));
  printf("offsetof(rootdelay) = %d\n", 
	 (int) offsetof(struct info_peer, rootdelay));
  printf("offsetof(rootdispersion) = %d\n", 
	 (int) offsetof(struct info_peer, rootdispersion));
  printf("offsetof(reftime) = %d\n", 
	 (int) offsetof(struct info_peer, reftime));
  printf("offsetof(org) = %d\n", 
	 (int) offsetof(struct info_peer, org));
  printf("offsetof(rec) = %d\n", 
	 (int) offsetof(struct info_peer, rec));
  printf("offsetof(xmt) = %d\n", 
	 (int) offsetof(struct info_peer, xmt));
  printf("offsetof(filtdelay) = %d\n", 
	 (int) offsetof(struct info_peer, filtdelay));
  printf("offsetof(filtoffset) = %d\n", 
	 (int) offsetof(struct info_peer, filtoffset));
  printf("offsetof(order) = %d\n", 
	 (int) offsetof(struct info_peer, order));
  printf("offsetof(delay) = %d\n", 
	 (int) offsetof(struct info_peer, delay));
  printf("offsetof(dispersion) = %d\n", 
	 (int) offsetof(struct info_peer, dispersion));
  printf("offsetof(offset) = %d\n", 
	 (int) offsetof(struct info_peer, offset));
  printf("offsetof(selectdisp) = %d\n", 
	 (int) offsetof(struct info_peer, selectdisp));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_peer, unused1));
  printf("offsetof(unused2) = %d\n", 
	 (int) offsetof(struct info_peer, unused2));
  printf("offsetof(unused3) = %d\n", 
	 (int) offsetof(struct info_peer, unused3));
  printf("offsetof(unused4) = %d\n", 
	 (int) offsetof(struct info_peer, unused4));
  printf("offsetof(unused5) = %d\n", 
	 (int) offsetof(struct info_peer, unused5));
  printf("offsetof(unused6) = %d\n", 
	 (int) offsetof(struct info_peer, unused6));
  printf("offsetof(unused7) = %d\n", 
	 (int) offsetof(struct info_peer, unused7));
  printf("offsetof(estbdelay) = %d\n", 
	 (int) offsetof(struct info_peer, estbdelay));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_peer, v6_flag));
  printf("offsetof(unused9) = %d\n", 
	 (int) offsetof(struct info_peer, unused9));
  printf("offsetof(dstadr6) = %d\n", 
	 (int) offsetof(struct info_peer, dstadr6));
  printf("offsetof(srcadr6) = %d\n", 
	 (int) offsetof(struct info_peer, srcadr6));
  printf("\n");

  printf("sizeof(struct info_peer_stats) = %d\n", 
	 (int) sizeof(struct info_peer_stats));
  printf("offsetof(dstadr) = %d\n", 
	 (int) offsetof(struct info_peer_stats, dstadr));
  printf("offsetof(srcadr) = %d\n", 
	 (int) offsetof(struct info_peer_stats, srcadr));
  printf("offsetof(srcport) = %d\n", 
	 (int) offsetof(struct info_peer_stats, srcport));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_peer_stats, flags));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_peer_stats, timereset));
  printf("offsetof(timereceived) = %d\n", 
	 (int) offsetof(struct info_peer_stats, timereceived));
  printf("offsetof(timetosend) = %d\n", 
	 (int) offsetof(struct info_peer_stats, timetosend));
  printf("offsetof(timereachable) = %d\n", 
	 (int) offsetof(struct info_peer_stats, timereachable));
  printf("offsetof(sent) = %d\n", 
	 (int) offsetof(struct info_peer_stats, sent));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused1));
  printf("offsetof(processed) = %d\n", 
	 (int) offsetof(struct info_peer_stats, processed));
  printf("offsetof(unused2) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused2));
  printf("offsetof(badauth) = %d\n", 
	 (int) offsetof(struct info_peer_stats, badauth));
  printf("offsetof(bogusorg) = %d\n", 
	 (int) offsetof(struct info_peer_stats, bogusorg));
  printf("offsetof(oldpkt) = %d\n", 
	 (int) offsetof(struct info_peer_stats, oldpkt));
  printf("offsetof(unused3) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused3));
  printf("offsetof(unused4) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused4));
  printf("offsetof(seldisp) = %d\n", 
	 (int) offsetof(struct info_peer_stats, seldisp));
  printf("offsetof(selbroken) = %d\n", 
	 (int) offsetof(struct info_peer_stats, selbroken));
  printf("offsetof(unused5) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused5));
  printf("offsetof(candidate) = %d\n", 
	 (int) offsetof(struct info_peer_stats, candidate));
  printf("offsetof(unused6) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused6));
  printf("offsetof(unused7) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused7));
  printf("offsetof(unused8) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused8));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_peer_stats, v6_flag));
  printf("offsetof(unused9) = %d\n", 
	 (int) offsetof(struct info_peer_stats, unused9));
  printf("offsetof(dstadr6) = %d\n", 
	 (int) offsetof(struct info_peer_stats, dstadr6));
  printf("offsetof(srcadr6) = %d\n", 
	 (int) offsetof(struct info_peer_stats, srcadr6));
  printf("\n");

  printf("sizeof(struct info_loop) = %d\n", 
	 (int) sizeof(struct info_loop));
  printf("offsetof(last_offset) = %d\n", 
	 (int) offsetof(struct info_loop, last_offset));
  printf("offsetof(drift_comp) = %d\n", 
	 (int) offsetof(struct info_loop, drift_comp));
  printf("offsetof(compliance) = %d\n", 
	 (int) offsetof(struct info_loop, compliance));
  printf("offsetof(watchdog_timer) = %d\n", 
	 (int) offsetof(struct info_loop, watchdog_timer));
  printf("\n");

  printf("sizeof(struct info_sys) = %d\n", 
	 (int) sizeof(struct info_sys));
  printf("offsetof(peer) = %d\n", 
	 (int) offsetof(struct info_sys, peer));
  printf("offsetof(peer_mode) = %d\n", 
	 (int) offsetof(struct info_sys, peer_mode));
  printf("offsetof(leap) = %d\n", 
	 (int) offsetof(struct info_sys, leap));
  printf("offsetof(stratum) = %d\n", 
	 (int) offsetof(struct info_sys, stratum));
  printf("offsetof(precision) = %d\n", 
	 (int) offsetof(struct info_sys, precision));
  printf("offsetof(rootdelay) = %d\n", 
	 (int) offsetof(struct info_sys, rootdelay));
  printf("offsetof(rootdispersion) = %d\n", 
	 (int) offsetof(struct info_sys, rootdispersion));
  printf("offsetof(refid) = %d\n", 
	 (int) offsetof(struct info_sys, refid));
  printf("offsetof(reftime) = %d\n", 
	 (int) offsetof(struct info_sys, reftime));
  printf("offsetof(poll) = %d\n", 
	 (int) offsetof(struct info_sys, poll));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_sys, flags));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_sys, unused1));
  printf("offsetof(unused2) = %d\n", 
	 (int) offsetof(struct info_sys, unused2));
  printf("offsetof(unused3) = %d\n", 
	 (int) offsetof(struct info_sys, unused3));
  printf("offsetof(bdelay) = %d\n", 
	 (int) offsetof(struct info_sys, bdelay));
  printf("offsetof(frequency) = %d\n", 
	 (int) offsetof(struct info_sys, frequency));
  printf("offsetof(authdelay) = %d\n", 
	 (int) offsetof(struct info_sys, authdelay));
  printf("offsetof(stability) = %d\n", 
	 (int) offsetof(struct info_sys, stability));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_sys, v6_flag));
  printf("offsetof(unused4) = %d\n", 
	 (int) offsetof(struct info_sys, unused4));
  printf("offsetof(peer6) = %d\n", 
	 (int) offsetof(struct info_sys, peer6));
  printf("\n");

  printf("sizeof(struct info_sys_stats) = %d\n", 
	 (int) sizeof(struct info_sys_stats));
  printf("offsetof(timeup) = %d\n", 
	 (int) offsetof(struct info_sys_stats, timeup));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_sys_stats, timereset));
  printf("offsetof(denied) = %d\n", 
	 (int) offsetof(struct info_sys_stats, denied));
  printf("offsetof(oldversionpkt) = %d\n", 
	 (int) offsetof(struct info_sys_stats, oldversionpkt));
  printf("offsetof(newversionpkt) = %d\n", 
	 (int) offsetof(struct info_sys_stats, newversionpkt));
  printf("offsetof(unknownversion) = %d\n", 
	 (int) offsetof(struct info_sys_stats, unknownversion));
  printf("offsetof(badlength) = %d\n", 
	 (int) offsetof(struct info_sys_stats, badlength));
  printf("offsetof(processed) = %d\n", 
	 (int) offsetof(struct info_sys_stats, processed));
  printf("offsetof(badauth) = %d\n", 
	 (int) offsetof(struct info_sys_stats, badauth));
  printf("offsetof(received) = %d\n", 
	 (int) offsetof(struct info_sys_stats, received));
  printf("offsetof(limitrejected) = %d\n", 
	 (int) offsetof(struct info_sys_stats, limitrejected));
  printf("\n");

  printf("sizeof(struct old_info_sys_stats) = %d\n", 
	 (int) sizeof(struct old_info_sys_stats));
  printf("offsetof(timeup) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, timeup));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, timereset));
  printf("offsetof(denied) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, denied));
  printf("offsetof(oldversionpkt) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, oldversionpkt));
  printf("offsetof(newversionpkt) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, newversionpkt));
  printf("offsetof(unknownversion) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, unknownversion));
  printf("offsetof(badlength) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, badlength));
  printf("offsetof(processed) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, processed));
  printf("offsetof(badauth) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, badauth));
  printf("offsetof(wanderhold) = %d\n", 
	 (int) offsetof(struct old_info_sys_stats, wanderhold));
  printf("\n");

  printf("sizeof(struct info_mem_stats) = %d\n", 
	 (int) sizeof(struct info_mem_stats));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_mem_stats, timereset));
  printf("offsetof(totalpeermem) = %d\n", 
	 (int) offsetof(struct info_mem_stats, totalpeermem));
  printf("offsetof(freepeermem) = %d\n", 
	 (int) offsetof(struct info_mem_stats, freepeermem));
  printf("offsetof(findpeer_calls) = %d\n", 
	 (int) offsetof(struct info_mem_stats, findpeer_calls));
  printf("offsetof(allocations) = %d\n", 
	 (int) offsetof(struct info_mem_stats, allocations));
  printf("offsetof(demobilizations) = %d\n", 
	 (int) offsetof(struct info_mem_stats, demobilizations));
  printf("offsetof(hashcount) = %d\n", 
	 (int) offsetof(struct info_mem_stats, hashcount));
  printf("\n");

  printf("sizeof(struct info_io_stats) = %d\n", 
	 (int) sizeof(struct info_io_stats));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_io_stats, timereset));
  printf("offsetof(totalrecvbufs) = %d\n", 
	 (int) offsetof(struct info_io_stats, totalrecvbufs));
  printf("offsetof(freerecvbufs) = %d\n", 
	 (int) offsetof(struct info_io_stats, freerecvbufs));
  printf("offsetof(fullrecvbufs) = %d\n", 
	 (int) offsetof(struct info_io_stats, fullrecvbufs));
  printf("offsetof(lowwater) = %d\n", 
	 (int) offsetof(struct info_io_stats, lowwater));
  printf("offsetof(dropped) = %d\n", 
	 (int) offsetof(struct info_io_stats, dropped));
  printf("offsetof(ignored) = %d\n", 
	 (int) offsetof(struct info_io_stats, ignored));
  printf("offsetof(received) = %d\n", 
	 (int) offsetof(struct info_io_stats, received));
  printf("offsetof(sent) = %d\n", 
	 (int) offsetof(struct info_io_stats, sent));
  printf("offsetof(notsent) = %d\n", 
	 (int) offsetof(struct info_io_stats, notsent));
  printf("offsetof(interrupts) = %d\n", 
	 (int) offsetof(struct info_io_stats, interrupts));
  printf("offsetof(int_received) = %d\n", 
	 (int) offsetof(struct info_io_stats, int_received));
  printf("\n");

  printf("sizeof(struct info_timer_stats) = %d\n", 
	 (int) sizeof(struct info_timer_stats));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_timer_stats, timereset));
  printf("offsetof(alarms) = %d\n", 
	 (int) offsetof(struct info_timer_stats, alarms));
  printf("offsetof(overflows) = %d\n", 
	 (int) offsetof(struct info_timer_stats, overflows));
  printf("offsetof(xmtcalls) = %d\n", 
	 (int) offsetof(struct info_timer_stats, xmtcalls));
  printf("\n");

  printf("sizeof(struct old_conf_peer) = %d\n", 
	 (int) sizeof(struct old_conf_peer));
  printf("offsetof(peeraddr) = %d\n", 
	 (int) offsetof(struct old_conf_peer, peeraddr));
  printf("offsetof(hmode) = %d\n", 
	 (int) offsetof(struct old_conf_peer, hmode));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct old_conf_peer, version));
  printf("offsetof(minpoll) = %d\n", 
	 (int) offsetof(struct old_conf_peer, minpoll));
  printf("offsetof(maxpoll) = %d\n", 
	 (int) offsetof(struct old_conf_peer, maxpoll));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct old_conf_peer, flags));
  printf("offsetof(ttl) = %d\n", 
	 (int) offsetof(struct old_conf_peer, ttl));
  printf("offsetof(unused) = %d\n", 
	 (int) offsetof(struct old_conf_peer, unused));
  printf("offsetof(keyid) = %d\n", 
	 (int) offsetof(struct old_conf_peer, keyid));
  printf("\n");

  printf("sizeof(struct conf_peer) = %d\n", 
	 (int) sizeof(struct conf_peer));
  printf("offsetof(peeraddr) = %d\n", 
	 (int) offsetof(struct conf_peer, peeraddr));
  printf("offsetof(hmode) = %d\n", 
	 (int) offsetof(struct conf_peer, hmode));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct conf_peer, version));
  printf("offsetof(minpoll) = %d\n", 
	 (int) offsetof(struct conf_peer, minpoll));
  printf("offsetof(maxpoll) = %d\n", 
	 (int) offsetof(struct conf_peer, maxpoll));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct conf_peer, flags));
  printf("offsetof(ttl) = %d\n", 
	 (int) offsetof(struct conf_peer, ttl));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct conf_peer, unused1));
  printf("offsetof(keyid) = %d\n", 
	 (int) offsetof(struct conf_peer, keyid));
  printf("offsetof(keystr) = %d\n", 
	 (int) offsetof(struct conf_peer, keystr));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct conf_peer, v6_flag));
  printf("offsetof(unused2) = %d\n", 
	 (int) offsetof(struct conf_peer, unused2));
  printf("offsetof(peeraddr6) = %d\n", 
	 (int) offsetof(struct conf_peer, peeraddr6));
  printf("\n");

  printf("sizeof(struct conf_unpeer) = %d\n", 
	 (int) sizeof(struct conf_unpeer));
  printf("offsetof(peeraddr) = %d\n", 
	 (int) offsetof(struct conf_unpeer, peeraddr));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct conf_unpeer, v6_flag));
  printf("offsetof(peeraddr6) = %d\n", 
	 (int) offsetof(struct conf_unpeer, peeraddr6));
  printf("\n");

  printf("sizeof(struct conf_sys_flags) = %d\n", 
	 (int) sizeof(struct conf_sys_flags));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct conf_sys_flags, flags));
  printf("\n");

  printf("sizeof(struct info_restrict) = %d\n", 
	 (int) sizeof(struct info_restrict));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct info_restrict, addr));
  printf("offsetof(mask) = %d\n", 
	 (int) offsetof(struct info_restrict, mask));
  printf("offsetof(count) = %d\n", 
	 (int) offsetof(struct info_restrict, count));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_restrict, flags));
  printf("offsetof(mflags) = %d\n", 
	 (int) offsetof(struct info_restrict, mflags));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_restrict, v6_flag));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_restrict, unused1));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct info_restrict, addr6));
  printf("offsetof(mask6) = %d\n", 
	 (int) offsetof(struct info_restrict, mask6));
  printf("\n");

  printf("sizeof(struct conf_restrict) = %d\n", 
	 (int) sizeof(struct conf_restrict));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct conf_restrict, addr));
  printf("offsetof(mask) = %d\n", 
	 (int) offsetof(struct conf_restrict, mask));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct conf_restrict, flags));
  printf("offsetof(mflags) = %d\n", 
	 (int) offsetof(struct conf_restrict, mflags));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct conf_restrict, v6_flag));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct conf_restrict, addr6));
  printf("offsetof(mask6) = %d\n", 
	 (int) offsetof(struct conf_restrict, mask6));
  printf("\n");

  printf("sizeof(struct info_monitor_1) = %d\n", 
	 (int) sizeof(struct info_monitor_1));
  printf("offsetof(avg_int) = %d\n", 
	 (int) offsetof(struct info_monitor_1, avg_int));
  printf("offsetof(last_int) = %d\n", 
	 (int) offsetof(struct info_monitor_1, last_int));
  printf("offsetof(restr) = %d\n", 
	 (int) offsetof(struct info_monitor_1, restr));
  printf("offsetof(count) = %d\n", 
	 (int) offsetof(struct info_monitor_1, count));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct info_monitor_1, addr));
  printf("offsetof(daddr) = %d\n", 
	 (int) offsetof(struct info_monitor_1, daddr));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_monitor_1, flags));
  printf("offsetof(port) = %d\n", 
	 (int) offsetof(struct info_monitor_1, port));
  printf("offsetof(mode) = %d\n", 
	 (int) offsetof(struct info_monitor_1, mode));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct info_monitor_1, version));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_monitor_1, v6_flag));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_monitor_1, unused1));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct info_monitor_1, addr6));
  printf("offsetof(daddr6) = %d\n", 
	 (int) offsetof(struct info_monitor_1, daddr6));
  printf("\n");

  printf("sizeof(struct info_monitor) = %d\n", 
	 (int) sizeof(struct info_monitor));
  printf("offsetof(avg_int) = %d\n", 
	 (int) offsetof(struct info_monitor, avg_int));
  printf("offsetof(last_int) = %d\n", 
	 (int) offsetof(struct info_monitor, last_int));
  printf("offsetof(restr) = %d\n", 
	 (int) offsetof(struct info_monitor, restr));
  printf("offsetof(count) = %d\n", 
	 (int) offsetof(struct info_monitor, count));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct info_monitor, addr));
  printf("offsetof(port) = %d\n", 
	 (int) offsetof(struct info_monitor, port));
  printf("offsetof(mode) = %d\n", 
	 (int) offsetof(struct info_monitor, mode));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct info_monitor, version));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_monitor, v6_flag));
  printf("offsetof(unused1) = %d\n", 
	 (int) offsetof(struct info_monitor, unused1));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct info_monitor, addr6));
  printf("\n");

  printf("sizeof(struct old_info_monitor) = %d\n", 
	 (int) sizeof(struct old_info_monitor));
  printf("offsetof(lasttime) = %d\n", 
	 (int) offsetof(struct old_info_monitor, lasttime));
  printf("offsetof(firsttime) = %d\n", 
	 (int) offsetof(struct old_info_monitor, firsttime));
  printf("offsetof(count) = %d\n", 
	 (int) offsetof(struct old_info_monitor, count));
  printf("offsetof(addr) = %d\n", 
	 (int) offsetof(struct old_info_monitor, addr));
  printf("offsetof(port) = %d\n", 
	 (int) offsetof(struct old_info_monitor, port));
  printf("offsetof(mode) = %d\n", 
	 (int) offsetof(struct old_info_monitor, mode));
  printf("offsetof(version) = %d\n", 
	 (int) offsetof(struct old_info_monitor, version));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct old_info_monitor, v6_flag));
  printf("offsetof(addr6) = %d\n", 
	 (int) offsetof(struct old_info_monitor, addr6));
  printf("\n");

  printf("sizeof(struct reset_flags) = %d\n", 
	 (int) sizeof(struct reset_flags));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct reset_flags, flags));
  printf("\n");

  printf("sizeof(struct info_auth) = %d\n", 
	 (int) sizeof(struct info_auth));
  printf("offsetof(timereset) = %d\n", 
	 (int) offsetof(struct info_auth, timereset));
  printf("offsetof(numkeys) = %d\n", 
	 (int) offsetof(struct info_auth, numkeys));
  printf("offsetof(numfreekeys) = %d\n", 
	 (int) offsetof(struct info_auth, numfreekeys));
  printf("offsetof(keylookups) = %d\n", 
	 (int) offsetof(struct info_auth, keylookups));
  printf("offsetof(keynotfound) = %d\n", 
	 (int) offsetof(struct info_auth, keynotfound));
  printf("offsetof(encryptions) = %d\n", 
	 (int) offsetof(struct info_auth, encryptions));
  printf("offsetof(decryptions) = %d\n", 
	 (int) offsetof(struct info_auth, decryptions));
  printf("offsetof(expired) = %d\n", 
	 (int) offsetof(struct info_auth, expired));
  printf("offsetof(keyuncached) = %d\n", 
	 (int) offsetof(struct info_auth, keyuncached));
  printf("\n");

  printf("sizeof(struct info_trap) = %d\n", 
	 (int) sizeof(struct info_trap));
  printf("offsetof(local_address) = %d\n", 
	 (int) offsetof(struct info_trap, local_address));
  printf("offsetof(trap_address) = %d\n", 
	 (int) offsetof(struct info_trap, trap_address));
  printf("offsetof(trap_port) = %d\n", 
	 (int) offsetof(struct info_trap, trap_port));
  printf("offsetof(sequence) = %d\n", 
	 (int) offsetof(struct info_trap, sequence));
  printf("offsetof(settime) = %d\n", 
	 (int) offsetof(struct info_trap, settime));
  printf("offsetof(origtime) = %d\n", 
	 (int) offsetof(struct info_trap, origtime));
  printf("offsetof(resets) = %d\n", 
	 (int) offsetof(struct info_trap, resets));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_trap, flags));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_trap, v6_flag));
  printf("offsetof(local_address6) = %d\n", 
	 (int) offsetof(struct info_trap, local_address6));
  printf("offsetof(trap_address6) = %d\n", 
	 (int) offsetof(struct info_trap, trap_address6));
  printf("\n");

  printf("sizeof(struct conf_trap) = %d\n", 
	 (int) sizeof(struct conf_trap));
  printf("offsetof(local_address) = %d\n", 
	 (int) offsetof(struct conf_trap, local_address));
  printf("offsetof(trap_address) = %d\n", 
	 (int) offsetof(struct conf_trap, trap_address));
  printf("offsetof(trap_port) = %d\n", 
	 (int) offsetof(struct conf_trap, trap_port));
  printf("offsetof(unused) = %d\n", 
	 (int) offsetof(struct conf_trap, unused));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct conf_trap, v6_flag));
  printf("offsetof(local_address6) = %d\n", 
	 (int) offsetof(struct conf_trap, local_address6));
  printf("offsetof(trap_address6) = %d\n", 
	 (int) offsetof(struct conf_trap, trap_address6));
  printf("\n");

  printf("sizeof(struct info_control) = %d\n", 
	 (int) sizeof(struct info_control));
  printf("offsetof(ctltimereset) = %d\n", 
	 (int) offsetof(struct info_control, ctltimereset));
  printf("offsetof(numctlreq) = %d\n", 
	 (int) offsetof(struct info_control, numctlreq));
  printf("offsetof(numctlbadpkts) = %d\n", 
	 (int) offsetof(struct info_control, numctlbadpkts));
  printf("offsetof(numctlresponses) = %d\n", 
	 (int) offsetof(struct info_control, numctlresponses));
  printf("offsetof(numctlfrags) = %d\n", 
	 (int) offsetof(struct info_control, numctlfrags));
  printf("offsetof(numctlerrors) = %d\n", 
	 (int) offsetof(struct info_control, numctlerrors));
  printf("offsetof(numctltooshort) = %d\n", 
	 (int) offsetof(struct info_control, numctltooshort));
  printf("offsetof(numctlinputresp) = %d\n", 
	 (int) offsetof(struct info_control, numctlinputresp));
  printf("offsetof(numctlinputfrag) = %d\n", 
	 (int) offsetof(struct info_control, numctlinputfrag));
  printf("offsetof(numctlinputerr) = %d\n", 
	 (int) offsetof(struct info_control, numctlinputerr));
  printf("offsetof(numctlbadoffset) = %d\n", 
	 (int) offsetof(struct info_control, numctlbadoffset));
  printf("offsetof(numctlbadversion) = %d\n", 
	 (int) offsetof(struct info_control, numctlbadversion));
  printf("offsetof(numctldatatooshort) = %d\n", 
	 (int) offsetof(struct info_control, numctldatatooshort));
  printf("offsetof(numctlbadop) = %d\n", 
	 (int) offsetof(struct info_control, numctlbadop));
  printf("offsetof(numasyncmsgs) = %d\n", 
	 (int) offsetof(struct info_control, numasyncmsgs));
  printf("\n");

  printf("sizeof(struct info_clock) = %d\n", 
	 (int) sizeof(struct info_clock));
  printf("offsetof(clockadr) = %d\n", 
	 (int) offsetof(struct info_clock, clockadr));
  printf("offsetof(type) = %d\n", 
	 (int) offsetof(struct info_clock, type));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_clock, flags));
  printf("offsetof(lastevent) = %d\n", 
	 (int) offsetof(struct info_clock, lastevent));
  printf("offsetof(currentstatus) = %d\n", 
	 (int) offsetof(struct info_clock, currentstatus));
  printf("offsetof(polls) = %d\n", 
	 (int) offsetof(struct info_clock, polls));
  printf("offsetof(noresponse) = %d\n", 
	 (int) offsetof(struct info_clock, noresponse));
  printf("offsetof(badformat) = %d\n", 
	 (int) offsetof(struct info_clock, badformat));
  printf("offsetof(baddata) = %d\n", 
	 (int) offsetof(struct info_clock, baddata));
  printf("offsetof(timestarted) = %d\n", 
	 (int) offsetof(struct info_clock, timestarted));
  printf("offsetof(fudgetime1) = %d\n", 
	 (int) offsetof(struct info_clock, fudgetime1));
  printf("offsetof(fudgetime2) = %d\n", 
	 (int) offsetof(struct info_clock, fudgetime2));
  printf("offsetof(fudgeval1) = %d\n", 
	 (int) offsetof(struct info_clock, fudgeval1));
  printf("offsetof(fudgeval2) = %d\n", 
	 (int) offsetof(struct info_clock, fudgeval2));
  printf("\n");

  printf("sizeof(struct conf_fudge) = %d\n", 
	 (int) sizeof(struct conf_fudge));
  printf("offsetof(clockadr) = %d\n", 
	 (int) offsetof(struct conf_fudge, clockadr));
  printf("offsetof(which) = %d\n", 
	 (int) offsetof(struct conf_fudge, which));
  printf("offsetof(fudgetime) = %d\n", 
	 (int) offsetof(struct conf_fudge, fudgetime));
  printf("offsetof(fudgeval_flags) = %d\n", 
	 (int) offsetof(struct conf_fudge, fudgeval_flags));
  printf("\n");

  printf("sizeof(struct info_clkbug) = %d\n", 
	 (int) sizeof(struct info_clkbug));
  printf("offsetof(clockadr) = %d\n", 
	 (int) offsetof(struct info_clkbug, clockadr));
  printf("offsetof(nvalues) = %d\n", 
	 (int) offsetof(struct info_clkbug, nvalues));
  printf("offsetof(ntimes) = %d\n", 
	 (int) offsetof(struct info_clkbug, ntimes));
  printf("offsetof(svalues) = %d\n", 
	 (int) offsetof(struct info_clkbug, svalues));
  printf("offsetof(stimes) = %d\n", 
	 (int) offsetof(struct info_clkbug, stimes));
  printf("offsetof(values) = %d\n", 
	 (int) offsetof(struct info_clkbug, values));
  printf("offsetof(times) = %d\n", 
	 (int) offsetof(struct info_clkbug, times));
  printf("\n");

  printf("sizeof(struct info_kernel) = %d\n", 
	 (int) sizeof(struct info_kernel));
  printf("offsetof(offset) = %d\n", 
	 (int) offsetof(struct info_kernel, offset));
  printf("offsetof(freq) = %d\n", 
	 (int) offsetof(struct info_kernel, freq));
  printf("offsetof(maxerror) = %d\n", 
	 (int) offsetof(struct info_kernel, maxerror));
  printf("offsetof(esterror) = %d\n", 
	 (int) offsetof(struct info_kernel, esterror));
  printf("offsetof(status) = %d\n", 
	 (int) offsetof(struct info_kernel, status));
  printf("offsetof(shift) = %d\n", 
	 (int) offsetof(struct info_kernel, shift));
  printf("offsetof(constant) = %d\n", 
	 (int) offsetof(struct info_kernel, constant));
  printf("offsetof(precision) = %d\n", 
	 (int) offsetof(struct info_kernel, precision));
  printf("offsetof(tolerance) = %d\n", 
	 (int) offsetof(struct info_kernel, tolerance));
  printf("offsetof(ppsfreq) = %d\n", 
	 (int) offsetof(struct info_kernel, ppsfreq));
  printf("offsetof(jitter) = %d\n", 
	 (int) offsetof(struct info_kernel, jitter));
  printf("offsetof(stabil) = %d\n", 
	 (int) offsetof(struct info_kernel, stabil));
  printf("offsetof(jitcnt) = %d\n", 
	 (int) offsetof(struct info_kernel, jitcnt));
  printf("offsetof(calcnt) = %d\n", 
	 (int) offsetof(struct info_kernel, calcnt));
  printf("offsetof(errcnt) = %d\n", 
	 (int) offsetof(struct info_kernel, errcnt));
  printf("offsetof(stbcnt) = %d\n", 
	 (int) offsetof(struct info_kernel, stbcnt));
  printf("\n");

  printf("sizeof(struct info_if_stats) = %d\n", 
	 (int) sizeof(struct info_if_stats));
  printf("offsetof(unaddr) = %d\n", 
	 (int) offsetof(struct info_if_stats, unaddr));
  printf("offsetof(unbcast) = %d\n", 
	 (int) offsetof(struct info_if_stats, unbcast));
  printf("offsetof(unmask) = %d\n", 
	 (int) offsetof(struct info_if_stats, unmask));
  printf("offsetof(v6_flag) = %d\n", 
	 (int) offsetof(struct info_if_stats, v6_flag));
  printf("offsetof(name) = %d\n", 
	 (int) offsetof(struct info_if_stats, name));
  printf("offsetof(flags) = %d\n", 
	 (int) offsetof(struct info_if_stats, flags));
  printf("offsetof(last_ttl) = %d\n", 
	 (int) offsetof(struct info_if_stats, last_ttl));
  printf("offsetof(num_mcast) = %d\n", 
	 (int) offsetof(struct info_if_stats, num_mcast));
  printf("offsetof(received) = %d\n", 
	 (int) offsetof(struct info_if_stats, received));
  printf("offsetof(sent) = %d\n", 
	 (int) offsetof(struct info_if_stats, sent));
  printf("offsetof(notsent) = %d\n", 
	 (int) offsetof(struct info_if_stats, notsent));
  printf("offsetof(uptime) = %d\n", 
	 (int) offsetof(struct info_if_stats, uptime));
  printf("offsetof(scopeid) = %d\n", 
	 (int) offsetof(struct info_if_stats, scopeid));
  printf("offsetof(ifindex) = %d\n", 
	 (int) offsetof(struct info_if_stats, ifindex));
  printf("offsetof(ifnum) = %d\n", 
	 (int) offsetof(struct info_if_stats, ifnum));
  printf("offsetof(peercnt) = %d\n", 
	 (int) offsetof(struct info_if_stats, peercnt));
  printf("offsetof(family) = %d\n", 
	 (int) offsetof(struct info_if_stats, family));
  printf("offsetof(ignore_packets) = %d\n", 
	 (int) offsetof(struct info_if_stats, ignore_packets));
  printf("offsetof(action) = %d\n", 
	 (int) offsetof(struct info_if_stats, action));
  printf("offsetof(_filler0) = %d\n", 
	 (int) offsetof(struct info_if_stats, _filler0));
  printf("\n");

  printf("sizeof(struct info_dns_assoc) = %d\n", 
	 (int) sizeof(struct info_dns_assoc));
  printf("offsetof(peeraddr) = %d\n", 
	 (int) offsetof(struct info_dns_assoc, peeraddr));
  printf("offsetof(associd) = %d\n", 
	 (int) offsetof(struct info_dns_assoc, associd));
  printf("offsetof(hostname) = %d\n", 
	 (int) offsetof(struct info_dns_assoc, hostname));
  printf("\n");


	return 0;
}

} // namespace
