#include "prelude.h"
#undef MCP251XFD_REG_FIFOSTA_TFERFFIF
#define MCP251XFD_REG_FIFOSTA_TFERFFIF BIT(2)
/*@ requires true; ensures \result == ((fifo_sta & MCP251XFD_REG_FIFOSTA_TFERFFIF) != 0); */
static inline bool mcp251xfd_tx_fifo_sta_empty(u32 fifo_sta)
{
	return fifo_sta & MCP251XFD_REG_FIFOSTA_TFERFFIF;
}
