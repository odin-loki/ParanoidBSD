static int d71_reset(struct d71_dev *d71)
{
	u32 __iomem *gcu = d71->gcu_addr;
	int ret;

	malidp_write32(gcu, BLK_CONTROL, GCU_CONTROL_SRST);

	ret = dp_wait_cond(!(malidp_read32(gcu, BLK_CONTROL) & GCU_CONTROL_SRST),
			   100, 1000, 10000);

	return ret;
}
