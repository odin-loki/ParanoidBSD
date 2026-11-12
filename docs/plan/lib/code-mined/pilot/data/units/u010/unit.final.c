#include "prelude.h"
#undef HOST1X_HW_HOST1X02_UCLASS_H
#define HOST1X_HW_HOST1X02_UCLASS_H
#undef HOST1X_UCLASS_INCR_SYNCPT
#define HOST1X_UCLASS_INCR_SYNCPT  	host1x_uclass_incr_syncpt_r()
#undef HOST1X_UCLASS_INCR_SYNCPT_COND_F
#define HOST1X_UCLASS_INCR_SYNCPT_COND_F(v)  	host1x_uclass_incr_syncpt_cond_f(v)
#undef HOST1X_UCLASS_INCR_SYNCPT_INDX_F
#define HOST1X_UCLASS_INCR_SYNCPT_INDX_F(v)  	host1x_uclass_incr_syncpt_indx_f(v)
#undef HOST1X_UCLASS_WAIT_SYNCPT
#define HOST1X_UCLASS_WAIT_SYNCPT  	host1x_uclass_wait_syncpt_r()
#undef HOST1X_UCLASS_WAIT_SYNCPT_INDX_F
#define HOST1X_UCLASS_WAIT_SYNCPT_INDX_F(v)  	host1x_uclass_wait_syncpt_indx_f(v)
#undef HOST1X_UCLASS_WAIT_SYNCPT_THRESH_F
#define HOST1X_UCLASS_WAIT_SYNCPT_THRESH_F(v)  	host1x_uclass_wait_syncpt_thresh_f(v)
#undef HOST1X_UCLASS_WAIT_SYNCPT_BASE
#define HOST1X_UCLASS_WAIT_SYNCPT_BASE  	host1x_uclass_wait_syncpt_base_r()
#undef HOST1X_UCLASS_WAIT_SYNCPT_BASE_INDX_F
#define HOST1X_UCLASS_WAIT_SYNCPT_BASE_INDX_F(v)  	host1x_uclass_wait_syncpt_base_indx_f(v)
#undef HOST1X_UCLASS_WAIT_SYNCPT_BASE_BASE_INDX_F
#define HOST1X_UCLASS_WAIT_SYNCPT_BASE_BASE_INDX_F(v)  	host1x_uclass_wait_syncpt_base_base_indx_f(v)
#undef HOST1X_UCLASS_WAIT_SYNCPT_BASE_OFFSET_F
#define HOST1X_UCLASS_WAIT_SYNCPT_BASE_OFFSET_F(v)  	host1x_uclass_wait_syncpt_base_offset_f(v)
#undef HOST1X_UCLASS_LOAD_SYNCPT_BASE
#define HOST1X_UCLASS_LOAD_SYNCPT_BASE  	host1x_uclass_load_syncpt_base_r()
#undef HOST1X_UCLASS_LOAD_SYNCPT_BASE_BASE_INDX_F
#define HOST1X_UCLASS_LOAD_SYNCPT_BASE_BASE_INDX_F(v)  	host1x_uclass_load_syncpt_base_base_indx_f(v)
#undef HOST1X_UCLASS_LOAD_SYNCPT_BASE_VALUE_F
#define HOST1X_UCLASS_LOAD_SYNCPT_BASE_VALUE_F(v)  	host1x_uclass_load_syncpt_base_value_f(v)
#undef HOST1X_UCLASS_INCR_SYNCPT_BASE_BASE_INDX_F
#define HOST1X_UCLASS_INCR_SYNCPT_BASE_BASE_INDX_F(v)  	host1x_uclass_incr_syncpt_base_base_indx_f(v)
#undef HOST1X_UCLASS_INCR_SYNCPT_BASE_OFFSET_F
#define HOST1X_UCLASS_INCR_SYNCPT_BASE_OFFSET_F(v)  	host1x_uclass_incr_syncpt_base_offset_f(v)
#undef HOST1X_UCLASS_INDOFF
#define HOST1X_UCLASS_INDOFF  	host1x_uclass_indoff_r()
#undef HOST1X_UCLASS_INDOFF_INDBE_F
#define HOST1X_UCLASS_INDOFF_INDBE_F(v)  	host1x_uclass_indoff_indbe_f(v)
#undef HOST1X_UCLASS_INDOFF_AUTOINC_F
#define HOST1X_UCLASS_INDOFF_AUTOINC_F(v)  	host1x_uclass_indoff_autoinc_f(v)
#undef HOST1X_UCLASS_INDOFF_INDMODID_F
#define HOST1X_UCLASS_INDOFF_INDMODID_F(v)  	host1x_uclass_indoff_indmodid_f(v)
#undef HOST1X_UCLASS_LOAD_SYNCPT_PAYLOAD_32
#define HOST1X_UCLASS_LOAD_SYNCPT_PAYLOAD_32  	host1x_uclass_load_syncpt_payload_32_r()
#undef HOST1X_UCLASS_WAIT_SYNCPT_32
#define HOST1X_UCLASS_WAIT_SYNCPT_32  	host1x_uclass_wait_syncpt_32_r()
/*@ requires true; ensures \result == ((v & 0xffff) << 2); */
static inline u32 host1x_uclass_indoff_indroffset_f(u32 v)
{
	return (v & 0xffff) << 2;
}
