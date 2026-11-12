static inline bool at_xdmac_chan_is_peripheral_xfer(u32 cfg)
{
	return cfg & AT_XDMAC_CC_TYPE_PER_TRAN;
}
