#include "prelude.h"
#undef _LINUX_ZSWAP_H
#define _LINUX_ZSWAP_H
/*@ requires true; ensures \result == 0; */
static inline int zswap_swapon(int type, unsigned long nr_pages)
{
	return 0;
}
