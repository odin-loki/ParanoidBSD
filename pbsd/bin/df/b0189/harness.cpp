/*
 * harness.cpp -- differential test for PBSD batch b0189 (df.c).
 */

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <climits>
#include <cerrno>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

import pbsd.bin.df.b0189;

namespace P = pbsd::bin_df::b0189;

#ifndef MNAMELEN
#define MNAMELEN 1024
#endif
#ifndef MFSNAMELEN
#define MFSNAMELEN 16
#endif
#ifndef MNT_WAIT
#define MNT_WAIT 0x0001
#endif
#ifndef MNT_NOWAIT
#define MNT_NOWAIT 0x0002
#endif
#ifndef MNT_IGNORE
#define MNT_IGNORE 0x800000ULL
#endif
#ifndef VFCF_NETWORK
#define VFCF_NETWORK 0x00000010
#endif

struct statfs {
	uint64_t f_flags;
	int64_t f_bsize;
	int64_t f_blocks;
	int64_t f_bfree;
	int64_t f_bavail;
	int64_t f_files;
	int64_t f_ffree;
	char f_fstypename[MFSNAMELEN];
	char f_mntonname[MNAMELEN];
	char f_mntfromname[MNAMELEN];
};

struct xvfsconf {
	uint32_t vfc_version;
	char vfc_name[MFSNAMELEN];
	int vfc_refcount;
	int vfc_flags;
};

struct maxwidths {
	int mntfrom;
	int fstype;
	int total;
	int used;
	int avail;
	int iused;
	int ifree;
};

extern "C" {
int ref_imax(int, int);
int ref_int64width(int64_t);
intmax_t ref_fsbtoblk(int64_t, uint64_t, unsigned long);
int ref_checkvfsname(const char *, const char **, int);
int ref_checkvfsselected(char *);
void ref_addstat(struct statfs *, struct statfs *);
const char **ref_makevfslist(char *, int *);
void ref_update_maxwidths(struct maxwidths *, const struct statfs *);
char *ref_getmntpt(const char *);
size_t ref_regetmntinfo(struct statfs **, long);
void ref_prthumanval(const char *, int64_t);
void ref_prthumanvalinode(const char *, int64_t);
void ref_prthuman(const struct statfs *, int64_t);
void ref_prtstat(struct statfs *, struct maxwidths *);
char *ref_makenetvfslist(void);
void ref_usage(void);
int ref_main(int argc, char *argv[]);
extern int ref_aflag, ref_cflag, ref_hflag, ref_iflag, ref_kflag;
extern int ref_lflag, ref_nflag, ref_Tflag, ref_thousands;
extern int ref_skipvfs_l, ref_skipvfs_t;
extern const char **ref_vfslist_l, **ref_vfslist_t;
}

#define SWEEP 200000L
#define MAX_SHOW 8
constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t GUARD_PAD = 64;

namespace {

bool g_test_child = false;
std::vector<std::string> g_xo_log;
std::vector<statfs> g_mnt_table;
std::vector<xvfsconf> g_vfs_conf;
int g_malloc_fail = 0;
int g_strdup_fail = 0;
int g_statfs_fail = 1;
int g_sysctl_fail = 0;
long g_getbsize_val = 512;
int g_xo_finish_rc = 0;
std::unordered_map<std::string, struct stat> g_stat_map;

struct Stat { const char *name; long cases; long fails; int shown; };
struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed) {}
	std::uint64_t next() {
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	int bits(int lo, int hi) {
		if (hi <= lo) return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}
	bool coin() { return (next() & 1u) != 0; }
	int64_t i64() { return (int64_t)next(); }
};

Rng rng(0xb0189faceULL);
Stat st_imax={"imax",0,0,0}, st_int64width={"int64width",0,0,0};
Stat st_fsbtoblk={"fsbtoblk",0,0,0}, st_checkvfsname={"checkvfsname",0,0,0};
Stat st_checkvfsselected={"checkvfsselected",0,0,0}, st_addstat={"addstat",0,0,0};
Stat st_makevfslist={"makevfslist",0,0,0}, st_update_maxwidths={"update_maxwidths",0,0,0};
Stat st_getmntpt={"getmntpt",0,0,0}, st_regetmntinfo={"regetmntinfo",0,0,0};
Stat st_prthumanval={"prthumanval",0,0,0}, st_prthumanvalinode={"prthumanvalinode",0,0,0};
Stat st_prthuman={"prthuman",0,0,0}, st_prtstat={"prtstat",0,0,0};
Stat st_makenetvfslist={"makenetvfslist",0,0,0}, st_usage={"usage",0,0,0};
Stat st_main={"main",0,0,0};

void mock_reset() {
	g_xo_log.clear(); g_mnt_table.clear(); g_vfs_conf.clear();
	g_malloc_fail = g_strdup_fail = g_sysctl_fail = 0;
	g_statfs_fail = 1; g_getbsize_val = 512; g_xo_finish_rc = 0;
	g_stat_map.clear();
}
void reset_port_globals() {
	P::aflag=P::cflag=P::hflag=P::iflag=P::kflag=P::lflag=P::nflag=P::Tflag=0;
	P::thousands=P::skipvfs_l=P::skipvfs_t=0;
	P::vfslist_l=P::vfslist_t=nullptr;
}
void reset_ref_globals() {
	ref_aflag=ref_cflag=ref_hflag=ref_iflag=ref_kflag=ref_lflag=ref_nflag=ref_Tflag=0;
	ref_thousands=ref_skipvfs_l=ref_skipvfs_t=0;
	ref_vfslist_l=ref_vfslist_t=nullptr;
}
bool fail(Stat &st, const char *what) {
	st.fails++;
	if (st.shown < MAX_SHOW) { st.shown++; std::printf("  FAIL %s: %s\n", st.name, what); }
	return false;
}
void xo_record(const char *tag, const char *fmt, va_list ap) {
	char buf[4096]; std::vsnprintf(buf, sizeof(buf), fmt, ap);
	g_xo_log.push_back(std::string(tag)+":"+buf);
}
extern "C" void *__real_malloc(size_t);
extern "C" void __real_free(void *);
extern "C" char *__real_strdup(const char *);
extern "C" void *__wrap_malloc(size_t n) {
	if (g_malloc_fail > 0) { g_malloc_fail--; errno=ENOMEM; return nullptr; }
	return __real_malloc(n);
}
extern "C" void __wrap_free(void *p) { __real_free(p); }
extern "C" char *__wrap_strdup(const char *s) {
	if (g_strdup_fail > 0) { g_strdup_fail--; errno=ENOMEM; return nullptr; }
	return __real_strdup(s);
}
extern "C" int __wrap_getmntinfo(struct statfs **mntbufp, int mode) {
	(void)mode;
	static std::vector<statfs> storage;
	storage = g_mnt_table;
	if (mntbufp) *mntbufp = storage.empty()?nullptr:storage.data();
	return (int)storage.size();
}
extern "C" int __wrap_statfs(const char *path, struct statfs *buf) {
	for (const auto &m : g_mnt_table) {
		if (std::strcmp(m.f_mntonname, path)==0) {
			if (g_statfs_fail==0) { errno=EIO; return -1; }
			*buf=m; return 0;
		}
	}
	errno=ENOENT; return -1;
}
extern "C" int __wrap_sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp, size_t newlen) {
	(void)newp;(void)newlen;
	if (g_sysctl_fail || std::strcmp(name,"vfs.conflist")!=0) { errno=ENOENT; return -1; }
	size_t need=g_vfs_conf.size()*sizeof(xvfsconf);
	if (!oldp||!oldlenp) { *oldlenp=need; return 0; }
	if (*oldlenp<need) { errno=ENOMEM; return -1; }
	std::memcpy(oldp,g_vfs_conf.data(),need); *oldlenp=need; return 0;
}
extern "C" char *__wrap_getbsize(int *h, long *b) {
	static char hdr[]="512-blocks";
	if (h) *h=(int)strlen(hdr); if (b) *b=g_getbsize_val; return hdr;
}
extern "C" int __wrap_xo_parse_args(int argc, char *argv[]) { (void)argv; return argc; }
extern "C" void __wrap_xo_emit(const char *fmt, ...) {
	va_list ap; va_start(ap,fmt); xo_record("emit",fmt,ap); va_end(ap);
}
extern "C" void __wrap_xo_attr(const char *name, const char *fmt, ...) {
	va_list ap; char tag[128]; std::snprintf(tag,sizeof(tag),"attr:%s",name);
	va_start(ap,fmt); xo_record(tag,fmt,ap); va_end(ap);
}
extern "C" void __wrap_xo_warn(const char *fmt, ...) { va_list ap; va_start(ap,fmt); xo_record("warn",fmt,ap); va_end(ap); }
extern "C" void __wrap_xo_warnx(const char *fmt, ...) { va_list ap; va_start(ap,fmt); xo_record("warnx",fmt,ap); va_end(ap); }
extern "C" void __wrap_xo_err(int eval, const char *fmt, ...) {
	va_list ap; va_start(ap,fmt); xo_record("err",fmt,ap); va_end(ap);
	if (g_test_child) ::_exit(eval); std::exit(eval);
}
extern "C" void __wrap_xo_errx(int eval, const char *fmt, ...) {
	va_list ap; va_start(ap,fmt); xo_record("errx",fmt,ap); va_end(ap);
	if (g_test_child) ::_exit(eval); std::exit(eval);
}
extern "C" void __wrap_xo_error(const char *fmt, ...) { va_list ap; va_start(ap,fmt); xo_record("error",fmt,ap); va_end(ap); }
extern "C" void __wrap_xo_open_container(const char *n) { g_xo_log.push_back(std::string("open_container:")+n); }
extern "C" void __wrap_xo_close_container(const char *n) { g_xo_log.push_back(std::string("close_container:")+n); }
extern "C" void __wrap_xo_open_list(const char *n) { g_xo_log.push_back(std::string("open_list:")+n); }
extern "C" void __wrap_xo_close_list(const char *n) { g_xo_log.push_back(std::string("close_list:")+n); }
extern "C" void __wrap_xo_open_instance(const char *n) { g_xo_log.push_back(std::string("open_instance:")+n); }
extern "C" void __wrap_xo_close_instance(const char *n) { g_xo_log.push_back(std::string("close_instance:")+n); }
extern "C" int __wrap_xo_finish(void) { return g_xo_finish_rc; }
extern "C" int __wrap_stat(const char *path, struct stat *sb) {
	auto it=g_stat_map.find(path);
	if (it==g_stat_map.end()) { errno=ENOENT; return -1; }
	*sb=it->second; return 0;
}
extern "C" void __real_exit(int);
extern "C" void __wrap_exit(int status) { if (g_test_child) ::_exit(status); __real_exit(status); }

bool logs_equal(const std::vector<std::string> &a, const std::vector<std::string> &b) { return a==b; }
bool statfs_equal(const statfs &a, const statfs &b) { return std::memcmp(&a,&b,sizeof(statfs))==0; }
bool maxwidths_equal(const maxwidths &a, const maxwidths &b) { return std::memcmp(&a,&b,sizeof(maxwidths))==0; }

statfs make_statfs(const char *from, const char *on, const char *type,
    int64_t bsize, int64_t blocks, int64_t bfree, int64_t bavail,
    int64_t files, int64_t ffree, uint64_t flags=0) {
	statfs s{}; s.f_flags=flags; s.f_bsize=bsize; s.f_blocks=blocks;
	s.f_bfree=bfree; s.f_bavail=bavail; s.f_files=files; s.f_ffree=ffree;
	std::strncpy(s.f_fstypename,type,MFSNAMELEN-1);
	std::strncpy(s.f_mntonname,on,MNAMELEN-1);
	std::strncpy(s.f_mntfromname,from,MNAMELEN-1);
	return s;
}

bool test_imax_case(int a,int b){ st_imax.cases++; if(ref_imax(a,b)!=P::imax(a,b)) return fail(st_imax,"return"); return true; }
bool test_int64width_case(int64_t v){ st_int64width.cases++; if(ref_int64width(v)!=P::int64width(v)) return fail(st_int64width,"return"); return true; }
bool test_fsbtoblk_case(int64_t n,uint64_t f,unsigned long b){ st_fsbtoblk.cases++; if(ref_fsbtoblk(n,f,b)!=P::fsbtoblk(n,f,b)) return fail(st_fsbtoblk,"return"); return true; }
bool test_checkvfsname_case(const char *n,const char **l,int sk){ st_checkvfsname.cases++; if(ref_checkvfsname(n,l,sk)!=P::checkvfsname(n,l,sk)) return fail(st_checkvfsname,"return"); return true; }

bool test_checkvfsselected_case(const char *type) {
	st_checkvfsselected.cases++;
	char rbuf[MFSNAMELEN], pbuf[MFSNAMELEN];
	std::strncpy(rbuf,type,sizeof(rbuf)-1); rbuf[sizeof(rbuf)-1]=0;
	std::strncpy(pbuf,type,sizeof(pbuf)-1); pbuf[sizeof(pbuf)-1]=0;
	int r=ref_checkvfsselected(rbuf), p=P::checkvfsselected(pbuf);
	if (r!=p) return fail(st_checkvfsselected,"return");
	return true;
}

bool compare_makevfslist(char *br,char *bp,int sr,int sp,const char **ar,const char **ap) {
	if ((ar==nullptr)!=(ap==nullptr) || sr!=sp) return false;
	if (!ar) return true;
	for (int i=0; ar[i]; ++i) if (!ap[i] || std::strcmp(ar[i],ap[i])!=0) return false;
	return std::memcmp(br,bp,std::strlen(br)+1)==0;
}

bool test_makevfslist_case(const char *input, bool mfail) {
	st_makevfslist.cases++;
	size_t len=std::strlen(input);
	std::vector<char> br(len+1+2*GUARD_PAD,GUARD), bp(len+1+2*GUARD_PAD,GUARD);
	std::memcpy(br.data()+GUARD_PAD,input,len+1);
	std::memcpy(bp.data()+GUARD_PAD,input,len+1);
	int sr=-1,sp=-1;
	g_malloc_fail=mfail?1:0;
	const char **ar=ref_makevfslist(br.data()+GUARD_PAD,&sr);
	g_malloc_fail=mfail?1:0;
	const char **ap=P::makevfslist(bp.data()+GUARD_PAD,&sp);
	bool ok=compare_makevfslist(br.data()+GUARD_PAD,bp.data()+GUARD_PAD,sr,sp,ar,ap);
	if (ar) free((void*)ar); if (ap) free((void*)ap);
	for (size_t i=0;i<br.size();++i) if (br[i]!=bp[i]) ok=false;
	if (!ok) return fail(st_makevfslist,"result");
	return true;
}

bool test_addstat_case(const statfs &add,int64_t tbs) {
	st_addstat.cases++;
	statfs tr{}, tp{}, ar=add, ap=add;
	tr.f_bsize=tp.f_bsize=tbs;
	ref_addstat(&tr,&ar); P::addstat(&tp,&ap);
	if (!statfs_equal(tr,tp)||!statfs_equal(ar,ap)) return fail(st_addstat,"statfs");
	return true;
}
bool test_update_maxwidths_case(const statfs &s) {
	st_update_maxwidths.cases++;
	maxwidths mr{}, mp{};
	ref_update_maxwidths(&mr,&s); P::update_maxwidths(&mp,&s);
	if (!maxwidths_equal(mr,mp)) return fail(st_update_maxwidths,"mwp");
	return true;
}
bool test_getmntpt_case(const char *n) {
	st_getmntpt.cases++;
	char *r=ref_getmntpt(n), *p=P::getmntpt(n);
	bool ok=(r==nullptr&&p==nullptr)||(r&&p&&std::strcmp(r,p)==0);
	if (!ok) return fail(st_getmntpt,"ptr");
	return true;
}
bool test_regetmntinfo_case(std::vector<statfs> t,long ms,bool lists) {
	st_regetmntinfo.cases++;
	std::vector<statfs> br=t,bp=t; struct statfs *rp=br.data(), *pp=bp.data();
	if (lists) {
		static char tl[]="ufs,zfs", ll[]="nfs";
		ref_vfslist_t=ref_makevfslist(tl,&ref_skipvfs_t);
		P::vfslist_t=P::makevfslist(tl,&P::skipvfs_t);
		ref_vfslist_l=ref_makevfslist(ll,&ref_skipvfs_l);
		P::vfslist_l=P::makevfslist(ll,&P::skipvfs_l);
	}
	size_t nr=ref_regetmntinfo(&rp,ms), np=P::regetmntinfo(&pp,ms);
	if (nr!=np) return fail(st_regetmntinfo,"count");
	for (size_t i=0;i<nr;++i) if (!statfs_equal(br[i],bp[i])) return fail(st_regetmntinfo,"entry");
	return true;
}
bool test_prthumanval_case(const char *fmt,int64_t b,int hf) {
	st_prthumanval.cases++; ref_hflag=hf; P::hflag=hf;
	g_xo_log.clear(); ref_prthumanval(fmt,b); auto lr=g_xo_log;
	g_xo_log.clear(); P::prthumanval(fmt,b);
	if (!logs_equal(lr,g_xo_log)) return fail(st_prthumanval,"xo");
	return true;
}
bool test_prthumanvalinode_case(const char *fmt,int64_t b) {
	st_prthumanvalinode.cases++;
	g_xo_log.clear(); ref_prthumanvalinode(fmt,b); auto lr=g_xo_log;
	g_xo_log.clear(); P::prthumanvalinode(fmt,b);
	if (!logs_equal(lr,g_xo_log)) return fail(st_prthumanvalinode,"xo");
	return true;
}
bool test_prthuman_case(const statfs &s,int64_t u,int hf) {
	st_prthuman.cases++; ref_hflag=hf; P::hflag=hf;
	g_xo_log.clear(); ref_prthuman(&s,u); auto lr=g_xo_log;
	g_xo_log.clear(); P::prthuman(&s,u);
	if (!logs_equal(lr,g_xo_log)) return fail(st_prthuman,"xo");
	return true;
}
bool test_prtstat_case(statfs s,int hf,int inf,int tf,int th) {
	st_prtstat.cases++;
	maxwidths mr{}, mp{}; statfs sr=s, sp=s;
	ref_hflag=hf; ref_iflag=inf; ref_Tflag=tf; ref_thousands=th;
	g_xo_log.clear(); ref_prtstat(&sr,&mr); auto lr=g_xo_log;
	reset_port_globals(); P::hflag=hf; P::iflag=inf; P::Tflag=tf; P::thousands=th;
	g_xo_log.clear(); P::prtstat(&sp,&mp);
	if (!logs_equal(lr,g_xo_log)||!statfs_equal(sr,sp)||!maxwidths_equal(mr,mp))
		return fail(st_prtstat,"mismatch");
	return true;
}
bool test_makenetvfslist_case() {
	st_makenetvfslist.cases++;
	char *r=ref_makenetvfslist(), *p=P::makenetvfslist();
	bool ok=(r==nullptr&&p==nullptr)||(r&&p&&std::strcmp(r,p)==0);
	if (r) free(r); if (p) free(p);
	if (!ok) return fail(st_makenetvfslist,"str");
	return true;
}
int run_usage_child(bool port) {
	pid_t pid=fork(); if (pid<0) return -1;
	if (!pid) { g_test_child=true; if (port) P::usage(); else ref_usage(); ::_exit(99); }
	int st=0; waitpid(pid,&st,0);
	return WIFEXITED(st)?WEXITSTATUS(st):-1;
}
bool test_usage_case() { st_usage.cases++; if(run_usage_child(false)!=run_usage_child(true)) return fail(st_usage,"exit"); return true; }
int run_main_child(bool port,int argc,char**argv) {
	pid_t pid=fork(); if (pid<0) return -1;
	if (!pid) {
		g_test_child=true; optind=1; opterr=1; optopt=0;
		if (port) { reset_port_globals(); ::_exit(P::main(argc,argv)); }
		reset_ref_globals(); ::_exit(ref_main(argc,argv));
	}
	int st=0; waitpid(pid,&st,0);
	return WIFEXITED(st)?WEXITSTATUS(st):-1;
}
bool test_main_case(int argc,char**argv) {
	st_main.cases++; mock_reset(); int r=run_main_child(false,argc,argv);
	mock_reset(); int p=run_main_child(true,argc,argv);
	if (r!=p) return fail(st_main,"exit"); return true;
}
void setup_mounts() {
	g_mnt_table.clear();
	g_mnt_table.push_back(make_statfs("dev/sda1","/mnt/a","ufs",4096,1000,200,700,50000,40000));
	g_mnt_table.push_back(make_statfs("dev/sdb1","/mnt/b","zfs",8192,2000,500,1400,100000,90000));
	g_mnt_table.push_back(make_statfs("server:/x","/mnt/nfs","nfs",4096,500,100,350,20000,15000));
}
void setup_vfs() {
	g_vfs_conf.clear();
	xvfsconf n{}; std::strncpy(n.vfc_name,"nfs",sizeof(n.vfc_name)-1); n.vfc_flags=VFCF_NETWORK; g_vfs_conf.push_back(n);
	xvfsconf s{}; std::strncpy(s.vfc_name,"smbfs",sizeof(s.vfc_name)-1); s.vfc_flags=VFCF_NETWORK; g_vfs_conf.push_back(s);
	xvfsconf u{}; std::strncpy(u.vfc_name,"ufs",sizeof(u.vfc_name)-1); g_vfs_conf.push_back(u);
}

void hand_tests() {
	test_imax_case(0,0); test_imax_case(1,2); test_imax_case(-1,1); test_imax_case(INT_MAX,INT_MIN);
	test_int64width_case(0); test_int64width_case(1); test_int64width_case(-1);
	test_int64width_case(INT64_MAX); test_int64width_case(INT64_MIN);
	test_fsbtoblk_case(0,512,512); test_fsbtoblk_case(100,4096,512);
	test_fsbtoblk_case(-1,512,512); test_fsbtoblk_case(7,3,2);
	const char *l1[]={"ufs","zfs",nullptr}, *l2[]={"nfs",nullptr};
	test_checkvfsname_case(nullptr,nullptr,0); test_checkvfsname_case("ufs",l1,0);
	test_checkvfsname_case("nfs",l1,0); test_checkvfsname_case("ufs",l1,1);
	mock_reset(); reset_port_globals(); reset_ref_globals();
	static char t[]="ufs,zfs", ll[]="nfs";
	ref_vfslist_t=ref_makevfslist(t,&ref_skipvfs_t); P::vfslist_t=P::makevfslist(t,&P::skipvfs_t);
	test_checkvfsselected_case("ufs");
	ref_vfslist_l=ref_makevfslist(ll,&ref_skipvfs_l); P::vfslist_l=P::makevfslist(ll,&P::skipvfs_l);
	test_checkvfsselected_case("nfs");
	mock_reset(); test_makevfslist_case("",false); test_makevfslist_case("ufs",false);
	test_makevfslist_case("ufs,zfs",false); test_makevfslist_case("nousfs,zfs",false);
	test_makevfslist_case("x",true);
	statfs add=make_statfs("x","/x","ufs",4096,100,20,70,1000,800);
	test_addstat_case(add,512); test_addstat_case(add,4096);
	statfs s=make_statfs("dev","/mnt","ufs",4096,10000,2000,7000,50000,40000);
	test_update_maxwidths_case(s);
	mock_reset(); setup_mounts(); test_getmntpt_case("dev/sda1"); test_getmntpt_case("missing");
	mock_reset(); setup_mounts(); test_regetmntinfo_case(g_mnt_table,(long)g_mnt_table.size(),false);
	ref_nflag=P::nflag=1; test_regetmntinfo_case(g_mnt_table,(long)g_mnt_table.size(),false);
	ref_nflag=P::nflag=0; test_regetmntinfo_case(g_mnt_table,(long)g_mnt_table.size(),true);
	test_prthumanval_case("{:%s}",0,0); test_prthumanval_case("{:%s}",1024,1);
	test_prthumanval_case("{:%s}",-512,2); test_prthumanvalinode_case("{:%s}",0);
	test_prthumanvalinode_case("{:%s}",999999);
	statfs hs=make_statfs("d","/m","ufs",512,100,10,80,0,0);
	test_prthuman_case(hs,90,1); test_prthuman_case(hs,90,2);
	statfs ps=make_statfs("dev/da0","/mnt","ufs",512,1000,100,800,10000,9000);
	test_prtstat_case(ps,0,0,0,0); test_prtstat_case(ps,1,1,1,1);
	test_prtstat_case(make_statfs("total","/mnt","ufs",0,0,0,0,0,0),2,0,0,0);
	test_prtstat_case(make_statfs("x","/zero","ufs",0,10,1,8,0,0),0,1,0,1);
	mock_reset(); setup_vfs(); test_makenetvfslist_case();
	g_vfs_conf.clear(); test_makenetvfslist_case();
	g_sysctl_fail=1; test_makenetvfslist_case();
	test_usage_case();
	static char prog[]="df", a[]="-a", k[]="-k", t[]="-t", type[]="ufs";
	static char path[]="/mnt/a", bad[]="/nope", comma[]="-,", i[]="-i";
	static char c[]="-c", H[]="-H", h[]="-h", T[]="-T", n[]="-n";
	static char Pflag[]="-P", b[]="-b", g[]="-g", m[]="-m", l[]="-l";
	char *av0[]={prog,nullptr}; test_main_case(1,av0);
	mock_reset(); setup_mounts();
	char *ava[]={prog,a,nullptr}; test_main_case(2,ava);
	char *avk[]={prog,k,nullptr}; test_main_case(2,avk);
	char *avp[]={prog,path,nullptr}; g_stat_map[path]=stat{}; test_main_case(2,avp);
	char *avb[]={prog,bad,nullptr}; test_main_case(2,avb);
	char *avt[]={prog,t,type,nullptr}; test_main_case(3,avt);
	char *avc[]={prog,comma,i,c,H,T,path,nullptr}; test_main_case(7,avc);
	char *avh[]={prog,h,n,nullptr}; test_main_case(3,avh);
	char *avpb[]={prog,Pflag,b,k,nullptr}; test_main_case(4,avpb);
	char *avgm[]={prog,g,m,nullptr}; test_main_case(3,avgm);
	mock_reset(); setup_mounts(); setup_vfs();
	char *avl[]={prog,l,nullptr}; test_main_case(2,avl);
}

void sweeps() {
	for (long i=0;i<SWEEP;++i) test_imax_case(rng.bits(-1000,1000),rng.bits(-1000,1000));
	for (long i=0;i<SWEEP;++i) test_int64width_case(rng.i64());
	for (long i=0;i<SWEEP;++i)
		test_fsbtoblk_case((int64_t)rng.next(),rng.next()%8192+1,(unsigned long)(rng.next()%4096+1));
	const char *types[]={"ufs","zfs","nfs","tmpfs","\xff",""};
	for (long i=0;i<SWEEP;++i) {
		const char *list[3]={types[rng.bits(0,4)],rng.coin()?types[rng.bits(0,4)]:nullptr,nullptr};
		test_checkvfsname_case(types[rng.bits(0,5)],list,rng.coin());
	}
	for (long i=0;i<SWEEP;++i) {
		mock_reset(); reset_port_globals(); reset_ref_globals();
		char tbuf[32],lbuf[32];
		std::snprintf(tbuf,sizeof(tbuf),"%s,%s",types[rng.bits(0,3)],types[rng.bits(0,3)]);
		std::snprintf(lbuf,sizeof(lbuf),"%s",types[rng.bits(0,3)]);
		ref_skipvfs_t=P::skipvfs_t=rng.coin(); ref_skipvfs_l=P::skipvfs_l=rng.coin();
		ref_vfslist_t=ref_makevfslist(tbuf,&ref_skipvfs_t);
		P::vfslist_t=P::makevfslist(tbuf,&P::skipvfs_t);
		if (rng.coin()) { ref_vfslist_l=ref_makevfslist(lbuf,&ref_skipvfs_l); P::vfslist_l=P::makevfslist(lbuf,&P::skipvfs_l); }
		test_checkvfsselected_case(types[rng.bits(0,3)]);
	}
	for (long i=0;i<SWEEP;++i) {
		mock_reset(); std::string s; int n=rng.bits(0,6);
		for (int j=0;j<n;++j) { if (j) s+=','; s+=(char)('a'+rng.bits(0,25)); }
		if (rng.coin()) s=std::string("no")+s;
		test_makevfslist_case(s.c_str(),rng.bits(0,50)==0);
	}
	for (long i=0;i<SWEEP;++i) {
		statfs add=make_statfs("d","/m","ufs",(int64_t)(rng.next()%8192+512),
			(int64_t)rng.bits(0,100000),(int64_t)rng.bits(0,50000),(int64_t)rng.bits(0,50000),
			(int64_t)rng.bits(0,1000000),(int64_t)rng.bits(0,1000000));
		test_addstat_case(add,(int64_t)(rng.next()%4096+512));
	}
	for (long i=0;i<SWEEP;++i) {
		statfs s=make_statfs("d","/m","ufs",(int64_t)(rng.next()%8192+1),
			(int64_t)rng.bits(0,1000000),(int64_t)rng.bits(0,500000),(int64_t)rng.bits(0,500000),
			(int64_t)rng.bits(0,10000000),(int64_t)rng.bits(0,10000000));
		test_update_maxwidths_case(s);
	}
	mock_reset(); setup_mounts();
	const char *mn[]={"dev/sda1","dev/sdb1","server:/x","nope"};
	for (long i=0;i<SWEEP;++i) test_getmntpt_case(mn[rng.bits(0,3)]);
	for (long i=0;i<SWEEP/10;++i) {
		mock_reset(); setup_mounts(); ref_nflag=P::nflag=rng.coin();
		g_statfs_fail=rng.coin()?0:1;
		test_regetmntinfo_case(g_mnt_table,(long)g_mnt_table.size(),rng.coin());
	}
	for (long i=0;i<SWEEP/2;++i) test_prthumanval_case("{:%s}",rng.i64(),rng.bits(0,2));
	for (long i=0;i<SWEEP/2;++i) test_prthumanvalinode_case("{:%s}",rng.i64());
	statfs hs=make_statfs("d","/m","ufs",512,100,10,80,0,0);
	for (long i=0;i<SWEEP/4;++i) test_prthuman_case(hs,rng.i64(),rng.bits(1,2));
	for (long i=0;i<SWEEP/20;++i) {
		statfs s=make_statfs("d","/m","ufs",(int64_t)(rng.next()%8192+1),
			(int64_t)rng.bits(1,100000),(int64_t)rng.bits(0,50000),(int64_t)rng.bits(0,50000),
			(int64_t)rng.bits(0,1000000),(int64_t)rng.bits(0,1000000));
		test_prtstat_case(s,rng.bits(0,2),rng.coin(),rng.coin(),rng.coin());
	}
	for (long i=0;i<SWEEP/50;++i) {
		mock_reset(); setup_vfs();
		if (rng.coin()) g_sysctl_fail=1;
		if (rng.bits(0,10)==0) g_malloc_fail=1;
		test_makenetvfslist_case();
	}
}

void print_table() {
	Stat *all[]={&st_imax,&st_int64width,&st_fsbtoblk,&st_checkvfsname,&st_checkvfsselected,
		&st_addstat,&st_makevfslist,&st_update_maxwidths,&st_getmntpt,&st_regetmntinfo,
		&st_prthumanval,&st_prthumanvalinode,&st_prthuman,&st_prtstat,&st_makenetvfslist,
		&st_usage,&st_main};
	std::printf("\n%-22s %10s %10s\n","function","cases","failures");
	std::printf("%-22s %10s %10s\n","--------","-----","--------");
	long tf=0;
	for (Stat *st:all) { std::printf("%-22s %10ld %10ld\n",st->name,st->cases,st->fails); tf+=st->fails; }
	std::printf("%-22s %10s %10ld\n","TOTAL","",tf);
}

} // namespace

int main() {
	mock_reset();
	hand_tests();
	sweeps();
	print_table();
	long tf=st_imax.fails+st_int64width.fails+st_fsbtoblk.fails+st_checkvfsname.fails+
		st_checkvfsselected.fails+st_addstat.fails+st_makevfslist.fails+st_update_maxwidths.fails+
		st_getmntpt.fails+st_regetmntinfo.fails+st_prthumanval.fails+st_prthumanvalinode.fails+
		st_prthuman.fails+st_prtstat.fails+st_makenetvfslist.fails+st_usage.fails+st_main.fails;
	return tf==0?0:1;
}
