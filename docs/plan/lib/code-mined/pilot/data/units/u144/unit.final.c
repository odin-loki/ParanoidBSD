#include "prelude.h"
#undef CREATE_TRACE_POINTS
#define CREATE_TRACE_POINTS
#undef F2FS_SPEC_background_gc
#define F2FS_SPEC_background_gc			(1 << 0)
#undef F2FS_SPEC_inline_xattr_size
#define F2FS_SPEC_inline_xattr_size		(1 << 1)
#undef F2FS_SPEC_active_logs
#define F2FS_SPEC_active_logs			(1 << 2)
#undef F2FS_SPEC_reserve_root
#define F2FS_SPEC_reserve_root			(1 << 3)
#undef F2FS_SPEC_resgid
#define F2FS_SPEC_resgid			(1 << 4)
#undef F2FS_SPEC_resuid
#define F2FS_SPEC_resuid			(1 << 5)
#undef F2FS_SPEC_mode
#define F2FS_SPEC_mode				(1 << 6)
#undef F2FS_SPEC_fault_injection
#define F2FS_SPEC_fault_injection		(1 << 7)
#undef F2FS_SPEC_fault_type
#define F2FS_SPEC_fault_type			(1 << 8)
#undef F2FS_SPEC_jqfmt
#define F2FS_SPEC_jqfmt				(1 << 9)
#undef F2FS_SPEC_alloc_mode
#define F2FS_SPEC_alloc_mode			(1 << 10)
#undef F2FS_SPEC_fsync_mode
#define F2FS_SPEC_fsync_mode			(1 << 11)
#undef F2FS_SPEC_checkpoint_disable_cap
#define F2FS_SPEC_checkpoint_disable_cap	(1 << 12)
#undef F2FS_SPEC_checkpoint_disable_cap_perc
#define F2FS_SPEC_checkpoint_disable_cap_perc	(1 << 13)
#undef F2FS_SPEC_compress_level
#define F2FS_SPEC_compress_level		(1 << 14)
#undef F2FS_SPEC_compress_algorithm
#define F2FS_SPEC_compress_algorithm		(1 << 15)
#undef F2FS_SPEC_compress_log_size
#define F2FS_SPEC_compress_log_size		(1 << 16)
#undef F2FS_SPEC_compress_extension
#define F2FS_SPEC_compress_extension		(1 << 17)
#undef F2FS_SPEC_nocompress_extension
#define F2FS_SPEC_nocompress_extension		(1 << 18)
#undef F2FS_SPEC_compress_chksum
#define F2FS_SPEC_compress_chksum		(1 << 19)
#undef F2FS_SPEC_compress_mode
#define F2FS_SPEC_compress_mode			(1 << 20)
#undef F2FS_SPEC_discard_unit
#define F2FS_SPEC_discard_unit			(1 << 21)
#undef F2FS_SPEC_memory_mode
#define F2FS_SPEC_memory_mode			(1 << 22)
#undef F2FS_SPEC_errors
#define F2FS_SPEC_errors			(1 << 23)
#undef F2FS_SPEC_lookup_mode
#define F2FS_SPEC_lookup_mode			(1 << 24)
#undef F2FS_SPEC_reserve_node
#define F2FS_SPEC_reserve_node			(1 << 25)
#undef F2FS_CTX_INFO
#define F2FS_CTX_INFO(ctx)	((ctx)->info)
#undef QTYPE2NAME
#define QTYPE2NAME(t) (quotatypes[t])
/*@ requires 0 <= mask <= 4294967295; ensures \result == 1 <==> (mask & 0x3f) != 0; */
static inline bool test_compression_spec(unsigned int mask)
{
	return mask & (F2FS_SPEC_compress_algorithm
			| F2FS_SPEC_compress_log_size
			| F2FS_SPEC_compress_extension
			| F2FS_SPEC_nocompress_extension
			| F2FS_SPEC_compress_chksum
			| F2FS_SPEC_compress_mode);
}
