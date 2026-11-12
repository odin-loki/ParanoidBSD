#include "prelude.h"
#undef IQK_DONE_8822C
#define IQK_DONE_8822C 0xaa
#undef RF_SET_POWER_TRIM
#define RF_SET_POWER_TRIM(_path, _seq, _idx)					 		do {								 			rtw_write_rf(rtwdev, _path, 0x33, RFREG_MASK, _seq);	 			rtw_write_rf(rtwdev, _path, 0x3f, RFREG_MASK,		 				     bb_gain[_path][_idx]);			 		} while (0)
#undef WLAN_TXQ_RPT_EN
#define WLAN_TXQ_RPT_EN		0x1F
#undef WLAN_SLOT_TIME
#define WLAN_SLOT_TIME		0x09
#undef WLAN_PIFS_TIME
#define WLAN_PIFS_TIME		0x1C
#undef WLAN_SIFS_CCK_CONT_TX
#define WLAN_SIFS_CCK_CONT_TX	0x0A
#undef WLAN_SIFS_OFDM_CONT_TX
#define WLAN_SIFS_OFDM_CONT_TX	0x0E
#undef WLAN_SIFS_CCK_TRX
#define WLAN_SIFS_CCK_TRX	0x0A
#undef WLAN_SIFS_OFDM_TRX
#define WLAN_SIFS_OFDM_TRX	0x10
#undef WLAN_NAV_MAX
#define WLAN_NAV_MAX		0xC8
#undef WLAN_RDG_NAV
#define WLAN_RDG_NAV		0x05
#undef WLAN_TXOP_NAV
#define WLAN_TXOP_NAV		0x1B
#undef WLAN_CCK_RX_TSF
#define WLAN_CCK_RX_TSF		0x30
#undef WLAN_OFDM_RX_TSF
#define WLAN_OFDM_RX_TSF	0x30
#undef WLAN_TBTT_PROHIBIT
#define WLAN_TBTT_PROHIBIT	0x04 
#undef WLAN_TBTT_HOLD_TIME
#define WLAN_TBTT_HOLD_TIME	0x064 
#undef WLAN_DRV_EARLY_INT
#define WLAN_DRV_EARLY_INT	0x04
#undef WLAN_BCN_CTRL_CLT0
#define WLAN_BCN_CTRL_CLT0	0x10
#undef WLAN_BCN_DMA_TIME
#define WLAN_BCN_DMA_TIME	0x02
#undef WLAN_BCN_MAX_ERR
#define WLAN_BCN_MAX_ERR	0xFF
#undef WLAN_SIFS_CCK_DUR_TUNE
#define WLAN_SIFS_CCK_DUR_TUNE	0x0A
#undef WLAN_SIFS_OFDM_DUR_TUNE
#define WLAN_SIFS_OFDM_DUR_TUNE	0x10
#undef WLAN_SIFS_CCK_CTX
#define WLAN_SIFS_CCK_CTX	0x0A
#undef WLAN_SIFS_CCK_IRX
#define WLAN_SIFS_CCK_IRX	0x0A
#undef WLAN_SIFS_OFDM_CTX
#define WLAN_SIFS_OFDM_CTX	0x0E
#undef WLAN_SIFS_OFDM_IRX
#define WLAN_SIFS_OFDM_IRX	0x0E
#undef WLAN_EIFS_DUR_TUNE
#define WLAN_EIFS_DUR_TUNE	0x40
#undef WLAN_EDCA_VO_PARAM
#define WLAN_EDCA_VO_PARAM	0x002FA226
#undef WLAN_EDCA_VI_PARAM
#define WLAN_EDCA_VI_PARAM	0x005EA328
#undef WLAN_EDCA_BE_PARAM
#define WLAN_EDCA_BE_PARAM	0x005EA42B
#undef WLAN_EDCA_BK_PARAM
#define WLAN_EDCA_BK_PARAM	0x0000A44F
#undef WLAN_RX_FILTER0
#define WLAN_RX_FILTER0		0xFFFFFFFF
#undef WLAN_RX_FILTER2
#define WLAN_RX_FILTER2		0xFFFF
#undef WLAN_RCR_CFG
#define WLAN_RCR_CFG		0xE400220E
#undef WLAN_RXPKT_MAX_SZ
#define WLAN_RXPKT_MAX_SZ	12288
#undef WLAN_RXPKT_MAX_SZ_512
#define WLAN_RXPKT_MAX_SZ_512	(WLAN_RXPKT_MAX_SZ >> 9)
#undef WLAN_AMPDU_MAX_TIME
#define WLAN_AMPDU_MAX_TIME		0x70
#undef WLAN_RTS_LEN_TH
#define WLAN_RTS_LEN_TH			0xFF
#undef WLAN_RTS_TX_TIME_TH
#define WLAN_RTS_TX_TIME_TH		0x08
#undef WLAN_MAX_AGG_PKT_LIMIT
#define WLAN_MAX_AGG_PKT_LIMIT		0x3f
#undef WLAN_RTS_MAX_AGG_PKT_LIMIT
#define WLAN_RTS_MAX_AGG_PKT_LIMIT	0x3f
#undef WLAN_PRE_TXCNT_TIME_TH
#define WLAN_PRE_TXCNT_TIME_TH		0x1E0
#undef FAST_EDCA_VO_TH
#define FAST_EDCA_VO_TH		0x06
#undef FAST_EDCA_VI_TH
#define FAST_EDCA_VI_TH		0x06
#undef FAST_EDCA_BE_TH
#define FAST_EDCA_BE_TH		0x06
#undef FAST_EDCA_BK_TH
#define FAST_EDCA_BK_TH		0x06
#undef WLAN_BAR_RETRY_LIMIT
#define WLAN_BAR_RETRY_LIMIT		0x01
#undef WLAN_BAR_ACK_TYPE
#define WLAN_BAR_ACK_TYPE		0x05
#undef WLAN_RA_TRY_RATE_AGG_LIMIT
#define WLAN_RA_TRY_RATE_AGG_LIMIT	0x08
#undef WLAN_RESP_TXRATE
#define WLAN_RESP_TXRATE		0x84
#undef WLAN_ACK_TO
#define WLAN_ACK_TO			0x21
#undef WLAN_ACK_TO_CCK
#define WLAN_ACK_TO_CCK			0x6A
#undef WLAN_DATA_RATE_FB_CNT_1_4
#define WLAN_DATA_RATE_FB_CNT_1_4	0x01000000
#undef WLAN_DATA_RATE_FB_CNT_5_8
#define WLAN_DATA_RATE_FB_CNT_5_8	0x08070504
#undef WLAN_RTS_RATE_FB_CNT_5_8
#define WLAN_RTS_RATE_FB_CNT_5_8	0x08070504
#undef WLAN_DATA_RATE_FB_RATE0
#define WLAN_DATA_RATE_FB_RATE0		0xFE01F010
#undef WLAN_DATA_RATE_FB_RATE0_H
#define WLAN_DATA_RATE_FB_RATE0_H	0x40000000
#undef WLAN_RTS_RATE_FB_RATE1
#define WLAN_RTS_RATE_FB_RATE1		0x003FF010
#undef WLAN_RTS_RATE_FB_RATE1_H
#define WLAN_RTS_RATE_FB_RATE1_H	0x40000000
#undef WLAN_RTS_RATE_FB_RATE4
#define WLAN_RTS_RATE_FB_RATE4		0x0600F010
#undef WLAN_RTS_RATE_FB_RATE4_H
#define WLAN_RTS_RATE_FB_RATE4_H	0x400003E0
#undef WLAN_RTS_RATE_FB_RATE5
#define WLAN_RTS_RATE_FB_RATE5		0x0600F015
#undef WLAN_RTS_RATE_FB_RATE5_H
#define WLAN_RTS_RATE_FB_RATE5_H	0x000000E0
#undef WLAN_MULTI_ADDR
#define WLAN_MULTI_ADDR			0xFFFFFFFF
#undef WLAN_TX_FUNC_CFG1
#define WLAN_TX_FUNC_CFG1		0x30
#undef WLAN_TX_FUNC_CFG2
#define WLAN_TX_FUNC_CFG2		0x30
#undef WLAN_MAC_OPT_NORM_FUNC1
#define WLAN_MAC_OPT_NORM_FUNC1		0x98
#undef WLAN_MAC_OPT_LB_FUNC1
#define WLAN_MAC_OPT_LB_FUNC1		0x80
#undef WLAN_MAC_OPT_FUNC2
#define WLAN_MAC_OPT_FUNC2		0xb0810041
#undef WLAN_MAC_INT_MIG_CFG
#define WLAN_MAC_INT_MIG_CFG		0x33330000
#undef WLAN_SIFS_CFG
#define WLAN_SIFS_CFG	(WLAN_SIFS_CCK_CONT_TX |  			(WLAN_SIFS_OFDM_CONT_TX << BIT_SHIFT_SIFS_OFDM_CTX) |  			(WLAN_SIFS_CCK_TRX << BIT_SHIFT_SIFS_CCK_TRX) |  			(WLAN_SIFS_OFDM_TRX << BIT_SHIFT_SIFS_OFDM_TRX))
#undef WLAN_SIFS_DUR_TUNE
#define WLAN_SIFS_DUR_TUNE	(WLAN_SIFS_CCK_DUR_TUNE |  				(WLAN_SIFS_OFDM_DUR_TUNE << 8))
#undef WLAN_TBTT_TIME
#define WLAN_TBTT_TIME	(WLAN_TBTT_PROHIBIT | 			(WLAN_TBTT_HOLD_TIME << BIT_SHIFT_TBTT_HOLD_TIME_AP))
#undef WLAN_NAV_CFG
#define WLAN_NAV_CFG		(WLAN_RDG_NAV | (WLAN_TXOP_NAV << 16))
#undef WLAN_RX_TSF_CFG
#define WLAN_RX_TSF_CFG		(WLAN_CCK_RX_TSF | (WLAN_OFDM_RX_TSF) << 8)
#undef MAC_CLK_SPEED
#define MAC_CLK_SPEED	80 
#undef EFUSE_PCB_INFO_OFFSET
#define EFUSE_PCB_INFO_OFFSET	0xCA
#undef FWCD_SIZE_REG_8822C
#define FWCD_SIZE_REG_8822C 0x2000
#undef FWCD_SIZE_DMEM_8822C
#define FWCD_SIZE_DMEM_8822C 0x10000
#undef FWCD_SIZE_IMEM_8822C
#define FWCD_SIZE_IMEM_8822C 0x10000
#undef FWCD_SIZE_EMEM_8822C
#define FWCD_SIZE_EMEM_8822C 0x20000
#undef FWCD_SIZE_ROM_8822C
#define FWCD_SIZE_ROM_8822C 0x10000
#undef __dump_fw_8822c
#define __dump_fw_8822c(_dev, _mem)  	rtw_dump_fw(_dev, OCPBASE_ ## _mem ## _88XX,  		    FWCD_SIZE_ ## _mem ## _8822C, RTW_FWCD_ ## _mem)
#undef RF18_BAND_MASK
#define RF18_BAND_MASK		(BIT(16) | BIT(9) | BIT(8))
#undef RF18_BAND_2G
#define RF18_BAND_2G		(0)
#undef RF18_BAND_5G
#define RF18_BAND_5G		(BIT(16) | BIT(8))
#undef RF18_CHANNEL_MASK
#define RF18_CHANNEL_MASK	(MASKBYTE0)
#undef RF18_RFSI_MASK
#define RF18_RFSI_MASK		(BIT(18) | BIT(17))
#undef RF18_RFSI_GE_CH80
#define RF18_RFSI_GE_CH80	(BIT(17))
#undef RF18_RFSI_GT_CH140
#define RF18_RFSI_GT_CH140	(BIT(18))
#undef RF18_BW_MASK
#define RF18_BW_MASK		(BIT(13) | BIT(12))
#undef RF18_BW_20M
#define RF18_BW_20M		(BIT(13) | BIT(12))
#undef RF18_BW_40M
#define RF18_BW_40M		(BIT(13))
#undef RF18_BW_80M
#define RF18_BW_80M		(BIT(12))
#undef XCAP_EXTEND
#define XCAP_EXTEND(val) ({typeof(val) _v = (val); _v | _v << 7; })
#undef REPORT_TO_KHZ
#define REPORT_TO_KHZ(val) ({typeof(val) _v = (val); (_v << 1) + (_v >> 1); })
#undef RTW_CCK_PD_MAX
#define RTW_CCK_PD_MAX 255
#undef RTW_CCK_CS_MAX
#define RTW_CCK_CS_MAX 31
#undef RTW_CCK_CS_ERR1
#define RTW_CCK_CS_ERR1 27
#undef RTW_CCK_CS_ERR2
#define RTW_CCK_CS_ERR2 29
#undef PWR_TRACK_MASK
#define PWR_TRACK_MASK 0x7f
struct dpk_cfg_pair { 	u32 addr; 	u32 bitmask; 	u32 data; };
struct rtw8822c_dpk_data { 	u8 txbb; 	u8 pga; 	u8 limited_pga; 	u8 agc_cnt; 	bool loss_only; 	bool gain_only; 	u8 path; };
/*@ requires is_fail == 0 || is_fail == 1; ensures \result != 0 ==> is_fail == 0; */
static bool rtw8822c_dpk_check_pass(struct rtw_dev *rtwdev, bool is_fail,
				    u32 dpk_txagc, u8 path)
{
	bool result;

	if (!is_fail) {
		if (rtw8822c_dpk_coef_read(rtwdev, path))
			result = true;
		else
			result = false;
	} else {
		result = false;
	}

	rtw8822c_dpk_fill_result(rtwdev, dpk_txagc, path, result);

	return result;
}
