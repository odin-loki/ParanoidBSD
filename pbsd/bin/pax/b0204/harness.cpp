/*
 * harness.cpp -- differential test for PBSD batch b0204.
 */

#define _GNU_SOURCE
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <regex.h>
#include <string>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

import pbsd.bin.pax.b0204;

namespace port = pbsd::bin_pax::b0204;

extern "C" {
int ref_fn_match_x(char *, char *, char **);
char *ref_range_match_x(char *, int);
int ref_fix_path_x(char *, int *, char *, int);
int ref_resub_x(regex_t *, regmatch_t *, char *, char *, char *, char *);
off_t ref_tar_endrd(void);
int ref_tar_trail(char *, int, int *);
int ref_ul_oct_x(unsigned long, char *, int, int);
int ref_uqd_oct_x(unsigned long long, char *, int, int);
unsigned long ref_tar_chksm_x(char *, int);
int ref_tar_id(char *, int);
int ref_tar_rd(port::ARCHD *, char *);
int ref_ustar_id(char *, int);
int ref_ustar_rd(port::ARCHD *, char *);
char *ref_name_split_x(char *, int);
int ref_cpio_trail(port::ARCHD *);
int ref_cpio_id(char *, int);
off_t ref_cpio_endrd(void);
int ref_vcpio_id(char *, int);
int ref_crc_id(char *, int);
off_t ref_vcpio_endrd(void);
int ref_bcpio_id(char *, int);
off_t ref_bcpio_endrd(void);
int ref_bcpio_swp_head(void);
int ref_sizeof_HD_TAR(void);
int ref_sizeof_HD_USTAR(void);
int ref_sizeof_HD_CPIO(void);
int ref_sizeof_HD_VCPIO(void);
int ref_sizeof_HD_BCPIO(void);
int ref_sizeof_ARCHD(void);

void paxwarn(int, const char *, ...);
int l_strncpy(char *, const char *, int);
unsigned long asc_ul(char *, int, int);
unsigned long long asc_uqd(char *, int, int);
int uid_name(char *, uid_t *);
int gid_name(char *, gid_t *);
}

int dflag = 0;

extern "C" void paxwarn(int, const char *, ...) {}
extern "C" int uid_name(char *, uid_t *) { return -1; }
extern "C" int gid_name(char *, gid_t *) { return -1; }

extern "C" int l_strncpy(char *dest, const char *src, int len)
{
	char *stop;
	char *start;

	stop = dest + len;
	start = dest;
	while ((dest < stop) && (*src != '\0'))
		*dest++ = *src++;
	len = (int)(dest - start);
	while (dest < stop)
		*dest++ = '\0';
	return (len);
}

extern "C" unsigned long asc_ul(char *str, int len, int base)
{
	char *stop;
	unsigned long tval = 0;

	stop = str + len;
	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;
	if (base == 16) {
		while (str < stop) {
			if ((*str >= '0') && (*str <= '9'))
				tval = (tval << 4) + (*str++ - '0');
			else if ((*str >= 'A') && (*str <= 'F'))
				tval = (tval << 4) + 10 + (*str++ - 'A');
			else if ((*str >= 'a') && (*str <= 'f'))
				tval = (tval << 4) + 10 + (*str++ - 'a');
			else
				break;
		}
	} else {
		while ((str < stop) && (*str >= '0') && (*str <= '7'))
			tval = (tval << 3) + (*str++ - '0');
	}
	return (tval);
}

extern "C" unsigned long long asc_uqd(char *str, int len, int base)
{
	char *stop;
	unsigned long long tval = 0;

	stop = str + len;
	while ((str < stop) && ((*str == ' ') || (*str == '0')))
		++str;
	if (base == 16) {
		while (str < stop) {
			if ((*str >= '0') && (*str <= '9'))
				tval = (tval << 4) + (*str++ - '0');
			else if ((*str >= 'A') && (*str <= 'F'))
				tval = (tval << 4) + 10 + (*str++ - 'A');
			else if ((*str >= 'a') && (*str <= 'f'))
				tval = (tval << 4) + 10 + (*str++ - 'a');
			else
				break;
		}
	} else {
		while ((str < stop) && (*str >= '0') && (*str <= '7'))
			tval = (tval << 3) + (*str++ - '0');
	}
	return (tval);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int MAX_REPORT = 8;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int HEX = 16;
constexpr int OCT = 8;
constexpr int BLKMULT = 512;
constexpr int TNMSZ = 100;
constexpr int TPFSZ = 155;
constexpr int PAXPATHLEN = 3072;
constexpr char TMAGIC[] = "ustar";

enum StatId {
	S_FN_MATCH, S_RANGE_MATCH, S_FIX_PATH, S_RESUB,
	S_TAR_ENDRD, S_TAR_TRAIL, S_UL_OCT, S_UQD_OCT, S_TAR_CHKSM,
	S_TAR_ID, S_TAR_RD, S_USTAR_ID, S_USTAR_RD, S_NAME_SPLIT,
	S_CPIO_TRAIL, S_CPIO_ID, S_CPIO_ENDRD,
	S_VCPIO_ID, S_CRC_ID, S_VCPIO_ENDRD,
	S_BCPIO_ID, S_BCPIO_ENDRD, S_BCPIO_SWP,
	NSTAT
};

struct Stats { const char *name; long long cases; long long fails; int reported; };

Stats g_stat[NSTAT] = {
	{ "fn_match", 0,0,0 }, { "range_match", 0,0,0 }, { "fix_path", 0,0,0 },
	{ "resub", 0,0,0 }, { "tar_endrd", 0,0,0 }, { "tar_trail", 0,0,0 },
	{ "ul_oct", 0,0,0 }, { "uqd_oct", 0,0,0 }, { "tar_chksm", 0,0,0 },
	{ "tar_id", 0,0,0 }, { "tar_rd", 0,0,0 }, { "ustar_id", 0,0,0 },
	{ "ustar_rd", 0,0,0 }, { "name_split", 0,0,0 }, { "cpio_trail", 0,0,0 },
	{ "cpio_id", 0,0,0 }, { "cpio_endrd", 0,0,0 }, { "vcpio_id", 0,0,0 },
	{ "crc_id", 0,0,0 }, { "vcpio_endrd", 0,0,0 }, { "bcpio_id", 0,0,0 },
	{ "bcpio_endrd", 0,0,0 }, { "bcpio_swp", 0,0,0 },
};

std::uint64_t rng_state = 0xb0204facefeedULL;
std::uint64_t rnd_u64(void) {
	std::uint64_t z; rng_state += 0x9e3779b97f4a7c15ULL; z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL; return z ^ (z >> 31);
}
unsigned rnd_u32(void) { return (unsigned)(rnd_u64() & 0xffffffffu); }
int rnd_i32(void) { return (int)rnd_u32(); }
unsigned char rnd_byte(void) { return (unsigned char)(rnd_u32() & 0xffu); }

void fail_msg(StatId w, const char *l, const char *d) {
	g_stat[w].fails++;
	if (g_stat[w].reported++ < MAX_REPORT)
		std::printf("  FAIL %-14s %-24s %s\n", g_stat[w].name, l, d);
}
void case_inc(StatId w) { g_stat[w].cases++; }

struct GuardedBuf { unsigned char data[BLKMULT + 64]; };
void guard_fill(GuardedBuf *g) { std::memset(g->data, GUARD, sizeof(g->data)); }
unsigned char *buf_mid(GuardedBuf *g) { return g->data + 32; }
bool guard_ok(const GuardedBuf *g) {
	for (std::size_t i = 0; i < 32; i++)
		if (g->data[i] != GUARD || g->data[32 + BLKMULT + i] != GUARD) return false;
	return true;
}

void fill_pat(unsigned char *dst, int len, bool wild) {
	for (int i = 0; i < len; i++) {
		unsigned char c = rnd_byte();
		if (!wild) {
			if (c == '\0' || c == '?' || c == '*' || c == '[' || c == ']' || c == '\\')
				c = 'a' + (c % 26);
		} else {
			unsigned pick = rnd_u32() % 12;
			if (pick == 0) c = '?'; else if (pick == 1) c = '*';
			else if (pick == 2) c = '['; else if (pick == 3) c = ']';
			else if (pick == 4) c = '\\'; else if (pick == 5) c = '\0';
			else if (c == '\0') c = 'x';
		}
		dst[i] = c;
	}
	dst[len] = '\0';
}

void test_fn_match_one(const char *label, const char *pat, const char *str, int dfl) {
	char prf[256], ptf[256], psf[256], ssf[256];
	char *pend_ref = nullptr, *pend_port = nullptr;
	int rref, rport;
	std::strncpy(prf, pat, sizeof(prf)-1); std::strncpy(psf, str, sizeof(psf)-1);
	std::strncpy(ptf, pat, sizeof(ptf)-1); std::strncpy(ssf, str, sizeof(ssf)-1);
	dflag = dfl; case_inc(S_FN_MATCH);
	rref = ref_fn_match_x(prf, psf, &pend_ref);
	rport = port::fn_match_x(ptf, ssf, &pend_port);
	if (rref != rport) fail_msg(S_FN_MATCH, label, "return");
	else if ((pend_ref==nullptr)!=(pend_port==nullptr)) fail_msg(S_FN_MATCH, label, "pend null");
	else if (pend_ref && (pend_ref-psf)!=(pend_port-ssf)) fail_msg(S_FN_MATCH, label, "pend off");
}

void test_fn_match_edge(void) {
	test_fn_match_one("exact","foo","foo",0);
	test_fn_match_one("empty","","",0);
	test_fn_match_one("mismatch","foo","bar",0);
	test_fn_match_one("qmark","f?o","fao",0);
	test_fn_match_one("qmark_fail","f?o","fo",0);
	test_fn_match_one("star_end","foo*","foobar",0);
	test_fn_match_one("star_only","*","anything",0);
	test_fn_match_one("prefix_slash","foo","foo/bar",0);
	test_fn_match_one("prefix_dflag","foo","foo/bar",1);
	test_fn_match_one("range","[a-z]","m",0);
	test_fn_match_one("range_neg","[!a-z]","9",0);
	test_fn_match_one("escape","f\\oo","foo",0);
	test_fn_match_one("highbit","\x80\xff","\x80\xff",0);
}

void test_fn_match_random(void) {
	unsigned char pat[128], str[128];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int plen = (int)(rnd_u32()%40), slen = (int)(rnd_u32()%40);
		fill_pat(pat, plen, (rnd_u32()&3u)==0);
		fill_pat(str, slen, false);
		dflag = (int)(rnd_u32()&1u);
		test_fn_match_one("random",(char*)pat,(char*)str,dflag);
	}
}

void test_range_match_one(const char *label, const char *pat, int test) {
	char prf[64], ptf[64]; char *rref, *rport;
	std::strncpy(prf,pat,sizeof(prf)-1); std::strncpy(ptf,pat,sizeof(ptf)-1);
	case_inc(S_RANGE_MATCH);
	rref = ref_range_match_x(prf, test); rport = port::range_match_x(ptf, test);
	if ((rref==nullptr)!=(rport==nullptr)) fail_msg(S_RANGE_MATCH,label,"null");
	else if (rref && (rref-prf)!=(rport-ptf)) fail_msg(S_RANGE_MATCH,label,"off");
}

void test_range_match_edge(void) {
	test_range_match_one("single","a]",'a');
	test_range_match_one("range","a-c]",'b');
	test_range_match_one("neg","!a]",'b');
	test_range_match_one("high","a-\xff]",0xff);
	test_range_match_one("illegal","a",'a');
}

void test_range_match_random(void) {
	unsigned char pat[64];
	for (unsigned i=0;i<RAND_ITERS;i++) {
		int t=(int)rnd_byte(), len=2+(int)(rnd_u32()%20);
		pat[0]=(unsigned char)('a'+(rnd_u32()%5));
		for(int j=1;j<len;j++) pat[j]=(unsigned char)('a'+(rnd_u32()%26));
		pat[len]=']'; pat[len+1]='\0';
		test_range_match_one("random",(char*)pat,t);
	}
}

void test_fix_path_one(const char *label, const char *or_name, int or_len, const char *dir, int dir_len) {
	char nref[PAXPATHLEN+32], nport[PAXPATHLEN+32];
	int lref=or_len, lport=or_len, rref, rport;
	std::memset(nref,GUARD,sizeof(nref)); std::memset(nport,GUARD,sizeof(nport));
	std::memcpy(nref+16,or_name,(std::size_t)or_len+1);
	std::memcpy(nport+16,or_name,(std::size_t)or_len+1);
	case_inc(S_FIX_PATH);
	rref=ref_fix_path_x(nref+16,&lref,(char*)dir,dir_len);
	rport=port::fix_path_x(nport+16,&lport,(char*)dir,dir_len);
	if (rref!=rport||lref!=lport||std::memcmp(nref,nport,sizeof(nref))!=0)
		fail_msg(S_FIX_PATH,label,"mismatch");
}

void test_fix_path_edge(void) {
	test_fix_path_one("simple","file",4,"/tmp/",5);
	test_fix_path_one("abs_skip","/abs",4,"/d/",3);
	test_fix_path_one("empty","",0,"/x/",3);
	test_fix_path_one("single","a",1,"/b/",3);
}

void test_fix_path_random(void) {
	char orname[PAXPATHLEN], dir[256];
	for (unsigned i=0;i<RAND_ITERS;i++) {
		int olen=(int)(rnd_u32()%200), dlen=1+(int)(rnd_u32()%100);
		for(int j=0;j<olen;j++) orname[j]=(char)(rnd_byte()?rnd_byte():'x'); orname[olen]='\0';
		dir[0]='/'; for(int j=1;j<dlen;j++) dir[j]=(char)('a'+(rnd_u32()%26));
		dir[dlen]='/'; dir[dlen+1]='\0';
		test_fix_path_one("random",orname,olen,dir,dlen+1);
	}
}

void test_resub_one(const char *label, const char *re, const char *orig, const char *sub) {
	regex_t rcmp; regmatch_t pm[10]; char dref[256], dport[256]; int rref, rport;
	if (regcomp(&rcmp,re,0)!=0) return;
	if (regexec(&rcmp,orig,10,pm,0)!=0) { regfree(&rcmp); return; }
	std::memset(dref,GUARD,sizeof(dref)); std::memset(dport,GUARD,sizeof(dport));
	case_inc(S_RESUB);
	rref=ref_resub_x(&rcmp,pm,(char*)orig,(char*)sub,dref+16,dref+216);
	rport=port::resub_x(&rcmp,pm,(char*)orig,(char*)sub,dport+16,dport+216);
	regfree(&rcmp);
	if (rref!=rport||std::memcmp(dref,dport,sizeof(dref))!=0) fail_msg(S_RESUB,label,"mismatch");
}

void test_resub_edge(void) {
	test_resub_one("amp","foo","foobar","&");
	test_resub_one("sub","(f)(o+)","foobar","\\1-\\2");
	test_resub_one("plain","bar","bar","X");
}

void test_resub_random(void) {
	const char *reps[]={"&","\\1","X","\\&","\\\\","Y\\1Z"};
	const char *res[]={"a","(a)","(a)(b)","f+","[a-z]"};
	const char *strs[]={"a","ab","foo","aaa","xyz"};
	for (unsigned i=0;i<RAND_ITERS;i++)
		test_resub_one("random",res[rnd_u32()%5],strs[rnd_u32()%5],reps[rnd_u32()%6]);
}

void test_tar_endrd(void) {
	case_inc(S_TAR_ENDRD);
	if (ref_tar_endrd()!=port::tar_endrd()) fail_msg(S_TAR_ENDRD,"value","mismatch");
}

void test_tar_trail_one(const char *label, bool zero, int resync, int cnt_in) {
	GuardedBuf gref,gport; int cref=cnt_in,cport=cnt_in,rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	std::memset(buf_mid(&gref),zero?0:1,BLKMULT);
	std::memset(buf_mid(&gport),zero?0:1,BLKMULT);
	case_inc(S_TAR_TRAIL);
	rref=ref_tar_trail((char*)buf_mid(&gref),resync,&cref);
	rport=port::tar_trail((char*)buf_mid(&gport),resync,&cport);
	if (rref!=rport||cref!=cport) fail_msg(S_TAR_TRAIL,label,"mismatch");
}

void test_tar_trail_edge(void) {
	test_tar_trail_one("nonzero",false,0,0);
	test_tar_trail_one("zero1",true,0,0);
	test_tar_trail_one("zero2",true,0,1);
	test_tar_trail_one("resync",true,1,0);
}

void test_tar_trail_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		GuardedBuf gref,gport; int cref=(int)(rnd_u32()%4),cport=cref,rref,rport;
		bool allz=(rnd_u32()&1u)!=0;
		guard_fill(&gref); guard_fill(&gport);
		for(int j=0;j<BLKMULT;j++) { unsigned char c=allz?0:rnd_byte(); buf_mid(&gref)[j]=c; buf_mid(&gport)[j]=c; }
		case_inc(S_TAR_TRAIL);
		int rs=(int)(rnd_u32()&1u);
		rref=ref_tar_trail((char*)buf_mid(&gref),rs,&cref);
		rport=port::tar_trail((char*)buf_mid(&gport),rs,&cport);
		if (rref!=rport||cref!=cport) fail_msg(S_TAR_TRAIL,"random","mismatch");
	}
}

void test_ul_oct_one(const char *label, unsigned long val, int len, int term) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport); case_inc(S_UL_OCT);
	rref=ref_ul_oct_x(val,(char*)buf_mid(&gref),len,term);
	rport=port::ul_oct_x(val,(char*)buf_mid(&gport),len,term);
	if (rref!=rport||std::memcmp(buf_mid(&gref),buf_mid(&gport),(std::size_t)len)!=0)
		fail_msg(S_UL_OCT,label,"mismatch");
}

void test_ul_oct_edge(void) {
	for (int term=0;term<=3;term++) {
		test_ul_oct_one("zero",0,8,term);
		test_ul_oct_one("one",1,8,term);
		test_ul_oct_one("maxfit",07777777UL,8,term);
		test_ul_oct_one("overflow",0100000000UL,8,term);
	}
}

void test_ul_oct_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++)
		test_ul_oct_one("random",rnd_u64(),1+(int)(rnd_u32()%12),(int)(rnd_u32()%4));
}

void test_uqd_oct_one(const char *label, unsigned long long val, int len, int term) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport); case_inc(S_UQD_OCT);
	rref=ref_uqd_oct_x(val,(char*)buf_mid(&gref),len,term);
	rport=port::uqd_oct_x(val,(char*)buf_mid(&gport),len,term);
	if (rref!=rport||std::memcmp(buf_mid(&gref),buf_mid(&gport),(std::size_t)len)!=0)
		fail_msg(S_UQD_OCT,label,"mismatch");
}

void test_uqd_oct_edge(void) {
	test_uqd_oct_one("zero",0,12,1);
	test_uqd_oct_one("big",0777777777777777ULL,12,3);
	test_uqd_oct_one("oflow",01000000000000000ULL,12,1);
}

void test_uqd_oct_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++)
		test_uqd_oct_one("random",rnd_u64(),1+(int)(rnd_u32()%12),(int)(rnd_u32()%4));
}

void test_tar_chksm_one(const char *label, int len) {
	GuardedBuf gref,gport; unsigned long rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	for(int i=0;i<len;i++) { buf_mid(&gref)[i]=rnd_byte(); buf_mid(&gport)[i]=buf_mid(&gref)[i]; }
	case_inc(S_TAR_CHKSM);
	rref=ref_tar_chksm_x((char*)buf_mid(&gref),len);
	rport=port::tar_chksm_x((char*)buf_mid(&gport),len);
	if (rref!=rport) fail_msg(S_TAR_CHKSM,label,"mismatch");
}

void test_tar_chksm_edge(void) {
	test_tar_chksm_one("min",149);
	test_tar_chksm_one("block",BLKMULT);
}

void test_tar_chksm_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) test_tar_chksm_one("random",149+(int)(rnd_u32()%363));
}

void fmt_oct(unsigned long v, char *s, int n) {
	char tmp[32]; int i=31; tmp[i--]='\0';
	if (v==0) tmp[i--]='0';
	while (v>0 && i>=0) { tmp[i--]=(char)('0'+(v&7)); v>>=3; }
	tmp[i--]=' '; int j=0;
	while (tmp[i+1]!='\0' && j<n-1) s[j++]=tmp[++i];
	while (j<n) s[j++]='\0';
}

void make_tar_hdr(GuardedBuf *g, const char *name, char linkflag, unsigned long mode,
    unsigned long uid, unsigned long gid, unsigned long long size,
    unsigned long long mtime, const char *linkname) {
	unsigned char *b=buf_mid(g); std::memset(b,0,BLKMULT);
	std::strncpy((char*)b,name,TNMSZ);
	fmt_oct(mode,(char*)b+100,8); fmt_oct(uid,(char*)b+108,8); fmt_oct(gid,(char*)b+116,8);
	ref_uqd_oct_x(size,(char*)b+124,12,1); ref_uqd_oct_x(mtime,(char*)b+136,12,1);
	b[156]=(unsigned char)linkflag;
	if (linkname) std::strncpy((char*)b+157,linkname,TNMSZ);
	unsigned long sum=ref_tar_chksm_x((char*)b,BLKMULT);
	ref_ul_oct_x(sum,(char*)b+148,8,3);
}

void test_tar_id_one(const char *label) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	make_tar_hdr(&gref,"testfile",'\0',0644,0,0,100,1000000,nullptr);
	std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT);
	case_inc(S_TAR_ID);
	rref=ref_tar_id((char*)buf_mid(&gref),BLKMULT);
	rport=port::tar_id((char*)buf_mid(&gport),BLKMULT);
	if (rref!=rport) fail_msg(S_TAR_ID,label,"mismatch");
}

void test_tar_id_edge(void) {
	GuardedBuf g; int r;
	guard_fill(&g); case_inc(S_TAR_ID);
	r=ref_tar_id((char*)buf_mid(&g),BLKMULT-1);
	if (r!=port::tar_id((char*)buf_mid(&g),BLKMULT-1)||r!=-1) fail_msg(S_TAR_ID,"small","mismatch");
	guard_fill(&g); std::memset(buf_mid(&g),0,BLKMULT); case_inc(S_TAR_ID);
	r=ref_tar_id((char*)buf_mid(&g),BLKMULT);
	if (r!=port::tar_id((char*)buf_mid(&g),BLKMULT)||r!=-1) fail_msg(S_TAR_ID,"zeros","mismatch");
	test_tar_id_one("valid");
}

void test_tar_id_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		GuardedBuf gref,gport; char name[32];
		int nlen=1+(int)(rnd_u32()%20);
		for(int j=0;j<nlen;j++) name[j]=(char)('a'+(rnd_u32()%26)); name[nlen]='\0';
		guard_fill(&gref); guard_fill(&gport);
		make_tar_hdr(&gref,name,'\0',rnd_u32()&07777u,rnd_u32()&07777u,rnd_u32()&07777u,
		    rnd_u64()&077777u,rnd_u64(),nullptr);
		std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT); case_inc(S_TAR_ID);
		if (ref_tar_id((char*)buf_mid(&gref),BLKMULT)!=port::tar_id((char*)buf_mid(&gport),BLKMULT))
			fail_msg(S_TAR_ID,"random","mismatch");
	}
}

void test_tar_rd_one(const char *label, const char *name, char linkflag, const char *linkname) {
	GuardedBuf gref,gport; port::ARCHD aref,aport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	make_tar_hdr(&gref,name,linkflag,0644,1000,1000,50,12345,linkname);
	std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT); case_inc(S_TAR_RD);
	rref=ref_tar_rd(&aref,(char*)buf_mid(&gref));
	rport=port::tar_rd(&aport,(char*)buf_mid(&gport));
	if (rref!=rport||aref.type!=aport.type||aref.nlen!=aport.nlen||
	    aref.sb.st_size!=aport.sb.st_size||std::strcmp(aref.name,aport.name)!=0)
		fail_msg(S_TAR_RD,label,"mismatch");
}

void test_tar_rd_edge(void) {
	test_tar_rd_one("reg","file.txt",'\0',nullptr);
	test_tar_rd_one("dir_slash","dir/",'\0',nullptr);
	test_tar_rd_one("symlink","lnk",'2',"target");
	test_tar_rd_one("hardlink","hlk",'1',"target");
	test_tar_rd_one("dirtype","d",'5',nullptr);
}

void test_tar_rd_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		char name[40]; int nlen=1+(int)(rnd_u32()%15);
		for(int j=0;j<nlen;j++) name[j]=(char)('a'+(rnd_u32()%26));
		if (rnd_u32()&1u) { name[nlen++]='/'; name[nlen]='\0'; } else name[nlen]='\0';
		test_tar_rd_one("random",name,(char)('\0'+(rnd_u32()%6)),"tgt");
	}
}

void make_ustar_hdr(GuardedBuf *g, const char *name, char typeflag, unsigned long mode, unsigned long long size) {
	unsigned char *b=buf_mid(g); std::memset(b,0,BLKMULT);
	std::strncpy((char*)b,name,TNMSZ); fmt_oct(mode,(char*)b+100,8);
	ref_uqd_oct_x(size,(char*)b+124,12,3); ref_uqd_oct_x(1000,(char*)b+136,12,3);
	b[156]=(unsigned char)typeflag; std::memcpy(b+257,TMAGIC,5); b[262]='0'; b[263]='0';
	unsigned long sum=ref_tar_chksm_x((char*)b,BLKMULT); ref_ul_oct_x(sum,(char*)b+148,8,3);
}

void test_ustar_id_one(const char *label, bool valid) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	if (valid) make_ustar_hdr(&gref,"ustarfile",'0',0644,0);
	else std::memset(buf_mid(&gref),0,BLKMULT);
	std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT); case_inc(S_USTAR_ID);
	rref=ref_ustar_id((char*)buf_mid(&gref),BLKMULT);
	rport=port::ustar_id((char*)buf_mid(&gport),BLKMULT);
	if (rref!=rport) fail_msg(S_USTAR_ID,label,"mismatch");
}

void test_ustar_id_edge(void) { test_ustar_id_one("valid",true); test_ustar_id_one("zero",false); }

void test_ustar_id_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		GuardedBuf gref,gport; char name[32];
		int nlen=1+(int)(rnd_u32()%20);
		for(int j=0;j<nlen;j++) name[j]=(char)('a'+(rnd_u32()%26)); name[nlen]='\0';
		guard_fill(&gref); guard_fill(&gport);
		make_ustar_hdr(&gref,name,'0',rnd_u32()&07777u,rnd_u64()&0777u);
		std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT); case_inc(S_USTAR_ID);
		if (ref_ustar_id((char*)buf_mid(&gref),BLKMULT)!=port::ustar_id((char*)buf_mid(&gport),BLKMULT))
			fail_msg(S_USTAR_ID,"random","mismatch");
	}
}

void test_ustar_rd_one(const char *label, const char *name, char typeflag) {
	GuardedBuf gref,gport; port::ARCHD aref,aport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	make_ustar_hdr(&gref,name,typeflag,0644,100);
	std::memcpy(buf_mid(&gport),buf_mid(&gref),BLKMULT); case_inc(S_USTAR_RD);
	rref=ref_ustar_rd(&aref,(char*)buf_mid(&gref));
	rport=port::ustar_rd(&aport,(char*)buf_mid(&gport));
	if (rref!=rport||aref.type!=aport.type||std::strcmp(aref.name,aport.name)!=0)
		fail_msg(S_USTAR_RD,label,"mismatch");
}

void test_ustar_rd_edge(void) {
	test_ustar_rd_one("reg","file",'0');
	test_ustar_rd_one("dir","dir",'5');
	test_ustar_rd_one("fifo","fifo",'6');
}

void test_ustar_rd_random(void) {
	static const char types[]={'0','1','2','3','4','5','6','7'};
	for (unsigned i=0;i<RAND_ITERS;i++) {
		char name[32]; int nlen=1+(int)(rnd_u32()%15);
		for(int j=0;j<nlen;j++) name[j]=(char)('a'+(rnd_u32()%26)); name[nlen]='\0';
		test_ustar_rd_one("random",name,types[rnd_u32()%8]);
	}
}

void test_name_split_one(const char *label, char *name, int len) {
	char nref[PAXPATHLEN+16], nport[PAXPATHLEN+16]; char *rref,*rport;
	std::memset(nref,GUARD,sizeof(nref)); std::memset(nport,GUARD,sizeof(nport));
	std::memcpy(nref+8,name,(std::size_t)len); nref[8+len]='\0';
	std::memcpy(nport+8,name,(std::size_t)len); nport[8+len]='\0';
	case_inc(S_NAME_SPLIT);
	rref=ref_name_split_x(nref+8,len); rport=port::name_split_x(nport+8,len);
	if ((rref==nullptr)!=(rport==nullptr)) fail_msg(S_NAME_SPLIT,label,"null");
	else if (rref && (rref-(nref+8))!=(rport-(nport+8))) fail_msg(S_NAME_SPLIT,label,"off");
}

void test_name_split_edge(void) {
	char shortn[]="file"; test_name_split_one("short",shortn,4);
	char longn[300]; int i;
	for(i=0;i<120;i++) longn[i]='a'; longn[50]='/'; longn[119]='z'; longn[120]='\0';
	test_name_split_one("long",longn,120);
	char noslash[150]; for(i=0;i<110;i++) noslash[i]='b'; noslash[110]='\0';
	test_name_split_one("noslash",noslash,110);
}

void test_name_split_random(void) {
	char name[PAXPATHLEN];
	for (unsigned i=0;i<RAND_ITERS;i++) {
		int len=TNMSZ+1+(int)(rnd_u32()%200);
		for(int j=0;j<len;j++) name[j]=(rnd_u32()%30==0)?'/':(char)('a'+(rnd_u32()%26));
		name[len]='\0'; test_name_split_one("random",name,len);
	}
}

void test_cpio_trail_one(const char *label, const char *name, off_t size) {
	port::ARCHD aref,aport; int rref,rport;
	std::memset(&aref,0,sizeof(aref)); std::memset(&aport,0,sizeof(aport));
	std::strncpy(aref.name,name,sizeof(aref.name)-1);
	std::strncpy(aport.name,name,sizeof(aport.name)-1);
	aref.sb.st_size=size; aport.sb.st_size=size; case_inc(S_CPIO_TRAIL);
	rref=ref_cpio_trail(&aref); rport=port::cpio_trail(&aport);
	if (rref!=rport) fail_msg(S_CPIO_TRAIL,label,"mismatch");
}

void test_cpio_trail_edge(void) {
	test_cpio_trail_one("trailer","TRAILER!!!",0);
	test_cpio_trail_one("not","file",0);
	test_cpio_trail_one("size","TRAILER!!!",1);
}

void test_cpio_trail_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		char name[32]; int nlen=(int)(rnd_u32()%12);
		for(int j=0;j<nlen;j++) name[j]=(char)('A'+(rnd_u32()%26)); name[nlen]='\0';
		test_cpio_trail_one("random",name,(off_t)rnd_i32());
	}
}

void test_cpio_id_one(const char *label, const char *magic, int size) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	std::memcpy(buf_mid(&gref),magic,6); std::memcpy(buf_mid(&gport),magic,6);
	case_inc(S_CPIO_ID);
	rref=ref_cpio_id((char*)buf_mid(&gref),size);
	rport=port::cpio_id((char*)buf_mid(&gport),size);
	if (rref!=rport) fail_msg(S_CPIO_ID,label,"mismatch");
}

void test_cpio_id_edge(void) {
	test_cpio_id_one("valid","070707",ref_sizeof_HD_CPIO());
	test_cpio_id_one("small","070707",ref_sizeof_HD_CPIO()-1);
	test_cpio_id_one("bad","BADMAG",ref_sizeof_HD_CPIO());
}

void test_cpio_id_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		char mag[8]; for(int j=0;j<6;j++) mag[j]=(char)('0'+(rnd_u32()%10)); mag[6]='\0';
		test_cpio_id_one("random",mag,(int)(rnd_u32()%(ref_sizeof_HD_CPIO()+4)));
	}
}

void test_cpio_endrd(void) {
	case_inc(S_CPIO_ENDRD);
	if (ref_cpio_endrd()!=port::cpio_endrd()) fail_msg(S_CPIO_ENDRD,"value","mismatch");
}

void test_vcpio_id_one(const char *label, const char *magic, int size, bool crc) {
	GuardedBuf gref,gport; int rref,rport;
	guard_fill(&gref); guard_fill(&gport);
	std::memcpy(buf_mid(&gref),magic,6); std::memcpy(buf_mid(&gport),magic,6);
	case_inc(crc?S_CRC_ID:S_VCPIO_ID);
	if (crc) { rref=ref_crc_id((char*)buf_mid(&gref),size); rport=port::crc_id((char*)buf_mid(&gport),size); }
	else { rref=ref_vcpio_id((char*)buf_mid(&gref),size); rport=port::vcpio_id((char*)buf_mid(&gport),size); }
	if (rref!=rport) fail_msg(crc?S_CRC_ID:S_VCPIO_ID,label,"mismatch");
}

void test_vcpio_id_edge(void) {
	test_vcpio_id_one("valid","070701",ref_sizeof_HD_VCPIO(),false);
	test_vcpio_id_one("crc","070702",ref_sizeof_HD_VCPIO(),true);
	test_vcpio_id_one("small","070701",1,false);
}

void test_vcpio_id_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++) {
		char mag[8]; for(int j=0;j<6;j++) mag[j]=(char)('0'+(rnd_u32()%10)); mag[6]='\0';
		bool crc=(rnd_u32()&1u)!=0;
		test_vcpio_id_one("random",mag,(int)(rnd_u32()%(ref_sizeof_HD_VCPIO()+4)),crc);
	}
}

void test_vcpio_endrd(void) {
	case_inc(S_VCPIO_ENDRD);
	if (ref_vcpio_endrd()!=port::vcpio_endrd()) fail_msg(S_VCPIO_ENDRD,"value","mismatch");
}

void test_bcpio_id_one(const char *label, unsigned short magic, bool swap) {
	unsigned char mref[2], mport[2]; int rref,rport,sref,sport;
	if (swap) { mref[0]=(unsigned char)(magic&0xff); mref[1]=(unsigned char)((magic>>8)&0xff); }
	else { mref[0]=(unsigned char)((magic>>8)&0xff); mref[1]=(unsigned char)(magic&0xff); }
	mport[0]=mref[0]; mport[1]=mref[1]; case_inc(S_BCPIO_ID);
	rref=ref_bcpio_id((char*)mref,ref_sizeof_HD_BCPIO());
	rport=port::bcpio_id((char*)mport,port::sizeof_HD_BCPIO());
	sref=ref_bcpio_swp_head(); sport=port::bcpio_swp_head();
	if (rref!=rport||sref!=sport) fail_msg(S_BCPIO_ID,label,"mismatch");
}

void test_bcpio_id_edge(void) {
	test_bcpio_id_one("normal",070707,false);
	test_bcpio_id_one("swapped",070707,true);
	test_bcpio_id_one("bad",012345,false);
}

void test_bcpio_id_random(void) {
	for (unsigned i=0;i<RAND_ITERS;i++)
		test_bcpio_id_one("random",(unsigned short)(rnd_u32()&0xffffu),(rnd_u32()&1u)!=0);
}

void test_bcpio_endrd(void) {
	case_inc(S_BCPIO_ENDRD);
	if (ref_bcpio_endrd()!=port::bcpio_endrd()) fail_msg(S_BCPIO_ENDRD,"value","mismatch");
}

int run_all(void) {
	test_fn_match_edge(); test_range_match_edge(); test_fix_path_edge(); test_resub_edge();
	test_tar_endrd(); test_tar_trail_edge(); test_ul_oct_edge(); test_uqd_oct_edge();
	test_tar_chksm_edge(); test_tar_id_edge(); test_tar_rd_edge();
	test_ustar_id_edge(); test_ustar_rd_edge(); test_name_split_edge();
	test_cpio_trail_edge(); test_cpio_id_edge(); test_cpio_endrd();
	test_vcpio_id_edge(); test_vcpio_endrd(); test_bcpio_id_edge(); test_bcpio_endrd();
	test_fn_match_random(); test_range_match_random(); test_fix_path_random(); test_resub_random();
	test_tar_trail_random(); test_ul_oct_random(); test_uqd_oct_random(); test_tar_chksm_random();
	test_tar_id_random(); test_tar_rd_random(); test_ustar_id_random(); test_ustar_rd_random();
	test_name_split_random(); test_cpio_trail_random(); test_cpio_id_random();
	test_vcpio_id_random(); test_bcpio_id_random();
	std::printf("\n%-16s %12s %12s\n","function","cases","failures");
	std::printf("%-16s %12s %12s\n","--------","-----","--------");
	long long tc=0,tf=0;
	for (int i=0;i<NSTAT;i++) {
		std::printf("%-16s %12lld %12lld\n",g_stat[i].name,g_stat[i].cases,g_stat[i].fails);
		tc+=g_stat[i].cases; tf+=g_stat[i].fails;
	}
	std::printf("%-16s %12lld %12lld\n","TOTAL",tc,tf);
	return tf==0?0:1;
}

} // namespace

int main(void) { return run_all(); }
