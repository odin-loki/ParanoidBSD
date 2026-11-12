static int kgdbfdc_read_char(void)
{
	unsigned int stat, channel, data;
	void __iomem *regs;

	/* No more data, try and read another FDC word from RX FIFO */
	if (kgdbfdc_rpos >= kgdbfdc_rbuflen) {
		kgdbfdc_rpos = 0;
		kgdbfdc_rbuflen = 0;

		regs = kgdbfdc_setup();
		if (IS_ERR(regs))
			return NO_POLL_CHAR;

		/* Read next word from KGDB channel */
		do {
			stat = __raw_readl(regs + REG_FDSTAT);

			/* No data waiting? */
			if (stat & REG_FDSTAT_RXE)
				return NO_POLL_CHAR;

			/* Read next word */
			channel = (stat & REG_FDSTAT_RXCHAN) >>
					REG_FDSTAT_RXCHAN_SHIFT;
			data = __raw_readl(regs + REG_FDRX);
		} while (channel != CONFIG_MIPS_EJTAG_FDC_KGDB_CHAN);

		/* Decode into rbuf */
		kgdbfdc_rbuflen = mips_ejtag_fdc_decode(data, kgdbfdc_rbuf);
	}
	pr_devel("kgdbfdc r %c\n", kgdbfdc_rbuf[kgdbfdc_rpos]);
	return kgdbfdc_rbuf[kgdbfdc_rpos++];
}
