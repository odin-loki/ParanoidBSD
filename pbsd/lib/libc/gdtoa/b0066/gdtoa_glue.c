#include "gdtoa_shim.h"

int __isthreaded = 0;
pthread_mutex_t __gdtoa_locks[2] = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER
};

Bigint *
increment_D2A(Bigint *b)
{
	ULong *x, *xe;
	Bigint *b1;

	x = b->x;
	xe = x + b->wds;
	do {
		if (*x < (ULong)0xffffffffUL) {
			++*x;
			return b;
		}
		*x++ = 0;
	} while (x < xe);
	if (b->wds >= b->maxwds) {
		b1 = Balloc(b->k + 1);
		Bcopy(b1, b);
		Bfree(b);
		b = b1;
	}
	b->x[b->wds++] = 1;
	b->wds = b->wds;
	return b;
}
