/*
 * harness.cpp -- differential test for PBSD batch b0148.
 */

import pbsd.bin.ed.b0148;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex.h>
#include <sys/wait.h>
#include <unistd.h>

namespace port = pbsd::bin_ed::b0148;

extern "C" {
extern char *ibuf;
extern char *ibufp;
extern int isbinary;
extern long addr_last;
extern long current_addr;
extern long first_addr;
extern long second_addr;
extern long u_addr_last;
extern long u_current_addr;
extern int patlock;
extern const char *errmsg;
extern int oracle_extended_line_len;
extern char *oracle_extended_line;
extern int oracle_quit_called;
extern int oracle_quit_status;
void oracle_reset_batch(void);
void oracle_quit_enter(void);
char *ref_parse_char_class(char *);
char *ref_translit_text(char *, int, int, int);
char *ref_extract_pattern(int);
regex_t *ref_get_compiled_pattern(void);
void *ref_get_addressed_line_node(long);
long ref_get_line_node_addr(void *);
char *ref_get_sbuf_line(void *);
const char *ref_put_sbuf_line(const char *);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
void *ref_push_undo_stack(int, long, long);
int ref_pop_undo_stack(void);
void ref_clear_undo_stack(void);
int ref_set_active_node(void *);
void *ref_next_active_node(void);
void ref_clear_active_list(void);
int ref_build_active_list(int);
long ref_exec_global(int, int);
void ref_init_buffers(void);
void ref_quit(int);
}

namespace {

constexpr int GUARD = 0x7f;
constexpr long RANDOM_ITERS = 200000;

struct Stat { const char *name; long cases, fails; };
Stat stats[24];
int nstats;
std::uint64_t rng = 0x243f6a8885a308d3ULL;

std::uint64_t rnd() {
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}
int rndb() { return (int)(rnd() & 0xff); }

Stat &reg(const char *n) { stats[nstats++] = {n,0,0}; return stats[nstats-1]; }

void reset_both() { oracle_reset_batch(); port::reset_batch(); }

void setup_ibuf(const char *s) {
	std::strcpy(ibuf, s); ibufp = ibuf;
	port::isbinary = isbinary;
	port::addr_last = addr_last;
	port::current_addr = current_addr;
	port::first_addr = first_addr;
	port::second_addr = second_addr;
	port::u_addr_last = u_addr_last;
	port::u_current_addr = u_current_addr;
	port::patlock = patlock;
	std::memcpy(port::ibuf, ibuf, 65536);
	port::ibufp = port::ibuf + (ibufp - ibuf);
}

struct G { unsigned char b[32], d[512], a[32]; };
void fg(G &g) { std::memset(g.b,GUARD,32); std::memset(g.a,GUARD,32); std::memset(g.d,0,512); }

bool scratch_both() {
	if (ref_open_sbuf()) return false;
	if (port::open_sbuf()) { ref_close_sbuf(); return false; }
	ref_get_addressed_line_node(0);
	port::get_addressed_line_node(0);
	return true;
}

void test_parse_char_class() {
	Stat &st = reg("parse_char_class");
	auto run = [&](const char *in, const char *tag) {
		G rg, pg; fg(rg); fg(pg);
		std::strncpy((char*)rg.d,in,511); std::strncpy((char*)pg.d,in,511);
		char *r = ref_parse_char_class((char*)rg.d);
		char *p = port::parse_char_class((char*)pg.d);
		long ro = r?(r-(char*)rg.d):-1, po = p?(p-(char*)pg.d):-1;
		st.cases++;
		if (ro!=po || std::memcmp(rg.d,pg.d,512)) st.fails++;
	};
	const char *e[]={"","]","^]","[]","[a]","[[:alpha:]]","[\n","[\xff]"};
	for (auto x:e) run(x,x);
	for (long i=0;i<RANDOM_ITERS/11;i++){
		char b[128];
		int n=rnd()%100+1;
		for(int j=0;j<n;j++) b[j]=(char)rndb();
		b[n]=0;
		for(int j=0;j<n;j++)
			if(b[j]=='['){
				if(j+1<n&&(b[j+1]=='.'||b[j+1]==':'||b[j+1]=='='))
					b[j]='a';
				else { b[n++]=(char)']'; b[n]=0; break; }
			}
		run(b,"r");
	}
}

void test_translit_text() {
	Stat &st = reg("translit_text");
	auto run = [&](const char *in,int len,int f,int t) {
		G rg,pg; fg(rg); fg(pg);
		std::memcpy(rg.d,in,len); std::memcpy(pg.d,in,len);
		ref_translit_text((char*)rg.d,len,f,t);
		port::translit_text((char*)pg.d,len,f,t);
		st.cases++;
		if (std::memcmp(rg.d,pg.d,512)) st.fails++;
	};
	run("",0,0,1); run("abc",3,'b','B'); run("\x80\xff",2,0x80,32);
	for (long i=0;i<RANDOM_ITERS/11;i++){ char b[200]; int n=rnd()%150; for(int j=0;j<n;j++)b[j]=rndb(); run(b,n,rndb(),rndb()); }
}

void test_extract_pattern() {
	Stat &st = reg("extract_pattern");
	auto run = [&](const char *cmd,int d) {
		reset_both(); setup_ibuf(cmd); isbinary=rnd()&1; setup_ibuf(cmd);
		char *r=ref_extract_pattern(d); long ri=ibufp-ibuf;
		reset_both(); setup_ibuf(cmd); port::isbinary=isbinary;
		char *p=port::extract_pattern(d); long pi=port::ibufp-port::ibuf;
		st.cases++;
		if ((r==nullptr)!=(p==nullptr)||ri!=pi||(r&&std::strcmp(r,p))) st.fails++;
	};
	run("/foo/bar", '/'); run("/[a]/x", '/'); run("/[/x", '/');
	for (long i=0;i<RANDOM_ITERS/22;i++){ char b[120]; int d='#'; b[0]=d; int n=rnd()%40+1; for(int j=1;j<n;j++)b[j]=32+rndb()%90; b[n]=d; b[n+1]='\n'; b[n+2]=0; run(b,d); }
}

void test_get_compiled_pattern() {
	Stat &st = reg("get_compiled_pattern");
	auto run = [&](const char *cmd) {
		reset_both(); setup_ibuf(cmd);
		regex_t *r=ref_get_compiled_pattern(); const char *re=errmsg;
		reset_both(); setup_ibuf(cmd);
		regex_t *p=port::get_compiled_pattern(); const char *pe=port::errmsg;
		st.cases++;
		if (((r==nullptr)!=(p==nullptr))||std::strcmp(re,pe)) st.fails++;
	};
	run("/a/\n"); run(" \n"); run("/[/\n");
	for (long i=0;i<RANDOM_ITERS/22;i++){ char b[80]; int d='a'+rnd()%26; b[0]=d; int n=rnd()%20+1; for(int j=1;j<n;j++)b[j]='x'; b[n]=d; b[n+1]='\n'; b[n+2]=0; run(b); }
}

void test_get_addressed_line_node() {
	Stat &st = reg("get_addressed_line_node");
	auto run = [&](long n) {
		reset_both(); scratch_both();
		for(int i=0;i<8;i++){ char l[16]; std::snprintf(l,16,"%d\n",i); ref_put_sbuf_line(l);} 
		reset_both(); scratch_both();
		for(int i=0;i<8;i++){ char l[16]; std::snprintf(l,16,"%d\n",i); port::put_sbuf_line(l);} 
		long ra=ref_get_line_node_addr(ref_get_addressed_line_node(n));
		long pa=port::get_line_node_addr(port::get_addressed_line_node(n));
		st.cases++; if(ra!=pa) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	};
	for(long n=0;n<=8;n++) run(n);
	for(long i=0;i<RANDOM_ITERS/22;i++) run(rnd()%9);
}

void test_get_line_node_addr() {
	Stat &st = reg("get_line_node_addr");
	auto run = [&](int nl,int t) {
		reset_both(); scratch_both();
		for(int i=0;i<nl;i++){ char l[16]; std::snprintf(l,16,"x%d\n",i); ref_put_sbuf_line(l);} 
		long r=ref_get_line_node_addr(ref_get_addressed_line_node(t>nl?nl+2:t));
		reset_both(); scratch_both();
		for(int i=0;i<nl;i++){ char l[16]; std::snprintf(l,16,"x%d\n",i); port::put_sbuf_line(l);} 
		long p=port::get_line_node_addr(port::get_addressed_line_node(t>nl?nl+2:t));
		st.cases++; if(r!=p) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	};
	run(5,3); run(5,0); run(3,99);
	for(long i=0;i<RANDOM_ITERS/22;i++) run(rnd()%8+1,rnd()%10);
}

void test_get_sbuf_line() {
	Stat &st = reg("get_sbuf_line");
	auto run = [&](int nl,int w) {
		char ex[32][32];
		reset_both(); scratch_both();
		for(int i=0;i<nl;i++){ std::snprintf(ex[i],32,"L%d\n",i); ref_put_sbuf_line(ex[i]); }
		char *rs=ref_get_sbuf_line(ref_get_addressed_line_node(w));
		reset_both(); scratch_both();
		for(int i=0;i<nl;i++) port::put_sbuf_line(ex[i]);
		char *ps=port::get_sbuf_line(port::get_addressed_line_node(w));
		st.cases++;
		if((rs==nullptr)!=(ps==nullptr)||(rs&&std::strcmp(rs,ps))) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	};
	run(5,2); run(1,1); run(0,0);
	for(long i=0;i<RANDOM_ITERS/22;i++){ int n=rnd()%6; run(n,n?rnd()%n+1:0); }
}

void test_open_sbuf() {
	Stat &so=reg("open_sbuf"), &sc=reg("close_sbuf");
	for(long i=0;i<20+RANDOM_ITERS/44;i++){
		reset_both(); int r1=ref_open_sbuf(), r2=ref_close_sbuf();
		reset_both(); int p1=port::open_sbuf(), p2=port::close_sbuf();
		so.cases++; sc.cases++;
		if(r1!=p1) so.fails++; if(r2!=p2) sc.fails++;
	}
}

void test_add_line_node() {
	Stat &st=reg("add_line_node");
	for(long i=0;i<50+RANDOM_ITERS/44;i++){
		reset_both(); scratch_both(); ref_put_sbuf_line("a\n"); long ar=addr_last;
		reset_both(); scratch_both(); port::put_sbuf_line("a\n"); long ap=port::addr_last;
		st.cases++; if(ar!=ap) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	}
}

void test_set_active_node() {
	Stat &ss=reg("set_active_node"), &sn=reg("next_active_node");
	Stat &sc=reg("clear_active_list"), &su=reg("unset_active_nodes");
	for(long i=0;i<30+RANDOM_ITERS/22;i++){
		reset_both(); scratch_both();
		for(int j=0;j<4;j++){ char l[8]; std::snprintf(l,8,"%d\n",j); ref_put_sbuf_line(l);} 
		for(int j=0;j<4;j++){ ss.cases++; if(ref_set_active_node(ref_get_addressed_line_node(j+1))) ss.fails++; }
		for(int j=0;j<5;j++) (void)ref_next_active_node(); sn.cases++;
		ref_clear_active_list(); sc.cases++; su.cases++;
		reset_both(); scratch_both();
		for(int j=0;j<4;j++){ char l[8]; std::snprintf(l,8,"%d\n",j); port::put_sbuf_line(l);} 
		for(int j=0;j<4;j++) if(port::set_active_node(port::get_addressed_line_node(j+1))) ss.fails++;
		for(int j=0;j<5;j++) (void)port::next_active_node();
		port::clear_active_list();
		ref_close_sbuf(); port::close_sbuf();
	}
}

void test_push_undo_stack() {
	Stat &sp=reg("push_undo_stack"), &so=reg("pop_undo_stack"), &sc=reg("clear_undo_stack");
	auto run = [&](){
		reset_both(); scratch_both(); ref_put_sbuf_line("a\n"); ref_put_sbuf_line("b\n");
		u_current_addr=2; u_addr_last=2;
		void *ru=ref_push_undo_stack(0,1,2); sp.cases++; int rp=ref_pop_undo_stack(); so.cases++;
		reset_both(); scratch_both(); port::put_sbuf_line("a\n"); port::put_sbuf_line("b\n");
		port::u_current_addr=2; port::u_addr_last=2;
		void *pu=port::push_undo_stack(0,1,2); int pp=port::pop_undo_stack();
		if(((ru==nullptr)!=(pu==nullptr))||rp!=pp){ sp.fails++; so.fails++; }
		ref_close_sbuf(); port::close_sbuf();
	};
	run(); sc.cases++;
	for(long i=0;i<RANDOM_ITERS/22;i++) run();
}

void test_build_active_list() {
	Stat &st=reg("build_active_list");
	static char cmd[]="/match/\n";
	auto run=[&](int g){
		reset_both(); scratch_both();
		ref_put_sbuf_line("match\n"); ref_put_sbuf_line("x\n"); ref_put_sbuf_line("match\n");
		first_addr=1; second_addr=3; setup_ibuf(cmd);
		int r=ref_build_active_list(g);
		reset_both(); scratch_both();
		port::put_sbuf_line("match\n"); port::put_sbuf_line("x\n"); port::put_sbuf_line("match\n");
		port::first_addr=1; port::second_addr=3; setup_ibuf(cmd);
		int p=port::build_active_list(g);
		st.cases++; if(r!=p) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	};
	run(0); run(1); run(0);
	for(long i=0;i<RANDOM_ITERS/22;i++) run(rnd()&1);
}

void test_exec_global() {
	Stat &st=reg("exec_global");
	static char cmd[]="p\n";
	for(long i=0;i<1+RANDOM_ITERS/22;i++){
		oracle_extended_line=cmd; oracle_extended_line_len=2;
		port::extended_line=cmd; port::extended_line_len=2;
		reset_both(); scratch_both(); ref_put_sbuf_line("x\n");
		ref_set_active_node(ref_get_addressed_line_node(1));
		long r=ref_exec_global(0,0);
		reset_both(); scratch_both(); port::put_sbuf_line("x\n");
		port::set_active_node(port::get_addressed_line_node(1));
		long p=port::exec_global(0,0);
		st.cases++; if(r!=p) st.fails++;
		ref_close_sbuf(); port::close_sbuf();
	}
}

void test_init_buffers() {
	Stat &st=reg("init_buffers");
	reset_both(); ref_init_buffers(); int r=ref_close_sbuf();
	reset_both(); port::init_buffers(); int p=port::close_sbuf();
	st.cases=1; if(r!=p) st.fails=1;
}

void test_quit() {
	Stat &st=reg("quit");
	if (fork()==0) {
		reset_both(); ref_open_sbuf(); oracle_quit_enter(); ref_quit(3);
		_exit(oracle_quit_called && oracle_quit_status==3 ? 0 : 1);
	}
	int refst=0; waitpid(-1,&refst,0);
	if (fork()==0) {
		reset_both(); port::open_sbuf(); port::quit_enter(); port::quit(3);
		_exit(port::quit_called && port::quit_status==3 ? 0 : 1);
	}
	int portst=0; waitpid(-1,&portst,0);
	st.cases=1;
	st.fails=(refst==0&&portst==0)?0:1;
}

} /* namespace */

int main() {
	test_parse_char_class();
	test_translit_text();
	test_extract_pattern();
	test_get_compiled_pattern();
	test_get_addressed_line_node();
	test_get_line_node_addr();
	test_get_sbuf_line();
	test_open_sbuf();
	test_add_line_node();
	test_set_active_node();
	test_push_undo_stack();
	test_build_active_list();
	test_exec_global();
	test_init_buffers();
	test_quit();

	std::printf("PBSD batch b0148 differential test\n\n");
	std::printf("%-28s %12s %12s %10s\n","function","cases","failures","result");
	long tc=0, tf=0;
	for(int i=0;i<nstats;i++){
		tc+=stats[i].cases; tf+=stats[i].fails;
		std::printf("%-28s %12ld %12ld %10s\n",stats[i].name,stats[i].cases,stats[i].fails,stats[i].fails?"FAIL":"PASS");
	}
	std::printf("%-28s %12ld %12ld %10s\n","TOTAL",tc,tf,tf?"FAIL":"PASS");
	return tf?1:0;
}
