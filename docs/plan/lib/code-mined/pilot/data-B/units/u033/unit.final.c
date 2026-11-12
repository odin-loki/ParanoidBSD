#include "prelude.h"
#undef HISI_PTT_TUNE_ATTR
#define HISI_PTT_TUNE_ATTR(_name, _val, _show, _store)			 	static struct hisi_ptt_tune_desc _name##_desc = {		 		.name = #_name,						 		.event_code = (_val),					 	};								 	static struct dev_ext_attribute hisi_ptt_##_name##_attr = {	 		.attr	= __ATTR(_name, 0600, _show, _store),		 		.var	= &_name##_desc,				 	}
#undef HISI_PTT_TUNE_ATTR_COMMON
#define HISI_PTT_TUNE_ATTR_COMMON(_name, _val)		 	HISI_PTT_TUNE_ATTR(_name, _val,			 			   hisi_ptt_tune_attr_show,	 			   hisi_ptt_tune_attr_store)
#undef HISI_PTT_TUNE_QOS_TX_CPL
#define HISI_PTT_TUNE_QOS_TX_CPL		(0x4 | (3 << 16))
#undef HISI_PTT_TUNE_QOS_TX_NP
#define HISI_PTT_TUNE_QOS_TX_NP			(0x4 | (4 << 16))
#undef HISI_PTT_TUNE_QOS_TX_P
#define HISI_PTT_TUNE_QOS_TX_P			(0x4 | (5 << 16))
#undef HISI_PTT_TUNE_RX_ALLOC_BUF_LEVEL
#define HISI_PTT_TUNE_RX_ALLOC_BUF_LEVEL	(0x5 | (6 << 16))
#undef HISI_PTT_TUNE_TX_ALLOC_BUF_LEVEL
#define HISI_PTT_TUNE_TX_ALLOC_BUF_LEVEL	(0x5 | (7 << 16))
/*@ requires val < 4294967296; ensures \result == 0 <==> val != 0 && (val & ~7) == 0; ensures \result == -EINVAL <==> val == 0 || (val & ~7) != 0; */
static int hisi_ptt_trace_valid_type(u32 val)
{
	/* Different types can be set simultaneously */
	static const u32 hisi_ptt_trace_available_type[] = {
		1,	/* posted_request */
		2,	/* non-posted_request */
		4,	/* completion */
	};
	int i;

	if (!val)
		return -EINVAL;

	/*
	 * Walk the available list and clear the valid bits of
	 * the config. If there is any resident bit after the
	 * walk then the config is invalid.
	 */
	for (i = 0; i < ARRAY_SIZE(hisi_ptt_trace_available_type); i++)
		val &= ~hisi_ptt_trace_available_type[i];

	if (val)
		return -EINVAL;

	return 0;
}
