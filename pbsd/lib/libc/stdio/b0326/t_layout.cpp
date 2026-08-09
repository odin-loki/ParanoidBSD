import pbsd.lib.libc.stdio.b0326;
#include <cstdio>
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
	printf("port=%zu ref=%zu blksize off p=%zu r=%zu\n",
	    sizeof(__b0326_FILE), sizeof(RefFILE),
	    offsetof(__b0326_FILE,_blksize), offsetof(RefFILE,_blksize));
}
