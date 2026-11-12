static inline bool validate_packet_id(enum packet_id id)
{
	switch (id) {
	case PACKET_WREG_32:
	case PACKET_WREG_BULK:
	case PACKET_MSG_LONG:
	case PACKET_MSG_SHORT:
	case PACKET_CP_DMA:
	case PACKET_MSG_PROT:
	case PACKET_FENCE:
	case PACKET_LIN_DMA:
	case PACKET_NOP:
	case PACKET_STOP:
		return true;
	default:
		return false;
	}
}
