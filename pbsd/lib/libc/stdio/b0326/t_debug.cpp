#include <cstdio>
import pbsd.lib.libc.stdio.b0326;
extern "C" {
extern int mock_fstat_ret, mock_fstat_calls;
extern struct stat mock_fstat_st;
void b0326_reset(void);
long long __sseek(void *, long long, int);
int ref___swhatbuf(void *, size_t *, int *);
}
struct RefSbuf { unsigned char *_base; int _size; };
struct RefFILE {
	unsigned char *_p; int _r, _w; short _flags, _file;
	RefSbuf _bf; int _lbfsize; void *_cookie;
	int (*_close)(void *); int (*_read)(void*,char*,int);
	long long (*_seek)(void*,long long,int);
	int (*_write)(void*,const char*,int);
	RefSbuf _ub; unsigned char *_up; int _ur;
	unsigned char _ubuf[3], _nbuf[1]; RefSbuf _lb;
	int _blksize; long long _offset;
	void *_fl_mutex, *_fl_owner; int _fl_count, _orientation;
	unsigned char _mbstate_pad[8]; int _flags2;
};
int main() {
	__b0326_FILE pf{}; RefFILE rf{}; size_t psz, rsz; int pc, rc;
	pf._file = rf._file = 3;
	pf._seek = (long long (*)(void*,long long,int))__sseek;
	rf._seek = __sseek;
	mock_fstat_st.st_mode = 0100000; mock_fstat_st.st_blksize = 4096;
	b0326_reset(); mock_fstat_ret = 0;
	int pret = pbsd::lib_libc_stdio::b0326::__swhatbuf(&pf, &psz, &pc);
	printf("port calls=%d pret=%#x psz=%zu blks=%d\n", mock_fstat_calls, pret, psz, pf._blksize);
	b0326_reset(); mock_fstat_ret = 0;
	mock_fstat_st.st_mode = 0100000; mock_fstat_st.st_blksize = 4096;
	int rret = ref___swhatbuf(&rf, &rsz, &rc);
	printf("ref calls=%d rret=%#x rsz=%zu blks=%d\n", mock_fstat_calls, rret, rsz, rf._blksize);
	return 0;
}
