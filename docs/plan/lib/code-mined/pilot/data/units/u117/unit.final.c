#include "prelude.h"
#undef _ASM_DMA_H
#define _ASM_DMA_H
#undef dma_outb
#define dma_outb	outb
#undef dma_inb
#define dma_inb		inb
#undef DMA_CHUNK_SIZE
#define DMA_CHUNK_SIZE	(BITS_PER_LONG*PAGE_SIZE)
#undef MAX_DMA_ADDRESS
#define MAX_DMA_ADDRESS (~0UL)
#undef MAX_DMA_CHANNELS
#define MAX_DMA_CHANNELS 8
#undef DMA_MODE_READ
#define DMA_MODE_READ	0x44	
#undef DMA_MODE_WRITE
#define DMA_MODE_WRITE	0x48	
#undef DMA_MODE_CASCADE
#define DMA_MODE_CASCADE 0xC0	
#undef DMA_AUTOINIT
#define DMA_AUTOINIT	0x10
#undef IO_DMA1_BASE
#define IO_DMA1_BASE	0x00	
#undef IO_DMA2_BASE
#define IO_DMA2_BASE	0xC0	
#undef DMA1_CMD_REG
#define DMA1_CMD_REG		0x08	
#undef DMA1_STAT_REG
#define DMA1_STAT_REG		0x08	
#undef DMA1_REQ_REG
#define DMA1_REQ_REG            0x09    
#undef DMA1_MASK_REG
#define DMA1_MASK_REG		0x0A	
#undef DMA1_MODE_REG
#define DMA1_MODE_REG		0x0B	
#undef DMA1_CLEAR_FF_REG
#define DMA1_CLEAR_FF_REG	0x0C	
#undef DMA1_TEMP_REG
#define DMA1_TEMP_REG           0x0D    
#undef DMA1_RESET_REG
#define DMA1_RESET_REG		0x0D	
#undef DMA1_CLR_MASK_REG
#define DMA1_CLR_MASK_REG       0x0E    
#undef DMA1_MASK_ALL_REG
#define DMA1_MASK_ALL_REG       0x0F    
#undef DMA1_EXT_MODE_REG
#define DMA1_EXT_MODE_REG	(0x400 | DMA1_MODE_REG)
#undef DMA2_CMD_REG
#define DMA2_CMD_REG		0xD0	
#undef DMA2_STAT_REG
#define DMA2_STAT_REG		0xD0	
#undef DMA2_REQ_REG
#define DMA2_REQ_REG            0xD2    
#undef DMA2_MASK_REG
#define DMA2_MASK_REG		0xD4	
#undef DMA2_MODE_REG
#define DMA2_MODE_REG		0xD6	
#undef DMA2_CLEAR_FF_REG
#define DMA2_CLEAR_FF_REG	0xD8	
#undef DMA2_TEMP_REG
#define DMA2_TEMP_REG           0xDA    
#undef DMA2_RESET_REG
#define DMA2_RESET_REG		0xDA	
#undef DMA2_CLR_MASK_REG
#define DMA2_CLR_MASK_REG       0xDC    
#undef DMA2_MASK_ALL_REG
#define DMA2_MASK_ALL_REG       0xDE    
#undef DMA2_EXT_MODE_REG
#define DMA2_EXT_MODE_REG	(0x400 | DMA2_MODE_REG)
#undef request_dma
#define request_dma(dmanr, device_id)	(0)
#undef free_dma
#define free_dma(dmanr)
/*@ requires true; ensures \result == 0; */
static __inline__ unsigned long claim_dma_lock(void)
{
	return 0;
}
