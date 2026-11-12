#include "prelude.h"
#undef WILC_RESP_MSG_TYPE_CONFIG_REPLY
#define WILC_RESP_MSG_TYPE_CONFIG_REPLY		'R'
#undef WILC_RESP_MSG_TYPE_STATUS_INFO
#define WILC_RESP_MSG_TYPE_STATUS_INFO		'I'
#undef WILC_RESP_MSG_TYPE_NETWORK_INFO
#define WILC_RESP_MSG_TYPE_NETWORK_INFO		'N'
#undef WILC_RESP_MSG_TYPE_SCAN_COMPLETE
#define WILC_RESP_MSG_TYPE_SCAN_COMPLETE	'S'
#undef WILC_MAX_CFG_FRAME_SIZE
#define WILC_MAX_CFG_FRAME_SIZE		1468
enum cfg_cmd_type { 	CFG_BYTE_CMD	= 0, 	CFG_HWORD_CMD	= 1, 	CFG_WORD_CMD	= 2, 	CFG_STR_CMD	= 3, 	CFG_BIN_CMD	= 4 };
/*@ requires offset + 4 < WILC_MAX_CFG_FRAME_SIZE; ensures \result == 5 || \result == 0; */
static int wilc_wlan_cfg_set_byte(u8 *frame, u32 offset, u16 id, u8 val8)
{
	if ((offset + 4) >= WILC_MAX_CFG_FRAME_SIZE)
		return 0;

	put_unaligned_le16(id, &frame[offset]);
	put_unaligned_le16(1, &frame[offset + 2]);
	frame[offset + 4] = val8;
	return 5;
}
