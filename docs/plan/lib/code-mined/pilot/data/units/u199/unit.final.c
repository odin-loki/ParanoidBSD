#include "prelude.h"
#undef AT_XDMAC_GTYPE
#define AT_XDMAC_GTYPE		0x00	
#undef AT_XDMAC_NB_CH
#define		AT_XDMAC_NB_CH(i)	(((i) & 0x1F) + 1)		
#undef AT_XDMAC_FIFO_SZ
#define		AT_XDMAC_FIFO_SZ(i)	(((i) >> 5) & 0x7FF)		
#undef AT_XDMAC_NB_REQ
#define		AT_XDMAC_NB_REQ(i)	((((i) >> 16) & 0x3F) + 1)	
#undef AT_XDMAC_GCFG
#define AT_XDMAC_GCFG		0x04	
#undef AT_XDMAC_WRHP
#define		AT_XDMAC_WRHP(i)		(((i) & 0xF) << 4)
#undef AT_XDMAC_WRMP
#define		AT_XDMAC_WRMP(i)		(((i) & 0xF) << 8)
#undef AT_XDMAC_WRLP
#define		AT_XDMAC_WRLP(i)		(((i) & 0xF) << 12)
#undef AT_XDMAC_RDHP
#define		AT_XDMAC_RDHP(i)		(((i) & 0xF) << 16)
#undef AT_XDMAC_RDMP
#define		AT_XDMAC_RDMP(i)		(((i) & 0xF) << 20)
#undef AT_XDMAC_RDLP
#define		AT_XDMAC_RDLP(i)		(((i) & 0xF) << 24)
#undef AT_XDMAC_RDSG
#define		AT_XDMAC_RDSG(i)		(((i) & 0xF) << 28)
#undef AT_XDMAC_GCFG_M2M
#define AT_XDMAC_GCFG_M2M	(AT_XDMAC_RDLP(0xF) | AT_XDMAC_WRLP(0xF))
#undef AT_XDMAC_GCFG_P2M
#define AT_XDMAC_GCFG_P2M	(AT_XDMAC_RDSG(0x1) | AT_XDMAC_RDHP(0x3) |  				AT_XDMAC_WRHP(0x5))
#undef AT_XDMAC_GWAC
#define AT_XDMAC_GWAC		0x08	
#undef AT_XDMAC_PW0
#define		AT_XDMAC_PW0(i)		(((i) & 0xF) << 0)
#undef AT_XDMAC_PW1
#define		AT_XDMAC_PW1(i)		(((i) & 0xF) << 4)
#undef AT_XDMAC_PW2
#define		AT_XDMAC_PW2(i)		(((i) & 0xF) << 8)
#undef AT_XDMAC_PW3
#define		AT_XDMAC_PW3(i)		(((i) & 0xF) << 12)
#undef AT_XDMAC_GWAC_M2M
#define AT_XDMAC_GWAC_M2M	0
#undef AT_XDMAC_GWAC_P2M
#define AT_XDMAC_GWAC_P2M	(AT_XDMAC_PW0(0xF) | AT_XDMAC_PW2(0xF))
#undef AT_XDMAC_GIE
#define AT_XDMAC_GIE		0x0C	
#undef AT_XDMAC_GID
#define AT_XDMAC_GID		0x10	
#undef AT_XDMAC_GIM
#define AT_XDMAC_GIM		0x14	
#undef AT_XDMAC_GIS
#define AT_XDMAC_GIS		0x18	
#undef AT_XDMAC_GE
#define AT_XDMAC_GE		0x1C	
#undef AT_XDMAC_GD
#define AT_XDMAC_GD		0x20	
#undef AT_XDMAC_GS
#define AT_XDMAC_GS		0x24	
#undef AT_XDMAC_VERSION
#define AT_XDMAC_VERSION	0xFFC	
#undef AT_XDMAC_CIE
#define AT_XDMAC_CIE		0x00	
#undef AT_XDMAC_CIE_BIE
#define		AT_XDMAC_CIE_BIE	BIT(0)	
#undef AT_XDMAC_CIE_LIE
#define		AT_XDMAC_CIE_LIE	BIT(1)	
#undef AT_XDMAC_CIE_DIE
#define		AT_XDMAC_CIE_DIE	BIT(2)	
#undef AT_XDMAC_CIE_FIE
#define		AT_XDMAC_CIE_FIE	BIT(3)	
#undef AT_XDMAC_CIE_RBEIE
#define		AT_XDMAC_CIE_RBEIE	BIT(4)	
#undef AT_XDMAC_CIE_WBEIE
#define		AT_XDMAC_CIE_WBEIE	BIT(5)	
#undef AT_XDMAC_CIE_ROIE
#define		AT_XDMAC_CIE_ROIE	BIT(6)	
#undef AT_XDMAC_CID
#define AT_XDMAC_CID		0x04	
#undef AT_XDMAC_CID_BID
#define		AT_XDMAC_CID_BID	BIT(0)	
#undef AT_XDMAC_CID_LID
#define		AT_XDMAC_CID_LID	BIT(1)	
#undef AT_XDMAC_CID_DID
#define		AT_XDMAC_CID_DID	BIT(2)	
#undef AT_XDMAC_CID_FID
#define		AT_XDMAC_CID_FID	BIT(3)	
#undef AT_XDMAC_CID_RBEID
#define		AT_XDMAC_CID_RBEID	BIT(4)	
#undef AT_XDMAC_CID_WBEID
#define		AT_XDMAC_CID_WBEID	BIT(5)	
#undef AT_XDMAC_CID_ROID
#define		AT_XDMAC_CID_ROID	BIT(6)	
#undef AT_XDMAC_CIM
#define AT_XDMAC_CIM		0x08	
#undef AT_XDMAC_CIM_BIM
#define		AT_XDMAC_CIM_BIM	BIT(0)	
#undef AT_XDMAC_CIM_LIM
#define		AT_XDMAC_CIM_LIM	BIT(1)	
#undef AT_XDMAC_CIM_DIM
#define		AT_XDMAC_CIM_DIM	BIT(2)	
#undef AT_XDMAC_CIM_FIM
#define		AT_XDMAC_CIM_FIM	BIT(3)	
#undef AT_XDMAC_CIM_RBEIM
#define		AT_XDMAC_CIM_RBEIM	BIT(4)	
#undef AT_XDMAC_CIM_WBEIM
#define		AT_XDMAC_CIM_WBEIM	BIT(5)	
#undef AT_XDMAC_CIM_ROIM
#define		AT_XDMAC_CIM_ROIM	BIT(6)	
#undef AT_XDMAC_CIS
#define AT_XDMAC_CIS		0x0C	
#undef AT_XDMAC_CIS_BIS
#define		AT_XDMAC_CIS_BIS	BIT(0)	
#undef AT_XDMAC_CIS_LIS
#define		AT_XDMAC_CIS_LIS	BIT(1)	
#undef AT_XDMAC_CIS_DIS
#define		AT_XDMAC_CIS_DIS	BIT(2)	
#undef AT_XDMAC_CIS_FIS
#define		AT_XDMAC_CIS_FIS	BIT(3)	
#undef AT_XDMAC_CIS_RBEIS
#define		AT_XDMAC_CIS_RBEIS	BIT(4)	
#undef AT_XDMAC_CIS_WBEIS
#define		AT_XDMAC_CIS_WBEIS	BIT(5)	
#undef AT_XDMAC_CIS_ROIS
#define		AT_XDMAC_CIS_ROIS	BIT(6)	
#undef AT_XDMAC_CSA
#define AT_XDMAC_CSA		0x10	
#undef AT_XDMAC_CDA
#define AT_XDMAC_CDA		0x14	
#undef AT_XDMAC_CNDA
#define AT_XDMAC_CNDA		0x18	
#undef AT_XDMAC_CNDA_NDAIF
#define		AT_XDMAC_CNDA_NDAIF(i)	((i) & 0x1)			
#undef AT_XDMAC_CNDA_NDA
#define		AT_XDMAC_CNDA_NDA(i)	((i) & 0xfffffffc)		
#undef AT_XDMAC_CNDC
#define AT_XDMAC_CNDC		0x1C	
#undef AT_XDMAC_CNDC_NDE
#define		AT_XDMAC_CNDC_NDE		(0x1 << 0)		
#undef AT_XDMAC_CNDC_NDSUP
#define		AT_XDMAC_CNDC_NDSUP		(0x1 << 1)		
#undef AT_XDMAC_CNDC_NDDUP
#define		AT_XDMAC_CNDC_NDDUP		(0x1 << 2)		
#undef AT_XDMAC_CNDC_NDVIEW_MASK
#define		AT_XDMAC_CNDC_NDVIEW_MASK	GENMASK(28, 27)
#undef AT_XDMAC_CNDC_NDVIEW_NDV0
#define		AT_XDMAC_CNDC_NDVIEW_NDV0	(0x0 << 3)		
#undef AT_XDMAC_CNDC_NDVIEW_NDV1
#define		AT_XDMAC_CNDC_NDVIEW_NDV1	(0x1 << 3)		
#undef AT_XDMAC_CNDC_NDVIEW_NDV2
#define		AT_XDMAC_CNDC_NDVIEW_NDV2	(0x2 << 3)		
#undef AT_XDMAC_CNDC_NDVIEW_NDV3
#define		AT_XDMAC_CNDC_NDVIEW_NDV3	(0x3 << 3)		
#undef AT_XDMAC_CUBC
#define AT_XDMAC_CUBC		0x20	
#undef AT_XDMAC_CBC
#define AT_XDMAC_CBC		0x24	
#undef AT_XDMAC_CC
#define AT_XDMAC_CC		0x28	
#undef AT_XDMAC_CC_TYPE
#define		AT_XDMAC_CC_TYPE	(0x1 << 0)	
#undef AT_XDMAC_CC_TYPE_MEM_TRAN
#define			AT_XDMAC_CC_TYPE_MEM_TRAN	(0x0 << 0)	
#undef AT_XDMAC_CC_TYPE_PER_TRAN
#define			AT_XDMAC_CC_TYPE_PER_TRAN	(0x1 << 0)	
#undef AT_XDMAC_CC_MBSIZE_MASK
#define		AT_XDMAC_CC_MBSIZE_MASK	(0x3 << 1)
#undef AT_XDMAC_CC_MBSIZE_SINGLE
#define			AT_XDMAC_CC_MBSIZE_SINGLE	(0x0 << 1)
#undef AT_XDMAC_CC_MBSIZE_FOUR
#define			AT_XDMAC_CC_MBSIZE_FOUR		(0x1 << 1)
#undef AT_XDMAC_CC_MBSIZE_EIGHT
#define			AT_XDMAC_CC_MBSIZE_EIGHT	(0x2 << 1)
#undef AT_XDMAC_CC_MBSIZE_SIXTEEN
#define			AT_XDMAC_CC_MBSIZE_SIXTEEN	(0x3 << 1)
#undef AT_XDMAC_CC_DSYNC
#define		AT_XDMAC_CC_DSYNC	(0x1 << 4)	
#undef AT_XDMAC_CC_DSYNC_PER2MEM
#define			AT_XDMAC_CC_DSYNC_PER2MEM	(0x0 << 4)
#undef AT_XDMAC_CC_DSYNC_MEM2PER
#define			AT_XDMAC_CC_DSYNC_MEM2PER	(0x1 << 4)
#undef AT_XDMAC_CC_PROT
#define		AT_XDMAC_CC_PROT	(0x1 << 5)	
#undef AT_XDMAC_CC_PROT_SEC
#define			AT_XDMAC_CC_PROT_SEC		(0x0 << 5)
#undef AT_XDMAC_CC_PROT_UNSEC
#define			AT_XDMAC_CC_PROT_UNSEC		(0x1 << 5)
#undef AT_XDMAC_CC_SWREQ
#define		AT_XDMAC_CC_SWREQ	(0x1 << 6)	
#undef AT_XDMAC_CC_SWREQ_HWR_CONNECTED
#define			AT_XDMAC_CC_SWREQ_HWR_CONNECTED	(0x0 << 6)
#undef AT_XDMAC_CC_SWREQ_SWR_CONNECTED
#define			AT_XDMAC_CC_SWREQ_SWR_CONNECTED	(0x1 << 6)
#undef AT_XDMAC_CC_MEMSET
#define		AT_XDMAC_CC_MEMSET	(0x1 << 7)	
#undef AT_XDMAC_CC_MEMSET_NORMAL_MODE
#define			AT_XDMAC_CC_MEMSET_NORMAL_MODE	(0x0 << 7)
#undef AT_XDMAC_CC_MEMSET_HW_MODE
#define			AT_XDMAC_CC_MEMSET_HW_MODE	(0x1 << 7)
#undef AT_XDMAC_CC_CSIZE
#define		AT_XDMAC_CC_CSIZE(i)	((0x7 & (i)) << 8)	
#undef AT_XDMAC_CC_DWIDTH_OFFSET
#define		AT_XDMAC_CC_DWIDTH_OFFSET	11
#undef AT_XDMAC_CC_DWIDTH_MASK
#define		AT_XDMAC_CC_DWIDTH_MASK	(0x3 << AT_XDMAC_CC_DWIDTH_OFFSET)
#undef AT_XDMAC_CC_DWIDTH
#define		AT_XDMAC_CC_DWIDTH(i)	((0x3 & (i)) << AT_XDMAC_CC_DWIDTH_OFFSET)	
#undef AT_XDMAC_CC_DWIDTH_BYTE
#define			AT_XDMAC_CC_DWIDTH_BYTE		0x0
#undef AT_XDMAC_CC_DWIDTH_HALFWORD
#define			AT_XDMAC_CC_DWIDTH_HALFWORD	0x1
#undef AT_XDMAC_CC_DWIDTH_WORD
#define			AT_XDMAC_CC_DWIDTH_WORD		0x2
#undef AT_XDMAC_CC_DWIDTH_DWORD
#define			AT_XDMAC_CC_DWIDTH_DWORD	0x3
#undef AT_XDMAC_CC_SIF
#define		AT_XDMAC_CC_SIF(i)	((0x1 & (i)) << 13)	
#undef AT_XDMAC_CC_DIF
#define		AT_XDMAC_CC_DIF(i)	((0x1 & (i)) << 14)	
#undef AT_XDMAC_CC_SAM_MASK
#define		AT_XDMAC_CC_SAM_MASK	(0x3 << 16)	
#undef AT_XDMAC_CC_SAM_FIXED_AM
#define			AT_XDMAC_CC_SAM_FIXED_AM	(0x0 << 16)
#undef AT_XDMAC_CC_SAM_INCREMENTED_AM
#define			AT_XDMAC_CC_SAM_INCREMENTED_AM	(0x1 << 16)
#undef AT_XDMAC_CC_SAM_UBS_AM
#define			AT_XDMAC_CC_SAM_UBS_AM		(0x2 << 16)
#undef AT_XDMAC_CC_SAM_UBS_DS_AM
#define			AT_XDMAC_CC_SAM_UBS_DS_AM	(0x3 << 16)
#undef AT_XDMAC_CC_DAM_MASK
#define		AT_XDMAC_CC_DAM_MASK	(0x3 << 18)	
#undef AT_XDMAC_CC_DAM_FIXED_AM
#define			AT_XDMAC_CC_DAM_FIXED_AM	(0x0 << 18)
#undef AT_XDMAC_CC_DAM_INCREMENTED_AM
#define			AT_XDMAC_CC_DAM_INCREMENTED_AM	(0x1 << 18)
#undef AT_XDMAC_CC_DAM_UBS_AM
#define			AT_XDMAC_CC_DAM_UBS_AM		(0x2 << 18)
#undef AT_XDMAC_CC_DAM_UBS_DS_AM
#define			AT_XDMAC_CC_DAM_UBS_DS_AM	(0x3 << 18)
#undef AT_XDMAC_CC_INITD
#define		AT_XDMAC_CC_INITD	(0x1 << 21)	
#undef AT_XDMAC_CC_INITD_TERMINATED
#define			AT_XDMAC_CC_INITD_TERMINATED	(0x0 << 21)
#undef AT_XDMAC_CC_INITD_IN_PROGRESS
#define			AT_XDMAC_CC_INITD_IN_PROGRESS	(0x1 << 21)
#undef AT_XDMAC_CC_RDIP
#define		AT_XDMAC_CC_RDIP	(0x1 << 22)	
#undef AT_XDMAC_CC_RDIP_DONE
#define			AT_XDMAC_CC_RDIP_DONE		(0x0 << 22)
#undef AT_XDMAC_CC_RDIP_IN_PROGRESS
#define			AT_XDMAC_CC_RDIP_IN_PROGRESS	(0x1 << 22)
#undef AT_XDMAC_CC_WRIP
#define		AT_XDMAC_CC_WRIP	(0x1 << 23)	
#undef AT_XDMAC_CC_WRIP_DONE
#define			AT_XDMAC_CC_WRIP_DONE		(0x0 << 23)
#undef AT_XDMAC_CC_WRIP_IN_PROGRESS
#define			AT_XDMAC_CC_WRIP_IN_PROGRESS	(0x1 << 23)
#undef AT_XDMAC_CC_PERID
#define		AT_XDMAC_CC_PERID(i)	((0x7f & (i)) << 24)	
#undef AT_XDMAC_CDS_MSP
#define AT_XDMAC_CDS_MSP	0x2C	
#undef AT_XDMAC_CSUS
#define AT_XDMAC_CSUS		0x30	
#undef AT_XDMAC_CDUS
#define AT_XDMAC_CDUS		0x34	
#undef AT_XDMAC_MBR_UBC_UBLEN_MAX
#define AT_XDMAC_MBR_UBC_UBLEN_MAX	0xFFFFFFUL	
#undef AT_XDMAC_MBR_UBC_NDE
#define AT_XDMAC_MBR_UBC_NDE		(0x1 << 24)	
#undef AT_XDMAC_MBR_UBC_NSEN
#define AT_XDMAC_MBR_UBC_NSEN		(0x1 << 25)	
#undef AT_XDMAC_MBR_UBC_NDEN
#define AT_XDMAC_MBR_UBC_NDEN		(0x1 << 26)	
#undef AT_XDMAC_MBR_UBC_NDV0
#define AT_XDMAC_MBR_UBC_NDV0		(0x0 << 27)	
#undef AT_XDMAC_MBR_UBC_NDV1
#define AT_XDMAC_MBR_UBC_NDV1		(0x1 << 27)	
#undef AT_XDMAC_MBR_UBC_NDV2
#define AT_XDMAC_MBR_UBC_NDV2		(0x2 << 27)	
#undef AT_XDMAC_MBR_UBC_NDV3
#define AT_XDMAC_MBR_UBC_NDV3		(0x3 << 27)	
#undef AT_XDMAC_MAX_CHAN
#define AT_XDMAC_MAX_CHAN	0x20
#undef AT_XDMAC_MAX_CSIZE
#define AT_XDMAC_MAX_CSIZE	16	
#undef AT_XDMAC_MAX_DWIDTH
#define AT_XDMAC_MAX_DWIDTH	8	
#undef AT_XDMAC_RESIDUE_MAX_RETRIES
#define AT_XDMAC_RESIDUE_MAX_RETRIES	5
#undef AT_XDMAC_DMA_BUSWIDTHS
#define AT_XDMAC_DMA_BUSWIDTHS 	(BIT(DMA_SLAVE_BUSWIDTH_UNDEFINED) | 	BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) | 	BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) | 	BIT(DMA_SLAVE_BUSWIDTH_4_BYTES) | 	BIT(DMA_SLAVE_BUSWIDTH_8_BYTES))
#undef at_xdmac_read
#define at_xdmac_read(atxdmac, reg) readl_relaxed((atxdmac)->regs + (reg))
#undef at_xdmac_write
#define at_xdmac_write(atxdmac, reg, value)  	writel_relaxed((value), (atxdmac)->regs + (reg))
#undef at_xdmac_chan_read
#define at_xdmac_chan_read(atchan, reg) readl_relaxed((atchan)->ch_regs + (reg))
#undef at_xdmac_chan_write
#define at_xdmac_chan_write(atchan, reg, value) writel_relaxed((value), (atchan)->ch_regs + (reg))
/*@ requires true; ensures \result == ((cfg & AT_XDMAC_CC_TYPE_PER_TRAN) != 0); */
static inline bool at_xdmac_chan_is_peripheral_xfer(u32 cfg)
{
	return cfg & AT_XDMAC_CC_TYPE_PER_TRAN;
}
