#include "prelude.h"
#undef __BNXT_RE_IB_VERBS_H__
#define __BNXT_RE_IB_VERBS_H__
#undef BNXT_RE_FENCE_BYTES
#define BNXT_RE_FENCE_BYTES	64
#undef MAX_CQL_PER_POLL
#define MAX_CQL_PER_POLL	1024
/*@ requires 0 <= port_id <= 65534; ensures \result == (u32)port_id + 1; */
static inline u32 __to_ib_port_num(u16 port_id)
{
	return (u32)port_id + 1;
}
