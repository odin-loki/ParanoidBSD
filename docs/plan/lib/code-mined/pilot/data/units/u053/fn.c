static int isp1760_udc_init(struct isp1760_udc *udc)
{
	u32 mode_reg = udc->is_isp1763 ? ISP1763_DC_MODE : ISP176x_DC_MODE;
	u16 scratch;
	u32 chipid;

	/*
	 * Check that the controller is present by writing to the scratch
	 * register, modifying the bus pattern by reading from the chip ID
	 * register, and reading the scratch register value back. The chip ID
	 * and scratch register contents must match the expected values.
	 */
	isp1760_udc_write(udc, DC_SCRATCH, 0xbabe);
	chipid = isp1760_udc_read(udc, DC_CHIP_ID_HIGH) << 16;
	chipid |= isp1760_udc_read(udc, DC_CHIP_ID_LOW);
	scratch = isp1760_udc_read(udc, DC_SCRATCH);

	if (scratch != 0xbabe) {
		dev_err(udc->isp->dev,
			"udc: scratch test failed (0x%04x/0x%08x)\n",
			scratch, chipid);
		return -ENODEV;
	}

	if (chipid != 0x00011582 && chipid != 0x00158210 &&
	    chipid != 0x00176320) {
		dev_err(udc->isp->dev, "udc: invalid chip ID 0x%08x\n", chipid);
		return -ENODEV;
	}

	/* Reset the device controller. */
	isp1760_udc_set(udc, DC_SFRESET);
	usleep_range(10000, 11000);
	isp1760_reg_write(udc->regs, mode_reg, 0);
	usleep_range(10000, 11000);

	return 0;
}
