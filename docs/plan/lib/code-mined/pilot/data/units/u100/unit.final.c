#include "prelude.h"
#undef KMSG_COMPONENT
#define KMSG_COMPONENT "zram"
#undef pr_fmt
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt
#undef ZRAM_MAX_ALGO_NAME_SZ
#define ZRAM_MAX_ALGO_NAME_SZ	128
#undef slot_dep_map
#define slot_dep_map(zram, index) (&(zram)->table[(index)].dep_map)
#undef ZRAM_PARTIAL_IO
#define ZRAM_PARTIAL_IO		1
#undef PP_BUCKET_SIZE_RANGE
#define PP_BUCKET_SIZE_RANGE	64
#undef NUM_PP_BUCKETS
#define NUM_PP_BUCKETS		((PAGE_SIZE / PP_BUCKET_SIZE_RANGE) + 1)
#undef PAGE_WRITEBACK
#define PAGE_WRITEBACK			0
#undef HUGE_WRITEBACK
#define HUGE_WRITEBACK			(1 << 0)
#undef IDLE_WRITEBACK
#define IDLE_WRITEBACK			(1 << 1)
#undef INCOMPRESSIBLE_WRITEBACK
#define INCOMPRESSIBLE_WRITEBACK	(1 << 2)
#undef FOUR_K
#define FOUR_K(x) ((x) * (1 << (PAGE_SHIFT - 12)))
#undef RECOMPRESS_IDLE
#define RECOMPRESS_IDLE		(1 << 0)
#undef RECOMPRESS_HUGE
#define RECOMPRESS_HUGE		(1 << 1)
/*@ requires val != NULL; ensures \result == 0 ==> mode != 0; ensures \result != 0 ==> (\result == -EINVAL && mode == 0); */
static int parse_mode(char *val, u32 *mode)
{
	*mode = 0;

	if (!strcmp(val, "idle"))
		*mode = IDLE_WRITEBACK;
	if (!strcmp(val, "huge"))
		*mode = HUGE_WRITEBACK;
	if (!strcmp(val, "huge_idle"))
		*mode = IDLE_WRITEBACK | HUGE_WRITEBACK;
	if (!strcmp(val, "incompressible"))
		*mode = INCOMPRESSIBLE_WRITEBACK;

	if (*mode == 0)
		return -EINVAL;
	return 0;
}
