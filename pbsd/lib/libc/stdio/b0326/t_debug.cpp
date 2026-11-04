#include <cstdio>
import pbsd.lib.libc.stdio.b0326;
extern "C" {
extern struct stat mock_fstat_st; extern int mock_fstat_ret; void b0326_reset(void);
extern unsigned char b0326_dbg_st_snapshot[64];
extern long long b0326_dbg_mock_blksize;
long long __sseek(void *, long long, int);
}
int main() {
	__b0326_FILE pf{}; size_t psz; int pc;
	pf._file = 3; pf._seek = (long long (*)(void*,long long,int))__sseek;
	b0326_reset(); mock_fstat_ret = 0;
	mock_fstat_st.st_mode = 0100000; mock_fstat_st.st_blksize = 4096;
	printf("before mock_blk=%lld\n", (long long)mock_fstat_st.st_blksize);
	int pret = pbsd::lib_libc_stdio::b0326::__swhatbuf(&pf, &psz, &pc);
	printf("pret=%#x psz=%zu mock_read=%lld snap56=%02x%02x\n", pret, psz,
	    (long long)b0326_dbg_mock_blksize, b0326_dbg_st_snapshot[56], b0326_dbg_st_snapshot[57]);
}
