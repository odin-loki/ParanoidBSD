#include "prelude.h"
#undef SHORT_BITS
#define	SHORT_BITS		16
#undef CHAR_BITS
#define	CHAR_BITS		8
#undef DATA_BLOB_LENGTH
#define	DATA_BLOB_LENGTH	3
#undef MATCH_DATA_LENGTH
#define	MATCH_DATA_LENGTH	8192
#undef ALTERA_REQUEST_SIZE
#define ALTERA_REQUEST_SIZE	1024
#undef ALTERA_BUFFER_SIZE
#define ALTERA_BUFFER_SIZE	(MATCH_DATA_LENGTH + ALTERA_REQUEST_SIZE)
/*@ requires n <= 65535; ensures n == 0 ==> \result == 1; ensures n > 0 ==> \result >= 1 && \result <= 16 && n >> (\result - 1) == 1 && (\result < 16 ==> n >> \result == 0); */
static u32 altera_bits_req(u32 n)
{
	u32 result = SHORT_BITS;

	if (n == 0)
		result = 1;
	else {
		/* Look for the highest non-zero bit position */
		while ((n & (1 << (SHORT_BITS - 1))) == 0) {
			n <<= 1;
			--result;
		}
	}

	return result;
}
