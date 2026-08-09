#include <cstdio>
import pbsd.lib.libc.stdio.b0326;
extern "C" { extern struct stat mock_fstat_st; extern int mock_fstat_ret; void b0326_reset(void); }
int main() {
	mock_fstat_st.st_blksize = 4096; b0326_reset(); mock_fstat_ret = 0; mock_fstat_st.st_blksize = 4096;
	int v = pbsd::lib_libc_stdio::b0326::b0326_test_fstat_blksize();
	printf("v=%d sizeof=%d blks=%d\n", v, v/10000, v%10000);
}
