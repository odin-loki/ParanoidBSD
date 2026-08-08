import pbsd.lib.libc.stdio.b0100;
#include <cstdio>
typedef struct {
	unsigned char *_p;
	int _r, _w;
	short _flags, _file;
} ref_FILE;
int main() {
	printf("ref_FILE=%zu P::FILE=%zu\n",
	    sizeof(ref_FILE), sizeof(pbsd::lib_libc_stdio::b0100::FILE));
	return 0;
}
