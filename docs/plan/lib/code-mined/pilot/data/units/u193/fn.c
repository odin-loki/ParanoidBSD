static inline bool mcp251xfd_tx_fifo_sta_empty(u32 fifo_sta)
{
	return fifo_sta & MCP251XFD_REG_FIFOSTA_TFERFFIF;
}
