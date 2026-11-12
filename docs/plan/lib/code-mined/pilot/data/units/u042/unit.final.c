#include "prelude.h"
#undef HOST1X_HW_HOST1X02_SYNC_H
#define HOST1X_HW_HOST1X02_SYNC_H
#undef REGISTER_STRIDE
#define REGISTER_STRIDE	4
#undef HOST1X_SYNC_SYNCPT
#define HOST1X_SYNC_SYNCPT(id)  	host1x_sync_syncpt_r(id)
#undef HOST1X_SYNC_SYNCPT_THRESH_CPU0_INT_STATUS
#define HOST1X_SYNC_SYNCPT_THRESH_CPU0_INT_STATUS(id)  	host1x_sync_syncpt_thresh_cpu0_int_status_r(id)
#undef HOST1X_SYNC_SYNCPT_THRESH_INT_DISABLE
#define HOST1X_SYNC_SYNCPT_THRESH_INT_DISABLE(id)  	host1x_sync_syncpt_thresh_int_disable_r(id)
#undef HOST1X_SYNC_SYNCPT_THRESH_INT_ENABLE_CPU0
#define HOST1X_SYNC_SYNCPT_THRESH_INT_ENABLE_CPU0(id)  	host1x_sync_syncpt_thresh_int_enable_cpu0_r(id)
#undef HOST1X_SYNC_CF_SETUP
#define HOST1X_SYNC_CF_SETUP(channel)  	host1x_sync_cf_setup_r(channel)
#undef HOST1X_SYNC_CF_SETUP_BASE_V
#define HOST1X_SYNC_CF_SETUP_BASE_V(r)  	host1x_sync_cf_setup_base_v(r)
#undef HOST1X_SYNC_CF_SETUP_LIMIT_V
#define HOST1X_SYNC_CF_SETUP_LIMIT_V(r)  	host1x_sync_cf_setup_limit_v(r)
#undef HOST1X_SYNC_CMDPROC_STOP
#define HOST1X_SYNC_CMDPROC_STOP  	host1x_sync_cmdproc_stop_r()
#undef HOST1X_SYNC_CH_TEARDOWN
#define HOST1X_SYNC_CH_TEARDOWN  	host1x_sync_ch_teardown_r()
#undef HOST1X_SYNC_USEC_CLK
#define HOST1X_SYNC_USEC_CLK  	host1x_sync_usec_clk_r()
#undef HOST1X_SYNC_CTXSW_TIMEOUT_CFG
#define HOST1X_SYNC_CTXSW_TIMEOUT_CFG  	host1x_sync_ctxsw_timeout_cfg_r()
#undef HOST1X_SYNC_IP_BUSY_TIMEOUT
#define HOST1X_SYNC_IP_BUSY_TIMEOUT  	host1x_sync_ip_busy_timeout_r()
#undef HOST1X_SYNC_MLOCK_OWNER
#define HOST1X_SYNC_MLOCK_OWNER(id)  	host1x_sync_mlock_owner_r(id)
#undef HOST1X_SYNC_MLOCK_OWNER_CHID_V
#define HOST1X_SYNC_MLOCK_OWNER_CHID_V(v)  	host1x_sync_mlock_owner_chid_v(v)
#undef HOST1X_SYNC_MLOCK_OWNER_CPU_OWNS_V
#define HOST1X_SYNC_MLOCK_OWNER_CPU_OWNS_V(r)  	host1x_sync_mlock_owner_cpu_owns_v(r)
#undef HOST1X_SYNC_MLOCK_OWNER_CH_OWNS_V
#define HOST1X_SYNC_MLOCK_OWNER_CH_OWNS_V(r)  	host1x_sync_mlock_owner_ch_owns_v(r)
#undef HOST1X_SYNC_SYNCPT_INT_THRESH
#define HOST1X_SYNC_SYNCPT_INT_THRESH(id)  	host1x_sync_syncpt_int_thresh_r(id)
#undef HOST1X_SYNC_SYNCPT_BASE
#define HOST1X_SYNC_SYNCPT_BASE(id)  	host1x_sync_syncpt_base_r(id)
#undef HOST1X_SYNC_SYNCPT_CPU_INCR
#define HOST1X_SYNC_SYNCPT_CPU_INCR(id)  	host1x_sync_syncpt_cpu_incr_r(id)
#undef HOST1X_SYNC_CBREAD
#define HOST1X_SYNC_CBREAD(channel)  	host1x_sync_cbread_r(channel)
#undef HOST1X_SYNC_CFPEEK_CTRL
#define HOST1X_SYNC_CFPEEK_CTRL  	host1x_sync_cfpeek_ctrl_r()
#undef HOST1X_SYNC_CFPEEK_CTRL_ADDR_F
#define HOST1X_SYNC_CFPEEK_CTRL_ADDR_F(v)  	host1x_sync_cfpeek_ctrl_addr_f(v)
#undef HOST1X_SYNC_CFPEEK_CTRL_CHANNR_F
#define HOST1X_SYNC_CFPEEK_CTRL_CHANNR_F(v)  	host1x_sync_cfpeek_ctrl_channr_f(v)
#undef HOST1X_SYNC_CFPEEK_CTRL_ENA_F
#define HOST1X_SYNC_CFPEEK_CTRL_ENA_F(v)  	host1x_sync_cfpeek_ctrl_ena_f(v)
#undef HOST1X_SYNC_CFPEEK_READ
#define HOST1X_SYNC_CFPEEK_READ  	host1x_sync_cfpeek_read_r()
#undef HOST1X_SYNC_CFPEEK_PTRS
#define HOST1X_SYNC_CFPEEK_PTRS  	host1x_sync_cfpeek_ptrs_r()
#undef HOST1X_SYNC_CFPEEK_PTRS_CF_RD_PTR_V
#define HOST1X_SYNC_CFPEEK_PTRS_CF_RD_PTR_V(r)  	host1x_sync_cfpeek_ptrs_cf_rd_ptr_v(r)
#undef HOST1X_SYNC_CFPEEK_PTRS_CF_WR_PTR_V
#define HOST1X_SYNC_CFPEEK_PTRS_CF_WR_PTR_V(r)  	host1x_sync_cfpeek_ptrs_cf_wr_ptr_v(r)
#undef HOST1X_SYNC_CBSTAT
#define HOST1X_SYNC_CBSTAT(channel)  	host1x_sync_cbstat_r(channel)
#undef HOST1X_SYNC_CBSTAT_CBOFFSET_V
#define HOST1X_SYNC_CBSTAT_CBOFFSET_V(r)  	host1x_sync_cbstat_cboffset_v(r)
#undef HOST1X_SYNC_CBSTAT_CBCLASS_V
#define HOST1X_SYNC_CBSTAT_CBCLASS_V(r)  	host1x_sync_cbstat_cbclass_v(r)
/*@ requires channel <= 0xffff; ensures \result == 0x720 + channel * 4; */
static inline u32 host1x_sync_cbread_r(unsigned int channel)
{
	return 0x720 + channel * REGISTER_STRIDE;
}
